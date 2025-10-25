/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Physical Map (pmap) Module
 *
 * This module manages the mapping between virtual and physical addresses.
 * Alpha uses a three-level page table structure with 8KB pages.
 */

#include <mach/mach_types.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

/*
 * Alpha page table entry format
 *
 * 63        32 31              16 15   8 7     0
 * +----------+------------------+------+-------+
 * |   PFN    |    reserved      | soft | flags |
 * +----------+------------------+------+-------+
 *
 * Flags:
 *   bit 0: Valid (V)
 *   bit 1: Fault on Read (FOR)
 *   bit 2: Fault on Write (FOW)
 *   bit 3: Fault on Execute (FOE)
 *   bit 4: Assembly (ASM)
 *   bit 5: Global (GH) - don't flush on ASN change
 *   bit 6: Kernel Read Enable (KRE)
 *   bit 7: User Read Enable (URE)
 *   bit 8: Kernel Write Enable (KWE)
 *   bit 9: User Write Enable (UWE)
 *   bits 10-31: Software
 *   bits 32-63: Page Frame Number (PFN)
 */

#define ALPHA_PTE_VALID		0x0001
#define ALPHA_PTE_FOR		0x0002
#define ALPHA_PTE_FOW		0x0004
#define ALPHA_PTE_FOE		0x0008
#define ALPHA_PTE_ASM		0x0010
#define ALPHA_PTE_GH		0x0020
#define ALPHA_PTE_KRE		0x0040
#define ALPHA_PTE_URE		0x0080
#define ALPHA_PTE_KWE		0x0100
#define ALPHA_PTE_UWE		0x0200

#define ALPHA_PTE_PROT_MASK	(ALPHA_PTE_KRE|ALPHA_PTE_URE|ALPHA_PTE_KWE|ALPHA_PTE_UWE)
#define ALPHA_PTE_PFN_SHIFT	32
#define ALPHA_PTE_PFN_MASK	0xFFFFFFFF00000000UL

typedef unsigned long alpha_pte_t;

/*
 * Page table structure
 *
 * Alpha uses three levels of page tables:
 * - Level 1: 1024 entries, each covering 8TB
 * - Level 2: 1024 entries, each covering 8GB
 * - Level 3: 1024 entries, each covering 8MB
 * - Pages: 8KB each
 */
#define ALPHA_PT_LEVELS		3
#define ALPHA_PT_ENTRIES	1024
#define ALPHA_PT_SIZE		(ALPHA_PT_ENTRIES * sizeof(alpha_pte_t))

/*
 * Virtual address breakdown:
 * 63-43: Level 1 index (21 bits, but only 10 used)
 * 42-33: Level 2 index (10 bits)
 * 32-23: Level 3 index (10 bits)
 * 22-13: Unused (10 bits)
 * 12-0:  Page offset (13 bits = 8KB)
 */
#define ALPHA_VA_L1_SHIFT	33
#define ALPHA_VA_L2_SHIFT	23
#define ALPHA_VA_L3_SHIFT	13

#define ALPHA_VA_L1_INDEX(va)	(((va) >> ALPHA_VA_L1_SHIFT) & 0x3FF)
#define ALPHA_VA_L2_INDEX(va)	(((va) >> ALPHA_VA_L2_SHIFT) & 0x3FF)
#define ALPHA_VA_L3_INDEX(va)	(((va) >> ALPHA_VA_L3_SHIFT) & 0x3FF)

/*
 * Kernel page table
 */
static alpha_pte_t *kernel_pt_l1;
static unsigned long kernel_pt_phys;

/*
 * Convert physical address to PFN
 */
static inline unsigned long
pa_to_pfn(unsigned long pa)
{
	return pa >> ALPHA_PGSHIFT;
}

/*
 * Convert PFN to physical address
 */
static inline unsigned long
pfn_to_pa(unsigned long pfn)
{
	return pfn << ALPHA_PGSHIFT;
}

/*
 * Create a PTE from physical address and protection
 */
static inline alpha_pte_t
make_pte(unsigned long pa, vm_prot_t prot, boolean_t kernel)
{
	alpha_pte_t pte;

	pte = ALPHA_PTE_VALID;
	pte |= (pa_to_pfn(pa) << ALPHA_PTE_PFN_SHIFT);

	if (kernel) {
		pte |= ALPHA_PTE_ASM;  /* Kernel mappings are global */
		if (prot & VM_PROT_READ)
			pte |= ALPHA_PTE_KRE;
		if (prot & VM_PROT_WRITE)
			pte |= ALPHA_PTE_KWE;
		if (!(prot & VM_PROT_EXECUTE))
			pte |= ALPHA_PTE_FOE;
	} else {
		if (prot & VM_PROT_READ)
			pte |= ALPHA_PTE_URE;
		if (prot & VM_PROT_WRITE)
			pte |= ALPHA_PTE_UWE;
		if (!(prot & VM_PROT_EXECUTE))
			pte |= ALPHA_PTE_FOE;
	}

	return pte;
}

/*
 * Extract PFN from PTE
 */
