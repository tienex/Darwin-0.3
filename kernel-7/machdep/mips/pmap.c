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
 * MIPS:	Physical memory mapping with software TLB management
 *
 * This implements the pmap (physical map) layer for MIPS architectures.
 * MIPS uses software-managed TLB, so this code handles:
 *  - Page table management
 *  - TLB entry allocation and flushing
 *  - ASID (Address Space ID) management
 *  - Virtual to physical address translation
 */

#include <cpus.h>

#include <mach/mach_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>

#include <kern/zalloc.h>
#include <kern/lock.h>

#include <vm/vm_kern.h>
#include <vm/vm_page.h>

#include <machdep/mips/pmap.h>
#include <machdep/mips/trap.h>

/*
 * Global kernel pmap
 */
struct pmap	kernel_pmap_store;
pmap_t		kernel_pmap = &kernel_pmap_store;

/*
 * Protection code mapping from VM to MIPS PTE bits
 */
static unsigned int	user_prot_codes[8];
static unsigned int	kernel_prot_codes[8];

/*
 * ASID management
 * MIPS has 8-bit ASID (256 possible values)
 * ASID 0 is reserved for kernel
 */
#define ASID_FIRST	1
#define ASID_LAST	255
#define ASID_COUNT	(ASID_LAST - ASID_FIRST + 1)

static asid_t	asid_next = ASID_FIRST;
static asid_t	asid_map[ASID_COUNT];	/* ASID to pmap mapping */
decl_simple_lock_data(static, asid_lock);

/*
 * Page table management
 */
static zone_t	pmap_zone;		/* Zone for pmap structures */
static zone_t	pte_zone;		/* Zone for page table pages */

/*
 * Statistics
 */
struct pmap_stats {
	int	resident_count;		/* Resident pages */
	int	wired_count;		/* Wired pages */
} pmap_stats;

/*
 * External functions from locore.s
 */
extern void mips_tlb_flush(void);
extern void mips_tlb_flush_addr(vm_offset_t va);
extern unsigned int mips_get_sr(void);
extern void mips_set_sr(unsigned int);

/*
 * Initialize protection code translation tables
 */
static void
pte_prot_init(void)
{
	unsigned int *kp, *up;
	int prot;

	kp = kernel_prot_codes;
	up = user_prot_codes;

	for (prot = 0; prot < 8; prot++) {
		switch ((vm_prot_t)prot) {
		case VM_PROT_NONE:
			/* No access */
			*kp++ = 0;
			*up++ = 0;
			break;

		case VM_PROT_READ:
		case VM_PROT_READ | VM_PROT_EXECUTE:
		case VM_PROT_EXECUTE:
			/* Read-only */
			*kp++ = MIPS_PTE_VALID;
			*up++ = MIPS_PTE_VALID;
			break;

		case VM_PROT_WRITE:
		case VM_PROT_READ | VM_PROT_WRITE:
		case VM_PROT_ALL:
			/* Read-write */
			*kp++ = MIPS_PTE_VALID | MIPS_PTE_DIRTY;
			*up++ = MIPS_PTE_VALID | MIPS_PTE_DIRTY;
			break;
		}
	}
}

/*
 * Set PTE protection bits
 */
static inline void
pte_set_prot(pt_entry_t *pte, pmap_t pmap, vm_prot_t prot)
{
	if (pmap == kernel_pmap)
		*pte = (*pte & ~(MIPS_PTE_VALID | MIPS_PTE_DIRTY)) | kernel_prot_codes[prot];
	else
		*pte = (*pte & ~(MIPS_PTE_VALID | MIPS_PTE_DIRTY)) | user_prot_codes[prot];
}

/*
 * Get page table entry for given virtual address
 * Returns PT_ENTRY_NULL if page table doesn't exist
 */
pt_entry_t *
pmap_pte(pmap_t pmap, vm_offset_t va)
{
	pd_entry_t *pd;
	unsigned int pd_idx, pt_idx;

	if (pmap == PMAP_NULL)
		return PT_ENTRY_NULL;

	/* Calculate page directory index */
	pd_idx = (va >> MIPS_SEGSHIFT) & (MIPS_NPD - 1);
	pd = &pmap->page_directory[pd_idx];

	/* Check if page table exists */
	if (*pd == 0)
		return PT_ENTRY_NULL;

	/* Calculate page table entry index */
	pt_idx = (va >> MIPS_PGSHIFT) & (MIPS_NPT - 1);

	return &((pt_entry_t *)*pd)[pt_idx];
}

