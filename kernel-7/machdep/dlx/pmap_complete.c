/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Complete PMAP Implementation for DLX Architecture
 * Based on actual DLXSIM specification
 */

#include <mach/mach_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>
#include <mach/machine/vm_param.h>

#include <kern/assert.h>
#include <kern/cpu_number.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/zalloc.h>

#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>

#include <machdep/dlx/pmap.h>

/*
 * Global variables
 */
struct pmap kernel_pmap_store;
pmap_t kernel_pmap = &kernel_pmap_store;

zone_t pmap_zone;
zone_t pv_entry_zone;

unsigned int dlx_status_register = DLX_STATUS_SYSMODE | DLX_STATUS_PAGE_TABLE;

/* Physical memory management */
static vm_offset_t avail_start;
static vm_offset_t avail_end;
static vm_offset_t virtual_avail;
static vm_offset_t virtual_end;

/* TLB management */
static dlx_tlb_entry_t dlx_tlb[DLX_TLB_ENTRIES];
static simple_lock_data_t dlx_tlb_lock;

/* Physical-to-virtual mapping (PV) entry management */
typedef struct pv_entry {
	struct pv_entry *next;
	pmap_t pmap;
	vm_offset_t va;
} pv_entry_t, *pv_list_t;

#define PV_ENTRY_NULL ((pv_entry_t *)0)

typedef struct pv_head {
	pv_list_t list;
	simple_lock_data_t lock;
} pv_head_t;

static pv_head_t *pv_head_table;
static int pv_head_count;

#define pa_index(pa) (atop(pa))
#define pai_to_pvh(pai) (&pv_head_table[pai])

/*
 * Forward declarations
 */
static pt_entry_t *pmap_pte(pmap_t pmap, vm_offset_t va);

/*
 * Statistics
 */
struct pmap_stats {
	long resident_count;
	long wired_count;
} pmap_stats;

/*
 * TLB Operations
 */

void
dlx_tlb_init(void)
{
	int i;

	simple_lock_init(&dlx_tlb_lock);

	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		dlx_tlb[i].virtual_page = 0;
		dlx_tlb[i].physical_page = 0;
	}
}

void
dlx_tlb_flush(void)
{
	int i, s;

	if (!(dlx_status_register & DLX_STATUS_TLB))
		return;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		dlx_tlb[i].physical_page &= ~DLX_TLB_VALID;
	}

	simple_unlock(&dlx_tlb_lock);
	splx(s);
}

void
dlx_tlb_flush_entry(vm_offset_t va)
{
	int i, s;
	vm_offset_t vpn = va & ~DLX_PAGE_MASK;

	if (!(dlx_status_register & DLX_STATUS_TLB))
		return;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		if ((dlx_tlb[i].virtual_page & ~DLX_PAGE_MASK) == vpn) {
			dlx_tlb[i].physical_page &= ~DLX_TLB_VALID;
		}
	}

	simple_unlock(&dlx_tlb_lock);
	splx(s);
}

int
dlx_tlb_lookup(vm_offset_t va, unsigned int *phys_page)
{
	int i, s;
	vm_offset_t vpn = va & ~DLX_PAGE_MASK;
	int found = 0;

	if (!(dlx_status_register & DLX_STATUS_TLB))
		return 0;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		if ((dlx_tlb[i].physical_page & DLX_TLB_VALID) &&
		    (dlx_tlb[i].virtual_page & ~DLX_PAGE_MASK) == vpn) {
			if (phys_page)
				*phys_page = dlx_tlb[i].physical_page;
			found = 1;
			break;
		}
	}

	simple_unlock(&dlx_tlb_lock);
	splx(s);

	return found;
}

