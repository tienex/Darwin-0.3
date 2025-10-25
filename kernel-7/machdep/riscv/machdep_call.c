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
 * RISC-V machine-dependent kernel calls
 */

#include <mach/mach_types.h>
#include <machdep/riscv/machdep_call.h>

/* Forward declarations */
extern kern_return_t kern_invalid(void);
extern kern_return_t thread_get_cthread_self(void);
extern kern_return_t thread_set_cthread_self(void);

/*
 * Machine-dependent call table
 */
machdep_call_t machdep_call_table[] = {
	/* 0: thread_get_cthread_self */
	{
		thread_get_cthread_self,
		0
	},
	/* 1: thread_set_cthread_self */
	{
		thread_set_cthread_self,
		1
	}
};

int machdep_call_count = sizeof(machdep_call_table) / sizeof(machdep_call_t);

/*
 * Invalid call handler
 */
kern_return_t
kern_invalid(void)
{
	return KERN_INVALID_ARGUMENT;
}

/*
 * Get cthread self pointer
 */
kern_return_t
thread_get_cthread_self(void)
{
	thread_t thread = current_thread();

	if (thread && thread->pcb) {
		return (kern_return_t)thread->pcb->cthread_self;
	}
	return KERN_FAILURE;
}

/*
 * Set cthread self pointer
 */
kern_return_t
thread_set_cthread_self(unsigned long value)
{
	thread_t thread = current_thread();

	if (thread && thread->pcb) {
		thread->pcb->cthread_self = value;
		return KERN_SUCCESS;
	}
	return KERN_FAILURE;
}
