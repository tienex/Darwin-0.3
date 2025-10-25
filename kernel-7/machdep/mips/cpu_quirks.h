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
 * MIPS CPU Vendor Quirks and Features
 *
 * This file handles vendor-specific variations in MIPS implementations.
 * Different MIPS vendors (Loongson, Broadcom, Cavium, Ingenic, etc.)
 * have different features, bugs, and extensions that need special handling.
 */

#ifndef _MACHDEP_MIPS_CPU_QUIRKS_H_
#define _MACHDEP_MIPS_CPU_QUIRKS_H_

/*
 * CPU Vendor IDs (from PRId register bits 23:16)
 */
#define MIPS_PRID_COMP_LEGACY		0x00	/* Legacy MIPS */
#define MIPS_PRID_COMP_MIPS		0x01	/* MIPS Technologies */
#define MIPS_PRID_COMP_BROADCOM		0x02	/* Broadcom */
#define MIPS_PRID_COMP_ALCHEMY		0x03	/* Alchemy/AMD */
#define MIPS_PRID_COMP_SIBYTE		0x04	/* SiByte (Broadcom) */
#define MIPS_PRID_COMP_SANDCRAFT	0x05	/* SandCraft */
#define MIPS_PRID_COMP_PHILIPS		0x06	/* Philips */
#define MIPS_PRID_COMP_TOSHIBA		0x07	/* Toshiba */
#define MIPS_PRID_COMP_LSI		0x08	/* LSI Logic */
#define MIPS_PRID_COMP_LEXRA		0x0B	/* Lexra */
#define MIPS_PRID_COMP_CAVIUM		0x0D	/* Cavium Networks */
#define MIPS_PRID_COMP_INGENIC		0xE1	/* Ingenic (JZ47xx) */
#define MIPS_PRID_COMP_LOONGSON		0x42	/* Loongson (STMicro ID) */

/*
 * CPU Implementation IDs (from PRId register bits 15:8)
 */

/* Legacy MIPS (0x00) */
#define MIPS_PRID_IMP_R2000		0x01
#define MIPS_PRID_IMP_R3000		0x02
#define MIPS_PRID_IMP_R6000		0x03
#define MIPS_PRID_IMP_R4000		0x04
#define MIPS_PRID_IMP_R10000		0x09

/* MIPS Technologies (0x01) */
#define MIPS_PRID_IMP_4KC		0x80
#define MIPS_PRID_IMP_5KC		0x81
#define MIPS_PRID_IMP_20KC		0x82
#define MIPS_PRID_IMP_24K		0x93
#define MIPS_PRID_IMP_34K		0x95
#define MIPS_PRID_IMP_74K		0x96
#define MIPS_PRID_IMP_1004K		0x99
#define MIPS_PRID_IMP_1074K		0x9A
#define MIPS_PRID_IMP_M5150		0xA3
#define MIPS_PRID_IMP_P5600		0xA8
#define MIPS_PRID_IMP_I6400		0xA9
#define MIPS_PRID_IMP_P6600		0xAA

/* Loongson/Godson (0x42) */
#define MIPS_PRID_IMP_LOONGSON_32	0x42	/* Loongson 1 (MIPS32) */
#define MIPS_PRID_IMP_LOONGSON_64	0x63	/* Loongson 2/3 (MIPS64) */

/* Ingenic (0xE1) */
#define MIPS_PRID_IMP_JZRISC		0x02	/* JZ47xx RISC */

/* Broadcom (0x02) */
#define MIPS_PRID_IMP_BMIPS32		0x00
#define MIPS_PRID_IMP_BMIPS3300		0x00
#define MIPS_PRID_IMP_BMIPS4380		0x40
#define MIPS_PRID_IMP_BMIPS5000		0x50

/* Cavium (0x0D) */
#define MIPS_PRID_IMP_CAVIUM_CN38XX	0x00
#define MIPS_PRID_IMP_CAVIUM_CN50XX	0x06
#define MIPS_PRID_IMP_CAVIUM_CN58XX	0x03
#define MIPS_PRID_IMP_CAVIUM_CN63XX	0x90
#define MIPS_PRID_IMP_CAVIUM_CN68XX	0x91

/*
 * CPU Feature Flags
 * These indicate what features/quirks the CPU has
 */