/*
 * ASID management functions
 */

static asid_t
pmap_asid_alloc(pmap_t pmap)
{
	asid_t asid;

	simple_lock(&asid_lock);

	/* Find next free ASID */
	asid = asid_next;
	do {
		if (asid_map[asid - ASID_FIRST] == 0) {
			/* Found free ASID */
			asid_map[asid - ASID_FIRST] = (asid_t)pmap;
			asid_next = (asid + 1);
			if (asid_next > ASID_LAST)
				asid_next = ASID_FIRST;
			simple_unlock(&asid_lock);
			return asid;
		}
		asid++;
		if (asid > ASID_LAST)
			asid = ASID_FIRST;
	} while (asid != asid_next);

	/* No free ASIDs - flush all TLBs and recycle */
	mips_tlb_flush();
	for (asid = ASID_FIRST; asid <= ASID_LAST; asid++)
		asid_map[asid - ASID_FIRST] = 0;

	asid = ASID_FIRST;
	asid_map[0] = (asid_t)pmap;
	asid_next = ASID_FIRST + 1;

	simple_unlock(&asid_lock);
	return asid;
}

static void
pmap_asid_free(asid_t asid)
{
	if (asid >= ASID_FIRST && asid <= ASID_LAST) {
		simple_lock(&asid_lock);
		asid_map[asid - ASID_FIRST] = 0;
		simple_unlock(&asid_lock);
	}
}

/*
 * Bootstrap the pmap system
 * Called very early during kernel initialization
 */
void
pmap_bootstrap(vm_offset_t *vstart, vm_offset_t *vend)
{
	vm_offset_t va;

	/* Initialize protection code tables */
	pte_prot_init();

	/* Initialize ASID lock */
	simple_lock_init(&asid_lock);

	/* Set up kernel pmap */
	kernel_pmap->ref_count = 1;
	kernel_pmap->asid = 0;	/* Kernel uses ASID 0 */
	simple_lock_init(&kernel_pmap->lock);

	/* Allocate kernel page directory */
	kernel_pmap->page_directory = (pd_entry_t *)*vstart;
	*vstart += MIPS_PGBYTES;

	/* Clear page directory */
	bzero((char *)kernel_pmap->page_directory, MIPS_PGBYTES);

	/* Map kernel text and data (identity mapping for now) */
	/* This would be expanded in a real implementation */

	/* Set up TLB */
	mips_tlb_flush();
}

/*
 * Initialize pmap module
 * Called after zone system is available
 */
void
pmap_init(void)
{
	/* Create zones for pmap structures */
	pmap_zone = zinit(sizeof(struct pmap), 1000 * sizeof(struct pmap),
			  MIPS_PGBYTES, FALSE, "pmap");

	pte_zone = zinit(MIPS_PGBYTES, 10000 * MIPS_PGBYTES,
			 MIPS_PGBYTES, FALSE, "pte");
}

/*
 * Create a new pmap
 */
pmap_t
pmap_create(vm_size_t size)
{
	pmap_t pmap;

	/* Allocate pmap structure */
	pmap = (pmap_t)zalloc(pmap_zone);
	if (pmap == PMAP_NULL)
		return PMAP_NULL;

	/* Initialize pmap */
	pmap->ref_count = 1;
	pmap->asid = pmap_asid_alloc(pmap);
	simple_lock_init(&pmap->lock);
	pmap->stats.resident_count = 0;
	pmap->stats.wired_count = 0;

	/* Allocate page directory */
	pmap->page_directory = (pd_entry_t *)zalloc(pte_zone);
	if (pmap->page_directory == NULL) {
		pmap_asid_free(pmap->asid);
		zfree(pmap_zone, (vm_offset_t)pmap);
		return PMAP_NULL;
	}

	bzero((char *)pmap->page_directory, MIPS_PGBYTES);

	return pmap;
}

/*
 * Increment reference count
 */
