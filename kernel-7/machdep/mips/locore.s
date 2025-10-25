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
 * MIPS Low-level Exception Handlers and Kernel Entry Points
 *
 * This file contains the critical exception vectors and handlers
 * that must be at specific addresses for MIPS exception processing.
 */

#include <machdep/mips/asm.h>
#include <assym.h>

	.set	noreorder		/* Don't reorder instructions */
	.set	noat			/* Don't use $at automatically */

/*
 * Exception Vector Base
 * MIPS requires exception vectors at specific offsets from the base
 */
	.section .text.vectors,"ax"
	.align	12			/* 4KB alignment for exception base */

/*
 * TLB Refill Exception Vector (0x000)
 * This is the most critical exception - handles TLB misses
 * Must be very fast as it's on the critical path
 */
	.org	0x000
LEAF(tlb_refill_vector)
	.set	push
	.set	noat
	mfc0	k0, $8			/* k0 = CP0_BADVADDR */
	mfc0	k1, $10			/* k1 = CP0_ENTRYHI */
	srl	k0, k0, 13		/* Page number */
	andi	k1, k1, 0xff		/* Extract ASID */
	sll	k0, k0, 3		/* *8 for PTE size */
	/* Load page table base - would come from current pmap */
	lui	k1, %hi(kernel_pmap_pte_base)
	lw	k1, %lo(kernel_pmap_pte_base)(k1)
	addu	k0, k0, k1		/* k0 = &PTE */
	lw	k1, 0(k0)		/* Load PTE */
	lw	k0, 4(k0)		/* Load PTE+4 (odd page) */
	mtc0	k1, $2			/* CP0_ENTRYLO0 */
	mtc0	k0, $3			/* CP0_ENTRYLO1 */
	nop
	tlbwr				/* Write random TLB entry */
	nop
	nop
	eret				/* Return from exception */
	nop
	.set	pop
END(tlb_refill_vector)

/*
 * XTLB Refill Exception Vector (0x080)
 * For MIPS64 extended addressing - 64-bit TLB refill
 */
	.org	0x080
LEAF(xtlb_refill_vector)
#if defined(_MIPS64) || defined(__mips64)
	.set	push
	.set	noat
	dmfc0	k0, $8			/* k0 = CP0_BADVADDR (64-bit) */
	dmfc0	k1, $10			/* k1 = CP0_ENTRYHI */
	dsrl	k0, k0, 13		/* Page number */
	andi	k1, k1, 0xff		/* Extract ASID */
	dsll	k0, k0, 4		/* *16 for 64-bit PTE */
	lui	k1, %hi(kernel_pmap_pte_base)
	ld	k1, %lo(kernel_pmap_pte_base)(k1)
	daddu	k0, k0, k1
	ld	k1, 0(k0)
	ld	k0, 8(k0)
	dmtc0	k1, $2			/* CP0_ENTRYLO0 */
	dmtc0	k0, $3			/* CP0_ENTRYLO1 */
	nop
	tlbwr
	nop
	nop
	eret
	nop
	.set	pop
#else
	/* MIPS32 - shouldn't get here */
	j	general_exception
	nop
#endif
END(xtlb_refill_vector)

/*
 * Cache Error Exception Vector (0x100)
 * Handles cache parity/ECC errors
 */
	.org	0x100
LEAF(cache_error_vector)
	/* Save minimal state and call C handler */
	j	cache_error_handler
	nop
END(cache_error_vector)

/*
 * General Exception Vector (0x180)
 * Handles all exceptions except TLB refill and cache errors
 */
	.org	0x180
LEAF(general_exception_vector)
	j	exception_handler
	nop
END(general_exception_vector)

/*
 * Interrupt Exception Vector (0x200)
 * Dedicated interrupt entry point (if SR[IV]=1)
 */
	.org	0x200
LEAF(interrupt_vector)
	j	exception_handler	/* Use common handler */
	nop
