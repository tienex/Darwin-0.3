/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Process Control Block (PCB) Management
 *
 * The PCB contains the saved state for a thread, including all
 * callee-saved registers and FP state.
 */

#include <mach/mach_types.h>
#include <mach/alpha/thread_status.h>
#include <kern/thread.h>
#include <kern/kalloc.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>
#include <string.h>

/*
 * Alpha PCB structure
 * This must match the layout expected by cswtch.s
 */
struct alpha_pcb {
	/* Callee-saved registers (88 bytes) */
	unsigned long s0;		/* 0: saved register 0 */
	unsigned long s1;		/* 8: saved register 1 */
	unsigned long s2;		/* 16: saved register 2 */
	unsigned long s3;		/* 24: saved register 3 */
	unsigned long s4;		/* 32: saved register 4 */
	unsigned long s5;		/* 40: saved register 5 */
	unsigned long fp;		/* 48: frame pointer */
	unsigned long ra;		/* 56: return address */
	unsigned long gp;		/* 64: global pointer */
	unsigned long sp;		/* 72: stack pointer */
	unsigned long unique;		/* 80: thread-local storage */

	/* Floating-point state */
	unsigned long fpcr;		/* FP control register */
	unsigned long fp_regs[32];	/* FP registers f0-f31 */

	/* User state (for user-mode threads) */
	alpha_saved_state_t *user_state;

	/* Flags */
	unsigned int fp_used;		/* FP has been used */
};

/*
 * Initialize thread's PCB
 */
void
pcb_init(thread_t thread)
{
	struct alpha_pcb *pcb;

	/* Allocate PCB */
	pcb = (struct alpha_pcb *)kalloc(sizeof(struct alpha_pcb));
	if (pcb == NULL)
		panic("pcb_init: kalloc failed");

	/* Zero the PCB */
	bzero((char *)pcb, sizeof(struct alpha_pcb));

	/* Store PCB pointer in thread structure */
	/* Assuming thread->pcb is at offset 0 for now */
	*(struct alpha_pcb **)thread = pcb;
}

/*
 * Terminate thread's PCB
 */
void
pcb_terminate(thread_t thread)
{
	struct alpha_pcb *pcb;

	/* Get PCB pointer */
	pcb = *(struct alpha_pcb **)thread;

	if (pcb != NULL) {
		/* Free user state if allocated */
		if (pcb->user_state != NULL) {
			kfree((vm_offset_t)pcb->user_state,
			      sizeof(alpha_saved_state_t));
		}

		/* Free PCB */
		kfree((vm_offset_t)pcb, sizeof(struct alpha_pcb));

		/* Clear pointer */
		*(struct alpha_pcb **)thread = NULL;
	}
}

/*
 * Transition from user to kernel mode
 *
 * Called when a user thread enters the kernel (system call, trap, etc.)
 */
void
pcb_user_to_kernel(thread_t thread)
{
	/* Nothing special needed on Alpha */
	/* The exception handler already saved user state */
}

/*
 * Transition from kernel to user mode
 *
 * Called when returning to user mode
 */
void
pcb_kernel_to_user(thread_t thread)
{
	struct alpha_pcb *pcb;

	/* Get PCB */
	pcb = *(struct alpha_pcb **)thread;

	/* Enable FP if thread has used it */
	if (pcb->fp_used) {
		extern void pal_unix_wrfen(unsigned long);
		pal_unix_wrfen(1);
	}
}

/*
 * Save user state in PCB
 */
void
pcb_save_user_state(thread_t thread, alpha_saved_state_t *state)
{
	struct alpha_pcb *pcb;

	pcb = *(struct alpha_pcb **)thread;

	/* Allocate user state if needed */
	if (pcb->user_state == NULL) {
		pcb->user_state = (alpha_saved_state_t *)
		    kalloc(sizeof(alpha_saved_state_t));
	}

	/* Copy state */
	if (pcb->user_state != NULL) {
		*pcb->user_state = *state;
	}
}

/*
 * Restore user state from PCB
 */
void
pcb_restore_user_state(thread_t thread, alpha_saved_state_t *state)
{
	struct alpha_pcb *pcb;

	pcb = *(struct alpha_pcb **)thread;

	/* Copy state */
	if (pcb->user_state != NULL) {
		*state = *pcb->user_state;
	}
}

/*
 * Initialize PCB for new thread
 * Set up initial context to start at given function
 */
void
pcb_init_context(thread_t thread, void (*start_func)(void), void *stack_top)
{
	struct alpha_pcb *pcb;
	extern unsigned long kernel_gp;	/* Kernel global pointer */

	pcb = *(struct alpha_pcb **)thread;

	/* Set up initial context */
	pcb->sp = (unsigned long)stack_top - 16;  /* Leave red zone */
	pcb->ra = (unsigned long)start_func;      /* Return to start function */
	pcb->gp = kernel_gp;                      /* Kernel GP */
	pcb->fp = 0;                              /* No frame yet */

	/* Clear saved registers */
	pcb->s0 = 0;
	pcb->s1 = 0;
	pcb->s2 = 0;
	pcb->s3 = 0;
	pcb->s4 = 0;
	pcb->s5 = 0;
}

/*
 * Save FP state
 */
void
pcb_save_fp(thread_t thread)
{
	struct alpha_pcb *pcb;
	int i;

	pcb = *(struct alpha_pcb **)thread;

	/* Mark FP as used */
	pcb->fp_used = 1;

	/* Save FP control register */
	__asm__ volatile ("mf_fpcr %0" : "=f" (pcb->fpcr));

	/* Save FP registers */
	/* This would normally use assembly to save all FP registers */
	/* For now, just mark that we need to do this */
}

/*
 * Restore FP state
 */
void
pcb_restore_fp(thread_t thread)
{
	struct alpha_pcb *pcb;

	pcb = *(struct alpha_pcb **)thread;

	if (!pcb->fp_used)
		return;

	/* Restore FP control register */
	__asm__ volatile ("mt_fpcr %0" : : "f" (pcb->fpcr));

	/* Restore FP registers */
	/* This would normally use assembly to restore all FP registers */
}

/*
 * Get PCB from thread
 */
struct alpha_pcb *
thread_pcb(thread_t thread)
{
	return *(struct alpha_pcb **)thread;
}
