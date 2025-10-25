/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Thread Status Definitions
 */

#ifndef _MACH_ALPHA_THREAD_STATUS_H_
#define _MACH_ALPHA_THREAD_STATUS_H_

#include <architecture/alpha/reg.h>

/*
 * Thread state flavors
 */
#define ALPHA_THREAD_STATE		1
#define ALPHA_FLOAT_STATE		2
#define ALPHA_EXCEPTION_STATE		3

/*
 * Thread state
 */
struct alpha_thread_state {
	unsigned long	r0;		/* v0: return value */
	unsigned long	r1;		/* t0-t7: temporaries */
	unsigned long	r2;
	unsigned long	r3;
	unsigned long	r4;
	unsigned long	r5;
	unsigned long	r6;
	unsigned long	r7;
	unsigned long	r8;
	unsigned long	r9;		/* s0-s5: saved registers */
	unsigned long	r10;
	unsigned long	r11;
	unsigned long	r12;
	unsigned long	r13;
	unsigned long	r14;
	unsigned long	r15;		/* fp: frame pointer */
	unsigned long	r16;		/* a0-a5: arguments */
	unsigned long	r17;
	unsigned long	r18;
	unsigned long	r19;
	unsigned long	r20;
	unsigned long	r21;
	unsigned long	r22;		/* t8-t11: temporaries */
	unsigned long	r23;
	unsigned long	r24;
	unsigned long	r25;
	unsigned long	r26;		/* ra: return address */
	unsigned long	r27;		/* t12/pv: procedure value */
	unsigned long	r28;		/* at: assembler temp */
	unsigned long	r29;		/* gp: global pointer */
	unsigned long	r30;		/* sp: stack pointer */
	unsigned long	pc;		/* program counter */
	unsigned long	ps;		/* processor status */
};

#define ALPHA_THREAD_STATE_COUNT \
	(sizeof(struct alpha_thread_state) / sizeof(unsigned long))

typedef struct alpha_thread_state	alpha_thread_state_t;

/*
 * Floating point state
 */
struct alpha_float_state {
	unsigned long	fpregs[32];	/* f0-f31 */
	unsigned long	fpcr;		/* FP control register */
};

#define ALPHA_FLOAT_STATE_COUNT \
	(sizeof(struct alpha_float_state) / sizeof(unsigned long))

typedef struct alpha_float_state	alpha_float_state_t;

/*
 * Exception state
 */
struct alpha_exception_state {
	unsigned long	trapno;		/* Exception/trap number */
	unsigned long	err;		/* Error code */
	unsigned long	exc_addr;	/* Exception address */
};

#define ALPHA_EXCEPTION_STATE_COUNT \
	(sizeof(struct alpha_exception_state) / sizeof(unsigned long))

typedef struct alpha_exception_state	alpha_exception_state_t;

#endif /* _MACH_ALPHA_THREAD_STATUS_H_ */