END(interrupt_vector)

	.text				/* Back to normal text section */

/*
 * exception_handler - General exception entry point
 *
 * Saves all registers and calls the C trap handler.
 * Entered from exception vectors with:
 *   - EXL bit set (exceptions disabled)
 *   - Cause register set
 *   - BadVAddr set (for address exceptions)
 *   - k0, k1 available as scratch registers
 */
NESTED(exception_handler, 0, sp)
	.set	push
	.set	noat

	/* Check if from kernel or user mode */
	mfc0	k0, $12			/* k0 = CP0_STATUS */
	andi	k1, k0, 0x10		/* Test KSU field */
	beqz	k1, exception_from_kernel
	nop

exception_from_user:
	/* Coming from user mode - use kernel stack */
	mfc0	k0, $4			/* k0 = CP0_CONTEXT (has kernel sp) */
	/* In real implementation, would load thread's kernel stack */
	/* For now, use current sp as we're already in kernel */

exception_from_kernel:
	/* Make room for saved state on stack */
	subu	sp, sp, 280		/* sizeof(mips_saved_state) */

	/* Save all general purpose registers */
	sw	$0, 0(sp)		/* zero */
	sw	$1, 4(sp)		/* at */
	sw	$2, 8(sp)		/* v0 */
	sw	$3, 12(sp)		/* v1 */
	sw	$4, 16(sp)		/* a0 */
	sw	$5, 20(sp)		/* a1 */
	sw	$6, 24(sp)		/* a2 */
	sw	$7, 28(sp)		/* a3 */
	sw	$8, 32(sp)		/* t0 */
	sw	$9, 36(sp)		/* t1 */
	sw	$10, 40(sp)		/* t2 */
	sw	$11, 44(sp)		/* t3 */
	sw	$12, 48(sp)		/* t4 */
	sw	$13, 52(sp)		/* t5 */
	sw	$14, 56(sp)		/* t6 */
	sw	$15, 60(sp)		/* t7 */
	sw	$16, 64(sp)		/* s0 */
	sw	$17, 68(sp)		/* s1 */
	sw	$18, 72(sp)		/* s2 */
	sw	$19, 76(sp)		/* s3 */
	sw	$20, 80(sp)		/* s4 */
	sw	$21, 84(sp)		/* s5 */
	sw	$22, 88(sp)		/* s6 */
	sw	$23, 92(sp)		/* s7 */
	sw	$24, 96(sp)		/* t8 */
	sw	$25, 100(sp)		/* t9 */
	/* k0, k1 saved later from CP0 */
	sw	$28, 112(sp)		/* gp */
	sw	$29, 116(sp)		/* sp (original) */
	sw	$30, 120(sp)		/* fp/s8 */
	sw	$31, 124(sp)		/* ra */

	/* Save multiply/divide registers */
	mflo	t0
	sw	t0, 128(sp)		/* lo */
	mfhi	t0
	sw	t0, 132(sp)		/* hi */

	/* Save CP0 registers */
	mfc0	t0, $14			/* EPC */
	sw	t0, 136(sp)		/* pc */
	mfc0	t0, $8			/* BadVAddr */
	sw	t0, 140(sp)		/* badvaddr */
	mfc0	t0, $13			/* Cause */
	sw	t0, 144(sp)		/* cause */
	mfc0	t0, $12			/* Status */
	sw	t0, 148(sp)		/* status */

	/* Adjust sp in saved state to original value */
	addu	t0, sp, 280
	sw	t0, 116(sp)

	/* Call C trap handler */
	move	a0, sp			/* arg0 = saved state */
	jal	trap			/* Call trap(state) */
	nop

	/* Fall through to exception_return */

