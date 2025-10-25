/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Context Switching
 *
 * This file implements thread context switching for MMIX.
 *
 * Context switching involves:
 *   1. Saving current thread's register state
 *   2. Saving current thread's special registers (rJ, rL, rG, etc.)
 *   3. Switching stack pointer to new thread
 *   4. Restoring new thread's special registers
 *   5. Restoring new thread's register state
 *   6. Returning in new thread's context
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <machdep/mmix/asm.h>
#include <machdep/mmix/thread.h>

	.file	"cswtch.s"

	.text
	.align	4

/*
 * switch_context
 *
 * Switch from old thread to new thread
 *
 * Arguments:
 *   $0 = old thread's kernel stack pointer (mmix_kernel_state **)
 *   $1 = new thread's kernel stack pointer (mmix_kernel_state *)
 *   $2 = new pmap (address space)
 *
 * Returns in context of new thread
 */

	.globl	switch_context
	.globl	_switch_context
switch_context:
_switch_context:

	/*
	 * Save old thread's context
	 *
	 * We need to save:
	 *   - Callee-saved registers ($16-$23)
	 *   - Frame pointer ($253)
	 *   - Stack pointer ($254)
	 *   - Return address (rJ)
	 *   - Local register threshold (rL)
	 *
	 * Layout matches mmix_kernel_state in thread.h
	 */

	/* Make space on stack for kernel state */
	SUBU	$254,$254,128		/* sizeof(mmix_kernel_state) */

	/* Save callee-saved registers */
	STOU	$16,$254,0
	STOU	$17,$254,8
	STOU	$18,$254,16
	STOU	$19,$254,24
	STOU	$20,$254,32
	STOU	$21,$254,40
	STOU	$22,$254,48
	STOU	$23,$254,56

	/* Save frame pointer and stack pointer */
	STOU	$253,$254,64		/* FP ($253) */
	ADDU	$3,$254,128		/* Original SP before SUBU */
	STOU	$3,$254,72		/* SP ($254) */

	/* Save return address from rJ */
	GET	$3,rJ
	STOU	$3,$254,80		/* rJ */

	/* Save local register threshold */
	GET	$3,rL
	STOU	$3,$254,88		/* rL */

	/* Save current SP to *old */
	STOU	$254,$0,0

	/*
	 * Switch to new thread's context
	 */

	/* Load new SP */
	SET	$254,$1

	/* Switch address space if needed (set page table) */
	BZ	$2,skip_pmap_switch	/* If pmap == 0, don't switch */

	/* Load new page table base from pmap */
	LDOU	$3,$2,0			/* Assume first field is pt base */
	/* TODO: Actually load into MMU registers */
	/* For now, this is a placeholder */

skip_pmap_switch:

	/*
	 * Restore new thread's context
	 */

	/* Restore callee-saved registers */
	LDOU	$16,$254,0
	LDOU	$17,$254,8
	LDOU	$18,$254,16
	LDOU	$19,$254,24
	LDOU	$20,$254,32
	LDOU	$21,$254,40
	LDOU	$22,$254,48
	LDOU	$23,$254,56

	/* Restore frame pointer */
	LDOU	$253,$254,64		/* FP ($253) */

	/* Restore return address to rJ */
	LDOU	$3,$254,80		/* rJ */
	PUT	rJ,$3

	/* Restore local register threshold */
	LDOU	$3,$254,88		/* rL */
	PUT	rL,$3

	/* Restore stack pointer */
	LDOU	$3,$254,72		/* Original SP */
	SET	$254,$3

	/* Return in new thread's context */
	POP	0,0

/*
 * load_context
 *
 * Load a thread's complete context (used for thread startup)
 *
 * Arguments:
 *   $0 = thread's saved state pointer (mmix_saved_state *)
 *
 * Does not return - resumes in new thread
 */

	.globl	load_context
	.globl	_load_context
