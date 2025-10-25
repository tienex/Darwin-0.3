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

/* PTE bit definitions (matches DLXSIM specification) */
#define DLX_PTE_VALID		0x00000001	/* Valid bit */
#define DLX_PTE_DIRTY		0x00000002	/* Dirty (modified) bit */
#define DLX_PTE_REFERENCE	0x00000004	/* Referenced bit */
#define DLX_PTE_MASK		(~0x7)		/* Physical address mask */
#define DLX_PTE_PFN_SHIFT	13		/* Shift to get PFN (8KB pages) */

/*
 * Two-level page table structure (configurable via special registers)
 * Default DLXSIM configuration:
 *   L1 page size: 512KB (2^19) - 32 entries for 16MB total
 *   L2 page size: 8KB (2^13) - 64 entries per L2 table
 * Virtual address breakdown (32 bits):
 *   bits 31-19: L1 index (13 bits)
 *   bits 18-13: L2 index (6 bits)
 *   bits 12-0:  Offset within page (13 bits, 8192 bytes)
 */
#define DLX_L1_PAGE_SIZE_BITS	19		/* L1 entry maps 512KB */
#define DLX_L2_PAGE_SIZE_BITS	13		/* L2 entry maps 8KB */
#define DLX_L1_MAX_ENTRIES	32		/* 32 L1 entries */
#define DLX_L2_MAX_ENTRIES	64		/* 64 L2 entries per table */

#define DLX_PAGE_SIZE		(1 << DLX_L2_PAGE_SIZE_BITS)
#define DLX_PAGE_MASK		(DLX_PAGE_SIZE - 1)

/* Extract indices from virtual address */
#define dlx_l1_index(va)	((va) >> DLX_L1_PAGE_SIZE_BITS)
#define dlx_l2_index(va)	(((va) >> DLX_L2_PAGE_SIZE_BITS) & (DLX_L2_MAX_ENTRIES - 1))

/*
 * DLX TLB Entry Structure
 * Fully associative software-managed TLB (typical: 16-64 entries)
 * Supports variable page sizes per entry
 */
#define DLX_TLB_ENTRIES		64

typedef struct dlx_tlb_entry {
	unsigned int	virtual_page;		/* Virtual page + entry flags */
	unsigned int	physical_page;		/* Physical page + page size */
} dlx_tlb_entry_t;

/* TLB entry page size encoding (in physical_page low bits) */
#define DLX_TLB_ENTRY_PAGESIZE_MASK	0x1F	/* Page size: 1 << (value) */

/* TLB uses same flags as PTEs */
#define DLX_TLB_VALID		DLX_PTE_VALID
#define DLX_TLB_DIRTY		DLX_PTE_DIRTY
#define DLX_TLB_REFERENCE	DLX_PTE_REFERENCE

/*
 * DLX Status Register Flags (from DLXSIM specification)
 */
#define DLX_STATUS_INTRMASK	0x0f	/* Interrupt mask (4 bits) */
#define DLX_STATUS_FPTRUE	0x20	/* FP comparison was true */
#define DLX_STATUS_SYSMODE	0x40	/* System (kernel) mode */
#define DLX_STATUS_PAGE_TABLE	0x100	/* Use page table translation */
#define DLX_STATUS_TLB		0x200	/* Use TLB translation */

/*
 * DLX Special Registers (for MMU configuration)
 */
#define DLX_SREG_STATUS		0	/* Status register */
#define DLX_SREG_PGTBL_BASE	1	/* Page table base address */
#define DLX_SREG_PGTBL_BITS	2	/* Page size config (L1|L2) */
#define DLX_SREG_PGTBL_SIZE	3	/* Page table size */
#define DLX_SREG_FAULT_ADDR	4	/* Faulting address */

/*
 * Physical map structure
 * Contains both page table root and TLB management
 */
struct pmap {
	decl_simple_lock_data(, lock)		/* Lock on map */
	int		ref_count;		/* Reference count */
	pt_entry_t	*page_directory;	/* L1 page table pointer */
	unsigned int	pgtbl_base;		/* Page table base (SREG) */
	unsigned int	pgtbl_bits;		/* Page size config (SREG) */
	unsigned int	pgtbl_size;		/* Page table size (SREG) */
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
 * Macros for pmap operations
 */
#define	PMAP_SWITCH_USER(th, map, my_cpu) th->map = map;
#define PMAP_ACTIVATE(pmap, th, cpu)	dlx_pmap_activate(pmap, th, cpu)
#define PMAP_DEACTIVATE(pmap, th, cpu)	dlx_pmap_deactivate(pmap, th, cpu)
#define PMAP_CONTEXT(pmap, th)

#define pmap_kernel_va(VA)	\
	(((VA) >= VM_MIN_KERNEL_ADDRESS) && ((VA) <= VM_MAX_KERNEL_ADDRESS))

/*
 * MMU mode configuration
 * The DLX status register flags control MMU behavior:
 * - DLX_STATUS_PAGE_TABLE (0x100): Enable page table translation
 * - DLX_STATUS_TLB (0x200): Enable TLB translation
 * - Both flags can be set simultaneously for hybrid mode
 */
extern unsigned int dlx_status_register;

/*
 * Function prototypes
 */

/* TLB management functions */
void		dlx_tlb_init(void);
void		dlx_tlb_flush(void);
void		dlx_tlb_flush_entry(vm_offset_t va);
int		dlx_tlb_lookup(vm_offset_t va, unsigned int *phys_page);
void		dlx_tlb_insert(vm_offset_t va, pt_entry_t pte, unsigned int page_size_bits);
void		dlx_tlb_remove(vm_offset_t va);

/* Exception handlers */
void		dlx_pagefault_handler(vm_offset_t va, int is_write);
void		dlx_tlbfault_handler(vm_offset_t va, int is_write);

/* PMAP activation/deactivation */
void		dlx_pmap_activate(pmap_t pmap, thread_t th, int cpu);
void		dlx_pmap_deactivate(pmap_t pmap, thread_t th, int cpu);

/* Page table utilities */
pt_entry_t	*dlx_pte_lookup(pmap_t pmap, vm_offset_t va);
pt_entry_t	*dlx_pte_allocate(pmap_t pmap, vm_offset_t va);

/* Memory translation */
int		dlx_translate_address(vm_offset_t vaddr, vm_offset_t *paddr,
				int is_write);

#endif	/* _DLX_PMAP_H_ */