void
dlx_tlb_insert(vm_offset_t va, pt_entry_t pte, unsigned int page_size_bits)
{
	static int tlb_next = 0;
	int s;

	if (!(dlx_status_register & DLX_STATUS_TLB))
		return;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	dlx_tlb[tlb_next].virtual_page = va & ~DLX_PAGE_MASK;
	dlx_tlb[tlb_next].physical_page = pte | (page_size_bits & DLX_TLB_ENTRY_PAGESIZE_MASK);

	tlb_next = (tlb_next + 1) % DLX_TLB_ENTRIES;

	simple_unlock(&dlx_tlb_lock);
	splx(s);
}

void
dlx_tlb_remove(vm_offset_t va)
{
	dlx_tlb_flush_entry(va);
}

/*
 * PMAP Bootstrap
 */

void
pmap_bootstrap(vm_offset_t load_start, vm_offset_t *startp, vm_offset_t *endp)
{
	vm_offset_t va;

	/* Initialize TLB */
	dlx_tlb_init();

	/* Set up kernel pmap */
	simple_lock_init(&kernel_pmap->lock);
	kernel_pmap->ref_count = 1;
	kernel_pmap->pgtbl_base = 0;
	kernel_pmap->pgtbl_bits = (DLX_L1_PAGE_SIZE_BITS) | (DLX_L2_PAGE_SIZE_BITS << 16);
	kernel_pmap->pgtbl_size = DLX_L1_MAX_ENTRIES;

	/* Allocate kernel L1 page table */
	kernel_pmap->page_directory = (pt_entry_t *)load_start;
	load_start += PAGE_SIZE;
	bzero((char *)kernel_pmap->page_directory, PAGE_SIZE);

	/* Set up physical memory range */
	avail_start = round_page(load_start);
	avail_end = *endp;

	/* Set up virtual address range */
	virtual_avail = VM_MIN_KERNEL_ADDRESS;
	virtual_end = VM_MAX_KERNEL_ADDRESS;

	*startp = avail_start;
}

void
pmap_init(void)
{
	vm_size_t s;
	vm_offset_t vaddr;
	int npages;

	/* Create pmap zone */
	pmap_zone = zinit(sizeof(struct pmap),
			  1000 * sizeof(struct pmap),
			  PAGE_SIZE,
			  "pmap");

	/* Create PV entry zone */
	pv_entry_zone = zinit(sizeof(struct pv_entry),
			      10000 * sizeof(struct pv_entry),
			      PAGE_SIZE,
			      "pv_entry");

	/* Allocate PV head table */
	npages = atop(avail_end - avail_start);
	pv_head_count = npages;
	s = npages * sizeof(pv_head_t);
	s = round_page(s);

	if (kmem_alloc_wired(kernel_map, &vaddr, s) != KERN_SUCCESS)
		panic("pmap_init: cannot allocate pv_head_table");

	pv_head_table = (pv_head_t *)vaddr;
	bzero((char *)pv_head_table, s);

	/* Initialize PV head locks */
	for (int i = 0; i < pv_head_count; i++) {
		simple_lock_init(&pv_head_table[i].lock);
		pv_head_table[i].list = PV_ENTRY_NULL;
	}
}

/*
 * Helper: Get PTE pointer for virtual address
 */
static pt_entry_t *
pmap_pte(pmap_t pmap, vm_offset_t va)
{
	pt_entry_t *l1_table, *l2_table;
	unsigned int l1_index, l2_index;

	if (pmap == PMAP_NULL || pmap->page_directory == NULL)
		return NULL;

	l1_table = pmap->page_directory;
	l1_index = dlx_l1_index(va);

	if (l1_index >= pmap->pgtbl_size)
		return NULL;

	if (!(l1_table[l1_index] & DLX_PTE_VALID))
		return NULL;

	l2_table = (pt_entry_t *)(l1_table[l1_index] & DLX_PTE_MASK);
	l2_index = dlx_l2_index(va);

	return &l2_table[l2_index];
}

/*
 * PMAP Create/Destroy
 */

