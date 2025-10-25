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
 * DLX Physical Map (PMAP) Implementation
 * Supports both Page Tables and Software-Managed TLB
 * Based on ETH DLXSIM architecture
 */

#include <mach/mach_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>

#include <kern/assert.h>
#include <kern/cpu_number.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/zalloc.h>

#include <vm/vm_kern.h>
#include <vm/vm_page.h>

#include <machdep/dlx/pmap.h>

/*
 * Global variables
 */
pmap_t		kernel_pmap;		/* The kernel's pmap */
zone_t		pmap_zone;		/* Zone of pmap structures */

dlx_mmu_mode_t	dlx_mmu_mode = DLX_MMU_MODE_BOTH;	/* Default: use both */

/*
 * TLB hardware simulation
 * In real DLX hardware, TLB is managed by software via special registers
 */
static dlx_tlb_entry_t dlx_tlb[DLX_TLB_ENTRIES];
static unsigned int dlx_tlb_next_replace = 0;	/* Round-robin replacement index */
static simple_lock_data_t dlx_tlb_lock;

/*
 * ASID management
 */
static unsigned int dlx_asid_next = DLX_ASID_FIRST_USER;
static unsigned int dlx_asid_generation = 0;
static simple_lock_data_t dlx_asid_lock;

/*
 * Page table management
 */
static vm_offset_t dlx_kernel_page_dir;		/* Kernel page directory */

/*
 * Statistics
 */
static unsigned int dlx_tlb_hits = 0;
static unsigned int dlx_tlb_misses = 0;
static unsigned int dlx_tlb_replacements = 0;

/*
 * Initialize the DLX TLB
 */
void
dlx_tlb_init(void)
{
	int i;

	simple_lock_init(&dlx_tlb_lock);

	/* Clear all TLB entries */
	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		dlx_tlb[i].virtual_page = 0;
		dlx_tlb[i].physical_page = 0;	/* Invalid */
	}

	dlx_tlb_next_replace = 0;
}

/*
 * Flush entire TLB
 * Used when switching address spaces or on global TLB invalidation
 */
void
dlx_tlb_flush(void)
{
	int i, s;

	if (dlx_mmu_mode == DLX_MMU_MODE_PAGE_TABLES_ONLY)
		return;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	/* Invalidate all non-global entries */
	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		if (!(dlx_tlb[i].physical_page & DLX_TLB_GLOBAL)) {
			dlx_tlb[i].physical_page &= ~DLX_TLB_VALID;
		}
	}

	simple_unlock(&dlx_tlb_lock);
	splx(s);
}

/*
 * Flush a specific TLB entry by virtual address
 */
void
dlx_tlb_flush_entry(vm_offset_t va)
{
	int i, s;
	unsigned int vpn = va & DLX_TLB_VPN_MASK;

	if (dlx_mmu_mode == DLX_MMU_MODE_PAGE_TABLES_ONLY)
		return;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	/* Find and invalidate matching entry */
	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		if ((dlx_tlb[i].virtual_page & DLX_TLB_VPN_MASK) == vpn) {
			dlx_tlb[i].physical_page &= ~DLX_TLB_VALID;
		}
	}

	simple_unlock(&dlx_tlb_lock);
	splx(s);
}

/*
 * Flush all TLB entries for a specific ASID
 * Used when destroying an address space
 */
void
dlx_tlb_flush_asid(unsigned int asid)
{
	int i, s;

	if (dlx_mmu_mode == DLX_MMU_MODE_PAGE_TABLES_ONLY)
		return;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	/* Invalidate all entries for this ASID */
	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		unsigned int entry_asid = dlx_tlb[i].virtual_page & DLX_TLB_ASID_MASK;
		if (entry_asid == asid) {
			dlx_tlb[i].physical_page &= ~DLX_TLB_VALID;
		}
	}

	simple_unlock(&dlx_tlb_lock);
	splx(s);
}

/*
 * Lookup a TLB entry
 * Returns 1 if found, 0 if not found
 */
int
dlx_tlb_lookup(vm_offset_t va, unsigned int asid, dlx_tlb_entry_t *entry)
{
	int i, s;
	unsigned int vpn = va & DLX_TLB_VPN_MASK;
	int found = 0;

	if (dlx_mmu_mode == DLX_MMU_MODE_PAGE_TABLES_ONLY)
		return 0;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	/* Search for matching entry */
	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		if ((dlx_tlb[i].physical_page & DLX_TLB_VALID) &&
		    (dlx_tlb[i].virtual_page & DLX_TLB_VPN_MASK) == vpn) {
			/* Check ASID match or global */
			unsigned int entry_asid = dlx_tlb[i].virtual_page & DLX_TLB_ASID_MASK;
			if (entry_asid == asid ||
			    (dlx_tlb[i].physical_page & DLX_TLB_GLOBAL)) {
				if (entry) {
					entry->virtual_page = dlx_tlb[i].virtual_page;
					entry->physical_page = dlx_tlb[i].physical_page;
				}
				found = 1;
				dlx_tlb_hits++;
				break;
			}
		}
	}

	if (!found) {
		dlx_tlb_misses++;
	}

	simple_unlock(&dlx_tlb_lock);
	splx(s);

	return found;
}

