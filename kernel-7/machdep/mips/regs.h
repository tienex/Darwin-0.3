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
 * MIPS:	General processor registers.
 *
 * Format of saved registers on the kernel stack.
 */

#ifndef _MACHDEP_MIPS_REGS_H_
#define _MACHDEP_MIPS_REGS_H_

/*
 * MIPS saved register state
 * Matches the mips_saved_state structure layout
 */

#if defined(_MIPS64) || defined(__mips64)

typedef struct regs {
	unsigned long	regs[32];	/* r0-r31 */
	unsigned long	lo;		/* Multiply/divide LO */
	unsigned long	hi;		/* Multiply/divide HI */
	unsigned long	pc;		/* Program counter */
	unsigned long	badvaddr;	/* Bad virtual address */
	unsigned int	cause;		/* Exception cause */
	unsigned int	status;		/* Status register */
} regs_t;

#else /* MIPS32 */

typedef struct regs {
	unsigned int	regs[32];	/* r0-r31 */
	unsigned int	lo;		/* Multiply/divide LO */
	unsigned int	hi;		/* Multiply/divide HI */
	unsigned int	pc;		/* Program counter */
	unsigned int	badvaddr;	/* Bad virtual address */
	unsigned int	cause;		/* Exception cause */
	unsigned int	status;		/* Status register */
} regs_t;

#endif /* _MIPS64 */

/*
 * Register indices for user state access
 */
#define MIPS_REG_ZERO	0
#define MIPS_REG_AT	1
#define MIPS_REG_V0	2
#define MIPS_REG_V1	3
#define MIPS_REG_A0	4
#define MIPS_REG_A1	5
#define MIPS_REG_A2	6
#define MIPS_REG_A3	7
#define MIPS_REG_T0	8
#define MIPS_REG_T1	9
#define MIPS_REG_T2	10
#define MIPS_REG_T3	11
#define MIPS_REG_T4	12
#define MIPS_REG_T5	13
#define MIPS_REG_T6	14
#define MIPS_REG_T7	15
#define MIPS_REG_S0	16
#define MIPS_REG_S1	17
#define MIPS_REG_S2	18
#define MIPS_REG_S3	19
#define MIPS_REG_S4	20
#define MIPS_REG_S5	21
#define MIPS_REG_S6	22
#define MIPS_REG_S7	23
#define MIPS_REG_T8	24
#define MIPS_REG_T9	25
#define MIPS_REG_K0	26
#define MIPS_REG_K1	27
#define MIPS_REG_GP	28
#define MIPS_REG_SP	29
#define MIPS_REG_FP	30	/* s8/fp */
#define MIPS_REG_RA	31

#endif /* _MACHDEP_MIPS_REGS_H_ */