pmap_t
pmap_create(vm_size_t size)
{
	pmap_t pmap;

	pmap = (pmap_t)zalloc(pmap_zone);
	if (pmap == PMAP_NULL)
		return PMAP_NULL;

	bzero(pmap, sizeof(*pmap));

	simple_lock_init(&pmap->lock);
	pmap->ref_count = 1;
	pmap->pgtbl_bits = (DLX_L1_PAGE_SIZE_BITS) | (DLX_L2_PAGE_SIZE_BITS << 16);
	pmap->pgtbl_size = DLX_L1_MAX_ENTRIES;

	/* Allocate L1 page table */
	if (kmem_alloc_wired(kernel_map, (vm_offset_t *)&pmap->page_directory,
			     PAGE_SIZE) != KERN_SUCCESS) {
		zfree(pmap_zone, (vm_offset_t)pmap);
		return PMAP_NULL;
	}

	bzero(pmap->page_directory, PAGE_SIZE);
	pmap->pgtbl_base = (vm_offset_t)pmap->page_directory;

	return pmap;
}

void
pmap_destroy(pmap_t pmap)
{
	int ref_count;
	int s;

	if (pmap == PMAP_NULL)
		return;

	PMAP_LOCK(pmap, s);
	ref_count = --pmap->ref_count;
	PMAP_UNLOCK(pmap, s);

	if (ref_count == 0) {
		/* Free all page tables */
		pmap_remove(pmap, VM_MIN_ADDRESS, VM_MAX_ADDRESS);

		/* Free L1 table */
		if (pmap->page_directory) {
			kmem_free(kernel_map, (vm_offset_t)pmap->page_directory, PAGE_SIZE);
		}

		zfree(pmap_zone, (vm_offset_t)pmap);
	}
}

void
pmap_reference(pmap_t pmap)
{
	int s;

	if (pmap != PMAP_NULL) {
		PMAP_LOCK(pmap, s);
		pmap->ref_count++;
		PMAP_UNLOCK(pmap, s);
	}
}

/*
 * PMAP Enter - Map a virtual address to a physical address
 */

void
pmap_enter(pmap_t pmap, vm_offset_t va, vm_offset_t pa,
	   vm_prot_t prot, boolean_t wired)
{
	pt_entry_t *pte, *l1_entry, *l2_table;
	unsigned int l1_index, l2_index;
	pv_entry_t pv, *pvh;
	int s, pai;
	pt_entry_t new_pte;

	if (pmap == PMAP_NULL)
		return;

	/* Build PTE */
	new_pte = (pa & DLX_PTE_MASK) | DLX_PTE_VALID;

	if (prot & VM_PROT_WRITE)
		new_pte |= DLX_PTE_DIRTY;
	if (prot & (VM_PROT_READ | VM_PROT_EXECUTE))
		new_pte |= DLX_PTE_REFERENCE;

	PMAP_LOCK(pmap, s);

	/* Get L1 entry */
	l1_index = dlx_l1_index(va);
	if (l1_index >= pmap->pgtbl_size) {
		PMAP_UNLOCK(pmap, s);
		return;
	}

	l1_entry = &pmap->page_directory[l1_index];

	/* Allocate L2 table if needed */
	if (!(*l1_entry & DLX_PTE_VALID)) {
		vm_offset_t l2_phys;

		PMAP_UNLOCK(pmap, s);

		if (kmem_alloc_wired(kernel_map, &l2_phys, PAGE_SIZE) != KERN_SUCCESS)
			return;

		bzero((char *)l2_phys, PAGE_SIZE);

		PMAP_LOCK(pmap, s);

		/* Check if someone else allocated it */
		if (!(*l1_entry & DLX_PTE_VALID)) {
			*l1_entry = (l2_phys & DLX_PTE_MASK) | DLX_PTE_VALID | DLX_PTE_DIRTY;
		} else {
			/* Someone beat us to it */
			kmem_free(kernel_map, l2_phys, PAGE_SIZE);
		}
	}

	/* Get L2 table and entry */
	l2_table = (pt_entry_t *)(*l1_entry & DLX_PTE_MASK);
	l2_index = dlx_l2_index(va);
	pte = &l2_table[l2_index];

	/* Remove old mapping if present */
	if (*pte & DLX_PTE_VALID) {
		vm_offset_t old_pa = *pte & DLX_PTE_MASK;
		if (old_pa != pa) {
			/* Remove from PV list */
			pai = pa_index(old_pa);
			if (pai < pv_head_count) {
				pvh = pai_to_pvh(pai);
				simple_lock(&pvh->lock);

				pv_entry_t *prev = NULL;
				for (pv = pvh->list; pv; pv = pv->next) {
					if (pv->pmap == pmap && pv->va == va) {
						if (prev)
							prev->next = pv->next;
						else
							pvh->list = pv->next;
						zfree(pv_entry_zone, (vm_offset_t)pv);
						break;
					}
					prev = pv;
				}

				simple_unlock(&pvh->lock);
			}
		}
	}

	/* Set new PTE */
	*pte = new_pte;

	/* Update TLB */
	if (dlx_status_register & DLX_STATUS_TLB) {
		dlx_tlb_insert(va, new_pte, DLX_L2_PAGE_SIZE_BITS);
	}

	/* Add to PV list */
	pai = pa_index(pa);
	if (pai < pv_head_count) {
		pvh = pai_to_pvh(pai);
		simple_lock(&pvh->lock);

		pv = (pv_entry_t)zalloc(pv_entry_zone);
		if (pv) {
			pv->pmap = pmap;
			pv->va = va;
			pv->next = pvh->list;
			pvh->list = pv;
		}

		simple_unlock(&pvh->lock);
	}

	/* Update statistics */
	pmap->stats.resident_count++;
	if (wired)
		pmap->stats.wired_count++;

	PMAP_UNLOCK(pmap, s);
}