exception_return:
	/* Restore registers from stack */
	lw	t0, 128(sp)		/* lo */
	mtlo	t0
	lw	t0, 132(sp)		/* hi */
	mthi	t0

	/* Restore CP0 EPC for return */
	lw	t0, 136(sp)		/* pc */
	mtc0	t0, $14			/* EPC */

	/* Restore general registers */
	lw	$1, 4(sp)		/* at */
	lw	$2, 8(sp)		/* v0 */
	lw	$3, 12(sp)		/* v1 */
	lw	$4, 16(sp)		/* a0 */
	lw	$5, 20(sp)		/* a1 */
	lw	$6, 24(sp)		/* a2 */
	lw	$7, 28(sp)		/* a3 */
	lw	$8, 32(sp)		/* t0 */
	lw	$9, 36(sp)		/* t1 */
	lw	$10, 40(sp)		/* t2 */
	lw	$11, 44(sp)		/* t3 */
	lw	$12, 48(sp)		/* t4 */
	lw	$13, 52(sp)		/* t5 */
	lw	$14, 56(sp)		/* t6 */
	lw	$15, 60(sp)		/* t7 */
	lw	$16, 64(sp)		/* s0 */
	lw	$17, 68(sp)		/* s1 */
	lw	$18, 72(sp)		/* s2 */
	lw	$19, 76(sp)		/* s3 */
	lw	$20, 80(sp)		/* s4 */
	lw	$21, 84(sp)		/* s5 */
	lw	$22, 88(sp)		/* s6 */
	lw	$23, 92(sp)		/* s7 */
	lw	$24, 96(sp)		/* t8 */
	lw	$25, 100(sp)		/* t9 */
	lw	$28, 112(sp)		/* gp */
	lw	$30, 120(sp)		/* fp/s8 */
	lw	$31, 124(sp)		/* ra */

	/* Restore stack pointer last */
	lw	$29, 116(sp)		/* sp */

	/* Return from exception */
	eret
	nop

	.set	pop
END(exception_handler)

/*
 * syscall_handler - System call entry point
 *
 * Entered via syscall instruction with:
 *   - v0 = syscall number
 *   - a0-a3 = arguments
 */
NESTED(syscall_handler, 0, sp)
	/* Save state similar to exception_handler */
	subu	sp, sp, 280

	/* Save registers (abbreviated - same as exception_handler) */
	sw	$2, 8(sp)		/* v0 - syscall number */
	sw	$4, 16(sp)		/* a0 */
	sw	$5, 20(sp)		/* a1 */
	sw	$6, 24(sp)		/* a2 */
	sw	$7, 28(sp)		/* a3 */
	sw	$31, 124(sp)		/* ra */
	mfc0	t0, $14
	sw	t0, 136(sp)		/* pc */

	/* Call system call handler */
	move	a0, sp
	jal	unix_syscall		/* unix_syscall(state) */
	nop

	/* Restore and return */
	j	exception_return
	nop
END(syscall_handler)

/*
 * switch_context - Switch thread context
 *
 * Arguments:
 *   a0 = old thread PCB
 *   a1 = new thread PCB
 */
LEAF(switch_context)
	/* Save old thread state (callee-saved registers only) */
	sw	$16, 0(a0)		/* s0 */
	sw	$17, 4(a0)		/* s1 */
	sw	$18, 8(a0)		/* s2 */
	sw	$19, 12(a0)		/* s3 */
	sw	$20, 16(a0)		/* s4 */
	sw	$21, 20(a0)		/* s5 */
	sw	$22, 24(a0)		/* s6 */
	sw	$23, 28(a0)		/* s7 */
	sw	$30, 32(a0)		/* fp/s8 */
	sw	$28, 36(a0)		/* gp */
	sw	$29, 40(a0)		/* sp */
	sw	$31, 44(a0)		/* ra */

	/* Restore new thread state */
	lw	$16, 0(a1)		/* s0 */
	lw	$17, 4(a1)		/* s1 */
	lw	$18, 8(a1)		/* s2 */
	lw	$19, 12(a1)		/* s3 */
	lw	$20, 16(a1)		/* s4 */
	lw	$21, 20(a1)		/* s5 */
	lw	$22, 24(a1)		/* s6 */
	lw	$23, 28(a1)		/* s7 */
	lw	$30, 32(a1)		/* fp/s8 */
	lw	$28, 36(a1)		/* gp */
	lw	$29, 40(a1)		/* sp */
	lw	$31, 44(a1)		/* ra */

	jr	ra			/* Return to new thread */
	nop
