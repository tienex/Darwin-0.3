/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Physical Memory Management (pmap)
 *
 * This file implements the machine-dependent portion of the
 * virtual memory system for MMIX.
 *
 * Key responsibilities:
 *   - Manage page tables
 *   - Map virtual addresses to physical addresses
 *   - TLB management
 *   - Memory protection
 *   - Address space management
 *
 * MMIX uses a two-level page table:
 *   - Page size: 8KB (8192 bytes)
 *   - Virtual address: 64 bits
 *   - Physical address: 64 bits
 *   - Page table entry: 64 bits
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <mach/mach_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>
#include <kern/queue.h>
#include <kern/zalloc.h>
#include <kern/lock.h>
#include <vm/pmap.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <machdep/mmix/pmap.h>
#include <machdep/mmix/thread.h>

#define NULL 0

/*
 * Page table entry structure
 */
typedef struct pte {
	unsigned long long phys_addr : 52;	/* Physical page number */
	unsigned long long valid     : 1;	/* Page is valid */
	unsigned long long writable  : 1;	/* Page is writable */
	unsigned long long user      : 1;	/* User accessible */
	unsigned long long accessed  : 1;	/* Page was accessed */
	unsigned long long dirty     : 1;	/* Page was modified */
	unsigned long long reserved  : 7;	/* Reserved bits */
} pte_t;

/*
 * Global variables
 */
static pmap_t	kernel_pmap_store;
pmap_t		kernel_pmap = &kernel_pmap_store;

static zone_t	pmap_zone;		/* Zone for pmap structures */
static zone_t	pv_list_zone;		/* Zone for pv_entry structures */

static unsigned long long avail_start;	/* First available physical page */
static unsigned long long avail_end;	/* Last available physical page */
static unsigned long long virtual_avail;	/* First free virtual address */
static unsigned long long virtual_end;	/* Last virtual address */

/*
 * Physical-to-virtual entry for managing reverse mappings
 */
struct pv_entry {
	struct pv_entry	*next;		/* Next mapping */
	pmap_t		pmap;		/* Pmap for this mapping */
	vm_offset_t	va;		/* Virtual address */
};
typedef struct pv_entry *pv_entry_t;

/*
 * Lock macros
 */
#define PMAP_LOCK(pmap)		simple_lock(&(pmap)->lock)
#define PMAP_UNLOCK(pmap)	simple_unlock(&(pmap)->lock)

/*
 * Convert addresses
 */
#define MMIX_PGSHIFT		13		/* log2(8192) */
#define MMIX_PGBYTES		(1 << MMIX_PGSHIFT)

#define mmix_btop(x)		((unsigned long long)(x) >> MMIX_PGSHIFT)
#define mmix_ptob(x)		((unsigned long long)(x) << MMIX_PGSHIFT)
#define mmix_trunc_page(x)	((unsigned long long)(x) & ~(MMIX_PGBYTES-1))
#define mmix_round_page(x)	mmix_trunc_page((x) + MMIX_PGBYTES - 1)

/*
 * Initialize pmap module
 *
 * Called once at boot time to set up the pmap system
 */
void
pmap_init(vm_offset_t phys_start, vm_offset_t phys_end)
{
	vm_size_t s;

	/* Create zone for pmap structures */
	s = (vm_size_t) sizeof(struct pmap);
	pmap_zone = zinit(s, 400*s, 4096, FALSE, "pmap");

	/* Create zone for pv_entry structures */
	s = (vm_size_t) sizeof(struct pv_entry);
	pv_list_zone = zinit(s, 10000*s, 4096, FALSE, "pv_list");

	/* Initialize kernel pmap */
	kernel_pmap->ref_count = 1;
	simple_lock_init(&kernel_pmap->lock);
	kernel_pmap->space = 0;		/* Kernel address space ID */
	kernel_pmap->next = NULL;
}

/*
 * Bootstrap the pmap system
 *
 * Called very early during kernel startup to initialize
 * virtual memory before the VM system is fully running
 */
void
pmap_bootstrap(unsigned long long mem_size, vm_offset_t *first_avail_p)
{
	vm_offset_t first_avail = *first_avail_p;

	/* Set up memory ranges */
	avail_start = mmix_btop(first_avail);
	avail_end = mmix_btop(mem_size);

	/* Set up kernel virtual address range */
	virtual_avail = VM_MIN_KERNEL_ADDRESS;
	virtual_end = VM_MAX_KERNEL_ADDRESS;

	/* Initialize kernel pmap */
	kernel_pmap->ref_count = 1;
	simple_lock_init(&kernel_pmap->lock);

	/* Allocate kernel page table if not already set up by bootloader */
	/* For now, assume bootloader has set up minimal page table */

	*first_avail_p = mmix_ptob(avail_start);

	printf("pmap_bootstrap: mem_size=0x%llx, first_avail=0x%llx\n",
	       mem_size, *first_avail_p);
}

/*
 * Create a new pmap
 */
