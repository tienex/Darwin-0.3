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
 * RISC-V Process Control Block (PCB) and Thread Management
 */

#include <mach/mach_types.h>
#include <mach/riscv/thread_status.h>
#include <kern/mach_param.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <machdep/riscv/thread.h>

/* PCB zone for allocation */
zone_t pcb_zone;

/* Current stack pointers for each CPU */
vm_offset_t stack_pointers[NCPUS];
boolean_t empty_stacks[NCPUS];

/*
 * Initialize the PCB module
 */
void
pcb_module_init(void)
{
	/* Create zone for PCBs */
	pcb_zone = zinit(sizeof(struct pcb),
	                 1000 * sizeof(struct pcb),
	                 PAGE_SIZE,
	                 "pcb");

	/* Initialize per-CPU stack tracking */
	int i;
	for (i = 0; i < NCPUS; i++) {
		stack_pointers[i] = 0;
		empty_stacks[i] = TRUE;
	}
}

/*
 * Initialize a PCB for a new thread
 */
void
pcb_init(thread_t thread)
{
	struct pcb *pcb;

	if (thread->pcb)
		return;

	/* Allocate PCB */
	pcb = (struct pcb *)zalloc(pcb_zone);
	if (!pcb)
		return;

	/* Clear PCB */
	bzero(pcb, sizeof(struct pcb));

	/* Initialize saved state */
	bzero(&pcb->ss, sizeof(struct riscv_thread_state));
	bzero(&pcb->fs, sizeof(struct riscv_float_state));
	bzero(&pcb->es, sizeof(struct riscv_exception_state));

	/* Set initial stack pointer if kernel stack exists */
	if (thread->kernel_stack) {
		pcb->ksp = thread->kernel_stack + KERNEL_STACK_SIZE;
	}

	/* Clear flags */
	pcb->flags = 0;
	pcb->cthread_self = 0;

	thread->pcb = pcb;
}

/*
 * Terminate a PCB
 */
void
pcb_terminate(thread_t thread)
{
	struct pcb *pcb;

	pcb = thread->pcb;
	if (pcb) {
		zfree(pcb_zone, (vm_offset_t)pcb);
		thread->pcb = NULL;
	}
}

/*
 * Collect garbage PCBs
 */
void
pcb_collect(thread_t thread)
{
	/* Nothing to do on RISC-V */
}

/*
 * Attach a kernel stack to a thread
 */
void
stack_attach(
	thread_t		thread,
	vm_offset_t		stack,
	void			(*continuation)(void)
)
{
	struct riscv_kernel_state *kstate;

	thread->kernel_stack = stack;

	if (!thread->pcb)
		pcb_init(thread);

	/* Set up kernel stack state at top of stack */
	kstate = STACK_IKS(stack);

	/* Initialize kernel state structure */
	kstate->pcb = thread->pcb;
	kstate->sp = (unsigned long)stack + KERNEL_STACK_SIZE;

	if (continuation) {
		/* If we have a continuation, set up to call it */
		kstate->ra = (unsigned long)continuation;
	}

	/* Update PCB kernel stack pointer */
	thread->pcb->ksp = (unsigned long)kstate;
}

/*
 * Detach kernel stack from a thread
 */
vm_offset_t
stack_detach(thread_t thread)
{
	vm_offset_t stack;

	stack = thread->kernel_stack;
	thread->kernel_stack = 0;

	if (thread->pcb)
		thread->pcb->ksp = 0;

	return stack;
}

/*
 * Switch to a new thread's context
 *
 * This is the C wrapper - actual register save/restore is in locore.s
 */
void
switch_context(
	thread_t		old,
	void			(*continuation)(void),
	thread_t		new
)
{
	struct riscv_kernel_state *old_kstate, *new_kstate;

	/* Get kernel state structures */
	old_kstate = STACK_IKS(old->kernel_stack);
	new_kstate = STACK_IKS(new->kernel_stack);

	/* Handle continuation if present */
	if (continuation) {
		old_kstate->ra = (unsigned long)continuation;
	}

	/* Update current thread pointer */
	current_thread() = new;

	/* Call assembly routine to do actual context switch */
	/* extern void _switch_to(struct riscv_kernel_state *old,
	                          struct riscv_kernel_state *new); */
	_switch_to(old_kstate, new_kstate);

	/* Execution resumes here when this thread is switched back to */
}

