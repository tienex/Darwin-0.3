/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * RISC-V Physical Address Map (pmap) Implementation
 *
 * Manages Sv39 (RV64) or Sv32 (RV32) page tables
 */

#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <mach/machine.h>
#include <vm/vm_kern.h>
#include <vm/pmap.h>
#include <kern/zalloc.h>
#include <kern/lock.h>
#include <machdep/riscv/pmap.h>

/* Global kernel pmap */
struct pmap kernel_pmap_store;
pmap_t kernel_pmap = &kernel_pmap_store;

/* Pmap zone for allocating pmaps */
static zone_t pmap_zone;

/* Page table page zone */
static zone_t pt_zone;

/* Lock for pmap system */
decl_simple_lock_data(static, pmap_system_lock)

/* Protection conversion table */
static unsigned long prot_table[8] = {
	0,                              /* VM_PROT_NONE */
	PTE_R,                         /* VM_PROT_READ */
	PTE_W,                         /* VM_PROT_WRITE (implies R) */
	PTE_R | PTE_W,                 /* VM_PROT_READ | VM_PROT_WRITE */
	PTE_X,                         /* VM_PROT_EXECUTE */
	PTE_R | PTE_X,                 /* VM_PROT_READ | VM_PROT_EXECUTE */
	PTE_W | PTE_X,                 /* VM_PROT_WRITE | VM_PROT_EXECUTE */
	PTE_R | PTE_W | PTE_X          /* VM_PROT_ALL */
};

/*
 * Page table manipulation macros
 */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define VPN_BITS    9
#define PTES_PER_PT 512
#define PT_SIZE     (PTES_PER_PT * sizeof(unsigned long))
#else
#define VPN_BITS    10
#define PTES_PER_PT 1024
#define PT_SIZE     (PTES_PER_PT * sizeof(unsigned long))
#endif

#define VPN(va, level)  (((va) >> (RISCV_PGSHIFT + (level) * VPN_BITS)) & ((1 << VPN_BITS) - 1))
#define PTE_TO_PA(pte)  (((pte) >> PTE_PPN_SHIFT) << RISCV_PGSHIFT)
#define PA_TO_PTE(pa)   (((pa) >> RISCV_PGSHIFT) << PTE_PPN_SHIFT)

/*
 * Get PTE pointer for a virtual address in a page table
 */
static inline unsigned long *
pmap_pte(pmap_t pmap, vm_offset_t va, int create)
{
	unsigned long *pdir = (unsigned long *)pmap->pm_pdir;
	unsigned long *pt, pte;
	int i;

	/* Walk page table levels */
	for (i = RISCV_LEVELS - 1; i > 0; i--) {
		unsigned int vpn = VPN(va, i);
		pte = pdir[vpn];

		if (!(pte & PTE_V)) {
			if (!create)
				return NULL;

			/* Allocate new page table page */
			pt = (unsigned long *)zalloc(pt_zone);
			if (!pt)
				return NULL;

			bzero(pt, PT_SIZE);

			/* Install pointer to new page table */
			pdir[vpn] = PA_TO_PTE(pmap_extract(kernel_pmap, (vm_offset_t)pt)) | PTE_V;
		} else if ((pte & (PTE_R | PTE_W | PTE_X)) != 0) {
			/* This is a leaf entry (huge page), can't traverse */
			return NULL;
		} else {
			/* Navigate to next level */
			pt = (unsigned long *)PHYS_TO_KERNEL(PTE_TO_PA(pte));
		}

		pdir = pt;
	}

	return &pdir[VPN(va, 0)];
}

/*
 * Bootstrap the pmap system
 */
void
pmap_bootstrap(vm_offset_t load_start)
{
	/* Initialize kernel pmap */
	kernel_pmap->pm_pdir = 0;  /* Will be set to kernel page table root */
	kernel_pmap->pm_count = 1;
	simple_lock_init(&kernel_pmap->pm_lock);

	/* Initialize lock */
	simple_lock_init(&pmap_system_lock);

	/* Kernel page table is already set up by boot code */
	/* Get current SATP value */
	unsigned long satp = get_satp();
	kernel_pmap->pm_pdir = (vm_offset_t)PHYS_TO_KERNEL((satp & SATP_PPN_MASK) << RISCV_PGSHIFT);
}

