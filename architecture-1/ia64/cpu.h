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
 * IA64:	Processor registers.
 *
 * IA-64 (Intel Itanium) is a 64-bit EPIC (Explicitly Parallel
 * Instruction Computing) architecture.  Instructions are packed three
 * to a 128-bit bundle along with a template that describes issue
 * groups.
 */

#ifndef _ARCHITECTURE_IA64_CPU_H_
#define _ARCHITECTURE_IA64_CPU_H_

/*
 * IA-64 has 128 general registers (r0-r127), 128 floating-point
 * registers (f0-f127), 64 predicate registers (p0-p63) and 8 branch
 * registers (b0-b7).  r32-r127 are stacked registers managed by the
 * Register Stack Engine (RSE).
 */

#define IA64_NGREGS	128
#define IA64_NFPREGS	128
#define IA64_NPREDS	64
#define IA64_NBREGS	8

/*
 * Fixed-function general registers
 */
#define IA64_GR_ZERO	0	/* r0: always reads as zero */
#define IA64_GR_GP	1	/* r1: global data pointer */
#define IA64_GR_RET0	8	/* r8: first return value register */
#define IA64_GR_SP	12	/* r12: memory stack pointer */
#define IA64_GR_TP	13	/* r13: thread pointer */
#define IA64_GR_STACKED	32	/* r32: first stacked (RSE) register */

/*
 * Fixed-function floating-point registers
 */
#define IA64_FR_ZERO	0	/* f0: always reads as +0.0 */
#define IA64_FR_ONE	1	/* f1: always reads as +1.0 */

/*
 * Fixed-function branch and predicate registers
 */
#define IA64_BR_RP	0	/* b0: return pointer */
#define IA64_PR_TRUE	0	/* p0: always reads as 1 */

/*
 * Application register indices (subset relevant to user state)
 */
#define IA64_AR_KR0	0	/* Kernel register 0 */
#define IA64_AR_RSC	16	/* Register stack configuration */
#define IA64_AR_BSP	17	/* Backing store pointer */
#define IA64_AR_BSPSTORE 18	/* Backing store store pointer */
#define IA64_AR_RNAT	19	/* RSE NaT collection */
#define IA64_AR_CCV	32	/* Compare-and-exchange compare value */
#define IA64_AR_UNAT	36	/* User NaT collection */
#define IA64_AR_FPSR	40	/* Floating-point status register */
#define IA64_AR_ITC	44	/* Interval time counter */
#define IA64_AR_PFS	64	/* Previous function state */
#define IA64_AR_LC	65	/* Loop count */
#define IA64_AR_EC	66	/* Epilog count */

/*
 * Instruction bundle geometry
 */
#define IA64_BUNDLE_SIZE	16	/* Bytes per instruction bundle */
#define IA64_SLOTS_PER_BUNDLE	3	/* Instruction slots per bundle */

#endif /* _ARCHITECTURE_IA64_CPU_H_ */
