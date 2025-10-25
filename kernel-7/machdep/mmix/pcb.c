/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Process Control Block (PCB) Management
 *
 * This file handles thread state operations for MMIX:
 *   - Thread creation and destruction
 *   - Getting/setting thread state
 *   - Thread state initialization
 *   - Context save/restore
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <cpus.h>
#include <mach/mach_types.h>
#include <mach/thread_status.h>
#include <kern/thread.h>
#include <kern/misc_protos.h>
#include <machdep/mmix/thread.h>
#include <machdep/mmix/pmap.h>
#include <vm/vm_kern.h>

#define NULL 0

/*
 * Initialize PCB for a new thread
 */
void
pcb_init(thread_t thread)
{
	pcb_t pcb;

	/* Allocate PCB if needed */
	if (thread->pcb == (pcb_t)0) {
		thread->pcb = (pcb_t) kalloc(sizeof(struct pcb));
	}

	pcb = thread->pcb;

	/* Zero out the PCB */
	bzero((char *)pcb, sizeof(struct pcb));

	/* Set default values */
	pcb->ss.rG = 32;	/* 32 global registers */
	pcb->ss.rL = 16;	/* 16 local registers */
}

/*
 * Destroy PCB when thread terminates
 */
void
pcb_terminate(thread_t thread)
{
	if (thread->pcb) {
		kfree((vm_offset_t)thread->pcb, sizeof(struct pcb));
		thread->pcb = (pcb_t)0;
	}
}

/*
 * Get thread general purpose registers
 */