static inline unsigned long
pte_to_pfn(alpha_pte_t pte)
{
	return (pte & ALPHA_PTE_PFN_MASK) >> ALPHA_PTE_PFN_SHIFT;
}

/*
 * Extract physical address from PTE
 */
static inline unsigned long
pte_to_pa(alpha_pte_t pte)
{
	return pfn_to_pa(pte_to_pfn(pte));
}

/*
 * Bootstrap the pmap module
 *
 * This is called very early in the boot process to set up initial
 * kernel page tables and enable virtual memory.
 */
void
alpha_pmap_bootstrap(unsigned long boot_pgtbl)
{
	unsigned long i;

	/*
	 * If bootloader provided a page table, use it
	 * Otherwise, allocate and build our own
	 */
	if (boot_pgtbl != 0) {
		kernel_pt_l1 = (alpha_pte_t *)boot_pgtbl;
		kernel_pt_phys = boot_pgtbl;
	} else {
		/*
		 * Allocate space for kernel page table
		 * For now, use a simple static allocation
		 */
		extern char _kernel_pt_start[];
		kernel_pt_l1 = (alpha_pte_t *)_kernel_pt_start;
		kernel_pt_phys = (unsigned long)_kernel_pt_start;

		/*
		 * Clear the page table
		 */
		for (i = 0; i < ALPHA_PT_ENTRIES; i++) {
			kernel_pt_l1[i] = 0;
		}

		/*
		 * Create identity mapping for kernel segment
		 * Map physical memory to KSEG (0xfffffc0000000000+)
		 */
		/* This would normally involve creating multi-level page tables */
	}

	/*
	 * Set up virtual page table pointer in PALcode
	 * This tells the PALcode where to find our page tables
	 */
	extern void pal_unix_wrvptptr(unsigned long);
	extern enum alpha_pal_variant alpha_get_pal_variant(void);

	switch (alpha_get_pal_variant()) {
	case PAL_VARIANT_UNIX:
		pal_unix_wrvptptr(kernel_pt_phys);
		break;

	case PAL_VARIANT_VMS:
		/* VMS uses PTBR (Page Table Base Register) */
		/* pal_vms_mtpr_ptbr(kernel_pt_phys); */
		break;

	case PAL_VARIANT_NT:
		/* NT PALcode uses different mechanism */
		break;

	default:
		break;
	}

	/*
	 * Flush all TLB entries
	 */
	alpha_mb();
}

/*
 * Initialize the pmap module
 */
void
pmap_bootstrap(unsigned long *first_avail)
{
	/*
	 * Called after alpha_pmap_bootstrap to complete initialization
	 */

	/*
	 * Set up kernel address space
	 */

	/*
	 * Initialize zones for page table pages
	 */
}

/*
 * Create a new pmap
 */
pmap_t
pmap_create(vm_size_t size)
{
	/* Allocate and initialize a new pmap structure */
	return PMAP_NULL;
}

/*
 * Destroy a pmap
 */
void
pmap_destroy(pmap_t pmap)
{
	/* Free all resources associated with pmap */
}

/*
 * Enter a mapping
 */
void
pmap_enter(pmap_t pmap, vm_offset_t va, vm_offset_t pa,
	   vm_prot_t prot, boolean_t wired)
{
	/*
	 * Create page table entries to map va -> pa
	 */
}

/*
 * Remove a mapping
 */
void
pmap_remove(pmap_t pmap, vm_offset_t sva, vm_offset_t eva)
{
	/*
	 * Remove all mappings in the specified range
	 */
}

/*
 * Protect a range of virtual addresses
 */
void
pmap_protect(pmap_t pmap, vm_offset_t sva, vm_offset_t eva, vm_prot_t prot)
{
	/*
	 * Change protection on mappings in the specified range
	 */
}

/*
 * Extract physical address from virtual address
 */
vm_offset_t
pmap_extract(pmap_t pmap, vm_offset_t va)
{
	/*
	 * Walk page tables to find physical address
	 */
	return 0;
}

/*
 * Page management stubs
 */

void pmap_page_protect(vm_offset_t pa, vm_prot_t prot) { }
void pmap_zero_page(vm_offset_t pa) { }
void pmap_copy_page(vm_offset_t src, vm_offset_t dst) { }
void pmap_clear_modify(vm_offset_t pa) { }
void pmap_clear_reference(vm_offset_t pa) { }
boolean_t pmap_is_modified(vm_offset_t pa) { return FALSE; }
boolean_t pmap_is_referenced(vm_offset_t pa) { return FALSE; }

/*
 * Reference counting stubs
 */
void pmap_reference(pmap_t pmap) { }

/*
 * Activation/deactivation
 */
void pmap_activate(pmap_t pmap, thread_t thread, int cpu) { }
void pmap_deactivate(pmap_t pmap, thread_t thread, int cpu) { }

/*
 * Statistics
 */
void pmap_statistics(pmap_t pmap, vm_statistics_t stats) { }

/*
 * Cache management
 */
void pmap_flush_range(pmap_t pmap, vm_offset_t sva, vm_offset_t eva)
{
	/* Flush instruction cache if needed */
	alpha_imb();
}