/*
 * Insert a TLB entry
 * Uses simple round-robin replacement policy
 */
void
dlx_tlb_insert(vm_offset_t va, unsigned int asid, pt_entry_t pte)
{
	int s;
	unsigned int vpn = va & DLX_TLB_VPN_MASK;

	if (dlx_mmu_mode == DLX_MMU_MODE_PAGE_TABLES_ONLY)
		return;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	/* Use round-robin replacement */
	dlx_tlb[dlx_tlb_next_replace].virtual_page = vpn | asid;
	dlx_tlb[dlx_tlb_next_replace].physical_page = pte;

	dlx_tlb_next_replace = (dlx_tlb_next_replace + 1) % DLX_TLB_ENTRIES;
	dlx_tlb_replacements++;

	simple_unlock(&dlx_tlb_lock);
	splx(s);
}

/*
 * Remove a TLB entry
 */
void
dlx_tlb_remove(vm_offset_t va, unsigned int asid)
{
	dlx_tlb_flush_entry(va);
}

/*
 * Update a TLB entry (used when PTE changes)
 */
void
dlx_tlb_update(vm_offset_t va, unsigned int asid, pt_entry_t pte)
{
	int i, s;
	unsigned int vpn = va & DLX_TLB_VPN_MASK;

	if (dlx_mmu_mode == DLX_MMU_MODE_PAGE_TABLES_ONLY)
		return;

	s = splhigh();
	simple_lock(&dlx_tlb_lock);

	/* Find and update matching entry */
	for (i = 0; i < DLX_TLB_ENTRIES; i++) {
		if ((dlx_tlb[i].virtual_page & DLX_TLB_VPN_MASK) == vpn) {
			unsigned int entry_asid = dlx_tlb[i].virtual_page & DLX_TLB_ASID_MASK;
			if (entry_asid == asid) {
				dlx_tlb[i].physical_page = pte;
				break;
			}
		}
	}

	simple_unlock(&dlx_tlb_lock);
	splx(s);
}

/*
 * TLB miss exception handler
 * Called when TLB lookup fails - must load entry from page table
 */
void
dlx_tlb_miss_handler(vm_offset_t va, int is_write)
{
	pmap_t pmap;
	pt_entry_t *pte;
	thread_t thread;

	/* Get current thread's pmap */
	thread = current_thread();
	if (thread == THREAD_NULL || thread->map == VM_MAP_NULL) {
		pmap = kernel_pmap;
	} else {
		pmap = vm_map_pmap(thread->map);
	}

	/* If using TLB-only mode, this is a fatal error */
	if (dlx_mmu_mode == DLX_MMU_MODE_TLB_ONLY) {
		panic("dlx_tlb_miss_handler: TLB miss in TLB-only mode at 0x%x", va);
	}

	/* Look up page table entry */
	pte = dlx_pte_lookup(pmap, va);
	if (pte == NULL || !(*pte & DLX_PTE_VALID)) {
		/* Page fault - let VM system handle it */
		return;
	}

	/* Check write permission if this is a store */
	if (is_write && !(*pte & DLX_PTE_WRITE)) {
		/* Protection fault - let VM system handle it */
		return;
	}

	/* Set reference bit */
	*pte |= DLX_PTE_REFERENCE;

	/* Set dirty bit if write */
	if (is_write) {
		*pte |= DLX_PTE_DIRTY;
	}

	/* Insert into TLB if using TLB */
	if (dlx_mmu_mode == DLX_MMU_MODE_BOTH) {
		dlx_tlb_insert(va, pmap->asid, *pte);
	}
}

/*
 * TLB modification exception handler
 * Called when attempting to write to a read-only page
 */
void
dlx_tlb_mod_handler(vm_offset_t va)
{
	pmap_t pmap;
	pt_entry_t *pte;
	thread_t thread;

	/* Get current thread's pmap */
	thread = current_thread();
	if (thread == THREAD_NULL || thread->map == VM_MAP_NULL) {
		pmap = kernel_pmap;
	} else {
		pmap = vm_map_pmap(thread->map);
	}

	/* Look up page table entry */
	pte = dlx_pte_lookup(pmap, va);
	if (pte == NULL || !(*pte & DLX_PTE_VALID)) {
		/* Page fault */
		return;
	}

	/* Check if writable */
	if (!(*pte & DLX_PTE_WRITE)) {
		/* Protection violation - let VM system handle */
		return;
	}

	/* Set dirty bit */
	*pte |= DLX_PTE_DIRTY;

	/* Update TLB */
	if (dlx_mmu_mode != DLX_MMU_MODE_PAGE_TABLES_ONLY) {
		dlx_tlb_update(va, pmap->asid, *pte);
	}
}

/*
 * Allocate an ASID for a pmap
 */
unsigned int
dlx_asid_alloc(pmap_t pmap)
{
	unsigned int asid;
	int s;

	s = splhigh();
	simple_lock(&dlx_asid_lock);

	asid = dlx_asid_next++;

	/* Handle ASID wraparound */
	if (dlx_asid_next > DLX_ASID_MAX) {
		dlx_asid_next = DLX_ASID_FIRST_USER;
		dlx_asid_generation++;

		/* Flush all TLB entries when generation wraps */
		dlx_tlb_flush();
	}

	simple_unlock(&dlx_asid_lock);
	splx(s);

	return asid;
}

/*
 * Free an ASID
 */
void
dlx_asid_free(unsigned int asid)
{
	/* Flush all TLB entries for this ASID */
	dlx_tlb_flush_asid(asid);
}

/*
 * Look up a PTE in the page table
 * Returns pointer to PTE or NULL if not found
 */
pt_entry_t *
dlx_pte_lookup(pmap_t pmap, vm_offset_t va)
{
	pt_entry_t *page_dir, *page_table;
	unsigned int pd_index, pt_index;

	if (pmap == PMAP_NULL || pmap->page_directory == NULL)
		return NULL;

	/* Get page directory entry */
	page_dir = pmap->page_directory;
	pd_index = dlx_pd_index(va);

	if (!(page_dir[pd_index] & DLX_PTE_VALID))
		return NULL;

	/* Get page table entry */
	page_table = (pt_entry_t *)(page_dir[pd_index] & ~PAGE_MASK);
	pt_index = dlx_pt_index(va);

	return &page_table[pt_index];
}

/*
 * Allocate a PTE in the page table
 * Creates page table if necessary
 */
pt_entry_t *
dlx_pte_allocate(pmap_t pmap, vm_offset_t va)
{
	pt_entry_t *page_dir, *page_table;
	unsigned int pd_index, pt_index;
	vm_offset_t pt_phys;

	if (pmap == PMAP_NULL || pmap->page_directory == NULL)
		return NULL;

	/* Get page directory entry */
	page_dir = pmap->page_directory;
	pd_index = dlx_pd_index(va);

	/* Allocate page table if not present */
	if (!(page_dir[pd_index] & DLX_PTE_VALID)) {
		/* Allocate a page for the page table */
		pt_phys = kmem_alloc_wired(kernel_map, PAGE_SIZE);
		if (pt_phys == 0)
			return NULL;

		/* Clear the page table */
		bzero((void *)pt_phys, PAGE_SIZE);

		/* Install in page directory */
		page_dir[pd_index] = (pt_phys & ~PAGE_MASK) |
				     DLX_PTE_VALID | DLX_PTE_WRITE | DLX_PTE_USER;
	}

	/* Get page table entry */
	page_table = (pt_entry_t *)(page_dir[pd_index] & ~PAGE_MASK);
	pt_index = dlx_pt_index(va);

	return &page_table[pt_index];
}

/*
 * Activate a pmap (switch to it)
 */
void
dlx_pmap_activate(pmap_t pmap, thread_t th, int cpu)
{
	if (pmap == PMAP_NULL)
		return;

	/* Switch page directory */
	/* In real hardware, would load page directory base register */

	/* Flush TLB if switching address spaces */
	if (dlx_mmu_mode != DLX_MMU_MODE_PAGE_TABLES_ONLY) {
		dlx_tlb_flush();
	}
}

/*
 * Deactivate a pmap
 */
void
dlx_pmap_deactivate(pmap_t pmap, thread_t th, int cpu)
{
	/* Nothing special needed for DLX */
}

/*
 * Initialize the pmap module
 */
void
pmap_bootstrap(vm_offset_t load_start)
{
	/* Initialize TLB */
	dlx_tlb_init();

	/* Initialize ASID management */
	simple_lock_init(&dlx_asid_lock);

	/* Allocate kernel page directory */
	dlx_kernel_page_dir = kmem_alloc_wired(kernel_map, PAGE_SIZE);
	bzero((void *)dlx_kernel_page_dir, PAGE_SIZE);

	/* Create kernel pmap */
	kernel_pmap = (pmap_t) &kernel_pmap_store;
	kernel_pmap->page_directory = (pt_entry_t *)dlx_kernel_page_dir;
	kernel_pmap->asid = DLX_ASID_KERNEL;
	kernel_pmap->ref_count = 1;
	simple_lock_init(&kernel_pmap->lock);
}

/*
 * Initialize pmap module (called after VM is up)
 */
void
pmap_init(void)
{
	/* Create pmap zone */
	pmap_zone = zinit(sizeof(struct pmap),
			  1000 * sizeof(struct pmap),
			  PAGE_SIZE,
			  "pmap");
}