pmap_t
pmap_create(vm_size_t size)
{
	pmap_t pmap;

	/* Allocate pmap structure */
	pmap = (pmap_t) zalloc(pmap_zone);
	if (pmap == NULL)
		return NULL;

	/* Initialize pmap */
	bzero((char *)pmap, sizeof(struct pmap));
	simple_lock_init(&pmap->lock);
	pmap->ref_count = 1;
	pmap->space = 0;	/* TODO: Allocate unique address space ID */

	/* Allocate page table */
	/* TODO: Allocate physical pages for page table */

	return pmap;
}

/*
 * Destroy a pmap
 */
void
pmap_destroy(pmap_t pmap)
{
	int c;

	if (pmap == NULL)
		return;

	PMAP_LOCK(pmap);
	c = --pmap->ref_count;
	PMAP_UNLOCK(pmap);

	if (c == 0) {
		/* Free page table */
		/* TODO: Free physical pages */

		/* Free pmap structure */
		zfree(pmap_zone, (vm_offset_t) pmap);
	}
}

/*
 * Add a reference to a pmap
 */
void
pmap_reference(pmap_t pmap)
{
	if (pmap != NULL) {
		PMAP_LOCK(pmap);
		pmap->ref_count++;
		PMAP_UNLOCK(pmap);
	}
}

/*
 * Enter a mapping in the pmap
 *
 * Insert a translation for virtual address to physical address
 */
void
pmap_enter(pmap_t pmap,
	  vm_offset_t va,
	  vm_offset_t pa,
	  vm_prot_t prot,
	  boolean_t wired)
{
	pte_t *pte;
	unsigned long long vpn, ppn;

	if (pmap == NULL)
		return;

	vpn = mmix_btop(va);
	ppn = mmix_btop(pa);

	PMAP_LOCK(pmap);

	/* Get or create page table entry */
	pte = pmap_pte(pmap, va);
	if (pte == NULL) {
		/* Need to allocate page table page */
		/* TODO: Allocate PTE page */
		PMAP_UNLOCK(pmap);
		return;
	}

	/* Set up PTE */
	pte->phys_addr = ppn;
	pte->valid = 1;
	pte->writable = (prot & VM_PROT_WRITE) ? 1 : 0;
	pte->user = (pmap != kernel_pmap) ? 1 : 0;
	pte->accessed = 0;
	pte->dirty = 0;

	/* Invalidate TLB entry */
	/* TODO: Flush TLB for this virtual address */

	PMAP_UNLOCK(pmap);

	/* Update statistics */
	pmap->stats.resident_count++;
	if (wired)
		pmap->stats.wired_count++;
}

/*
 * Remove a mapping from the pmap
 */
void
pmap_remove(pmap_t pmap, vm_offset_t start, vm_offset_t end)
{
	pte_t *pte;
	vm_offset_t va;

	if (pmap == NULL)
		return;

	PMAP_LOCK(pmap);

	for (va = mmix_trunc_page(start);
	     va < mmix_round_page(end);
	     va += MMIX_PGBYTES) {

		pte = pmap_pte(pmap, va);
		if (pte && pte->valid) {
			/* Clear PTE */
			pte->valid = 0;

			/* Invalidate TLB entry */
			/* TODO: Flush TLB */

			/* Update statistics */
			pmap->stats.resident_count--;
		}
	}

	PMAP_UNLOCK(pmap);
}

/*
 * Change protection for a range of addresses
 */
void
pmap_protect(pmap_t pmap, vm_offset_t start, vm_offset_t end, vm_prot_t prot)
{
	pte_t *pte;
	vm_offset_t va;

	if (pmap == NULL)
		return;

	/* If no access, just remove mappings */
	if ((prot & VM_PROT_ALL) == VM_PROT_NONE) {
		pmap_remove(pmap, start, end);
		return;
	}

	PMAP_LOCK(pmap);

	for (va = mmix_trunc_page(start);
	     va < mmix_round_page(end);
	     va += MMIX_PGBYTES) {

		pte = pmap_pte(pmap, va);
		if (pte && pte->valid) {
			/* Update protection */
			pte->writable = (prot & VM_PROT_WRITE) ? 1 : 0;

			/* Invalidate TLB entry */
			/* TODO: Flush TLB */
		}
	}

	PMAP_UNLOCK(pmap);
}

/*
 * Extract physical address from virtual address
 */
vm_offset_t
pmap_extract(pmap_t pmap, vm_offset_t va)
{
	pte_t *pte;
	vm_offset_t pa;

	if (pmap == NULL)
		return 0;

	PMAP_LOCK(pmap);

	pte = pmap_pte(pmap, va);
	if (pte && pte->valid) {
		pa = mmix_ptob(pte->phys_addr) | (va & (MMIX_PGBYTES - 1));
	} else {
		pa = 0;
	}

	PMAP_UNLOCK(pmap);

	return pa;
}

/*
 * Get page table entry for virtual address
 */
