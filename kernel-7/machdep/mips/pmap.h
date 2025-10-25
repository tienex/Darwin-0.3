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

#ifndef	_MIPS_PMAP_H_
#define	_MIPS_PMAP_H_

#include <mach/mips/vm_param.h>
#include <mach/machine/vm_types.h>

/*
 * MIPS Physical Address Map (pmap)
 *
 * MIPS uses a software-managed TLB with:
 * - Typically 48-64 TLB entries (processor dependent)
 * - Each TLB entry maps a pair of pages (even/odd)
 * - Software TLB refill exception handler
 */

/* Page size */
#define MIPS_PGSHIFT	12
#define MIPS_PGSIZE	(1 << MIPS_PGSHIFT)	/* 4096 bytes */
#define MIPS_PGMASK	(MIPS_PGSIZE - 1)

/* TLB configuration */
#define MIPS_TLB_ENTRIES	64	/* Typical number of TLB entries */
#define MIPS_TLB_WIRED		8	/* Number of wired (permanent) entries */

/* TLB EntryLo bit definitions */
#define TLB_PFN_MASK	0x03FFFFC0	/* Physical frame number */
#define TLB_PFN_SHIFT	6
#define TLB_C_MASK	0x00000038	/* Cache coherency attribute */
#define TLB_C_SHIFT	3
#define TLB_D		0x00000004	/* Dirty (writable) */
#define TLB_V		0x00000002	/* Valid */
#define TLB_G		0x00000001	/* Global */

/* Cache coherency attributes */
#define CACHE_CACHEABLE_NONCOHERENT	3	/* Cacheable, non-coherent */
#define CACHE_UNCACHED			2	/* Uncached */
#define CACHE_CACHEABLE_COHERENT	5	/* Cacheable, coherent */

/* TLB EntryHi bit definitions */
#define TLB_VPN2_MASK	0xFFFFE000	/* Virtual page number / 2 */
#define TLB_ASID_MASK	0x000000FF	/* Address space ID */

/* Page table entry (PTE) */
#if defined(_MIPS64) || defined(__mips64)
typedef unsigned long long pte_t;
#else
typedef unsigned int pte_t;
#endif

/* PTE bit definitions */
#define PG_V		0x00000001	/* Valid */
#define PG_R		0x00000002	/* Referenced */
#define PG_M		0x00000004	/* Modified (dirty) */
#define PG_G		0x00000008	/* Global */
#define PG_U		0x00000010	/* User accessible */
#define PG_W		0x00000020	/* Wired */
#define PG_RO		0x00000040	/* Read-only */
#define PG_FRAME	0xFFFFF000	/* Physical frame number */

/*
 * Physical map structure
 */
struct pmap {
	pte_t		*pm_pdir;	/* Page directory */
	int		pm_count;	/* Reference count */
	simple_lock_data_t pm_lock;	/* Lock for this pmap */
	struct pmap_statistics pm_stats;/* Pmap statistics */
	unsigned int	pm_asid;	/* Address space ID */
};

typedef struct pmap *pmap_t;

#define PMAP_NULL	((pmap_t) 0)

/* Kernel pmap */
extern pmap_t kernel_pmap;

/* ASID management */
#define ASID_MIN	1
#define ASID_MAX	255
#define ASID_KERNEL	0

/* Macros for address translation */
#define mips_trunc_page(x)	((unsigned long)(x) & ~MIPS_PGMASK)
#define mips_round_page(x)	mips_trunc_page((unsigned long)(x) + MIPS_PGMASK)
#define mips_page_offset(x)	((unsigned long)(x) & MIPS_PGMASK)

/* KSEG (kernel segment) macros for MIPS32 */
#ifndef __mips64
#define KSEG0_BASE	0x80000000	/* Unmapped, cached */
#define KSEG1_BASE	0xA0000000	/* Unmapped, uncached */
#define KSEG2_BASE	0xC0000000	/* Mapped */
#define KSEG3_BASE	0xE0000000	/* Mapped */

#define IS_KSEG0(addr)	(((unsigned int)(addr) >= KSEG0_BASE) && \
			 ((unsigned int)(addr) < KSEG1_BASE))
#define IS_KSEG1(addr)	(((unsigned int)(addr) >= KSEG1_BASE) && \
			 ((unsigned int)(addr) < KSEG2_BASE))
#define IS_KSEG2(addr)	(((unsigned int)(addr) >= KSEG2_BASE) && \
			 ((unsigned int)(addr) < KSEG3_BASE))

#define PHYS_TO_KSEG0(addr)	((unsigned int)(addr) | KSEG0_BASE)
#define PHYS_TO_KSEG1(addr)	((unsigned int)(addr) | KSEG1_BASE)
#define KSEG0_TO_PHYS(addr)	((unsigned int)(addr) & 0x1FFFFFFF)
#define KSEG1_TO_PHYS(addr)	((unsigned int)(addr) & 0x1FFFFFFF)
#endif

/* Function prototypes */
#ifndef __ASSEMBLER__

void pmap_bootstrap(void);
void pmap_init(void);
pmap_t pmap_create(vm_size_t size);
void pmap_destroy(pmap_t pmap);
void pmap_reference(pmap_t pmap);
void pmap_remove(pmap_t pmap, vm_offset_t sva, vm_offset_t eva);
void pmap_enter(pmap_t pmap, vm_offset_t va, vm_offset_t pa,
                vm_prot_t prot, boolean_t wired);
void pmap_activate(pmap_t pmap, int cpu);
void pmap_deactivate(pmap_t pmap, int cpu);

#endif /* __ASSEMBLER__ */

#endif	/* _MIPS_PMAP_H_ */
