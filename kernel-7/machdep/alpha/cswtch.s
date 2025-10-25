/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Context Switching
 *
 * Context switching between threads. Saves the current thread's
 * state and restores the new thread's state.
 */

#include <architecture/alpha/asm_help.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

	.text
	.set noreorder
	.set noat

/*
 * switch_context(old_thread, new_thread)
 *
 * Switch from old_thread (a0) to new_thread (a1)
 *
 * Saves callee-saved registers for old thread and restores them
 * for new thread. The thread structure contains a saved context
 * pointer (pcb) which points to the saved register area.
 *
 * Note: This function is called with interrupts disabled.
 */
NESTED(switch_context, 0, ra)
	.prologue 0

	/*
	 * Get PCB pointers from thread structures
	 * Assume PCB is at offset 0 in thread structure for now
	 * (will need to match actual thread_t layout)
	 */
	ldq	t0, 0(a0)		/* t0 = old_thread->pcb */
	ldq	t1, 0(a1)		/* t1 = new_thread->pcb */

	/*
	 * Save callee-saved registers for old thread
	 * We only need to save registers that must be preserved across
	 * function calls (s0-s5, fp, ra, gp, sp)
	 */
	stq	s0, 0(t0)		/* Save s0 */
	stq	s1, 8(t0)		/* Save s1 */
	stq	s2, 16(t0)		/* Save s2 */
	stq	s3, 24(t0)		/* Save s3 */
	stq	s4, 32(t0)		/* Save s4 */
	stq	s5, 40(t0)		/* Save s5 */
	stq	fp, 48(t0)		/* Save frame pointer */
	stq	ra, 56(t0)		/* Save return address */
	stq	gp, 64(t0)		/* Save global pointer */
	stq	sp, 72(t0)		/* Save stack pointer */

	/*
	 * Save current process context (PTBR - page table base register)
	 * This is only needed if switching between different address spaces
	 */
	call_pal PAL_UNIX_rdunique	/* Read unique value (can be used for thread-local storage) */
	stq	v0, 80(t0)		/* Save it */

	/*
	 * Load callee-saved registers for new thread
	 */
	ldq	s0, 0(t1)		/* Restore s0 */
	ldq	s1, 8(t1)		/* Restore s1 */
	ldq	s2, 16(t1)		/* Restore s2 */
	ldq	s3, 24(t1)		/* Restore s3 */
	ldq	s4, 32(t1)		/* Restore s4 */
	ldq	s5, 40(t1)		/* Restore s5 */
	ldq	fp, 48(t1)		/* Restore frame pointer */
	ldq	ra, 56(t1)		/* Restore return address */
	ldq	gp, 64(t1)		/* Restore global pointer */
	ldq	sp, 72(t1)		/* Restore stack pointer */

	/*
	 * Restore thread-local storage
	 */
	ldq	a0, 80(t1)		/* Load unique value */
	call_pal PAL_UNIX_wrunique	/* Write unique value */

	/*
	 * If switching address spaces, need to call swpctx PALcode
	 * For now, assume we're in the same address space (kernel threads)
	 */

	/*
	 * Memory barrier to ensure all stores complete
	 */
	mb

	/*
	 * Return to new thread
	 * The return address (ra) now points to where the new thread
	 * was when it was last context switched out
	 */
	ret	zero, (ra), 1

END(switch_context)

/*
 * load_context(thread)
 *
 * Load a thread's context without saving current context.
 * Used for initial thread startup.
 *
 * a0 = thread pointer
 */
NESTED(load_context, 0, ra)
	.prologue 0

	/* Get PCB pointer */
	ldq	t0, 0(a0)		/* t0 = thread->pcb */

	/* Load all saved registers */
	ldq	s0, 0(t0)
	ldq	s1, 8(t0)
	ldq	s2, 16(t0)
	ldq	s3, 24(t0)
	ldq	s4, 32(t0)
	ldq	s5, 40(t0)
	ldq	fp, 48(t0)
	ldq	ra, 56(t0)
	ldq	gp, 64(t0)
	ldq	sp, 72(t0)

	/* Load thread-local storage */
	ldq	a0, 80(t0)
	call_pal PAL_UNIX_wrunique

	/* Memory barrier */
	mb

	/* Jump to thread */
	ret	zero, (ra), 1

END(load_context)

/*
 * save_context(thread)
 *
 * Save current context to thread without switching.
 * Used for thread creation.
 *
 * a0 = thread pointer
 */
NESTED(save_context, 0, ra)
	.prologue 0

	/* Get PCB pointer */
	ldq	t0, 0(a0)		/* t0 = thread->pcb */

	/* Save all callee-saved registers */
	stq	s0, 0(t0)
	stq	s1, 8(t0)
	stq	s2, 16(t0)
	stq	s3, 24(t0)
	stq	s4, 32(t0)
	stq	s5, 40(t0)
	stq	fp, 48(t0)
	stq	ra, 56(t0)
	stq	gp, 64(t0)
	stq	sp, 72(t0)

	/* Save thread-local storage */
	call_pal PAL_UNIX_rdunique
	stq	v0, 80(t0)

	/* Memory barrier */
	mb

	/* Return */
	ret	zero, (ra), 1

END(save_context)
