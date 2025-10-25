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
 * DLX Physical Map (PMAP) - MMU Support
 * Software-Managed TLB (similar to MIPS R2000/R3000)
 * Based on standard DLX architecture for OS support
 */

#ifndef	_DLX_PMAP_H_
#define	_DLX_PMAP_H_

#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>
#include <mach/machine/kern_return.h>
#include <kern/lock.h>

/*
 * DLX Page Table Entry (PTE) format
 * 32-bit entries for 4KB pages
 */
typedef unsigned int		pt_entry_t;

/* PTE bit definitions */
#define DLX_PTE_VALID		0x80000000	/* Valid bit */
#define DLX_PTE_DIRTY		0x40000000	/* Dirty (modified) bit */
#define DLX_PTE_REFERENCE	0x20000000	/* Referenced bit */
#define DLX_PTE_WRITE		0x10000000	/* Writable */
#define DLX_PTE_USER		0x08000000	/* User accessible */
#define DLX_PTE_GLOBAL		0x04000000	/* Global (not ASID-specific) */
#define DLX_PTE_CACHED		0x02000000	/* Cached */
#define DLX_PTE_PFN_MASK	0x000FFFFF	/* Physical frame number (20 bits) */

#define DLX_PTE_PFN_SHIFT	12		/* Shift to get PFN */

/*
 * Two-level page table structure
 * Virtual address breakdown (32 bits):
 *   bits 31-22: Page directory index (10 bits, 1024 entries)
 *   bits 21-12: Page table index (10 bits, 1024 entries)
 *   bits 11-0:  Offset within page (12 bits, 4096 bytes)
 */
#define DLX_PD_SHIFT		22		/* Page directory shift */
#define DLX_PT_SHIFT		12		/* Page table shift */
#define DLX_PD_MASK		0x3FF		/* 10 bits for PD index */
#define DLX_PT_MASK		0x3FF		/* 10 bits for PT index */

#define DLX_PTES_PER_PAGE	1024		/* PTEs per page table */
#define DLX_PDES_PER_PAGE	1024		/* PDEs per page directory */

/* Extract indices from virtual address */
#define dlx_pd_index(va)	(((va) >> DLX_PD_SHIFT) & DLX_PD_MASK)
#define dlx_pt_index(va)	(((va) >> DLX_PT_SHIFT) & DLX_PT_MASK)

/*
 * DLX TLB Entry Structure
 * Software-managed TLB with 64 entries (typical for DLX)
 */
#define DLX_TLB_ENTRIES		64

typedef struct dlx_tlb_entry {
	unsigned int	virtual_page;		/* Virtual page number + ASID */
	unsigned int	physical_page;		/* Physical page + attributes */
} dlx_tlb_entry_t;

/* TLB virtual page fields */
#define DLX_TLB_VPN_MASK	0xFFFFF000	/* Virtual page number */
#define DLX_TLB_ASID_MASK	0x00000FFF	/* Address Space ID (12 bits) */
#define DLX_TLB_ASID_SHIFT	0

/* TLB physical page fields (same as PTE bits) */
#define DLX_TLB_PFN_MASK	DLX_PTE_PFN_MASK
#define DLX_TLB_VALID		DLX_PTE_VALID
#define DLX_TLB_DIRTY		DLX_PTE_DIRTY
#define DLX_TLB_WRITE		DLX_PTE_WRITE
#define DLX_TLB_USER		DLX_PTE_USER
#define DLX_TLB_GLOBAL		DLX_PTE_GLOBAL
#define DLX_TLB_CACHED		DLX_PTE_CACHED

/*
 * Physical map structure
 * Contains both page table root and TLB management
 */
struct pmap {
	decl_simple_lock_data(, lock)		/* Lock on map */
	int		ref_count;		/* Reference count */
	pt_entry_t	*page_directory;	/* Page directory pointer */
	unsigned int	asid;			/* Address Space ID for TLB */
	struct pmap	*next;			/* Linked list of free pmaps */
	struct pmap_statistics stats;		/* Statistics */

	/* TLB management fields */
	unsigned int	tlb_gen;		/* TLB generation number */
	unsigned int	tlb_flush_pending;	/* TLB flush needed */
};

typedef struct pmap *pmap_t;

#define PMAP_NULL	((pmap_t) 0)

extern pmap_t	kernel_pmap;			/* The kernel's map */

/*
 * ASID management
 * ASIDs are used to tag TLB entries with address space IDs
 * to avoid flushing the entire TLB on context switch
 */
#define DLX_ASID_KERNEL		0		/* Kernel ASID */
#define DLX_ASID_MAX		4095		/* Max ASID (12 bits) */
#define DLX_ASID_FIRST_USER	1		/* First user ASID */

/*
 * Macros for pmap operations
 */
#define	PMAP_SWITCH_USER(th, map, my_cpu) th->map = map;
#define PMAP_ACTIVATE(pmap, th, cpu)	dlx_pmap_activate(pmap, th, cpu)
#define PMAP_DEACTIVATE(pmap, th, cpu)	dlx_pmap_deactivate(pmap, th, cpu)
#define PMAP_CONTEXT(pmap, th)

#define pmap_kernel_va(VA)	\
	(((VA) >= VM_MIN_KERNEL_ADDRESS) && ((VA) <= VM_MAX_KERNEL_ADDRESS))

/*
 * TLB management mode
 * Allows runtime selection between page table only, TLB only, or both
 */
typedef enum {
	DLX_MMU_MODE_PAGE_TABLES_ONLY = 0,	/* Use only page tables (no TLB) */
	DLX_MMU_MODE_TLB_ONLY = 1,		/* Use only TLB (no page tables) */
	DLX_MMU_MODE_BOTH = 2			/* Use both page tables and TLB */
} dlx_mmu_mode_t;

extern dlx_mmu_mode_t dlx_mmu_mode;

/*
 * Function prototypes
 */

/* TLB management functions */
void		dlx_tlb_init(void);
void		dlx_tlb_flush(void);
void		dlx_tlb_flush_entry(vm_offset_t va);
void		dlx_tlb_flush_asid(unsigned int asid);
int		dlx_tlb_lookup(vm_offset_t va, unsigned int asid,
				dlx_tlb_entry_t *entry);
void		dlx_tlb_insert(vm_offset_t va, unsigned int asid,
				pt_entry_t pte);
void		dlx_tlb_remove(vm_offset_t va, unsigned int asid);
void		dlx_tlb_update(vm_offset_t va, unsigned int asid,
				pt_entry_t pte);

/* Exception handlers */
void		dlx_tlb_miss_handler(vm_offset_t va, int is_write);
void		dlx_tlb_mod_handler(vm_offset_t va);

/* PMAP activation/deactivation */
void		dlx_pmap_activate(pmap_t pmap, thread_t th, int cpu);
void		dlx_pmap_deactivate(pmap_t pmap, thread_t th, int cpu);

/* ASID management */
unsigned int	dlx_asid_alloc(pmap_t pmap);
void		dlx_asid_free(unsigned int asid);

/* Page table utilities */
pt_entry_t	*dlx_pte_lookup(pmap_t pmap, vm_offset_t va);
pt_entry_t	*dlx_pte_allocate(pmap_t pmap, vm_offset_t va);

#endif	/* _DLX_PMAP_H_ */
