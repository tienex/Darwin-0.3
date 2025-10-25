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

#ifndef	_MACH_MMIX_THREAD_STATUS_H_
#define _MACH_MMIX_THREAD_STATUS_H_

/*
 * mmix_thread_state is the structure that is exported to user threads for
 * use in status/mutate calls.  This structure should never change.
 *
 * MMIX has 256 general-purpose 64-bit registers and 32 special registers.
 */

#define MMIX_THREAD_STATE       1
#define MMIX_EXCEPTION_STATE    2
#define THREAD_STATE_NONE       7

/*
 * For efficiency, we only save/restore the subset of registers
 * that are typically used. Full 256 register state would be too large.
 */
#define MMIX_SAVED_GREGS        32      /* Save first 32 general registers */

struct mmix_thread_state {
	unsigned long long pc;          /* Program counter (rW) */

	/* General-purpose registers $0-$31 */
	unsigned long long r[MMIX_SAVED_GREGS];

	/* Important special registers */
	unsigned long long rA;          /* Arithmetic status register */
	unsigned long long rB;          /* Bootstrap register */
	unsigned long long rC;          /* Continuation register */
	unsigned long long rD;          /* Dividend register */
	unsigned long long rE;          /* Epsilon register */
	unsigned long long rF;          /* Failure location register */
	unsigned long long rG;          /* Global threshold register */
	unsigned long long rH;          /* Himult register */
	unsigned long long rI;          /* Interval counter */
	unsigned long long rJ;          /* Return-jump register */
	unsigned long long rK;          /* Interrupt mask register */
	unsigned long long rL;          /* Local threshold register */
	unsigned long long rM;          /* Multiplex mask register */
	unsigned long long rN;          /* Serial number */
	unsigned long long rO;          /* Register stack offset */
	unsigned long long rP;          /* Prediction register */
	unsigned long long rQ;          /* Interrupt request register */
	unsigned long long rR;          /* Remainder register */
	unsigned long long rS;          /* Register stack pointer */
	unsigned long long rT;          /* Trap address register */
	unsigned long long rU;          /* Usage counter */
	unsigned long long rV;          /* Virtual translation register */
	unsigned long long rW;          /* Where-interrupted register */
	unsigned long long rX;          /* Execution register */
	unsigned long long rY;          /* Y operand */
	unsigned long long rZ;          /* Z operand */

	unsigned long long rBB;         /* Bootstrap register (trap) */
	unsigned long long rTT;         /* Dynamic trap address register */
	unsigned long long rWW;         /* Where-interrupted register (trap) */
	unsigned long long rXX;         /* Execution register (trap) */
	unsigned long long rYY;         /* Y operand (trap) */
	unsigned long long rZZ;         /* Z operand (trap) */
};

typedef struct mmix_thread_state mmix_thread_state_t;

#define MMIX_THREAD_STATE_COUNT	\
	(sizeof(struct mmix_thread_state) / sizeof(int))

/*
 * Exception state
 */
struct mmix_exception_state {
	unsigned long long dar;         /* Data access register */
	unsigned long long dsisr;       /* Exception syndrome register */
	unsigned long long exception;   /* Exception type */
};

typedef struct mmix_exception_state mmix_exception_state_t;

#define MMIX_EXCEPTION_STATE_COUNT	\
	(sizeof(struct mmix_exception_state) / sizeof(int))

#endif	/* _MACH_MMIX_THREAD_STATUS_H_ */
