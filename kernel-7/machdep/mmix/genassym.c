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
 *	File:	mmix/genassym.c
 *
 *	Generate assembly language constants for MMIX structures.
 *	Used by assembly code to access C structure fields.
 */

#include <mach/mmix/thread_status.h>
#include <mmix/thread.h>
#include <kern/thread.h>

/*
 * This file is processed to produce assembly constants.
 * The genassym.awk script extracts the values.
 */

/* Thread offsets */
int	thread_pcb = offsetof(struct thread, pcb);
int	thread_kernel_stack = offsetof(struct thread, kernel_stack);
int	thread_recover = offsetof(struct thread, recover);

/* PCB offsets */
int	pcb_ss = offsetof(struct pcb, ss);
int	pcb_ksp = offsetof(struct pcb, ksp);
int	pcb_flags = offsetof(struct pcb, flags);

/* Saved state offsets */
int	ss_pc = offsetof(struct mmix_thread_state, pc);
int	ss_r254 = offsetof(struct mmix_thread_state, r) + (254-0)*sizeof(unsigned long long);
int	ss_rJ = offsetof(struct mmix_thread_state, rJ);

/* Sizes */
int	pcb_size = sizeof(struct pcb);
int	thread_size = sizeof(struct thread);
int	mmix_saved_state_size = sizeof(struct mmix_thread_state);