/*
 * Initialize the pmap module
 */
void
pmap_init(void)
{
	/* Create pmap zone */
	pmap_zone = zinit(sizeof(struct pmap), 400 * sizeof(struct pmap),
	                  PAGE_SIZE, "pmap");

	/* Create page table zone */
	pt_zone = zinit(PT_SIZE, 1000 * PT_SIZE, PAGE_SIZE, "pmap page table");
}

/*
 * Create a new pmap
 */
pmap_t
pmap_create(vm_size_t size)
{
	pmap_t pmap;
	unsigned long *pdir;

	/* Allocate pmap structure */
	pmap = (pmap_t)zalloc(pmap_zone);
	if (!pmap)
		return PMAP_NULL;

	/* Allocate root page table */
	pdir = (unsigned long *)zalloc(pt_zone);
	if (!pdir) {
		zfree(pmap_zone, (vm_offset_t)pmap);
		return PMAP_NULL;
	}

	/* Clear page table */
	bzero(pdir, PT_SIZE);

	/* Initialize pmap */
	pmap->pm_pdir = pmap_extract(kernel_pmap, (vm_offset_t)pdir);
	pmap->pm_count = 1;
	simple_lock_init(&pmap->pm_lock);
	pmap->pm_stats.resident_count = 0;
	pmap->pm_stats.wired_count = 0;

	return pmap;
}

/*
 * Destroy a pmap
 */
void
pmap_destroy(pmap_t pmap)
{
	int count;

	if (pmap == PMAP_NULL)
		return;

	simple_lock(&pmap->pm_lock);
	count = --pmap->pm_count;
	simple_unlock(&pmap->pm_lock);

	if (count == 0) {
		/* Free page tables and pmap structure */
		if (pmap->pm_pdir) {
			/* TODO: Walk and free all page table pages */
			zfree(pt_zone, (vm_offset_t)PHYS_TO_KERNEL(pmap->pm_pdir));
		}
		zfree(pmap_zone, (vm_offset_t)pmap);
	}
}

/*
 * Add a reference to a pmap
 */
void
pmap_reference(pmap_t pmap)
{
	if (pmap != PMAP_NULL) {
		simple_lock(&pmap->pm_lock);
		pmap->pm_count++;
		simple_unlock(&pmap->pm_lock);
	}
}

/*
 * Insert a page mapping
 */
void
pmap_enter(
	pmap_t		pmap,
	vm_offset_t	va,
	vm_offset_t	pa,
	vm_prot_t	prot,
	boolean_t	wired
)
{
	unsigned long *pte;
	unsigned long newpte;

	if (pmap == PMAP_NULL)
		return;

	/* Get PTE, creating page tables as needed */
	pte = pmap_pte(pmap, va, 1);
	if (!pte)
		return;

	/* Build new PTE */
	newpte = PA_TO_PTE(pa) | PTE_V | prot_table[prot & VM_PROT_ALL];

	/* Add user bit for user space mappings */
	if (pmap != kernel_pmap)
		newpte |= PTE_U;

	/* Check if this is a new mapping */
	if (!(*pte & PTE_V)) {
		simple_lock(&pmap->pm_lock);
		pmap->pm_stats.resident_count++;
		if (wired)
			pmap->pm_stats.wired_count++;
		simple_unlock(&pmap->pm_lock);
	}

	/* Install PTE */
	*pte = newpte;

	/* Flush TLB for this address */
	tlb_flush_page(va);
}

/*
 * Remove page mappings
 */
void
pmap_remove(
	pmap_t		pmap,
	vm_offset_t	sva,
	vm_offset_t	eva
)
{
	vm_offset_t va;
	unsigned long *pte;

	if (pmap == PMAP_NULL)
		return;

	simple_lock(&pmap->pm_lock);

	for (va = sva; va < eva; va += PAGE_SIZE) {
		pte = pmap_pte(pmap, va, 0);
		if (pte && (*pte & PTE_V)) {
			*pte = 0;
			pmap->pm_stats.resident_count--;
			tlb_flush_page(va);
		}
	}

	simple_unlock(&pmap->pm_lock);
}

