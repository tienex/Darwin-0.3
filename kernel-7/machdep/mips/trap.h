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

#ifndef	_MIPS_TRAP_H_
#define	_MIPS_TRAP_H_

/*
 * MIPS exception and trap definitions
 */

/* Exception vector offsets */
#define MIPS_VEC_TLB_REFILL	0x000	/* TLB refill exception (32-bit) */
#define MIPS_VEC_XTLB_REFILL	0x080	/* XTLB refill exception (64-bit) */
#define MIPS_VEC_CACHE_ERROR	0x100	/* Cache error exception */
#define MIPS_VEC_GENERAL	0x180	/* General exception */
#define MIPS_VEC_INTERRUPT	0x200	/* Interrupt exception (R2+) */

/* Exception codes (from CP0 Cause register) */
#define T_INT		0	/* Interrupt */
#define T_TLB_MOD	1	/* TLB modification exception */
#define T_TLB_LD	2	/* TLB exception (load or instruction fetch) */
#define T_TLB_ST	3	/* TLB exception (store) */
#define T_ADDR_ERR_LD	4	/* Address error (load or instruction fetch) */
#define T_ADDR_ERR_ST	5	/* Address error (store) */
#define T_BUS_ERR_IF	6	/* Bus error (instruction fetch) */
#define T_BUS_ERR_LD	7	/* Bus error (data reference: load or store) */
#define T_SYSCALL	8	/* System call */
#define T_BREAK		9	/* Breakpoint */
#define T_RES_INST	10	/* Reserved instruction */
#define T_COP_UNUSABLE	11	/* Coprocessor unusable */
#define T_OVFLOW	12	/* Arithmetic overflow */
#define T_TRAP		13	/* Trap instruction */
#define T_FPE		15	/* Floating point exception */
#define T_WATCH		23	/* Reference to watchpoint address */
#define T_MCHECK	24	/* Machine check */

/* Number of exception types */
#define T_NEXC		32

#ifndef __ASSEMBLER__

#include <mach/mips/thread_status.h>

/*
 * Trap frame structure - saved state on exception/interrupt entry
 * This structure is saved on the kernel stack when an exception occurs
 */
#if defined(_MIPS64) || defined(__mips64)
struct mips_saved_state {
	unsigned long long zero;	/* $0 - always zero, saved for convenience */
	unsigned long long at;		/* $1 - assembler temporary */
	unsigned long long v0;		/* $2 - return value 0 */
	unsigned long long v1;		/* $3 - return value 1 */
	unsigned long long a0;		/* $4 - argument 0 */
	unsigned long long a1;		/* $5 - argument 1 */
	unsigned long long a2;		/* $6 - argument 2 */
	unsigned long long a3;		/* $7 - argument 3 */
	unsigned long long t0;		/* $8 - temporary 0 */
	unsigned long long t1;		/* $9 - temporary 1 */
	unsigned long long t2;		/* $10 - temporary 2 */
	unsigned long long t3;		/* $11 - temporary 3 */
	unsigned long long t4;		/* $12 - temporary 4 */
	unsigned long long t5;		/* $13 - temporary 5 */
	unsigned long long t6;		/* $14 - temporary 6 */
	unsigned long long t7;		/* $15 - temporary 7 */
	unsigned long long s0;		/* $16 - saved 0 */
	unsigned long long s1;		/* $17 - saved 1 */
	unsigned long long s2;		/* $18 - saved 2 */
	unsigned long long s3;		/* $19 - saved 3 */
	unsigned long long s4;		/* $20 - saved 4 */
	unsigned long long s5;		/* $21 - saved 5 */
	unsigned long long s6;		/* $22 - saved 6 */
	unsigned long long s7;		/* $23 - saved 7 */
	unsigned long long t8;		/* $24 - temporary 8 */
	unsigned long long t9;		/* $25 - temporary 9 */
	unsigned long long k0;		/* $26 - kernel reserved 0 */
	unsigned long long k1;		/* $27 - kernel reserved 1 */
	unsigned long long gp;		/* $28 - global pointer */
	unsigned long long sp;		/* $29 - stack pointer */
	unsigned long long fp;		/* $30 - frame pointer */
	unsigned long long ra;		/* $31 - return address */

	unsigned long long lo;		/* Multiply/divide result low */
	unsigned long long hi;		/* Multiply/divide result high */

	/* CP0 registers */
	unsigned long long status;	/* Status register */
	unsigned long long cause;	/* Cause register */
	unsigned long long epc;		/* Exception program counter */
	unsigned long long badvaddr;	/* Bad virtual address */
};
#else
struct mips_saved_state {
	unsigned int zero;		/* $0 - always zero, saved for convenience */
	unsigned int at;		/* $1 - assembler temporary */
	unsigned int v0;		/* $2 - return value 0 */
	unsigned int v1;		/* $3 - return value 1 */
	unsigned int a0;		/* $4 - argument 0 */
	unsigned int a1;		/* $5 - argument 1 */
	unsigned int a2;		/* $6 - argument 2 */
	unsigned int a3;		/* $7 - argument 3 */
	unsigned int t0;		/* $8 - temporary 0 */
	unsigned int t1;		/* $9 - temporary 1 */
	unsigned int t2;		/* $10 - temporary 2 */
	unsigned int t3;		/* $11 - temporary 3 */
	unsigned int t4;		/* $12 - temporary 4 */
	unsigned int t5;		/* $13 - temporary 5 */
	unsigned int t6;		/* $14 - temporary 6 */
	unsigned int t7;		/* $15 - temporary 7 */
	unsigned int s0;		/* $16 - saved 0 */
	unsigned int s1;		/* $17 - saved 1 */
	unsigned int s2;		/* $18 - saved 2 */
	unsigned int s3;		/* $19 - saved 3 */
	unsigned int s4;		/* $20 - saved 4 */
	unsigned int s5;		/* $21 - saved 5 */
	unsigned int s6;		/* $22 - saved 6 */
	unsigned int s7;		/* $23 - saved 7 */
	unsigned int t8;		/* $24 - temporary 8 */
	unsigned int t9;		/* $25 - temporary 9 */
	unsigned int k0;		/* $26 - kernel reserved 0 */
	unsigned int k1;		/* $27 - kernel reserved 1 */
	unsigned int gp;		/* $28 - global pointer */
	unsigned int sp;		/* $29 - stack pointer */
	unsigned int fp;		/* $30 - frame pointer */
	unsigned int ra;		/* $31 - return address */

	unsigned int lo;		/* Multiply/divide result low */
	unsigned int hi;		/* Multiply/divide result high */

	/* CP0 registers */
	unsigned int status;		/* Status register */
	unsigned int cause;		/* Cause register */
	unsigned int epc;		/* Exception program counter */
	unsigned int badvaddr;		/* Bad virtual address */
};
#endif

typedef struct mips_saved_state mips_saved_state_t;

#endif /* __ASSEMBLER__ */

#endif	/* _MIPS_TRAP_H_ */
