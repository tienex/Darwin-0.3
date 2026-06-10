/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * MMIX:	Special processor registers.
 *
 * MMIX is a 64-bit RISC architecture designed by Donald Knuth.
 */

#ifndef _ARCHITECTURE_MMIX_CPU_H_
#define _ARCHITECTURE_MMIX_CPU_H_

/*
 * MMIX has 256 general-purpose registers ($0-$255)
 * and 32 special-purpose registers (rA-rZZ).
 */

#define MMIX_NREGS	256

/*
 * Special register indices
 */
#define MMIX_rA		21	/* Arithmetic status register */
#define MMIX_rB		0	/* Bootstrap register */
#define MMIX_rC		8	/* Continuation register */
#define MMIX_rD		1	/* Dividend register */
#define MMIX_rE		2	/* Epsilon register */
#define MMIX_rG		19	/* Global threshold register */
#define MMIX_rH		3	/* Himult register */
#define MMIX_rI		12	/* Interval counter */
#define MMIX_rJ		4	/* Return-jump register */
#define MMIX_rK		15	/* Interrupt mask register */
#define MMIX_rL		20	/* Local threshold register */
#define MMIX_rM		5	/* Multiplex mask register */
#define MMIX_rN		9	/* Serial number */
#define MMIX_rO		10	/* Register stack offset */
#define MMIX_rP		23	/* Prediction register */
#define MMIX_rQ		16	/* Interrupt request register */
#define MMIX_rR		6	/* Remainder register */
#define MMIX_rS		11	/* Register stack pointer */
#define MMIX_rT		13	/* Trap address register */
#define MMIX_rU		17	/* Usage counter */
#define MMIX_rV		18	/* Virtual translation register */
#define MMIX_rW		24	/* Where-interrupted register */
#define MMIX_rX		25	/* Execution register */
#define MMIX_rY		26	/* Y operand */
#define MMIX_rZ		27	/* Z operand */
#define MMIX_rBB	7	/* Bootstrap register (trap) */
#define MMIX_rTT	14	/* Dynamic trap address register */
#define MMIX_rWW	28	/* Where-interrupted register (trap) */
#define MMIX_rXX	29	/* Execution register (trap) */
#define MMIX_rYY	30	/* Y operand (trap) */
#define MMIX_rZZ	31	/* Z operand (trap) */

#endif /* _ARCHITECTURE_MMIX_CPU_H_ */
