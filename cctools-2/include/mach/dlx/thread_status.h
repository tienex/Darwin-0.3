/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * DLX Thread State Structures
 * Defines register context for threads
 */

#ifndef _MACH_DLX_THREAD_STATUS_H_
#define _MACH_DLX_THREAD_STATUS_H_

/*
 * DLX has 32 general-purpose registers (r0-r31)
 * r0 is always zero
 * r29 is stack pointer
 * r30 is frame pointer
 * r31 is return address
 */

#define DLX_THREAD_STATE_COUNT	35	/* 32 regs + PC + status + padding */

struct dlx_thread_state {
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int r12;
	unsigned int r13;
	unsigned int r14;
	unsigned int r15;
	unsigned int r16;
	unsigned int r17;
	unsigned int r18;
	unsigned int r19;
	unsigned int r20;
	unsigned int r21;
	unsigned int r22;
	unsigned int r23;
	unsigned int r24;
	unsigned int r25;
	unsigned int r26;
	unsigned int r27;
	unsigned int r28;
	unsigned int r29;		/* Stack pointer */
	unsigned int r30;		/* Frame pointer */
	unsigned int r31;		/* Return address */
	unsigned int pc;		/* Program counter */
	unsigned int status;		/* Status register */
	unsigned int padding;
};

typedef struct dlx_thread_state dlx_thread_state_t;

/*
 * DLX Floating Point State
 * DLX has 32 floating-point registers (f0-f31)
 */

#define DLX_FLOAT_STATE_COUNT	33	/* 32 FP regs + status */

struct dlx_float_state {
	unsigned int f[32];		/* FP registers */
	unsigned int fpstatus;		/* FP status register */
};

typedef struct dlx_float_state dlx_float_state_t;

/*
 * DLX Exception State
 * Saved during exception handling
 */

#define DLX_EXCEPTION_STATE_COUNT	8

struct dlx_exception_state {
	unsigned int trapno;		/* Exception/trap number */
	unsigned int err;		/* Error code */
	unsigned int faultaddr;		/* Faulting address */
	unsigned int status;		/* Status at exception */
	unsigned int cause;		/* Exception cause */
	unsigned int epc;		/* Exception PC */
	unsigned int badvaddr;		/* Bad virtual address */
	unsigned int padding;
};

typedef struct dlx_exception_state dlx_exception_state_t;

/*
 * Thread state flavors
 */

#define DLX_THREAD_STATE		1
#define DLX_FLOAT_STATE			2
#define DLX_EXCEPTION_STATE		3

/*
 * Combined thread saved state for kernel use
 */

struct dlx_saved_state {
	struct dlx_thread_state		thread;
	struct dlx_float_state		float_state;
	struct dlx_exception_state	exception;
};

typedef struct dlx_saved_state dlx_saved_state_t;

#endif /* _MACH_DLX_THREAD_STATUS_H_ */
