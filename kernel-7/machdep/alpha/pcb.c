/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Process Control Block (PCB) Management
 */

#include <mach/mach_types.h>
#include <kern/thread.h>
#include <architecture/alpha/reg.h>

/*
 * Thread PCB operations
 */

void
pcb_init(thread_t thread)
{
	/* Initialize thread's PCB */
}

void
pcb_terminate(thread_t thread)
{
	/* Clean up thread's PCB */
}

void
pcb_user_to_kernel(thread_t thread)
{
	/* Transition from user to kernel mode */
}

void
pcb_kernel_to_user(thread_t thread)
{
	/* Transition from kernel to user mode */
}
