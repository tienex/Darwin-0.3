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
 * MIPS CPU definitions
 */

#ifndef _BSD_MIPS_CPU_H_
#define _BSD_MIPS_CPU_H_

/*
 * Definitions for MIPS CPU types and features
 */

/* CPU identification from CP0 PRId register */
#define MIPS_PRID_COMPANY_MASK		0xFF000000
#define MIPS_PRID_COMPANY_SHIFT		24
#define MIPS_PRID_PROCESSOR_MASK	0x00FF0000
#define MIPS_PRID_PROCESSOR_SHIFT	16
#define MIPS_PRID_REVISION_MASK		0x000000FF

/* Known CPU companies */
#define MIPS_COMPANY_LEGACY		0x00	/* Legacy/Unknown */
#define MIPS_COMPANY_MIPS		0x01	/* MIPS Technologies */
#define MIPS_COMPANY_BROADCOM		0x02	/* Broadcom */
#define MIPS_COMPANY_ALCHEMY		0x03	/* Alchemy/AMD */
#define MIPS_COMPANY_SIBYTE		0x04	/* SiByte/Broadcom */
#define MIPS_COMPANY_SANDCRAFT		0x05	/* SandCraft */
#define MIPS_COMPANY_PHILIPS		0x06	/* Philips */
#define MIPS_COMPANY_TOSHIBA		0x07	/* Toshiba */
#define MIPS_COMPANY_LSI		0x08	/* LSI Logic */

/* Known CPU types (processor ID field) */
#define MIPS_R2000			0x01
#define MIPS_R3000			0x02
#define MIPS_R6000			0x03
#define MIPS_R4000			0x04
#define MIPS_R8000			0x10
#define MIPS_R10000			0x09
#define MIPS_R12000			0x0E
#define MIPS_R14000			0x0F
#define MIPS_24K			0x93	/* MIPS 24K */
#define MIPS_34K			0x95	/* MIPS 34K */
#define MIPS_74K			0x97	/* MIPS 74K */
#define MIPS_1004K			0x99	/* MIPS 1004K */

/*
 * CPU features and capabilities
 */
#define MIPS_HAS_FPU			0x0001	/* Has FPU */
#define MIPS_HAS_64BIT			0x0002	/* 64-bit capable */
#define MIPS_HAS_LLSC			0x0004	/* Has LL/SC */
#define MIPS_HAS_MIPS16			0x0008	/* Has MIPS16 */
#define MIPS_HAS_MDMX			0x0010	/* Has MDMX */
#define MIPS_HAS_MIPS3D			0x0020	/* Has MIPS-3D */
#define MIPS_HAS_SMARTMIPS		0x0040	/* Has SmartMIPS */
#define MIPS_HAS_DSP			0x0080	/* Has DSP ASE */
#define MIPS_HAS_MT			0x0100	/* Has MT ASE */

/*
 * Cache types
 */
#define CACHE_TYPE_NONE			0
#define CACHE_TYPE_DIRECT_MAPPED	1
#define CACHE_TYPE_2WAY			2
#define CACHE_TYPE_4WAY			4

#ifndef __ASSEMBLER__

/*
 * CPU information structure
 */
struct mips_cpu_info {
	unsigned int	cpu_prid;	/* PRId value */
	unsigned int	cpu_company;	/* Company ID */
	unsigned int	cpu_type;	/* Processor type */
	unsigned int	cpu_revision;	/* Revision number */
	unsigned int	cpu_features;	/* Feature flags */
	unsigned int	icache_size;	/* I-cache size */
	unsigned int	dcache_size;	/* D-cache size */
	unsigned int	icache_linesize;/* I-cache line size */
	unsigned int	dcache_linesize;/* D-cache line size */
	unsigned int	tlb_entries;	/* Number of TLB entries */
};

extern struct mips_cpu_info mips_cpu;

#endif /* !__ASSEMBLER__ */

#endif /* _BSD_MIPS_CPU_H_ */