/*
 * PMAP Remove - Remove mappings in a range
 */

void
pmap_remove(pmap_t pmap, vm_offset_t sva, vm_offset_t eva)
{
	pt_entry_t *pte;
	vm_offset_t va;
	int s, pai;

	if (pmap == PMAP_NULL)
		return;

	PMAP_LOCK(pmap, s);

	for (va = sva; va < eva; va += PAGE_SIZE) {
		pte = pmap_pte(pmap, va);

		if (pte && (*pte & DLX_PTE_VALID)) {
			vm_offset_t pa = *pte & DLX_PTE_MASK;

			/* Remove from PV list */
			pai = pa_index(pa);
			if (pai < pv_head_count) {
				pv_head_t *pvh = pai_to_pvh(pai);
				simple_lock(&pvh->lock);

				pv_entry_t *pv, *prev = NULL;
				for (pv = pvh->list; pv; pv = pv->next) {
					if (pv->pmap == pmap && pv->va == va) {
						if (prev)
							prev->next = pv->next;
						else
							pvh->list = pv->next;
						zfree(pv_entry_zone, (vm_offset_t)pv);
						break;
					}
					prev = pv;
				}

				simple_unlock(&pvh->lock);
			}

			/* Clear PTE */
			*pte = 0;

			/* Flush TLB */
			if (dlx_status_register & DLX_STATUS_TLB) {
				dlx_tlb_flush_entry(va);
			}

			pmap->stats.resident_count--;
		}
	}

	PMAP_UNLOCK(pmap, s);
}

/*
 * PMAP Protect - Change protection on a range
 */

void
pmap_protect(pmap_t pmap, vm_offset_t sva, vm_offset_t eva, vm_prot_t prot)
{
	pt_entry_t *pte;
	vm_offset_t va;
	int s;

	if (pmap == PMAP_NULL)
		return;

	if (prot == VM_PROT_NONE) {
		pmap_remove(pmap, sva, eva);
		return;
	}

	PMAP_LOCK(pmap, s);

	for (va = sva; va < eva; va += PAGE_SIZE) {
		pte = pmap_pte(pmap, va);

		if (pte && (*pte & DLX_PTE_VALID)) {
			pt_entry_t new_pte = *pte & DLX_PTE_MASK;
			new_pte |= DLX_PTE_VALID;

			if (prot & VM_PROT_WRITE)
				new_pte |= DLX_PTE_DIRTY;
			if (prot & (VM_PROT_READ | VM_PROT_EXECUTE))
				new_pte |= DLX_PTE_REFERENCE;

			*pte = new_pte;

			/* Update TLB */
			if (dlx_status_register & DLX_STATUS_TLB) {
				dlx_tlb_flush_entry(va);
				dlx_tlb_insert(va, new_pte, DLX_L2_PAGE_SIZE_BITS);
			}
		}
	}

	PMAP_UNLOCK(pmap, s);
}

