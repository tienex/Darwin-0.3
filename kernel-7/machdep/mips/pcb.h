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

#ifndef	_MIPS_PCB_H_
#define	_MIPS_PCB_H_

#include <mach/mips/thread_status.h>

/*
 * MIPS Process Control Block (PCB)
 * Contains per-thread machine-dependent state
 */

#if defined(_MIPS64) || defined(__mips64)
struct mips_pcb {
	/* General purpose registers (callee-saved only) */
	unsigned long long s0;		/* $16 */
	unsigned long long s1;		/* $17 */
	unsigned long long s2;		/* $18 */
	unsigned long long s3;		/* $19 */
	unsigned long long s4;		/* $20 */
	unsigned long long s5;		/* $21 */
	unsigned long long s6;		/* $22 */
	unsigned long long s7;		/* $23 */
	unsigned long long s8;		/* $30 (fp) */

	unsigned long long gp;		/* $28 - global pointer */
	unsigned long long sp;		/* $29 - stack pointer */
	unsigned long long ra;		/* $31 - return address */
	unsigned long long pc;		/* Program counter */

	/* Floating point state */
	struct mips_float_state pcb_fpregs;

	/* Kernel stack pointer */
	unsigned long long kernel_sp;

	/* TLB ASID (Address Space ID) */
	unsigned int asid;

	/* PCB flags */
	unsigned int flags;
};
#else
struct mips_pcb {
	/* General purpose registers (callee-saved only) */
	unsigned int s0;		/* $16 */
	unsigned int s1;		/* $17 */
	unsigned int s2;		/* $18 */
	unsigned int s3;		/* $19 */
	unsigned int s4;		/* $20 */
	unsigned int s5;		/* $21 */
	unsigned int s6;		/* $22 */
	unsigned int s7;		/* $23 */
	unsigned int s8;		/* $30 (fp) */

	unsigned int gp;		/* $28 - global pointer */
	unsigned int sp;		/* $29 - stack pointer */
	unsigned int ra;		/* $31 - return address */
	unsigned int pc;		/* Program counter */

	/* Floating point state */
	struct mips_float_state pcb_fpregs;

	/* Kernel stack pointer */
	unsigned int kernel_sp;

	/* TLB ASID (Address Space ID) */
	unsigned int asid;

	/* PCB flags */
	unsigned int flags;
};
#endif

typedef struct mips_pcb *pcb_t;

/* PCB flags */
#define PCB_FPU_USED	0x00000001	/* FPU has been used */
#define PCB_FPU_OWNER	0x00000002	/* Thread owns FPU state */
#define PCB_USER_MODE	0x00000004	/* Running in user mode */

#endif	/* _MIPS_PCB_H_ */