/*
 * Extract physical address from virtual address
 */
vm_offset_t
pmap_extract(pmap_t pmap, vm_offset_t va)
{
	unsigned long *pte;
	unsigned long pte_val;

	if (pmap == PMAP_NULL)
		return 0;

	pte = pmap_pte(pmap, va, 0);
	if (!pte)
		return 0;

	pte_val = *pte;
	if (!(pte_val & PTE_V))
		return 0;

	return PTE_TO_PA(pte_val) | (va & PAGE_MASK);
}

/*
 * Change protection on a range of addresses
 */
void
pmap_protect(
	pmap_t		pmap,
	vm_offset_t	sva,
	vm_offset_t	eva,
	vm_prot_t	prot
)
{
	vm_offset_t va;
	unsigned long *pte;

	if (pmap == PMAP_NULL)
		return;

	if (prot == VM_PROT_NONE) {
		pmap_remove(pmap, sva, eva);
		return;
	}

	simple_lock(&pmap->pm_lock);

	for (va = sva; va < eva; va += PAGE_SIZE) {
		pte = pmap_pte(pmap, va, 0);
		if (pte && (*pte & PTE_V)) {
			/* Update protection bits */
			*pte = (*pte & ~(PTE_R | PTE_W | PTE_X)) | prot_table[prot & VM_PROT_ALL];
			tlb_flush_page(va);
		}
	}

	simple_unlock(&pmap->pm_lock);
}

/*
 * Set caching attributes
 */
void
pmap_set_modify(vm_offset_t pa)
{
	/* Mark page as modified */
	/* TODO: Implement page attributes tracking */
}

/*
 * Check if page is modified
 */
boolean_t
pmap_is_modified(vm_offset_t pa)
{
	/* TODO: Implement page attributes tracking */
	return FALSE;
}

/*
 * Clear modified bit
 */
void
pmap_clear_modify(vm_offset_t pa)
{
	/* TODO: Implement page attributes tracking */
}

/*
 * Activate a pmap (switch to it)
 */
void
pmap_activate(pmap_t pmap, thread_t thread, int cpu)
{
	if (pmap == PMAP_NULL)
		return;

	/* Set SATP register to switch page tables */
#if defined(__riscv_xlen) && __riscv_xlen == 64
	set_satp(SATP_MODE_SV39 | (pmap->pm_pdir >> RISCV_PGSHIFT));
#else
	set_satp(SATP_MODE_SV32 | (pmap->pm_pdir >> RISCV_PGSHIFT));
#endif
}

/*
 * Deactivate a pmap
 */
void
pmap_deactivate(pmap_t pmap, thread_t thread, int cpu)
{
	/* Nothing to do on RISC-V */
}

/*
 * Return kernel pmap
 */
pmap_t
pmap_kernel(void)
{
	return kernel_pmap;
}

/*
 * Copy a range of addresses from one pmap to another
 */
void
pmap_copy(
	pmap_t		dst_pmap,
	pmap_t		src_pmap,
	vm_offset_t	dst_addr,
	vm_size_t	len,
	vm_offset_t	src_addr
)
{
	/* Optional optimization - just let page faults handle it */
}

/*
 * Physical page management stubs
 */
void
pmap_page_protect(vm_offset_t pa, vm_prot_t prot)
{
	/* TODO: Implement if needed */
}

void
pmap_zero_page(vm_offset_t pa)
{
	bzero((void *)PHYS_TO_KERNEL(pa), PAGE_SIZE);
}

void
pmap_copy_page(vm_offset_t src, vm_offset_t dst)
{
	bcopy((void *)PHYS_TO_KERNEL(src), (void *)PHYS_TO_KERNEL(dst), PAGE_SIZE);
}

/*
 * Routines to manage the physical map data structure
 */
boolean_t
pmap_valid_page(vm_offset_t pa)
{
	/* TODO: Check if PA is in valid physical memory range */
	return TRUE;
}
