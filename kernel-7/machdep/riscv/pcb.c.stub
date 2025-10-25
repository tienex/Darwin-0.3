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
 * RISC-V Family: Machine dependent thread module (PCB).
 */

#include <mach/mach_types.h>
#include <kern/mach_param.h>
#include <sys/param.h>
#include <sys/proc.h>

zone_t		pcb_zone;

vm_offset_t	stack_pointers[NCPUS];
boolean_t	empty_stacks[NCPUS];

/*
 * Attach a kernel stack to a thread
 */
void
stack_attach(
    thread_t		thread,
    vm_offset_t		stack,
    void		(*continuation)(void)
)
{
	thread->kernel_stack = stack;
	/* Set up stack pointer and continuation */
	/* This would set up RISC-V sp and ra registers */
}

/*
 * Detach kernel stack from thread
 */
vm_offset_t
stack_detach(
    thread_t		thread
)
{
	vm_offset_t stack = thread->kernel_stack;
	thread->kernel_stack = 0;
	return stack;
}

/*
 * Switch to a new thread
 */
void
switch_context(
    thread_t		old,
    void		(*continuation)(void),
    thread_t		new
)
{
	/* Save old thread state */
	/* Load new thread state */
	/* This would save/restore RISC-V registers */
}

/*
 * Initialize PCB zone
 */
void
pcb_module_init(void)
{
	/* Initialize the PCB zone */
}

/*
 * Initialize a PCB
 */
void
pcb_init(thread_t thread)
{
	/* Initialize PCB for new thread */
}

/*
 * Terminate a PCB
 */
void
pcb_terminate(thread_t thread)
{
	/* Clean up PCB */
}