/*
 * Set up user thread state for newly created thread
 */
void
thread_set_child(thread_t parent, thread_t child)
{
	struct riscv_thread_state *parent_state, *child_state;

	if (!child->pcb)
		pcb_init(child);

	parent_state = &parent->pcb->ss;
	child_state = &child->pcb->ss;

	/* Copy parent state to child */
	*child_state = *parent_state;

	/* Child's fork() returns 0 in a0 */
	child_state->a0 = 0;
}

/*
 * Set up user thread state for exec
 */
void
thread_setrun(thread_t thread, unsigned long entry, unsigned long stack)
{
	struct riscv_thread_state *state;

	if (!thread->pcb)
		pcb_init(thread);

	state = &thread->pcb->ss;

	/* Clear thread state */
	bzero(state, sizeof(struct riscv_thread_state));

	/* Set up entry point and stack */
	state->pc = entry;
	state->sp = stack;

	/* Clear other registers */
	/* a0-a7 are already zero (arguments) */
	/* RISC-V ABI: gp and tp should be set by runtime */
}

/*
 * Get thread state
 */
kern_return_t
thread_getstatus(
	thread_t				thread,
	int						flavor,
	thread_state_t			tstate,
	unsigned int			*count
)
{
	struct riscv_thread_state *state;
	struct riscv_float_state *fstate;

	if (!thread->pcb)
		return KERN_FAILURE;

	switch (flavor) {
	case RISCV_THREAD_STATE:
		if (*count < RISCV_THREAD_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		state = (struct riscv_thread_state *)tstate;
		*state = thread->pcb->ss;
		*count = RISCV_THREAD_STATE_COUNT;
		return KERN_SUCCESS;

	case RISCV_FLOAT_STATE:
		if (*count < RISCV_FLOAT_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		fstate = (struct riscv_float_state *)tstate;
		*fstate = thread->pcb->fs;
		*count = RISCV_FLOAT_STATE_COUNT;
		return KERN_SUCCESS;

	default:
		return KERN_INVALID_ARGUMENT;
	}
}

/*
 * Set thread state
 */
kern_return_t
thread_setstatus(
	thread_t				thread,
	int						flavor,
	thread_state_t			tstate,
	unsigned int			count
)
{
	struct riscv_thread_state *state;
	struct riscv_float_state *fstate;

	if (!thread->pcb)
		pcb_init(thread);

	switch (flavor) {
	case RISCV_THREAD_STATE:
		if (count < RISCV_THREAD_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		state = (struct riscv_thread_state *)tstate;
		thread->pcb->ss = *state;
		return KERN_SUCCESS;

	case RISCV_FLOAT_STATE:
		if (count < RISCV_FLOAT_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		fstate = (struct riscv_float_state *)tstate;
		thread->pcb->fs = *fstate;
		thread->pcb->flags |= PCB_FPVALID;
		return KERN_SUCCESS;

	default:
		return KERN_INVALID_ARGUMENT;
	}
}

/*
 * Duplicate parent state to child (for fork)
 */
void
thread_dup(thread_t parent, thread_t child)
{
	if (!child->pcb)
		pcb_init(child);

	if (parent->pcb) {
		/* Copy entire PCB state */
		child->pcb->ss = parent->pcb->ss;
		child->pcb->fs = parent->pcb->fs;
		child->pcb->es = parent->pcb->es;
		child->pcb->flags = parent->pcb->flags;

		/* Child returns 0 from fork */
		child->pcb->ss.a0 = 0;
	}
}

/*
 * Get user thread state pointer
 */
void *
thread_user_state(thread_t thread)
{
	if (!thread->pcb)
		return NULL;

	return &thread->pcb->ss;
}
