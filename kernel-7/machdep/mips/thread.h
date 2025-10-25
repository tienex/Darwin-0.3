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
 * MIPS:	Machine dependent thread state.
 *
 * Defines the saved state format for MIPS threads.
 */

#ifndef	_MACHDEP_MIPS_THREAD_H_
#define _MACHDEP_MIPS_THREAD_H_

#import <kern/kernel_stack.h>
#import <machdep/mips/regs.h>
#import <machdep/mips/pcb.h>

/*
 * Thread state is saved in this format whenever an
 * interrupt/trap/system call occurs.
 */
typedef struct thread_saved_state {
	regs_t		regs;		/* All GP registers + CP0 state */
	unsigned int	trapno;		/* Exception code */
	unsigned int	pad;		/* Alignment padding */
} thread_saved_state_t;

/*
 * A thread uses a private save area as its kernel stack while
 * executing in user mode. After the user state is saved, it
 * switches to the active kernel stack.
 *
 * For MIPS, we need space for:
 * - User saved state
 * - Kernel saved state (if interrupted while in kernel)
 * - Small buffer for interrupt frame
 */
typedef struct thread_save_area {
	thread_saved_state_t	sa_k;	/* Kernel state */
	thread_saved_state_t	sa_u;	/* User state */
} thread_save_area_t;

/*
 * Macro to get user register state from thread
 */
#define USER_REGS(thread)						\
	((thread)->pcb->save_area ?					\
		(&(thread)->pcb->save_area->sa_u) :			\
		(void *)(thread_user_state(thread)))

/*
 * Kernel stack structure - used for exception entry
 */
typedef struct kernel_stack_frame {
	unsigned long	arg[4];		/* a0-a3 argument space */
	unsigned long	saved_regs[8];	/* s0-s7 callee-saved */
	unsigned long	saved_fp;	/* fp/s8 */
	unsigned long	saved_ra;	/* Return address */
	unsigned long	saved_gp;	/* Global pointer */
	unsigned long	padding;	/* 16-byte alignment */
} kernel_stack_frame_t;

#endif	/* _MACHDEP_MIPS_THREAD_H_ */