void
pmap_reference(pmap_t pmap)
{
	if (pmap != PMAP_NULL) {
		simple_lock(&pmap->lock);
		pmap->ref_count++;
		simple_unlock(&pmap->lock);
	}
}

/*
 * Decrement reference count
 */
void
pmap_release(pmap_t pmap)
{
	int ref_count;

	if (pmap == PMAP_NULL)
		return;

	simple_lock(&pmap->lock);
	ref_count = --pmap->ref_count;
	simple_unlock(&pmap->lock);

	if (ref_count == 0)
		pmap_destroy(pmap);
}

/*
 * Destroy a pmap
 */
void
pmap_destroy(pmap_t pmap)
{
	int i;

	if (pmap == PMAP_NULL || pmap == kernel_pmap)
		return;

	simple_lock(&pmap->lock);
	if (pmap->ref_count != 0) {
		simple_unlock(&pmap->lock);
		return;
	}

	/* Free page tables */
	for (i = 0; i < MIPS_NPD; i++) {
		if (pmap->page_directory[i] != 0) {
			zfree(pte_zone, pmap->page_directory[i]);
		}
	}

	/* Free page directory */
	zfree(pte_zone, (vm_offset_t)pmap->page_directory);

	/* Free ASID */
	pmap_asid_free(pmap->asid);

	/* Free pmap structure */
	simple_unlock(&pmap->lock);
	zfree(pmap_zone, (vm_offset_t)pmap);
}

/*
 * Map a page of physical memory
 */
void
pmap_enter(pmap_t pmap, vm_offset_t va, vm_offset_t pa,
	   vm_prot_t prot, boolean_t wired)
{
	pt_entry_t *pte;
	pd_entry_t *pd;
	unsigned int pd_idx, pt_idx;
	pt_entry_t new_pte;

	if (pmap == PMAP_NULL)
		return;

	simple_lock(&pmap->lock);

	/* Get page directory entry */
	pd_idx = (va >> MIPS_SEGSHIFT) & (MIPS_NPD - 1);
	pd = &pmap->page_directory[pd_idx];

	/* Allocate page table if needed */
	if (*pd == 0) {
		*pd = (pd_entry_t)zalloc(pte_zone);
		if (*pd == 0) {
			simple_unlock(&pmap->lock);
			return;
		}
		bzero((char *)*pd, MIPS_PGBYTES);
	}

	/* Get page table entry */
	pt_idx = (va >> MIPS_PGSHIFT) & (MIPS_NPT - 1);
	pte = &((pt_entry_t *)*pd)[pt_idx];

	/* Build new PTE */
	new_pte = (pa >> MIPS_PGSHIFT) << MIPS_PTE_PFNSHIFT;
	new_pte |= (pmap == kernel_pmap) ? kernel_prot_codes[prot] : user_prot_codes[prot];
	if (wired)
		new_pte |= MIPS_PTE_WIRED;

	/* Check if replacing existing mapping */
	if (*pte & MIPS_PTE_VALID) {
		/* Flush old TLB entry */
		mips_tlb_flush_addr(va);
	} else {
		/* New mapping - update statistics */
		pmap->stats.resident_count++;
		if (wired)
			pmap->stats.wired_count++;
	}

	/* Install new PTE */
	*pte = new_pte;

	simple_unlock(&pmap->lock);
}

/*
 * Remove page mappings
 */
void
pmap_remove(pmap_t pmap, vm_offset_t sva, vm_offset_t eva)
{
	pt_entry_t *pte;
	vm_offset_t va;

	if (pmap == PMAP_NULL)
		return;

	simple_lock(&pmap->lock);

	for (va = sva; va < eva; va += MIPS_PGBYTES) {
		pte = pmap_pte(pmap, va);
		if (pte != PT_ENTRY_NULL && (*pte & MIPS_PTE_VALID)) {
			/* Remove mapping */
			if (*pte & MIPS_PTE_WIRED)
				pmap->stats.wired_count--;
			pmap->stats.resident_count--;

			*pte = 0;
			mips_tlb_flush_addr(va);
		}
	}

	simple_unlock(&pmap->lock);
}

/*
 * Change protection on a range of addresses
 */