#define MIPS_CPU_FPU		0x00000001	/* Has FPU */
#define MIPS_CPU_32FPR		0x00000002	/* 32 FP registers */
#define MIPS_CPU_LLSC		0x00000004	/* LL/SC instructions */
#define MIPS_CPU_MIPS16		0x00000008	/* MIPS16 ASE */
#define MIPS_CPU_MDMX		0x00000010	/* MDMX ASE */
#define MIPS_CPU_MIPS3D		0x00000020	/* MIPS-3D ASE */
#define MIPS_CPU_SMARTMIPS	0x00000040	/* SmartMIPS ASE */
#define MIPS_CPU_DSP		0x00000080	/* DSP ASE */
#define MIPS_CPU_DSP2		0x00000100	/* DSP2 ASE */
#define MIPS_CPU_MIPSMT		0x00000200	/* MIPS MT (multithreading) */
#define MIPS_CPU_64BIT		0x00000400	/* 64-bit capable */
#define MIPS_CPU_COUNTER	0x00000800	/* Count register available */
#define MIPS_CPU_WATCH		0x00001000	/* Watch registers */
#define MIPS_CPU_DIVEC		0x00002000	/* Dedicated interrupt vector */
#define MIPS_CPU_VCE		0x00004000	/* VCED/VCEI exceptions */
#define MIPS_CPU_CACHE_CDEX_P	0x00008000	/* Create_Dirty_Exclusive CACHE op */
#define MIPS_CPU_MCHECK		0x00010000	/* Machine check exception */
#define MIPS_CPU_EJTAG		0x00020000	/* EJTAG debug */
#define MIPS_CPU_NOFPUEX	0x00040000	/* No FPU exceptions */
#define MIPS_CPU_PREFETCH	0x00080000	/* PREFETCH instruction */
#define MIPS_CPU_VINT		0x00100000	/* Vectored interrupts */
#define MIPS_CPU_VEIC		0x00200000	/* External interrupt controller */
#define MIPS_CPU_ULRI		0x00400000	/* UserLocal register */
#define MIPS_CPU_PCI		0x00800000	/* Performance counter interrupt */
#define MIPS_CPU_RIXI		0x01000000	/* Read/Execute inhibit */
#define MIPS_CPU_MICROMIPS	0x02000000	/* microMIPS ASE */
#define MIPS_CPU_TLBINV		0x04000000	/* TLB invalidate instruction */
#define MIPS_CPU_SEGMENTS	0x08000000	/* Segmentation control */
#define MIPS_CPU_EVA		0x10000000	/* Enhanced Virtual Addressing */
#define MIPS_CPU_HTW		0x20000000	/* Hardware TLB walker */
#define MIPS_CPU_XPA		0x40000000	/* Extended Physical Addressing */
#define MIPS_CPU_CDMM		0x80000000	/* Common Device Memory Map */

/*
 * CPU Quirk Flags
 * These indicate bugs or special behaviors that need workarounds
 */
#define MIPS_QUIRK_LOONGSON2_LLSC	0x00000001	/* Loongson 2 LL/SC bug */
#define MIPS_QUIRK_LOONGSON2_CACHE	0x00000002	/* Loongson 2 cache ops */
#define MIPS_QUIRK_LOONGSON2_BTAC	0x00000004	/* Loongson 2 branch target cache */
#define MIPS_QUIRK_LOONGSON3_LLSC	0x00000008	/* Loongson 3 LL/SC improvements */
#define MIPS_QUIRK_BMIPS_CACHE		0x00000010	/* Broadcom cache quirks */
#define MIPS_QUIRK_CAVIUM_CN38XX	0x00000020	/* Cavium CN38XX errata */
#define MIPS_QUIRK_INGENIC_CACHE	0x00000040	/* Ingenic cache operations */
#define MIPS_QUIRK_R4000_SC		0x00000080	/* R4000 SC/MC bugs */
#define MIPS_QUIRK_R5432_CP0		0x00000100	/* R5432 CP0 hazards */
#define MIPS_QUIRK_NO_WRITE_COMBINE	0x00000200	/* No write combining */
#define MIPS_QUIRK_ICACHE_REFILL_WAR	0x00000400	/* ICache refill workaround */
#define MIPS_QUIRK_DCACHE_ALIAS		0x00000800	/* DCache aliasing issues */
#define MIPS_QUIRK_TLB_WRITE_WAR	0x00001000	/* TLB write hazard */

/*
 * CPU Information Structure
 */
struct mips_cpu_info {
	unsigned int		prid;		/* Full PRId value */
	unsigned int		company;	/* Vendor ID */
	unsigned int		implementation;	/* Implementation ID */
	unsigned int		revision;	/* Revision number */

	const char		*vendor_name;
	const char		*cpu_name;

	unsigned int		features;	/* Feature flags */
	unsigned int		quirks;		/* Quirk flags */

	unsigned int		cpu_frequency;	/* CPU frequency (Hz) */

	/* Cache information */
	unsigned int		icache_size;
	unsigned int		icache_line_size;
	unsigned int		icache_ways;

	unsigned int		dcache_size;
	unsigned int		dcache_line_size;
	unsigned int		dcache_ways;

