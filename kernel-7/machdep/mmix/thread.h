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
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 *	File:	mmix/thread.h
 *
 *	This file defines machine specific, thread related structures,
 *	variables and macros for MMIX architecture.
 */

#ifndef	_MACHDEP_MMIX_THREAD_
#define _MACHDEP_MMIX_THREAD_

#include <kern/kernel_stack.h>
#include <mach/mmix/thread_status.h>

/*
 * MMIX process control block
 *
 * In the continuation model, the PCB holds the user context. It is used
 * on entry to the kernel from user mode, either by system call or trap,
 * to store the necessary user registers and other state.
 */
struct pcb
{
	struct mmix_thread_state ss;		/* saved general registers */
	struct mmix_exception_state es;		/* exception state */
	struct mmix_float_state fs;		/* floating point state */
	unsigned long long ksp;			/* kernel stack pointer */
	unsigned long long cthread_self;	/* for use of cthread package */
        unsigned int flags;
};

typedef struct pcb *pcb_t;

struct mmix_kernel_state
{
        pcb_t    pcb;				/* PCB pointer */
        unsigned long long r254;		/* Stack pointer ($254) */
        unsigned long long r253;		/* Frame pointer ($253) */
        unsigned long long r16[8];		/* Callee saved regs ($16-$23) */
        unsigned long long rJ;			/* Return address (rJ) */
        unsigned long long rL;			/* Local register threshold */
						/* Floating point state saved separately */
};

#define pcb_synch(thread)	/* FP state management */

#define USER_REGS(thread)       (&(thread)->pcb->ss)

#define STACK_IKS(stack)                \
        (((struct mmix_kernel_state *)(((vm_offset_t)stack)+KERNEL_STACK_SIZE))-1)

#ifndef __ASSEMBLER__
/*
 * These routines are called from task_create() to
 * manage any pcb state common to all threads in a task
 * via the "task->pcb_common" pointer. Currently null for MMIX.
 */
#define pcb_common_init(task)
#define pcb_common_terminate(task)
#endif  /* !__ASSEMBLER__ */

#endif	/* _MACHDEP_MMIX_THREAD_ */