END(switch_context)

/*
 * mips_enable_interrupts - Enable interrupts
 * Returns old SR value
 */
LEAF(mips_enable_interrupts)
	mfc0	v0, $12			/* Get current SR */
	ori	t0, v0, 0x01		/* Set IE bit */
	mtc0	t0, $12			/* Update SR */
	jr	ra
	nop
END(mips_enable_interrupts)

/*
 * mips_disable_interrupts - Disable interrupts
 * Returns old SR value
 */
LEAF(mips_disable_interrupts)
	mfc0	v0, $12			/* Get current SR */
	li	t0, 0xfffffffe
	and	t0, v0, t0		/* Clear IE bit */
	mtc0	t0, $12			/* Update SR */
	jr	ra
	nop
END(mips_disable_interrupts)

/*
 * mips_get_sr - Read Status Register
 */
LEAF(mips_get_sr)
	mfc0	v0, $12
	jr	ra
	nop
END(mips_get_sr)

/*
 * mips_set_sr - Write Status Register
 */
LEAF(mips_set_sr)
	mtc0	a0, $12
	jr	ra
	nop
END(mips_set_sr)

/*
 * mips_get_cause - Read Cause Register
 */
LEAF(mips_get_cause)
	mfc0	v0, $13
	jr	ra
	nop
END(mips_get_cause)

/*
 * mips_tlb_flush - Flush entire TLB
 */
LEAF(mips_tlb_flush)
	mfc0	t0, $12			/* Save SR */
	mtc0	zero, $10		/* Clear EntryHi */
	mtc0	zero, $2		/* Clear EntryLo0 */
	mtc0	zero, $3		/* Clear EntryLo1 */
	li	t1, 64			/* TLB size */
	li	t2, 0
1:
	mtc0	t2, $0			/* Set Index */
	nop
	nop
	tlbwi				/* Write Indexed */
	addiu	t2, t2, 1
	bne	t2, t1, 1b
	nop
	mtc0	t0, $12			/* Restore SR */
	jr	ra
	nop
END(mips_tlb_flush)

/*
 * mips_tlb_flush_addr - Flush single TLB entry by address
 * a0 = virtual address
 */
LEAF(mips_tlb_flush_addr)
	mfc0	t0, $10			/* Save EntryHi */
	mtc0	a0, $10			/* Set address to probe */
	nop
	nop
	tlbp				/* Probe TLB */
	nop
	nop
	mfc0	t1, $0			/* Read Index */
	bltz	t1, 1f			/* Not found, skip */
	nop
	mtc0	zero, $2		/* Clear EntryLo0 */
	mtc0	zero, $3		/* Clear EntryLo1 */
	nop
	nop
	tlbwi				/* Write Indexed */
	nop
1:
	mtc0	t0, $10			/* Restore EntryHi */
	jr	ra
	nop
END(mips_tlb_flush_addr)

/*
 * cache_error_handler - Handle cache errors
 * This is a stub - real implementation would decode cache error info
 */
LEAF(cache_error_handler)
	/* Save minimal state */
	move	k0, ra
	jal	handle_cache_error	/* Call C handler */
	nop
	move	ra, k0
	eret
	nop
END(cache_error_handler)

/*
 * Data area for kernel
 */
	.data
	.align	3
	.globl	kernel_pmap_pte_base
kernel_pmap_pte_base:
	.word	0			/* Filled in by pmap_bootstrap */

	.globl	exception_vectors_base
exception_vectors_base:
	.word	tlb_refill_vector	/* For relocation */