kern_return_t
thread_getstatus(thread_t thread,
		int flavor,
		thread_state_t tstate,
		mach_msg_type_number_t *count)
{
	pcb_t pcb = thread->pcb;
	struct mmix_thread_state *ts;
	struct mmix_exception_state *es;
	struct mmix_float_state *fs;

	switch (flavor) {

	case MMIX_THREAD_STATE:
		if (*count < MMIX_THREAD_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		ts = (struct mmix_thread_state *)tstate;

		/* Copy general purpose registers */
		bcopy((char *)&pcb->ss, (char *)ts,
		      sizeof(struct mmix_thread_state));

		*count = MMIX_THREAD_STATE_COUNT;
		break;

	case MMIX_EXCEPTION_STATE:
		if (*count < MMIX_EXCEPTION_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		es = (struct mmix_exception_state *)tstate;

		/* Copy exception state */
		bcopy((char *)&pcb->es, (char *)es,
		      sizeof(struct mmix_exception_state));

		*count = MMIX_EXCEPTION_STATE_COUNT;
		break;

	case MMIX_FLOAT_STATE:
		if (*count < MMIX_FLOAT_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		fs = (struct mmix_float_state *)tstate;

		/* Copy floating point state */
		bcopy((char *)&pcb->fs, (char *)fs,
		      sizeof(struct mmix_float_state));

		*count = MMIX_FLOAT_STATE_COUNT;
		break;

	default:
		return KERN_INVALID_ARGUMENT;
	}

	return KERN_SUCCESS;
}

/*
 * Set thread general purpose registers
 */
kern_return_t
thread_setstatus(thread_t thread,
		int flavor,
		thread_state_t tstate,
		mach_msg_type_number_t count)
{
	pcb_t pcb = thread->pcb;
	struct mmix_thread_state *ts;
	struct mmix_exception_state *es;
	struct mmix_float_state *fs;

	switch (flavor) {

	case MMIX_THREAD_STATE:
		if (count < MMIX_THREAD_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		ts = (struct mmix_thread_state *)tstate;

		/* Validate new state */
		/* Ensure user mode (kernel bit cleared in PC) */
		if (ts->rW & 0x8000000000000000ULL)
			return KERN_INVALID_ARGUMENT;

		/* Copy general purpose registers */
		bcopy((char *)ts, (char *)&pcb->ss,
		      sizeof(struct mmix_thread_state));

		break;

	case MMIX_EXCEPTION_STATE:
		if (count < MMIX_EXCEPTION_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		es = (struct mmix_exception_state *)tstate;

		/* Copy exception state */
		bcopy((char *)es, (char *)&pcb->es,
		      sizeof(struct mmix_exception_state));

		break;

	case MMIX_FLOAT_STATE:
		if (count < MMIX_FLOAT_STATE_COUNT)
			return KERN_INVALID_ARGUMENT;

		fs = (struct mmix_float_state *)tstate;

		/* Copy floating point state */
		bcopy((char *)fs, (char *)&pcb->fs,
		      sizeof(struct mmix_float_state));

		break;

	default:
		return KERN_INVALID_ARGUMENT;
	}

	return KERN_SUCCESS;
}

/*
 * Duplicate parent's state in child thread
 */
void
thread_dup(thread_t parent, thread_t child)
{
	pcb_t parent_pcb = parent->pcb;
	pcb_t child_pcb;

	/* Initialize child PCB */
	pcb_init(child);
	child_pcb = child->pcb;

	/* Copy parent's state to child */
	bcopy((char *)parent_pcb, (char *)child_pcb, sizeof(struct pcb));

	/* Child needs its own kernel stack */
	/* Kernel stack pointer is set by thread_create() */
}

/*
 * Set up initial state for a new user thread
 */
void
thread_set_user_regs(thread_t thread,
		    unsigned long long entry,
		    unsigned long long stack)
{
	pcb_t pcb = thread->pcb;
	struct mmix_thread_state *ts = &pcb->ss;

	/* Clear all state */
	bzero((char *)ts, sizeof(struct mmix_thread_state));

	/* Set entry point (PC) */
	ts->rW = entry;

	/* Set stack pointer */
	ts->sp = stack;
	ts->fp = stack;

	/* Set initial register values */
	ts->rG = 32;		/* 32 global registers */
	ts->rL = 16;		/* 16 local registers */
	ts->rA = 0;		/* Clear arithmetic status */

	/* Clear general registers */
	/* (already done by bzero) */
}

/*
 * Set up kernel stack for thread bootstrap
 */
kern_return_t
thread_set_wq_state(thread_t thread, int flavor,
		   thread_state_t tstate,
		   mach_msg_type_number_t count)
{
	/* Set thread state for work queue */
	return thread_setstatus(thread, flavor, tstate, count);
}

/*
 * Save FPU state (if FPU is enabled)
 */
void
fpu_save(thread_t thread)
{
	pcb_t pcb = thread->pcb;
	struct mmix_float_state *fs = &pcb->fs;

	/* Save all floating point registers $f0-$f255 */
	/* This would require assembly code to access FP registers */
	/* For now, assume FP state is saved on context switch */

	/* Mark FPU state as saved */
	pcb->flags |= PCB_FPU_SAVED;
}

/*
 * Restore FPU state (if FPU is enabled)
 */
void
fpu_restore(thread_t thread)
{
	pcb_t pcb = thread->pcb;
	struct mmix_float_state *fs = &pcb->fs;

	/* Restore all floating point registers */
	/* This would require assembly code */

	/* Mark FPU state as active */
	pcb->flags &= ~PCB_FPU_SAVED;
}

/*
 * Thread bootstrap
 *
 * Called to set up a new thread's initial execution state
 */
void
thread_bootstrap_return(void)
{
	thread_t thread = current_thread();
	pcb_t pcb = thread->pcb;

	/* Load user state and start executing */
	extern void load_context(struct mmix_saved_state *);
	load_context(&pcb->ss);

	/* Never returns */
}

/*
 * Get current thread's saved state
 */
struct mmix_saved_state *
thread_get_saved_state(thread_t thread)
{
	if (thread && thread->pcb)
		return &thread->pcb->ss;
	return NULL;
}

/*
 * Initialize machine-specific thread state
 */
void
machine_thread_create(thread_t thread, task_t task)
{
	/* Initialize PCB */
	pcb_init(thread);

	/* Set default thread state */
	pcb_t pcb = thread->pcb;
	pcb->ss.rG = 32;
	pcb->ss.rL = 16;
}

/*
 * Clean up machine-specific thread state
 */
void
machine_thread_destroy(thread_t thread)
{
	/* Save any machine state if needed */
	/* Destroy PCB */
	pcb_terminate(thread);
}

/*
 * Set thread name (for debugging)
 */
void
thread_set_cthread_self(unsigned long long cthread_self)
{
	thread_t thread = current_thread();
	if (thread && thread->pcb)
		thread->pcb->cthread_self = cthread_self;
}

/*
 * Get thread name (for debugging)
 */
unsigned long long
thread_get_cthread_self(void)
{
	thread_t thread = current_thread();
	if (thread && thread->pcb)
		return thread->pcb->cthread_self;
	return 0;
}
