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
 * x86_64 Family:	External thread state.
 */

#import <architecture/x86_64/frame.h>
#import <architecture/x86_64/fpu.h>

/*
 * Main thread state consists of
 * general registers, segment registers,
 * rip and rflags.
 *
 * x86-64 has 16 general purpose 64-bit registers.
 */

#define x86_64_THREAD_STATE	-1

typedef struct {
    unsigned long	rax;
    unsigned long	rbx;
    unsigned long	rcx;
    unsigned long	rdx;
    unsigned long	rdi;
    unsigned long	rsi;
    unsigned long	rbp;
    unsigned long	rsp;
    unsigned long	r8;
    unsigned long	r9;
    unsigned long	r10;
    unsigned long	r11;
    unsigned long	r12;
    unsigned long	r13;
    unsigned long	r14;
    unsigned long	r15;
    unsigned long	rip;
    unsigned long	rflags;
    unsigned long	cs;
    unsigned long	fs;
    unsigned long	gs;
} x86_64_thread_state_t;

#define x86_64_THREAD_STATE_COUNT		\
    ( sizeof (x86_64_thread_state_t) / sizeof (unsigned long) )

/*
 * Default segment register values.
 * In x86-64 long mode, segmentation is mostly disabled.
 */

#define USER_CODE_SELECTOR	0x0033
#define USER_DATA_SELECTOR	0x002b
#define KERN_CODE_SELECTOR	0x0008
#define KERN_DATA_SELECTOR	0x0010

/*
 * Thread floating point state
 * includes FPU environment as
 * well as the register stack.
 * x86-64 uses SSE/SSE2 for floating point.
 */

#define x86_64_THREAD_FPSTATE	-2

typedef struct {
    fp_env_t		environ;
    fp_stack_t		stack;
} x86_64_thread_fpstate_t;

#define x86_64_THREAD_FPSTATE_COUNT 	\
    ( sizeof (x86_64_thread_fpstate_t) / sizeof (int) )

/*
 * Extra state that may be
 * useful to exception handlers.
 */

#define x86_64_THREAD_EXCEPTSTATE	-3

typedef struct {
    unsigned int	trapno;
    err_code_t		err;
} x86_64_thread_exceptstate_t;

#define x86_64_THREAD_EXCEPTSTATE_COUNT	\
    ( sizeof (x86_64_thread_exceptstate_t) / sizeof (int) )

/*
 * Per-thread variable used
 * to store 'self' id for cthreads.
 */

#define x86_64_THREAD_CTHREADSTATE	-4

typedef struct {
    unsigned long	self;
} x86_64_thread_cthreadstate_t;

#define x86_64_THREAD_CTHREADSTATE_COUNT	\
    ( sizeof (x86_64_thread_cthreadstate_t) / sizeof (unsigned long) )

/*
 * Machine-independent way for servers and Mach's exception mechanism to
 * choose the most efficient state flavor for exception RPC's:
 */
#define MACHINE_THREAD_STATE            x86_64_THREAD_STATE
#define MACHINE_THREAD_STATE_COUNT      x86_64_THREAD_STATE_COUNT
