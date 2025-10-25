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

#ifndef	_MACH_LOONGARCH_THREAD_STATUS_H_
#define _MACH_LOONGARCH_THREAD_STATUS_H_

/*
 * loongarch_thread_state is the structure that is exported to user threads for
 * use in status/mutate calls.  This structure should never change.
 */

#define LOONGARCH_THREAD_STATE		1
#define LOONGARCH_FLOAT_STATE		2
#define LOONGARCH_EXCEPTION_STATE	3
#define THREAD_STATE_NONE		7

/*
 * LoongArch general purpose thread state (32 registers)
 */
typedef struct loongarch_thread_state {
	unsigned int r0;	/* Always zero */
	unsigned int r1;	/* Return address */
	unsigned int r2;	/* Thread pointer */
	unsigned int r3;	/* Stack pointer */
	unsigned int r4;	/* Argument/return value 0 */
	unsigned int r5;	/* Argument/return value 1 */
	unsigned int r6;	/* Argument 2 */
	unsigned int r7;	/* Argument 3 */
	unsigned int r8;	/* Argument 4 */
	unsigned int r9;	/* Argument 5 */
	unsigned int r10;	/* Argument 6 */
	unsigned int r11;	/* Argument 7 */
	unsigned int r12;	/* Temporary 0 */
	unsigned int r13;	/* Temporary 1 */
	unsigned int r14;	/* Temporary 2 */
	unsigned int r15;	/* Temporary 3 */
	unsigned int r16;	/* Temporary 4 */
	unsigned int r17;	/* Temporary 5 */
	unsigned int r18;	/* Temporary 6 */
	unsigned int r19;	/* Temporary 7 */
	unsigned int r20;	/* Temporary 8 */
	unsigned int r21;	/* Reserved */
	unsigned int r22;	/* Frame pointer */
	unsigned int r23;	/* Saved 0 */
	unsigned int r24;	/* Saved 1 */
	unsigned int r25;	/* Saved 2 */
	unsigned int r26;	/* Saved 3 */
	unsigned int r27;	/* Saved 4 */
	unsigned int r28;	/* Saved 5 */
	unsigned int r29;	/* Saved 6 */
	unsigned int r30;	/* Saved 7 */
	unsigned int r31;	/* Saved 8 */
	unsigned int pc;	/* Program counter */
} loongarch_thread_state_t;

/*
 * LoongArch 64-bit general purpose thread state
 */
typedef struct loongarch64_thread_state {
	unsigned long long r0;		/* Always zero */
	unsigned long long r1;		/* Return address */
	unsigned long long r2;		/* Thread pointer */
	unsigned long long r3;		/* Stack pointer */
	unsigned long long r4;		/* Argument/return value 0 */
	unsigned long long r5;		/* Argument/return value 1 */
	unsigned long long r6;		/* Argument 2 */
	unsigned long long r7;		/* Argument 3 */
	unsigned long long r8;		/* Argument 4 */
	unsigned long long r9;		/* Argument 5 */
	unsigned long long r10;		/* Argument 6 */
	unsigned long long r11;		/* Argument 7 */
	unsigned long long r12;		/* Temporary 0 */
	unsigned long long r13;		/* Temporary 1 */
	unsigned long long r14;		/* Temporary 2 */
	unsigned long long r15;		/* Temporary 3 */
	unsigned long long r16;		/* Temporary 4 */
	unsigned long long r17;		/* Temporary 5 */
	unsigned long long r18;		/* Temporary 6 */
	unsigned long long r19;		/* Temporary 7 */
	unsigned long long r20;		/* Temporary 8 */
	unsigned long long r21;		/* Reserved */
	unsigned long long r22;		/* Frame pointer */
	unsigned long long r23;		/* Saved 0 */
	unsigned long long r24;		/* Saved 1 */
	unsigned long long r25;		/* Saved 2 */
	unsigned long long r26;		/* Saved 3 */
	unsigned long long r27;		/* Saved 4 */
	unsigned long long r28;		/* Saved 5 */
	unsigned long long r29;		/* Saved 6 */
	unsigned long long r30;		/* Saved 7 */
	unsigned long long r31;		/* Saved 8 */
	unsigned long long pc;		/* Program counter */
} loongarch64_thread_state_t;

/*
 * LoongArch floating point thread state (32 FP registers)
 * This structure should be double-word aligned for performance
 */
typedef struct loongarch_float_state {
	double f0;
	double f1;
	double f2;
	double f3;
	double f4;
	double f5;
	double f6;
	double f7;
	double f8;
	double f9;
	double f10;
	double f11;
	double f12;
	double f13;
	double f14;
	double f15;
	double f16;
	double f17;
	double f18;
	double f19;
	double f20;
	double f21;
	double f22;
	double f23;
	double f24;
	double f25;
	double f26;
	double f27;
	double f28;
	double f29;
	double f30;
	double f31;
	unsigned int fcsr;	/* Floating point control and status register */
	unsigned int fcc;	/* Floating point condition code */
} loongarch_float_state_t;

/*
 * LoongArch exception state
 * This structure corresponds to additional state saved upon kernel entry
 * for exception handling.
 */
typedef struct loongarch_exception_state {
	unsigned int badvaddr;	/* Bad virtual address (fault address) */
	unsigned int cause;	/* Exception cause */
	unsigned int era;	/* Exception return address */
	unsigned int pad0;	/* Padding for alignment */
} loongarch_exception_state_t;

/*
 * State count macros
 */
#define LOONGARCH_THREAD_STATE_COUNT \
	(sizeof(struct loongarch_thread_state) / sizeof(unsigned int))

#define LOONGARCH64_THREAD_STATE_COUNT \
	(sizeof(struct loongarch64_thread_state) / sizeof(unsigned int))

#define LOONGARCH_FLOAT_STATE_COUNT \
	(sizeof(struct loongarch_float_state) / sizeof(unsigned int))

#define LOONGARCH_EXCEPTION_STATE_COUNT \
	(sizeof(struct loongarch_exception_state) / sizeof(unsigned int))

/*
 * Machine-independent way for servers and Mach's exception mechanism to
 * choose the most efficient state flavor for exception RPC's:
 */
#define LOONGARCH_MACHINE_THREAD_STATE		LOONGARCH_THREAD_STATE
#define LOONGARCH_MACHINE_THREAD_STATE_COUNT	LOONGARCH_THREAD_STATE_COUNT

/*
 * Largest state on this machine:
 */
#define LOONGARCH_THREAD_MACHINE_STATE_MAX	LOONGARCH_FLOAT_STATE_COUNT

#endif /* _MACH_LOONGARCH_THREAD_STATUS_H_ */
