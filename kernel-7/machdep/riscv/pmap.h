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
 * RISC-V Physical Map (pmap) definitions
 */

#ifndef _MACHDEP_RISCV_PMAP_H_
#define _MACHDEP_RISCV_PMAP_H_

#include <mach/vm_types.h>
#include <mach/boolean.h>

/*
 * RISC-V page table entry definitions (Sv39/Sv48 for RV64, Sv32 for RV32)
 */
#define PTE_V           0x001   /* Valid */
#define PTE_R           0x002   /* Readable */
#define PTE_W           0x004   /* Writable */
#define PTE_X           0x008   /* Executable */
#define PTE_U           0x010   /* User accessible */
#define PTE_G           0x020   /* Global */
#define PTE_A           0x040   /* Accessed */
#define PTE_D           0x080   /* Dirty */

/* Software bits (bits 8-9) */
#define PTE_SW0         0x100
#define PTE_SW1         0x200

/* PPN (Physical Page Number) field */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define PTE_PPN_SHIFT   10
#define PTE_PPN_MASK    0x3FFFFFFFFFFC00UL
#else
#define PTE_PPN_SHIFT   10
#define PTE_PPN_MASK    0xFFFFFC00UL
#endif

/*
 * Page size definitions
 */
#define RISCV_PGSHIFT   12
#define RISCV_PGSIZE    (1 << RISCV_PGSHIFT)
#define RISCV_PGMASK    (RISCV_PGSIZE - 1)

/*
 * Virtual address breakdown for Sv39 (RV64)
 */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define RISCV_VPN_BITS  9
#define RISCV_VPN_MASK  0x1FF
#define RISCV_LEVELS    3       /* Sv39 uses 3 levels */
#else
#define RISCV_VPN_BITS  10
#define RISCV_VPN_MASK  0x3FF
#define RISCV_LEVELS    2       /* Sv32 uses 2 levels */
#endif

/*
 * RISC-V pmap structure
 */
struct pmap {
	vm_offset_t     pm_pdir;        /* Page directory base (physical address) */
	int             pm_count;       /* Reference count */
	simple_lock_data_t pm_lock;     /* Lock for this pmap */
	struct pmap_statistics pm_stats; /* Statistics */
};

typedef struct pmap *pmap_t;

#define PMAP_NULL       ((pmap_t) 0)

/*
 * Physical to kernel virtual address translation
 * Assumes direct-mapped kernel space
 */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define PHYS_TO_KERNEL(pa)  ((pa) + 0xFFFFFFE000000000UL)
#define KERNEL_TO_PHYS(va)  ((va) - 0xFFFFFFE000000000UL)
#define SATP_PPN_MASK       0x00000FFFFFFFFFFFULL
#else
#define PHYS_TO_KERNEL(pa)  ((pa) + 0xC0000000UL)
#define KERNEL_TO_PHYS(va)  ((va) - 0xC0000000UL)
#define SATP_PPN_MASK       0x003FFFFFUL
#endif

/*
 * Physical map kernel data structure
 */
extern pmap_t kernel_pmap;

/*
 * Macros for page table access
 */
#define pmap_kernel()           kernel_pmap
#define pmap_resident_count(p)  ((p)->pm_stats.resident_count)
#define pmap_reference(p)       ((p)->pm_count++)
#define pmap_valid_page(x)      (1)

/*
 * TLB flush operations
 */
#ifndef __ASSEMBLER__

static inline void tlb_flush_all(void)
{
	__asm__ __volatile__("sfence.vma" ::: "memory");
}

static inline void tlb_flush_page(unsigned long addr)
{
	__asm__ __volatile__("sfence.vma %0" :: "r"(addr) : "memory");
}

static inline void tlb_flush_asid(unsigned long asid)
{
	__asm__ __volatile__("sfence.vma zero, %0" :: "r"(asid) : "memory");
}

/*
 * SATP (Supervisor Address Translation and Protection) register
 */
static inline void set_satp(unsigned long val)
{
	__asm__ __volatile__("csrw satp, %0" :: "r"(val) : "memory");
	tlb_flush_all();
}

static inline unsigned long get_satp(void)
{
	unsigned long val;
	__asm__ __volatile__("csrr %0, satp" : "=r"(val));
	return val;
}

/* SATP mode field values */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define SATP_MODE_SV39  (8UL << 60)
#define SATP_MODE_SV48  (9UL << 60)
#define SATP_MODE_BARE  (0UL << 60)
#else
#define SATP_MODE_SV32  (1UL << 31)
#define SATP_MODE_BARE  (0UL << 31)
#endif

#endif /* !__ASSEMBLER__ */

#endif /* _MACHDEP_RISCV_PMAP_H_ */