void
pmap_protect(pmap_t pmap, vm_offset_t sva, vm_offset_t eva, vm_prot_t prot)
{
	pt_entry_t *pte;
	vm_offset_t va;

	if (pmap == PMAP_NULL)
		return;

	/* Check for complete removal */
	if (prot == VM_PROT_NONE) {
		pmap_remove(pmap, sva, eva);
		return;
	}

	simple_lock(&pmap->lock);

	for (va = sva; va < eva; va += MIPS_PGBYTES) {
		pte = pmap_pte(pmap, va);
		if (pte != PT_ENTRY_NULL && (*pte & MIPS_PTE_VALID)) {
			pte_set_prot(pte, pmap, prot);
			mips_tlb_flush_addr(va);
		}
	}

	simple_unlock(&pmap->lock);
}

/*
 * Change protection for a physical page
 */
void
pmap_page_protect(vm_offset_t pa, vm_prot_t prot)
{
	/* This would iterate through all pmaps referencing this page */
	/* Simplified implementation - just flush TLB */
	if (prot == VM_PROT_NONE) {
		mips_tlb_flush();
	}
}

/*
 * Extract physical address from virtual address
 */
boolean_t
pmap_extract(pmap_t pmap, vm_offset_t va, vm_offset_t *pa)
{
	pt_entry_t *pte;

	if (pmap == PMAP_NULL)
		return FALSE;

	pte = pmap_pte(pmap, va);
	if (pte == PT_ENTRY_NULL || !(*pte & MIPS_PTE_VALID))
		return FALSE;

	*pa = ((*pte >> MIPS_PTE_PFNSHIFT) << MIPS_PGSHIFT) | (va & (MIPS_PGBYTES - 1));
	return TRUE;
}

/*
 * Zero a physical page
 */
void
pmap_zero_page(vm_offset_t pa)
{
	vm_offset_t va;

	/* Map page temporarily into kernel space */
	va = phystokv(pa);
	bzero((char *)va, MIPS_PGBYTES);
}

/*
 * Copy a physical page
 */
void
pmap_copy_page(vm_offset_t src_pa, vm_offset_t dst_pa)
{
	vm_offset_t src_va, dst_va;

	src_va = phystokv(src_pa);
	dst_va = phystokv(dst_pa);

	bcopy((char *)src_va, (char *)dst_va, MIPS_PGBYTES);
}

/*
 * Page attribute functions
 */

boolean_t
pmap_is_modified(vm_offset_t pa)
{
	/* Would check dirty bit in PTE */
	/* Simplified - always return FALSE */
	return FALSE;
}

boolean_t
pmap_is_referenced(vm_offset_t pa)
{
	/* Would check accessed bit in PTE */
	/* Simplified - always return FALSE */
	return FALSE;
}

void
pmap_clear_modify(vm_offset_t pa)
{
	/* Would clear dirty bit in PTE */
}

void
pmap_clear_reference(vm_offset_t pa)
{
	/* Would clear accessed bit in PTE */
}

/*
 * Make pages pageable
 */
void
pmap_pageable(pmap_t pmap, vm_offset_t sva, vm_offset_t eva, boolean_t pageable)
{
	/* MIPS implementation doesn't need this - all pages are pageable */
}

/*
 * Activate pmap for current thread
 */
void
pmap_activate(pmap_t pmap, thread_t thread, int cpu)
{
	/* Set ASID in CP0 EntryHi register */
	if (pmap != PMAP_NULL) {
		unsigned int entryhi = pmap->asid;
		__asm__ volatile("mtc0 %0, $10" : : "r" (entryhi));
	}
}

/*
 * Deactivate pmap
 */
void
pmap_deactivate(pmap_t pmap, thread_t thread, int cpu)
{
	/* Nothing special needed for MIPS */
}

/*
 * Statistics
 */

int
pmap_resident_count(pmap_t pmap)
{
	return pmap->stats.resident_count;
}

/*
 * Convert physical to kernel virtual (for KSEG0 region)
 */
vm_offset_t
phystokv(vm_offset_t pa)
{
#if defined(_MIPS64) || defined(__mips64)
	return pa | 0xFFFFFFFF80000000ULL;
#else
	return pa | 0x80000000;
#endif
}
