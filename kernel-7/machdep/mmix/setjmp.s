/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX setjmp/longjmp Implementation
 *
 * Non-local goto support for kernel error handling and unwinding
 *
 * jmp_buf layout (24 octabytes = 192 bytes):
 *   [0]  = rJ (return address)
 *   [1]  = $254 (stack pointer)
 *   [2]  = $253 (frame pointer)
 *   [3-10] = $16-$23 (callee-saved registers)
 *   [11] = rL (local threshold)
 *   [12] = rG (global threshold)
 *   [13] = rA (arithmetic status)
 *   [14-23] = reserved
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <machdep/mmix/asm.h>

	.file	"setjmp.s"

	.text
	.align	4

/*
 * int setjmp(jmp_buf env)
 *
 * Save current execution context
 *
 * Arguments:
 *   $0 = pointer to jmp_buf (24 octabytes)
 *
 * Returns:
 *   0 when called directly
 *   non-zero when returning from longjmp
 */

	.globl	setjmp
	.globl	_setjmp
setjmp:
_setjmp:

	/* Save return address from rJ */
	GET	$1,rJ
	STOU	$1,$0,0		/* env[0] = rJ */

	/* Save stack pointer and frame pointer */
	STOU	$254,$0,8	/* env[1] = SP */
	STOU	$253,$0,16	/* env[2] = FP */

	/* Save callee-saved registers $16-$23 */
	STOU	$16,$0,24	/* env[3] = $16 */
	STOU	$17,$0,32	/* env[4] = $17 */
	STOU	$18,$0,40	/* env[5] = $18 */
	STOU	$19,$0,48	/* env[6] = $19 */
	STOU	$20,$0,56	/* env[7] = $20 */
	STOU	$21,$0,64	/* env[8] = $21 */
	STOU	$22,$0,72	/* env[9] = $22 */
	STOU	$23,$0,80	/* env[10] = $23 */

	/* Save special registers */
	GET	$1,rL
	STOU	$1,$0,88	/* env[11] = rL */
	GET	$1,rG
	STOU	$1,$0,96	/* env[12] = rG */
	GET	$1,rA
	STOU	$1,$0,104	/* env[13] = rA */

	/* Clear remaining entries */
	SET	$1,0
	STOU	$1,$0,112	/* env[14] = 0 */
	STOU	$1,$0,120	/* env[15] = 0 */
	STOU	$1,$0,128	/* env[16] = 0 */
	STOU	$1,$0,136	/* env[17] = 0 */
	STOU	$1,$0,144	/* env[18] = 0 */
	STOU	$1,$0,152	/* env[19] = 0 */
	STOU	$1,$0,160	/* env[20] = 0 */
	STOU	$1,$0,168	/* env[21] = 0 */
	STOU	$1,$0,176	/* env[22] = 0 */
	STOU	$1,$0,184	/* env[23] = 0 */

	/* Return 0 on direct call */
	SET	$0,0
	POP	1,0

/*
 * void longjmp(jmp_buf env, int val)
 *
 * Restore execution context saved by setjmp
 *
 * Arguments:
 *   $0 = pointer to jmp_buf
 *   $1 = return value (must be non-zero; if 0, changed to 1)
 *
 * Does not return here - returns to setjmp caller
 */

	.globl	longjmp
	.globl	_longjmp
longjmp:
_longjmp:

	/* Ensure return value is non-zero */
	BNZ	$1,1f
	SET	$1,1		/* If val == 0, use 1 */
1:
	SET	$2,$1		/* Save return value */

	/* Restore callee-saved registers */
	LDOU	$16,$0,24	/* $16 = env[3] */
	LDOU	$17,$0,32	/* $17 = env[4] */
	LDOU	$18,$0,40	/* $18 = env[5] */
	LDOU	$19,$0,48	/* $19 = env[6] */
	LDOU	$20,$0,56	/* $20 = env[7] */
	LDOU	$21,$0,64	/* $21 = env[8] */
	LDOU	$22,$0,72	/* $22 = env[9] */
	LDOU	$23,$0,80	/* $23 = env[10] */

	/* Restore special registers */
	LDOU	$1,$0,88	/* rL = env[11] */
	PUT	rL,$1
	LDOU	$1,$0,96	/* rG = env[12] */
	PUT	rG,$1
	LDOU	$1,$0,104	/* rA = env[13] */
	PUT	rA,$1

	/* Restore return address to rJ */
	LDOU	$1,$0,0		/* rJ = env[0] */
	PUT	rJ,$1

	/* Restore frame pointer and stack pointer */
	LDOU	$253,$0,16	/* FP = env[2] */
	LDOU	$254,$0,8	/* SP = env[1] */

	/* Return to setjmp caller with non-zero value */
	SET	$0,$2		/* Return value in $0 */
	POP	1,0		/* Return via rJ */

/*
 * int _setjmp(jmp_buf env)
 *
 * setjmp without signal mask saving
 * (same as setjmp on MMIX - we don't save signal masks in setjmp)
 */

	.globl	__setjmp
__setjmp:
	JMP	setjmp

/*
 * void _longjmp(jmp_buf env, int val)
 *
 * longjmp without signal mask restoration
 * (same as longjmp on MMIX)
 */

	.globl	__longjmp
__longjmp:
	JMP	longjmp

/*
 * int sigsetjmp(sigjmp_buf env, int savemask)
 *
 * setjmp with optional signal mask saving
 * For kernel use, we ignore savemask
 */

	.globl	sigsetjmp
	.globl	_sigsetjmp
sigsetjmp:
_sigsetjmp:
	JMP	setjmp

/*
 * void siglongjmp(sigjmp_buf env, int val)
 *
 * longjmp with signal mask restoration
 * For kernel use, same as longjmp
 */

	.globl	siglongjmp
	.globl	_siglongjmp
siglongjmp:
_siglongjmp:
	JMP	longjmp
