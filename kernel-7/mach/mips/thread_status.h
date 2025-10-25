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

#ifndef	_MACH_MIPS_THREAD_STATUS_H_
#define _MACH_MIPS_THREAD_STATUS_H_

/*
 * mips_thread_state is the structure that is exported to user threads for
 * use in status/mutate calls.  This structure should never change.
 *
 */

#define MIPS_THREAD_STATE        1
#define MIPS_FLOAT_STATE         2
#define MIPS_EXCEPTION_STATE     3
#define THREAD_STATE_NONE        7

/*
 * MIPS General Purpose Register State
 * Supports both MIPS32 and MIPS64
 */
#if defined(_MIPS64) || defined(__mips64)
/* MIPS64 thread state */
struct mips_thread_state {
	unsigned long long pc;		/* Program counter */
	unsigned long long regs[32];	/* General purpose registers $0-$31 */
	unsigned long long lo;		/* Multiply/divide result LO */
	unsigned long long hi;		/* Multiply/divide result HI */
};
#else
/* MIPS32 thread state */
struct mips_thread_state {
	unsigned int pc;		/* Program counter */
	unsigned int regs[32];		/* General purpose registers $0-$31 */
	unsigned int lo;		/* Multiply/divide result LO */
	unsigned int hi;		/* Multiply/divide result HI */
};
#endif

typedef struct mips_thread_state mips_thread_state_t;

/*
 * MIPS Floating Point Register State
 * FPU registers - 32 registers, each can be single or paired for double
 */
struct mips_float_state {
	double  fpregs[32];		/* FP registers $f0-$f31 */
	unsigned int fpcsr;		/* FP control/status register */
	unsigned int fpir;		/* FP implementation register */
};

typedef struct mips_float_state mips_float_state_t;

/*
 * MIPS Exception State
 * Contains CP0 (coprocessor 0) registers for exception handling
 */
#if defined(_MIPS64) || defined(__mips64)
struct mips_exception_state {
	unsigned long long cause;	/* Exception cause register */
	unsigned long long badvaddr;	/* Bad virtual address */
	unsigned long long status;	/* Status register */
	unsigned long long epc;		/* Exception program counter */
};
#else
struct mips_exception_state {
	unsigned int cause;		/* Exception cause register */
	unsigned int badvaddr;		/* Bad virtual address */
	unsigned int status;		/* Status register */
	unsigned int epc;		/* Exception program counter */
};
#endif

typedef struct mips_exception_state mips_exception_state_t;

/*
 * Thread state flavor counts
 */
#define MIPS_THREAD_STATE_COUNT \
	(sizeof(struct mips_thread_state) / sizeof(int))
#define MIPS_FLOAT_STATE_COUNT \
	(sizeof(struct mips_float_state) / sizeof(int))
#define MIPS_EXCEPTION_STATE_COUNT \
	(sizeof(struct mips_exception_state) / sizeof(int))

#endif	/* _MACH_MIPS_THREAD_STATUS_H_ */
