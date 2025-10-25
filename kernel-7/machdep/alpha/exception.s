/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Exception and Interrupt Handlers
 */

#include <architecture/alpha/asm_help.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

	.text
	.set	noreorder
	.set	noat

/*
 * Exception entry point
 *
 * This is called by PALcode when an exception occurs.
 * The PALcode has saved the state in a PAL frame on the kernel stack.
 */
NESTED(alpha_exception_entry, 0, ra)
	.prologue 0

	/*
	 * Save all registers to create a full saved state
	 */
	lda	sp, -256(sp)		/* Allocate space for saved state */

	/* Save integer registers */
	stq	v0, 0(sp)
	stq	t0, 8(sp)
	stq	t1, 16(sp)
	stq	t2, 24(sp)
	stq	t3, 32(sp)
	stq	t4, 40(sp)
	stq	t5, 48(sp)
	stq	t6, 56(sp)
	stq	t7, 64(sp)
	stq	s0, 72(sp)
	stq	s1, 80(sp)
	stq	s2, 88(sp)
	stq	s3, 96(sp)
	stq	s4, 104(sp)
	stq	s5, 112(sp)
	stq	fp, 120(sp)
	stq	a0, 128(sp)
	stq	a1, 136(sp)
	stq	a2, 144(sp)
	stq	a3, 152(sp)
	stq	a4, 160(sp)
	stq	a5, 168(sp)
	stq	t8, 176(sp)
	stq	t9, 184(sp)
	stq	t10, 192(sp)
	stq	t11, 200(sp)
	stq	ra, 208(sp)
	stq	t12, 216(sp)
	stq	AT, 224(sp)
	stq	gp, 232(sp)
	/* sp is saved implicitly */

	/*
	 * Initialize GP for kernel
	 */
	br	gp, 1f
1:	ldgp	gp, 0(gp)

	/*
	 * Call C exception handler
	 * Pass pointer to saved state as argument
	 */
	bis	sp, sp, a0
	jsr	ra, alpha_exception_handler
	ldgp	gp, 0(ra)

	/*
	 * Restore registers
	 */
	ldq	v0, 0(sp)
	ldq	t0, 8(sp)
	ldq	t1, 16(sp)
	ldq	t2, 24(sp)
	ldq	t3, 32(sp)
	ldq	t4, 40(sp)
	ldq	t5, 48(sp)
	ldq	t6, 56(sp)
	ldq	t7, 64(sp)
	ldq	s0, 72(sp)
	ldq	s1, 80(sp)
	ldq	s2, 88(sp)
	ldq	s3, 96(sp)
	ldq	s4, 104(sp)
	ldq	s5, 112(sp)
	ldq	fp, 120(sp)
	ldq	a0, 128(sp)
	ldq	a1, 136(sp)
	ldq	a2, 144(sp)
	ldq	a3, 152(sp)
	ldq	a4, 160(sp)
	ldq	a5, 168(sp)
	ldq	t8, 176(sp)
	ldq	t9, 184(sp)
	ldq	t10, 192(sp)
	ldq	t11, 200(sp)
	ldq	ra, 208(sp)
	ldq	t12, 216(sp)
	ldq	AT, 224(sp)
	ldq	gp, 232(sp)

	lda	sp, 256(sp)		/* Restore stack pointer */

	/*
	 * Return from exception using PALcode
	 */
	call_pal PAL_UNIX_rti

END(alpha_exception_entry)

/*
 * Interrupt entry point
 */
NESTED(alpha_interrupt_entry, 0, ra)
	.prologue 0

	/*
	 * Save all registers
	 */
	lda	sp, -256(sp)

	stq	v0, 0(sp)
	stq	t0, 8(sp)
	stq	t1, 16(sp)
	stq	t2, 24(sp)
	stq	t3, 32(sp)
	stq	t4, 40(sp)
	stq	t5, 48(sp)
	stq	t6, 56(sp)
	stq	t7, 64(sp)
	stq	s0, 72(sp)
	stq	s1, 80(sp)
	stq	s2, 88(sp)
	stq	s3, 96(sp)
	stq	s4, 104(sp)
	stq	s5, 112(sp)
	stq	fp, 120(sp)
	stq	a0, 128(sp)
	stq	a1, 136(sp)
	stq	a2, 144(sp)
	stq	a3, 152(sp)
	stq	a4, 160(sp)
	stq	a5, 168(sp)
	stq	t8, 176(sp)
	stq	t9, 184(sp)
	stq	t10, 192(sp)
	stq	t11, 200(sp)
	stq	ra, 208(sp)
	stq	t12, 216(sp)
	stq	AT, 224(sp)
	stq	gp, 232(sp)

	/*
	 * Initialize GP
	 */
	br	gp, 1f
