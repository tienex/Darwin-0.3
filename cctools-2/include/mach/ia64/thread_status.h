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

#ifndef	_MACH_IA64_THREAD_STATUS_H_
#define _MACH_IA64_THREAD_STATUS_H_

/*
 * ia64_thread_state is the structure that is exported to user threads for
 * use in status/mutate calls.  This structure should never change.
 *
 * IA-64 has 128 general registers (r0 hardwired to 0, r1 = gp,
 * r12 = sp, r13 = tp), 128 floating-point registers, 64 one-bit
 * predicate registers, 8 branch registers and a large set of
 * application registers.  Only the static subset (r1-r31) plus the
 * control state needed to resume a thread is exported here; the
 * stacked registers r32-r127 live in the backing store governed by
 * ar.bsp/ar.bspstore.
 */

#define IA64_THREAD_STATE       1
#define IA64_EXCEPTION_STATE    2
#define IA64_FLOAT_STATE        3
#define THREAD_STATE_NONE       7

#define IA64_STATIC_GREGS       32      /* r0-r31 (r0 reads as zero) */
#define IA64_BRANCH_REGS        8       /* b0-b7 */

struct ia64_thread_state {
	unsigned long long ip;          /* Instruction pointer (bundle addr) */
	unsigned long long psr;         /* Processor status register */
	unsigned long long cfm;         /* Current frame marker */

	/* Static general registers r0-r31 (r0 always 0) */
	unsigned long long r[IA64_STATIC_GREGS];

	/* NaT collection bits for r0-r31 (bit i is r[i].nat) */
	unsigned long long nat;

	/* Branch registers b0-b7 (b0 is the return link) */
	unsigned long long b[IA64_BRANCH_REGS];

	/* Predicate registers p0-p63, one bit each (p0 always 1) */
	unsigned long long pr;

	/* Application registers needed to resume a thread */
	unsigned long long ar_rsc;      /* Register stack configuration */
	unsigned long long ar_bsp;      /* Backing store pointer */
	unsigned long long ar_bspstore; /* Backing store store pointer */
	unsigned long long ar_rnat;     /* RSE NaT collection register */
	unsigned long long ar_ccv;      /* Compare-and-exchange value */
	unsigned long long ar_unat;     /* User NaT collection register */
	unsigned long long ar_fpsr;     /* Floating-point status register */
	unsigned long long ar_pfs;      /* Previous function state */
	unsigned long long ar_lc;       /* Loop count register */
	unsigned long long ar_ec;       /* Epilog count register */
};

typedef struct ia64_thread_state ia64_thread_state_t;

#define IA64_THREAD_STATE_COUNT	\
	(sizeof(struct ia64_thread_state) / sizeof(int))

/*
 * Exception state
 */
struct ia64_exception_state {
	unsigned long long ifa;         /* Interruption faulting address */
	unsigned long long isr;         /* Interruption status register */
	unsigned long long iip;         /* Interruption instruction pointer */
	unsigned long long exception;   /* Exception type */
};

typedef struct ia64_exception_state ia64_exception_state_t;

#define IA64_EXCEPTION_STATE_COUNT	\
	(sizeof(struct ia64_exception_state) / sizeof(int))

/*
 * Floating-point state.  The 128 FP registers are 82 bits wide and
 * are exported as 128-bit spill images (as produced by spill/fill).
 * f0 reads as +0.0 and f1 as +1.0; both are included for layout
 * regularity.  Only the low static partition f0-f31 is exported;
 * f32-f127 are the rotating registers handled with the RSE state.
 */
#define IA64_STATIC_FPREGS      32

struct ia64_fp_register {
	unsigned long long lo;          /* Low 64 bits of spill image */
	unsigned long long hi;          /* High 64 bits of spill image */
};

struct ia64_float_state {
	struct ia64_fp_register f[IA64_STATIC_FPREGS];
};

typedef struct ia64_float_state ia64_float_state_t;

#define IA64_FLOAT_STATE_COUNT	\
	(sizeof(struct ia64_float_state) / sizeof(int))

#endif	/* _MACH_IA64_THREAD_STATUS_H_ */