	unsigned int		scache_size;	/* Secondary cache */
	unsigned int		scache_line_size;
	unsigned int		scache_ways;

	/* TLB information */
	unsigned int		tlb_entries;
	unsigned int		tlbsize_vtlb;	/* Variable TLB */
	unsigned int		tlbsize_ftlb;	/* Fixed TLB */

	/* Performance counters */
	unsigned int		num_perfcounters;
	unsigned int		perfcounter_width;
};

/*
 * Global CPU info
 */
extern struct mips_cpu_info mips_cpu;

/*
 * CPU Detection and Initialization
 */
void mips_cpu_detect(void);
void mips_cpu_quirks_init(void);
const char *mips_cpu_name(void);

/*
 * Cache Operations (vendor-specific)
 */
void mips_cache_init(void);
void mips_icache_flush_all(void);
void mips_dcache_flush_all(void);
void mips_icache_flush_range(unsigned long start, unsigned long end);
void mips_dcache_flush_range(unsigned long start, unsigned long end);

/*
 * Loongson-specific operations
 */
#ifdef SUPPORT_LOONGSON
void loongson2_cache_init(void);
void loongson2_llsc_war(void);
void loongson3_features_init(void);
#endif

/*
 * Broadcom-specific operations
 */
#ifdef SUPPORT_BMIPS
void bmips_cache_init(void);
void bmips_quirks_init(void);
#endif

/*
 * Cavium-specific operations
 */
#ifdef SUPPORT_CAVIUM
void cavium_cache_init(void);
void cavium_tlb_init(void);
#endif

/*
 * Ingenic-specific operations
 */
#ifdef SUPPORT_INGENIC
void ingenic_cache_init(void);
void ingenic_quirks_init(void);
#endif

/*
 * Feature Test Macros
 */
#define cpu_has_fpu		(mips_cpu.features & MIPS_CPU_FPU)
#define cpu_has_llsc		(mips_cpu.features & MIPS_CPU_LLSC)
#define cpu_has_mips16		(mips_cpu.features & MIPS_CPU_MIPS16)
#define cpu_has_dsp		(mips_cpu.features & MIPS_CPU_DSP)
#define cpu_has_64bits		(mips_cpu.features & MIPS_CPU_64BIT)
#define cpu_has_counter		(mips_cpu.features & MIPS_CPU_COUNTER)
#define cpu_has_watch		(mips_cpu.features & MIPS_CPU_WATCH)
#define cpu_has_ejtag		(mips_cpu.features & MIPS_CPU_EJTAG)
#define cpu_has_vtag_icache	(mips_cpu.features & MIPS_CPU_VCE)
#define cpu_has_mcheck		(mips_cpu.features & MIPS_CPU_MCHECK)
#define cpu_has_prefetch	(mips_cpu.features & MIPS_CPU_PREFETCH)
#define cpu_has_vint		(mips_cpu.features & MIPS_CPU_VINT)
#define cpu_has_veic		(mips_cpu.features & MIPS_CPU_VEIC)

/*
 * Quirk Test Macros
 */
#define cpu_has_loongson2_bug	(mips_cpu.quirks & MIPS_QUIRK_LOONGSON2_LLSC)
#define cpu_needs_cache_war	(mips_cpu.quirks & \
				 (MIPS_QUIRK_LOONGSON2_CACHE | \
				  MIPS_QUIRK_BMIPS_CACHE | \
				  MIPS_QUIRK_INGENIC_CACHE))

/*
 * Vendor-specific CP0 register definitions
 */

/* Loongson CP0 DiagControl register (CP0 $22) */
#define LOONGSON_DIAG_ITLB	0x00000001	/* ITLB enable */
#define LOONGSON_DIAG_DTLB	0x00000002	/* DTLB enable */
#define LOONGSON_DIAG_BTAC	0x00000004	/* Branch target cache */
#define LOONGSON_DIAG_RAS	0x00000008	/* Return address stack */
#define LOONGSON_DIAG_ICACHE	0x00000010	/* ICache enable */
#define LOONGSON_DIAG_DCACHE	0x00000020	/* DCache enable */

/* Broadcom BCM63xx CP0 */
#define BMIPS_CP0_CONFIG3_SM	0x00000001	/* SmartMIPS */
#define BMIPS_CP0_CONFIG3_VINT	0x00000020	/* Vectored interrupts */

/* Cavium Octeon CP0 */
#define CAVIUM_CP0_CVMCTL	$9, 7		/* Cavium control */
#define CAVIUM_CP0_CVMMEMCTL	$11, 7		/* Memory control */
#define CAVIUM_CP0_CVM_COUNT	$9, 6		/* COP0 count */

#endif /* _MACHDEP_MIPS_CPU_QUIRKS_H_ */