/*
 * PMAP Extract - Get physical address for virtual address
 */

vm_offset_t
pmap_extract(pmap_t pmap, vm_offset_t va)
{
	pt_entry_t *pte;
	int s;
	vm_offset_t pa = 0;

	if (pmap == PMAP_NULL)
		return 0;

	PMAP_LOCK(pmap, s);

	pte = pmap_pte(pmap, va);
	if (pte && (*pte & DLX_PTE_VALID)) {
		pa = (*pte & DLX_PTE_MASK) | (va & DLX_PAGE_MASK);
	}

	PMAP_UNLOCK(pmap, s);

	return pa;
}

/*
 * PMAP Page Protect - Change protection for all mappings of a physical page
 */

void
pmap_page_protect(vm_offset_t pa, vm_prot_t prot)
{
	pv_head_t *pvh;
	pv_entry_t pv;
	int pai, s;

	pai = pa_index(pa);
	if (pai >= pv_head_count)
		return;

	pvh = pai_to_pvh(pai);
	simple_lock(&pvh->lock);

	if (prot == VM_PROT_NONE) {
		/* Remove all mappings */
		while ((pv = pvh->list) != PV_ENTRY_NULL) {
			pmap_remove(pv->pmap, pv->va, pv->va + PAGE_SIZE);
		}
	} else {
		/* Change protection on all mappings */
		for (pv = pvh->list; pv; pv = pv->next) {
			pmap_protect(pv->pmap, pv->va, pv->va + PAGE_SIZE, prot);
		}
	}

	simple_unlock(&pvh->lock);
}

/*
 * Reference/Modify bit operations
 */

boolean_t
pmap_is_referenced(vm_offset_t pa)
{
	pv_head_t *pvh;
	pv_entry_t pv;
	pt_entry_t *pte;
	int pai, s;
	boolean_t referenced = FALSE;

	pai = pa_index(pa);
	if (pai >= pv_head_count)
		return FALSE;

	pvh = pai_to_pvh(pai);
	simple_lock(&pvh->lock);

	for (pv = pvh->list; pv && !referenced; pv = pv->next) {
		PMAP_LOCK(pv->pmap, s);
		pte = pmap_pte(pv->pmap, pv->va);
		if (pte && (*pte & DLX_PTE_REFERENCE))
			referenced = TRUE;
		PMAP_UNLOCK(pv->pmap, s);
	}

	simple_unlock(&pvh->lock);

	return referenced;
}

boolean_t
pmap_is_modified(vm_offset_t pa)
{
	pv_head_t *pvh;
	pv_entry_t pv;
	pt_entry_t *pte;
	int pai, s;
	boolean_t modified = FALSE;

	pai = pa_index(pa);
	if (pai >= pv_head_count)
		return FALSE;

	pvh = pai_to_pvh(pai);
	simple_lock(&pvh->lock);

	for (pv = pvh->list; pv && !modified; pv = pv->next) {
		PMAP_LOCK(pv->pmap, s);
		pte = pmap_pte(pv->pmap, pv->va);
		if (pte && (*pte & DLX_PTE_DIRTY))
			modified = TRUE;
		PMAP_UNLOCK(pv->pmap, s);
	}

	simple_unlock(&pvh->lock);

	return modified;
}