1:	ldgp	gp, 0(gp)

	/*
	 * Call C interrupt handler
	 */
	bis	sp, sp, a0
	jsr	ra, alpha_interrupt_handler
	ldgp	gp, 0(ra)

	/*
	 * Restore registers
	 */
	ldq	v0, 0(sp)
	ldq	t0, 8(sp)
	ldq	t1, 16(sp)
	ldq	t2, 24(sp)
	ldq	t3, 32(sp)
	ldq	t4, 40(sp)
	ldq	t5, 48(sp)
	ldq	t6, 56(sp)
	ldq	t7, 64(sp)
	ldq	s0, 72(sp)
	ldq	s1, 80(sp)
	ldq	s2, 88(sp)
	ldq	s3, 96(sp)
	ldq	s4, 104(sp)
	ldq	s5, 112(sp)
	ldq	fp, 120(sp)
	ldq	a0, 128(sp)
	ldq	a1, 136(sp)
	ldq	a2, 144(sp)
	ldq	a3, 152(sp)
	ldq	a4, 160(sp)
	ldq	a5, 168(sp)
	ldq	t8, 176(sp)
	ldq	t9, 184(sp)
	ldq	t10, 192(sp)
	ldq	t11, 200(sp)
	ldq	ra, 208(sp)
	ldq	t12, 216(sp)
	ldq	AT, 224(sp)
	ldq	gp, 232(sp)

	lda	sp, 256(sp)

	/*
	 * Return from interrupt
	 */
	call_pal PAL_UNIX_rti

END(alpha_interrupt_entry)

/*
 * System call entry point
 */
NESTED(alpha_syscall_entry, 0, ra)
	.prologue 0

	/*
	 * Save registers needed for system call
	 */
	lda	sp, -256(sp)

	stq	v0, 0(sp)		/* Save syscall number */
	stq	t0, 8(sp)
	stq	t1, 16(sp)
	stq	t2, 24(sp)
	stq	t3, 32(sp)
	stq	t4, 40(sp)
	stq	t5, 48(sp)
	stq	t6, 56(sp)
	stq	t7, 64(sp)
	stq	s0, 72(sp)
	stq	s1, 80(sp)
	stq	s2, 88(sp)
	stq	s3, 96(sp)
	stq	s4, 104(sp)
	stq	s5, 112(sp)
	stq	fp, 120(sp)
	stq	a0, 128(sp)		/* Save syscall arguments */
	stq	a1, 136(sp)
	stq	a2, 144(sp)
	stq	a3, 152(sp)
	stq	a4, 160(sp)
	stq	a5, 168(sp)
	stq	t8, 176(sp)
	stq	t9, 184(sp)
	stq	t10, 192(sp)
	stq	t11, 200(sp)
	stq	ra, 208(sp)
	stq	t12, 216(sp)
	stq	AT, 224(sp)
	stq	gp, 232(sp)

	/*
	 * Initialize GP
	 */
	br	gp, 1f
1:	ldgp	gp, 0(gp)

	/*
	 * Call C system call handler
	 */
	bis	sp, sp, a0		/* Pass saved state pointer */
	jsr	ra, alpha_syscall_handler
	ldgp	gp, 0(ra)

	/*
	 * Restore registers (v0 contains return value)
	 */
	/* Don't restore v0 - it contains the return value */
	ldq	t0, 8(sp)
	ldq	t1, 16(sp)
	ldq	t2, 24(sp)
	ldq	t3, 32(sp)
	ldq	t4, 40(sp)
	ldq	t5, 48(sp)
	ldq	t6, 56(sp)
	ldq	t7, 64(sp)
	ldq	s0, 72(sp)
	ldq	s1, 80(sp)
	ldq	s2, 88(sp)
	ldq	s3, 96(sp)
	ldq	s4, 104(sp)
	ldq	s5, 112(sp)
	ldq	fp, 120(sp)
	ldq	a0, 128(sp)
	ldq	a1, 136(sp)
	ldq	a2, 144(sp)
	ldq	a3, 152(sp)
	ldq	a4, 160(sp)
	ldq	a5, 168(sp)
	ldq	t8, 176(sp)
	ldq	t9, 184(sp)
	ldq	t10, 192(sp)
	ldq	t11, 200(sp)
	ldq	ra, 208(sp)
	ldq	t12, 216(sp)
	ldq	AT, 224(sp)
	ldq	gp, 232(sp)

	lda	sp, 256(sp)

	/*
	 * Return from system call
	 */
	call_pal PAL_UNIX_retsys

END(alpha_syscall_entry)
