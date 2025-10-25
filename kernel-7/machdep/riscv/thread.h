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
 * RISC-V Family: Machine dependent thread state.
 *
 * This file defines machine specific, thread related structures,
 * variables and macros for RISC-V architecture.
 */

#ifndef	_MACHDEP_RISCV_THREAD_H_
#define _MACHDEP_RISCV_THREAD_H_

#include <kern/kernel_stack.h>
#include <mach/riscv/thread_status.h>

/*
 * RISC-V process control block
 *
 * In the continuation model, the PCB holds the user context. It is used
 * on entry to the kernel from user mode, either by system call or trap,
 * to store the necessary user registers and other state.
 */
struct pcb
{
	struct riscv_thread_state ss;          /* Saved thread state */
	struct riscv_float_state fs;           /* Floating point state */
	struct riscv_exception_state es;       /* Exception state */
	unsigned long ksp;                     /* Kernel stack pointer */
	unsigned long cthread_self;            /* cthread self pointer */
	unsigned int flags;                    /* PCB flags */
};

typedef struct pcb *pcb_t;

/* PCB flags */
#define PCB_FPUSED      0x0001  /* FP registers have been used */
#define PCB_FPVALID     0x0002  /* FP state is valid */

/*
 * RISC-V kernel state for context switching
 */
struct riscv_kernel_state
{
	pcb_t pcb;                      /* PCB pointer */
	unsigned long sp;               /* Stack pointer (x2) */
	unsigned long ra;               /* Return address (x1) */
	unsigned long s0;               /* Saved register s0 (x8) */
	unsigned long s1;               /* Saved register s1 (x9) */
	unsigned long s2;               /* Saved register s2 (x18) */
	unsigned long s3;               /* Saved register s3 (x19) */
	unsigned long s4;               /* Saved register s4 (x20) */
	unsigned long s5;               /* Saved register s5 (x21) */
	unsigned long s6;               /* Saved register s6 (x22) */
	unsigned long s7;               /* Saved register s7 (x23) */
	unsigned long s8;               /* Saved register s8 (x24) */
	unsigned long s9;               /* Saved register s9 (x25) */
	unsigned long s10;              /* Saved register s10 (x26) */
	unsigned long s11;              /* Saved register s11 (x27) */
};

#define pcb_synch(thread)       /* FP state sync - implement if needed */

#define USER_REGS(thread)       (&(thread)->pcb->ss)

#define STACK_IKS(stack)                \
	(((struct riscv_kernel_state *)(((vm_offset_t)stack)+KERNEL_STACK_SIZE))-1)

#ifndef __ASSEMBLER__
/*
 * These routines are called from task_create() to
 * manage any pcb state common to all threads in a task
 */
#define pcb_common_init(task)
#define pcb_common_terminate(task)
#endif  /* !__ASSEMBLER__ */

#endif	/* _MACHDEP_RISCV_THREAD_H_ */