void
pmap_clear_reference(vm_offset_t pa)
{
	pv_head_t *pvh;
	pv_entry_t pv;
	pt_entry_t *pte;
	int pai, s;

	pai = pa_index(pa);
	if (pai >= pv_head_count)
		return;

	pvh = pai_to_pvh(pai);
	simple_lock(&pvh->lock);

	for (pv = pvh->list; pv; pv = pv->next) {
		PMAP_LOCK(pv->pmap, s);
		pte = pmap_pte(pv->pmap, pv->va);
		if (pte && (*pte & DLX_PTE_VALID)) {
			*pte &= ~DLX_PTE_REFERENCE;
			if (dlx_status_register & DLX_STATUS_TLB)
				dlx_tlb_flush_entry(pv->va);
		}
		PMAP_UNLOCK(pv->pmap, s);
	}

	simple_unlock(&pvh->lock);
}

void
pmap_clear_modify(vm_offset_t pa)
{
	pv_head_t *pvh;
	pv_entry_t pv;
	pt_entry_t *pte;
	int pai, s;

	pai = pa_index(pa);
	if (pai >= pv_head_count)
		return;

	pvh = pai_to_pvh(pai);
	simple_lock(&pvh->lock);

	for (pv = pvh->list; pv; pv = pv->next) {
		PMAP_LOCK(pv->pmap, s);
		pte = pmap_pte(pv->pmap, pv->va);
		if (pte && (*pte & DLX_PTE_VALID)) {
			*pte &= ~DLX_PTE_DIRTY;
			if (dlx_status_register & DLX_STATUS_TLB)
				dlx_tlb_flush_entry(pv->va);
		}
		PMAP_UNLOCK(pv->pmap, s);
	}

	simple_unlock(&pvh->lock);
}

/*
 * Additional required functions
 */

void
pmap_copy(pmap_t dst_pmap, pmap_t src_pmap, vm_offset_t dst_addr,
	  vm_size_t len, vm_offset_t src_addr)
{
	/* Not implemented for DLX - optimization only */
}

void
pmap_update(void)
{
	/* Nothing needed for DLX */
}

void
pmap_collect(pmap_t pmap)
{
	/* Garbage collection - not critical */
}

void
pmap_activate(pmap_t pmap, thread_t th, int cpu)
{
	dlx_pmap_activate(pmap, th, cpu);
}

void
pmap_deactivate(pmap_t pmap, thread_t th, int cpu)
{
	dlx_pmap_deactivate(pmap, th, cpu);
}

void
dlx_pmap_activate(pmap_t pmap, thread_t th, int cpu)
{
	if (pmap == PMAP_NULL)
		return;

	/* Set page table base register */
	/* In real hardware: write to DLX_SREG_PGTBL_BASE */

	/* Flush TLB */
	if (dlx_status_register & DLX_STATUS_TLB)
		dlx_tlb_flush();
}

void
dlx_pmap_deactivate(pmap_t pmap, thread_t th, int cpu)
{
	/* Nothing needed */
}

void
pmap_remove_all(vm_offset_t pa)
{
	pmap_page_protect(pa, VM_PROT_NONE);
}

void
pmap_copy_part_page(vm_offset_t src, vm_offset_t src_offset,
		    vm_offset_t dst, vm_offset_t dst_offset, vm_size_t len)
{
	bcopy((char *)src + src_offset, (char *)dst + dst_offset, len);
}

void
pmap_zero_page(vm_offset_t pa)
{
	bzero((char *)pa, PAGE_SIZE);
}

void
pmap_copy_page(vm_offset_t src, vm_offset_t dst)
{
	bcopy((char *)src, (char *)dst, PAGE_SIZE);
}

boolean_t
pmap_valid_page(vm_offset_t pa)
{
	return (pa >= avail_start && pa < avail_end);
}

/* Lock/unlock macros */
#define PMAP_LOCK(pmap, s) \
	s = splhigh(); \
	simple_lock(&(pmap)->lock)

#define PMAP_UNLOCK(pmap, s) \
	simple_unlock(&(pmap)->lock); \
	splx(s)