pte_t *
pmap_pte(pmap_t pmap, vm_offset_t va)
{
	/* TODO: Implement page table lookup */
	/* This would walk the page table to find the PTE */
	/* For now, return NULL */
	return NULL;
}

/*
 * Activate a pmap (switch to it)
 */
void
pmap_activate(pmap_t pmap, thread_t th, int which_cpu)
{
	if (pmap == NULL)
		return;

	/* Load page table base into MMU */
	/* TODO: Load pmap->space into address space register */

	/* Flush TLB */
	/* TODO: Flush entire TLB */
}

/*
 * Deactivate a pmap
 */
void
pmap_deactivate(pmap_t pmap, thread_t th, int which_cpu)
{
	/* Nothing to do on MMIX */
}

/*
 * Page is referenced
 */
boolean_t
pmap_is_referenced(vm_offset_t pa)
{
	/* TODO: Check if any PTE for this physical page has accessed bit set */
	return FALSE;
}

/*
 * Page is modified
 */
boolean_t
pmap_is_modified(vm_offset_t pa)
{
	/* TODO: Check if any PTE for this physical page has dirty bit set */
	return FALSE;
}

/*
 * Clear referenced bit
 */
void
pmap_clear_reference(vm_offset_t pa)
{
	/* TODO: Clear accessed bit in all PTEs for this physical page */
}

/*
 * Clear modified bit
 */
void
pmap_clear_modify(vm_offset_t pa)
{
	/* TODO: Clear dirty bit in all PTEs for this physical page */
}

/*
 * Return physical page number
 */
ppnum_t
pmap_find_phys(pmap_t pmap, addr64_t va)
{
	vm_offset_t pa = pmap_extract(pmap, va);
	return mmix_btop(pa);
}

/*
 * Copy range from one pmap to another
 */
void
pmap_copy(pmap_t dst_pmap, pmap_t src_pmap,
	 vm_offset_t dst_addr, vm_size_t len,
	 vm_offset_t src_addr)
{
	/* TODO: Copy page table entries from src to dst */
	/* For now, do nothing - pages will be faulted in on demand */
}

/*
 * Require that all active translations have write-back caching
 */
void
pmap_update(void)
{
	/* Flush TLB and caches */
	/* TODO: Implement cache flush */
}

/*
 * Set cache attributes for a range
 */
void
pmap_set_cache_attributes(ppnum_t pn, unsigned int cacheattr)
{
	/* TODO: Set cache attributes in page table entry */
}

/*
 * Zero a physical page
 */
void
pmap_zero_page(ppnum_t pn)
{
	vm_offset_t va = mmix_ptob(pn);
	bzero((char *)va, MMIX_PGBYTES);
}

/*
 * Copy a physical page
 */
void
pmap_copy_page(ppnum_t src, ppnum_t dst)
{
	vm_offset_t src_va = mmix_ptob(src);
	vm_offset_t dst_va = mmix_ptob(dst);
	bcopy((char *)src_va, (char *)dst_va, MMIX_PGBYTES);
}

/*
 * Page is managed by VM
 */
boolean_t
pmap_is_managed(ppnum_t pn)
{
	return (pn >= avail_start && pn < avail_end);
}

/*
 * Page is valid
 */
boolean_t
pmap_valid_page(ppnum_t pn)
{
	return (pn >= avail_start && pn < avail_end);
}

/*
 * Statistics
 */
void
pmap_statistics(pmap_t pmap, struct vm_statistics *stats)
{
	if (pmap == NULL || stats == NULL)
		return;

	stats->resident_count = pmap->stats.resident_count;
	stats->wired_count = pmap->stats.wired_count;
}

/*
 * Get kernel virtual address range
 */
vm_offset_t
pmap_map(vm_offset_t virt, vm_offset_t start, vm_offset_t end, vm_prot_t prot)
{
	vm_offset_t va;

	for (va = start; va < end; va += MMIX_PGBYTES, virt += MMIX_PGBYTES) {
		pmap_enter(kernel_pmap, virt, va, prot, FALSE);
	}

	return virt;
}

/*
 * Allocate and map kernel virtual memory
 */
vm_offset_t
pmap_map_bd(vm_offset_t virt, vm_offset_t start, vm_offset_t end, vm_prot_t prot)
{
	return pmap_map(virt, start, end, prot);
}

/*
 * Kernel virtual to physical
 */
vm_offset_t
pmap_kernel_va_to_pa(vm_offset_t va)
{
	/* Simple identity mapping for kernel */
	return va & 0x00FFFFFFFFFFFFFFULL;
}

/*
 * Steal memory before VM is initialized
 */
vm_offset_t
pmap_steal_memory(vm_size_t size)
{
	vm_offset_t addr;

	size = mmix_round_page(size);
	addr = mmix_ptob(avail_start);
	avail_start += mmix_btop(size);

	bzero((char *)addr, size);

	return addr;
}

/*
 * Return true if pmap is kernel pmap
 */
boolean_t
pmap_is_kernel(pmap_t pmap)
{
	return (pmap == kernel_pmap);
}
