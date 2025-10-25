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
 * RISC-V low-level kernel routines
 */

.section .text

/*
 * Trap/Exception entry point
 * Save all registers and call C trap handler
 */
.globl trap_entry
.align 4
trap_entry:
	/* Save all general-purpose registers */
	addi	sp, sp, -256
	sd	x1, 0(sp)	/* ra */
	sd	x2, 8(sp)	/* sp - will be adjusted */
	sd	x3, 16(sp)	/* gp */
	sd	x4, 24(sp)	/* tp */
	sd	x5, 32(sp)	/* t0 */
	sd	x6, 40(sp)	/* t1 */
	sd	x7, 48(sp)	/* t2 */
	sd	x8, 56(sp)	/* s0/fp */
	sd	x9, 64(sp)	/* s1 */
	sd	x10, 72(sp)	/* a0 */
	sd	x11, 80(sp)	/* a1 */
	sd	x12, 88(sp)	/* a2 */
	sd	x13, 96(sp)	/* a3 */
	sd	x14, 104(sp)	/* a4 */
	sd	x15, 112(sp)	/* a5 */
	sd	x16, 120(sp)	/* a6 */
	sd	x17, 128(sp)	/* a7 */
	sd	x18, 136(sp)	/* s2 */
	sd	x19, 144(sp)	/* s3 */
	sd	x20, 152(sp)	/* s4 */
	sd	x21, 160(sp)	/* s5 */
	sd	x22, 168(sp)	/* s6 */
	sd	x23, 176(sp)	/* s7 */
	sd	x24, 184(sp)	/* s8 */
	sd	x25, 192(sp)	/* s9 */
	sd	x26, 200(sp)	/* s10 */
	sd	x27, 208(sp)	/* s11 */
	sd	x28, 216(sp)	/* t3 */
	sd	x29, 224(sp)	/* t4 */
	sd	x30, 232(sp)	/* t5 */
	sd	x31, 240(sp)	/* t6 */

	/* Read exception cause and tval */
	csrr	a0, scause
	csrr	a1, stval
	mv	a2, sp		/* pointer to saved registers */

	/* Determine if from user or kernel */
	csrr	t0, sstatus
	andi	t0, t0, 0x100	/* SPP bit */
	beqz	t0, 1f
	li	a3, 0		/* kernel mode */
	j	2f
1:
	li	a3, 1		/* user mode */
2:
	/* Call C trap handler */
	call	trap_handler

	/* Restore registers */
	ld	x1, 0(sp)
	ld	x3, 16(sp)
	ld	x4, 24(sp)
	ld	x5, 32(sp)
	ld	x6, 40(sp)
	ld	x7, 48(sp)
	ld	x8, 56(sp)
	ld	x9, 64(sp)
	ld	x10, 72(sp)
	ld	x11, 80(sp)
	ld	x12, 88(sp)
	ld	x13, 96(sp)
	ld	x14, 104(sp)
	ld	x15, 112(sp)
	ld	x16, 120(sp)
	ld	x17, 128(sp)
	ld	x18, 136(sp)
	ld	x19, 144(sp)
	ld	x20, 152(sp)
	ld	x21, 160(sp)
	ld	x22, 168(sp)
	ld	x23, 176(sp)
	ld	x24, 184(sp)
	ld	x25, 192(sp)
	ld	x26, 200(sp)
	ld	x27, 208(sp)
	ld	x28, 216(sp)
	ld	x29, 224(sp)
	ld	x30, 232(sp)
	ld	x31, 240(sp)
	ld	x2, 8(sp)
	addi	sp, sp, 256

	/* Return from trap */
	sret

/*
 * Context switch routine
 * switch_to(old_sp, new_sp)
 */
.globl switch_to
switch_to:
	/* Save callee-saved registers of old thread */
	sd	ra, 0(a0)
	sd	sp, 8(a0)
	sd	s0, 16(a0)
	sd	s1, 24(a0)
	sd	s2, 32(a0)
	sd	s3, 40(a0)
	sd	s4, 48(a0)
	sd	s5, 56(a0)
	sd	s6, 64(a0)
	sd	s7, 72(a0)
	sd	s8, 80(a0)
	sd	s9, 88(a0)
	sd	s10, 96(a0)
	sd	s11, 104(a0)

	/* Restore callee-saved registers of new thread */
	ld	ra, 0(a1)
	ld	sp, 8(a1)
	ld	s0, 16(a1)
	ld	s1, 24(a1)
	ld	s2, 32(a1)
	ld	s3, 40(a1)
	ld	s4, 48(a1)
	ld	s5, 56(a1)
	ld	s6, 64(a1)
	ld	s7, 72(a1)
	ld	s8, 80(a1)
	ld	s9, 88(a1)
	ld	s10, 96(a1)
	ld	s11, 104(a1)

	ret

/*
 * Atomic operations
 */
.globl atomic_add
atomic_add:
	amoadd.w.aqrl zero, a1, (a0)
	ret

.globl atomic_sub
atomic_sub:
	neg	a1, a1
	amoadd.w.aqrl zero, a1, (a0)
	ret

/*
 * Copy routines
 */
.globl bcopy
bcopy:
	/* Simple byte copy - can be optimized */
	beqz	a2, 2f
1:
	lb	t0, 0(a0)
	sb	t0, 0(a1)
	addi	a0, a0, 1
	addi	a1, a1, 1
	addi	a2, a2, -1
	bnez	a2, 1b
2:
	ret

.globl bzero
bzero:
	/* Zero memory */
	beqz	a1, 2f
1:
	sb	zero, 0(a0)
	addi	a0, a0, 1
	addi	a1, a1, -1
	bnez	a1, 1b
2:
	ret

/*
 * Get current time from cycle counter
 */
.globl get_cycles
get_cycles:
	rdtime	a0
	ret

/*
 * Enable/Disable interrupts
 */
.globl enable_interrupts
enable_interrupts:
	csrsi	sstatus, 0x2	/* SIE bit */
	ret

.globl disable_interrupts
disable_interrupts:
	csrci	sstatus, 0x2	/* SIE bit */
	ret

.globl save_and_disable_interrupts
save_and_disable_interrupts:
	csrrci	a0, sstatus, 0x2
	ret

.globl restore_interrupts
restore_interrupts:
	csrw	sstatus, a0
	ret