load_context:
_load_context:

	SET	$254,$0		/* Point to saved state */

	/* Restore general registers $1-$23 */
	LDOU	$1,$254,8
	LDOU	$2,$254,16
	LDOU	$3,$254,24
	LDOU	$4,$254,32
	LDOU	$5,$254,40
	LDOU	$6,$254,48
	LDOU	$7,$254,56
	LDOU	$8,$254,64
	LDOU	$9,$254,72
	LDOU	$10,$254,80
	LDOU	$11,$254,88
	LDOU	$12,$254,96
	LDOU	$13,$254,104
	LDOU	$14,$254,112
	LDOU	$15,$254,120
	LDOU	$16,$254,128
	LDOU	$17,$254,136
	LDOU	$18,$254,144
	LDOU	$19,$254,152
	LDOU	$20,$254,160
	LDOU	$21,$254,168
	LDOU	$22,$254,176
	LDOU	$23,$254,184

	/* Restore special registers */
	LDOU	$31,$254,240		/* rJ */
	PUT	rJ,$31
	LDOU	$31,$254,248		/* rL */
	PUT	rL,$31
	LDOU	$31,$254,256		/* rG */
	PUT	rG,$31

	/* Restore return address in rW (for RESUME) */
	LDOU	$31,$254,208		/* rW */
	PUT	rW,$31

	/* Restore FP and SP */
	LDOU	$253,$254,200		/* FP */
	LDOU	$31,$254,192		/* SP (save to temp) */

	/* Restore $0 */
	LDOU	$0,$254,0

	/* Restore SP last */
	SET	$254,$31

	/* Resume execution at saved rW */
	RESUME	1

/*
 * call_continuation
 *
 * Call a continuation function in a thread context
 *
 * Arguments:
 *   $0 = continuation function pointer
 *   $1 = continuation parameter
 *   $2 = new stack pointer
 *
 * This sets up a new stack and calls the continuation
 */

	.globl	call_continuation
	.globl	_call_continuation
call_continuation:
_call_continuation:

	/* Save continuation info */
	SET	$3,$0		/* Function pointer */
	SET	$4,$1		/* Parameter */

	/* Switch to new stack */
	SET	$254,$2

	/* Set up frame pointer */
	SET	$253,$254

	/* Clear rJ (no return address) */
	SET	$5,0
	PUT	rJ,$5

	/* Call continuation(parameter) */
	SET	$0,$4		/* Pass parameter */
	PUSHJ	$255,$3		/* Call function */

	/*
	 * Continuation should never return
	 * If it does, panic
	 */
	TRAP	0,0,0		/* Force trap */

/*
 * thread_bootstrap_return
 *
 * Special return path for newly created threads
 * Called from thread_bootstrap()
 */

	.globl	thread_bootstrap_return
thread_bootstrap_return:

	/* Load saved state pointer from stack */
	POP	1,0		/* Get saved state from stack */

	/* Jump to load_context to restore thread state */
	JMP	load_context

/*
 * Switch to interrupt stack
 *
 * Used when handling interrupts to switch from thread stack
 * to dedicated interrupt stack
 *
 * Arguments:
 *   $0 = interrupt handler function
 *   $1 = interrupt handler argument
 *
 * Returns after handler completes
 */

	.globl	switch_to_interrupt_stack
switch_to_interrupt_stack:

	/* Save return address */
	GET	$2,rJ
	PUSH	0,$2

	/* Save current stack pointer */
	PUSH	0,$254
	PUSH	0,$253

	/* Switch to interrupt stack */
	GETA	$254,intstack_top
	SET	$253,$254

	/* Call handler(argument) */
	SET	$2,$0		/* Save function pointer */
	SET	$0,$1		/* Pass argument */
	PUSHJ	$255,$2		/* Call handler */

	/* Restore original stack */
	POP	1,$253
	POP	1,$254

	/* Restore return address and return */
	POP	1,$2
	PUT	rJ,$2
	POP	0,0

/*
 * Interrupt stack
 */
	.data
	.align	MMIX_PGSHIFT

	.globl	intstack
intstack:
	.space	16384		/* 16KB */

	.globl	intstack_top
intstack_top:
