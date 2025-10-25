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
 * RISC-V Family:	External thread state.
 */

/*
 * Main thread state consists of general registers and PC.
 */

#define riscv_THREAD_STATE	-1

#if defined(__riscv_xlen) && __riscv_xlen == 64
typedef struct {
    unsigned long	pc;		/* Program counter */
    unsigned long	ra;		/* Return address (x1) */
    unsigned long	sp;		/* Stack pointer (x2) */
    unsigned long	gp;		/* Global pointer (x3) */
    unsigned long	tp;		/* Thread pointer (x4) */
    unsigned long	t0;		/* Temporary (x5) */
    unsigned long	t1;		/* Temporary (x6) */
    unsigned long	t2;		/* Temporary (x7) */
    unsigned long	s0;		/* Saved register/frame pointer (x8) */
    unsigned long	s1;		/* Saved register (x9) */
    unsigned long	a0;		/* Argument/return value (x10) */
    unsigned long	a1;		/* Argument/return value (x11) */
    unsigned long	a2;		/* Argument (x12) */
    unsigned long	a3;		/* Argument (x13) */
    unsigned long	a4;		/* Argument (x14) */
    unsigned long	a5;		/* Argument (x15) */
    unsigned long	a6;		/* Argument (x16) */
    unsigned long	a7;		/* Argument (x17) */
    unsigned long	s2;		/* Saved register (x18) */
    unsigned long	s3;		/* Saved register (x19) */
    unsigned long	s4;		/* Saved register (x20) */
    unsigned long	s5;		/* Saved register (x21) */
    unsigned long	s6;		/* Saved register (x22) */
    unsigned long	s7;		/* Saved register (x23) */
    unsigned long	s8;		/* Saved register (x24) */
    unsigned long	s9;		/* Saved register (x25) */
    unsigned long	s10;		/* Saved register (x26) */
    unsigned long	s11;		/* Saved register (x27) */
    unsigned long	t3;		/* Temporary (x28) */
    unsigned long	t4;		/* Temporary (x29) */
    unsigned long	t5;		/* Temporary (x30) */
    unsigned long	t6;		/* Temporary (x31) */
} riscv_thread_state_t;
#else
typedef struct {
    unsigned int	pc;		/* Program counter */
    unsigned int	ra;		/* Return address (x1) */
    unsigned int	sp;		/* Stack pointer (x2) */
    unsigned int	gp;		/* Global pointer (x3) */
    unsigned int	tp;		/* Thread pointer (x4) */
    unsigned int	t0;		/* Temporary (x5) */
    unsigned int	t1;		/* Temporary (x6) */
    unsigned int	t2;		/* Temporary (x7) */
    unsigned int	s0;		/* Saved register/frame pointer (x8) */
    unsigned int	s1;		/* Saved register (x9) */
    unsigned int	a0;		/* Argument/return value (x10) */
    unsigned int	a1;		/* Argument/return value (x11) */
    unsigned int	a2;		/* Argument (x12) */
    unsigned int	a3;		/* Argument (x13) */
    unsigned int	a4;		/* Argument (x14) */
    unsigned int	a5;		/* Argument (x15) */
    unsigned int	a6;		/* Argument (x16) */
    unsigned int	a7;		/* Argument (x17) */
    unsigned int	s2;		/* Saved register (x18) */
    unsigned int	s3;		/* Saved register (x19) */
    unsigned int	s4;		/* Saved register (x20) */
    unsigned int	s5;		/* Saved register (x21) */
    unsigned int	s6;		/* Saved register (x22) */
    unsigned int	s7;		/* Saved register (x23) */
    unsigned int	s8;		/* Saved register (x24) */
    unsigned int	s9;		/* Saved register (x25) */
    unsigned int	s10;		/* Saved register (x26) */
    unsigned int	s11;		/* Saved register (x27) */
    unsigned int	t3;		/* Temporary (x28) */
    unsigned int	t4;		/* Temporary (x29) */
    unsigned int	t5;		/* Temporary (x30) */
    unsigned int	t6;		/* Temporary (x31) */
} riscv_thread_state_t;
#endif

#define riscv_THREAD_STATE_COUNT		\
    ( sizeof (riscv_thread_state_t) / sizeof (int) )

/*
 * Thread floating point state
 * includes FPU environment as well as the register stack.
 */

#define riscv_THREAD_FPSTATE	-2

#if defined(__riscv_xlen) && __riscv_xlen == 64
typedef struct {
    unsigned long	f[32];		/* f0-f31 floating point registers */
    unsigned int	fcsr;		/* Floating-point control and status */
} riscv_thread_fpstate_t;
#else
typedef struct {
    unsigned int	f[32];		/* f0-f31 floating point registers */
    unsigned int	fcsr;		/* Floating-point control and status */
} riscv_thread_fpstate_t;
#endif

#define riscv_THREAD_FPSTATE_COUNT 	\
    ( sizeof (riscv_thread_fpstate_t) / sizeof (int) )

/*
 * Extra state that may be useful to exception handlers.
 */

#define riscv_THREAD_EXCEPTSTATE	-3

typedef struct {
    unsigned int	cause;		/* Exception cause */
    unsigned long	tval;		/* Trap value */
} riscv_thread_exceptstate_t;

#define riscv_THREAD_EXCEPTSTATE_COUNT	\
    ( sizeof (riscv_thread_exceptstate_t) / sizeof (int) )

/*
 * Per-thread variable used to store 'self' id for cthreads.
 */

#define riscv_THREAD_CTHREADSTATE	-4

typedef struct {
    unsigned long	self;
} riscv_thread_cthreadstate_t;

#define riscv_THREAD_CTHREADSTATE_COUNT	\
    ( sizeof (riscv_thread_cthreadstate_t) / sizeof (int) )

/*
 * Machine-independent way for servers and Mach's exception mechanism to
 * choose the most efficient state flavor for exception RPC's:
 */
#define MACHINE_THREAD_STATE            riscv_THREAD_STATE
#define MACHINE_THREAD_STATE_COUNT      riscv_THREAD_STATE_COUNT
