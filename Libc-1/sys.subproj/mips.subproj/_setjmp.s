/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * MIPS setjmp/longjmp implementation
 *
 * setjmp saves the register context to a jmp_buf
 * longjmp restores the register context from a jmp_buf
 *
 * jmp_buf layout (in words):
 *  0-7:  s0-s7 (callee-saved registers)
 *  8:    s8/fp (frame pointer)
 *  9:    sp (stack pointer)
 *  10:   ra (return address)
 *  11:   gp (global pointer)
 */

#include <architecture/mips/asm_help.h>

	.text
	.set noreorder

/*
 * int setjmp(jmp_buf env)
 *
 * Save calling environment in env
 * Returns 0
 */
LEAF(setjmp)
	/* Save callee-saved registers */
	sw	s0, 0(a0)	/* env[0] = s0 */
	sw	s1, 4(a0)	/* env[1] = s1 */
	sw	s2, 8(a0)	/* env[2] = s2 */
	sw	s3, 12(a0)	/* env[3] = s3 */
	sw	s4, 16(a0)	/* env[4] = s4 */
	sw	s5, 20(a0)	/* env[5] = s5 */
	sw	s6, 24(a0)	/* env[6] = s6 */
	sw	s7, 28(a0)	/* env[7] = s7 */

	/* Save frame pointer, stack pointer, return address, global pointer */
	sw	s8, 32(a0)	/* env[8] = s8/fp */
	sw	sp, 36(a0)	/* env[9] = sp */
	sw	ra, 40(a0)	/* env[10] = ra */
	sw	gp, 44(a0)	/* env[11] = gp */

	/* Return 0 */
	jr	ra
	move	v0, zero
END(setjmp)

/*
 * void longjmp(jmp_buf env, int val)
 *
 * Restore environment saved by setjmp and return val (or 1 if val == 0)
 */
LEAF(longjmp)
	/* Restore callee-saved registers */
	lw	s0, 0(a0)	/* s0 = env[0] */
	lw	s1, 4(a0)	/* s1 = env[1] */
	lw	s2, 8(a0)	/* s2 = env[2] */
	lw	s3, 12(a0)	/* s3 = env[3] */
	lw	s4, 16(a0)	/* s4 = env[4] */
	lw	s5, 20(a0)	/* s5 = env[5] */
	lw	s6, 24(a0)	/* s6 = env[6] */
	lw	s7, 28(a0)	/* s7 = env[7] */

	/* Restore frame pointer, stack pointer, return address, global pointer */
	lw	s8, 32(a0)	/* s8/fp = env[8] */
	lw	sp, 36(a0)	/* sp = env[9] */
	lw	ra, 40(a0)	/* ra = env[10] */
	lw	gp, 44(a0)	/* gp = env[11] */

	/* Set return value (val or 1 if val == 0) */
	move	v0, a1		/* v0 = val */
	bnez	v0, 1f		/* if (val != 0) goto 1 */
	nop
	li	v0, 1		/* v0 = 1 */
1:
	/* Return to saved context */
	jr	ra
	nop
END(longjmp)

/*
 * int _setjmp(jmp_buf env)
 *
 * Same as setjmp but doesn't save signal mask
 */
LEAF(_setjmp)
	j	setjmp
	nop
END(_setjmp)

/*
 * void _longjmp(jmp_buf env, int val)
 *
 * Same as longjmp but doesn't restore signal mask
 */
LEAF(_longjmp)
	j	longjmp
	nop
END(_longjmp)
