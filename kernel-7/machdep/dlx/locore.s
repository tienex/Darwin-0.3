/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX Low-Level Kernel Operations
 * Context switching, system calls, and CPU operations
 */

	.text

/*
 * Context Switch
 * switch_context(old_thread, new_thread)
 *   r1 = old thread pointer
 *   r2 = new thread pointer
 */
	.globl	_switch_context
_switch_context:
	/* Save old context */
	sw	r1, 0(r1)		/* Save r1 */
	sw	r2, 4(r1)
	sw	r3, 8(r1)
	sw	r4, 12(r1)
	sw	r5, 16(r1)
	sw	r6, 20(r1)
	sw	r7, 24(r1)
	sw	r8, 28(r1)
	sw	r9, 32(r1)
	sw	r10, 36(r1)
	sw	r11, 40(r1)
	sw	r12, 44(r1)
	sw	r13, 48(r1)
	sw	r14, 52(r1)
	sw	r15, 56(r1)
	sw	r16, 60(r1)
	sw	r17, 64(r1)
	sw	r18, 68(r1)
	sw	r19, 72(r1)
	sw	r20, 76(r1)
	sw	r21, 80(r1)
	sw	r22, 84(r1)
	sw	r23, 88(r1)
	sw	r24, 92(r1)
	sw	r25, 96(r1)
	sw	r26, 100(r1)
	sw	r27, 104(r1)
	sw	r28, 108(r1)
	sw	r29, 112(r1)		/* Stack pointer */
	sw	r30, 116(r1)		/* Frame pointer */
	sw	r31, 120(r1)		/* Return address */

	/* Save PC */
	movi2s	r3, r0			/* Get status register */
	sw	r3, 124(r1)

	/* Load new context */
	lw	r1, 0(r2)
	lw	r3, 8(r2)
	lw	r4, 12(r2)
	lw	r5, 16(r2)
	lw	r6, 20(r2)
	lw	r7, 24(r2)
	lw	r8, 28(r2)
	lw	r9, 32(r2)
	lw	r10, 36(r2)
	lw	r11, 40(r2)
	lw	r12, 44(r2)
	lw	r13, 48(r2)
	lw	r14, 52(r2)
	lw	r15, 56(r2)
	lw	r16, 60(r2)
	lw	r17, 64(r2)
	lw	r18, 68(r2)
	lw	r19, 72(r2)
	lw	r20, 76(r2)
	lw	r21, 80(r2)
	lw	r22, 84(r2)
	lw	r23, 88(r2)
	lw	r24, 92(r2)
	lw	r25, 96(r2)
	lw	r26, 100(r2)
	lw	r27, 104(r2)
	lw	r28, 108(r2)
	lw	r29, 112(r2)
	lw	r30, 116(r2)
	lw	r31, 120(r2)

	/* Restore status register */
	lw	r3, 124(r2)
	movs2i	r3, r0

	/* Return to new context */
	lw	r2, 4(r2)
	jr	r31

/*
 * System Call Entry
 * Called when trap instruction executed
 */
	.globl	_syscall_entry
_syscall_entry:
	/* Save user registers on kernel stack */
	subi	r29, r29, #128		/* Allocate stack frame */

	sw	r1, 0(r29)
	sw	r2, 4(r29)
	sw	r3, 8(r29)
	sw	r4, 12(r29)
	sw	r5, 16(r29)
	sw	r6, 20(r29)
	sw	r7, 24(r29)
	sw	r8, 28(r29)
	sw	r31, 120(r29)		/* Return address */

	/* Get syscall number */
	movi2s	r4, r0			/* Get status */

	/* Call C syscall handler */
	jal	_syscall_handler

	/* Restore registers */
	lw	r1, 0(r29)
	lw	r2, 4(r29)
	lw	r3, 8(r29)
	lw	r4, 12(r29)
	lw	r5, 16(r29)
	lw	r6, 20(r29)
	lw	r7, 24(r29)
	lw	r8, 28(r29)
	lw	r31, 120(r29)

	addi	r29, r29, #128		/* Deallocate frame */

	/* Return to user */
	rfe				/* Return from exception */

/*
 * Page Fault Handler Entry
 */
	.globl	_pagefault_entry
_pagefault_entry:
	/* Save minimal registers */
	subi	r29, r29, #32
	sw	r1, 0(r29)
	sw	r2, 4(r29)
	sw	r3, 8(r29)
	sw	r31, 24(r29)

	/* Get faulting address */
	getreg	r1, 4			/* DLX_SREG_FAULT_ADDR = 4 */

	/* Determine if write */
	movi2s	r2, r0
	andi	r2, r2, #0x8000		/* Check write bit */
	srl	r2, r2, #15		/* is_write = 0 or 1 */

	/* Call C handler */
	jal	_dlx_pagefault_handler

	/* Restore registers */
	lw	r1, 0(r29)
	lw	r2, 4(r29)
	lw	r3, 8(r29)
	lw	r31, 24(r29)
	addi	r29, r29, #32

	/* Return */
	rfe

/*
 * TLB Miss Handler Entry
 */
	.globl	_tlbfault_entry
_tlbfault_entry:
	/* Save minimal registers */
	subi	r29, r29, #32
	sw	r1, 0(r29)
	sw	r2, 4(r29)
	sw	r3, 8(r29)
	sw	r31, 24(r29)

	/* Get faulting address */
	getreg	r1, 4			/* DLX_SREG_FAULT_ADDR */

	/* Determine if write */
	movi2s	r2, r0
	andi	r2, r2, #0x8000
	srl	r2, r2, #15

	/* Call C handler */
	jal	_dlx_tlbfault_handler

	/* Restore registers */
	lw	r1, 0(r29)
	lw	r2, 4(r29)
	lw	r3, 8(r29)
	lw	r31, 24(r29)
	addi	r29, r29, #32

	/* Return */
	rfe

/*
 * General Exception Handler
 */
	.globl	_exception_entry
_exception_entry:
	/* Save all registers */
	subi	r29, r29, #128

	sw	r1, 0(r29)
	sw	r2, 4(r29)
	sw	r3, 8(r29)
	sw	r4, 12(r29)
	sw	r5, 16(r29)
	sw	r6, 20(r29)
	sw	r7, 24(r29)
	sw	r8, 28(r29)
	sw	r9, 32(r29)
	sw	r10, 36(r29)
	sw	r11, 40(r29)
	sw	r12, 44(r29)
	sw	r13, 48(r29)
	sw	r14, 52(r29)
	sw	r15, 56(r29)
	sw	r16, 60(r29)
	sw	r17, 64(r29)
	sw	r18, 68(r29)
	sw	r19, 72(r29)
	sw	r20, 76(r29)
	sw	r21, 80(r29)
	sw	r22, 84(r29)
	sw	r23, 88(r29)
	sw	r24, 92(r29)
	sw	r25, 96(r29)
	sw	r26, 100(r29)
	sw	r27, 104(r29)
	sw	r28, 108(r29)
	sw	r30, 116(r29)
	sw	r31, 120(r29)

	/* Get exception info */
	movi2s	r1, r0			/* Status register */
	getreg	r2, 4			/* Fault address */

	/* Call C exception handler */
	jal	_dlx_exception_handler

	/* Restore all registers */
	lw	r1, 0(r29)
	lw	r2, 4(r29)
	lw	r3, 8(r29)
	lw	r4, 12(r29)
	lw	r5, 16(r29)
	lw	r6, 20(r29)
	lw	r7, 24(r29)
	lw	r8, 28(r29)
	lw	r9, 32(r29)
	lw	r10, 36(r29)
	lw	r11, 40(r29)
	lw	r12, 44(r29)
	lw	r13, 48(r29)
	lw	r14, 52(r29)
	lw	r15, 56(r29)
	lw	r16, 60(r29)
	lw	r17, 64(r29)
	lw	r18, 68(r29)
	lw	r19, 72(r29)
	lw	r20, 76(r29)
	lw	r21, 80(r29)
	lw	r22, 84(r29)
	lw	r23, 88(r29)
	lw	r24, 92(r29)
	lw	r25, 96(r29)
	lw	r26, 100(r29)
	lw	r27, 104(r29)
	lw	r28, 108(r29)
	lw	r30, 116(r29)
	lw	r31, 120(r29)

	addi	r29, r29, #128

	/* Return */
	rfe

/*
 * Atomic Operations
 */

	.globl	_test_and_set
_test_and_set:
	/* Test and set word at (r1) */
	lw	r2, 0(r1)		/* Load current value */
	li	r3, #1
	sw	r3, 0(r1)		/* Store 1 */
	jr	r31			/* Return old value in r2 */

	.globl	_atomic_add
_atomic_add:
	/* Atomic add r2 to (r1) */
	lw	r3, 0(r1)
	add	r3, r3, r2
	sw	r3, 0(r1)
	jr	r31

/*
 * Special Register Access
 */

	.globl	_get_status_register
_get_status_register:
	movi2s	r1, r0			/* Move status to r1 */
	jr	r31

	.globl	_set_status_register
_set_status_register:
	movs2i	r1, r0			/* Move r1 to status */
	jr	r31

	.globl	_get_special_register
_get_special_register:
	getreg	r1, r1			/* Get special register r1 */
	jr	r31

	.globl	_set_special_register
_set_special_register:
	setreg	r2, r1			/* Set special register r1 to r2 */
	jr	r31

/*
 * Exception Vector Table
 * Placed at address 0x00000000 by linker script
 * Uses Mach-O section __TEXT,__vectors
 */
	.section __TEXT,__vectors
	.globl	_exception_vectors
_exception_vectors:
	j	_start			/* 0x00: Reset */
	j	_exception_entry	/* 0x04: Illegal instruction */
	j	_exception_entry	/* 0x08: Address error */
	j	_exception_entry	/* 0x0C: Access violation */
	j	_exception_entry	/* 0x10: Overflow */
	j	_exception_entry	/* 0x14: Divide by zero */
	j	_exception_entry	/* 0x18: Privilege violation */
	j	_exception_entry	/* 0x1C: Format error */
	.space	16			/* 0x20-0x2F: Reserved */
	j	_pagefault_entry	/* 0x30: Page fault */
	.space	12			/* 0x34-0x3F: Reserved */
	j	_tlbfault_entry		/* 0x40: TLB fault */
	.space	12			/* 0x44-0x4F: Reserved */
	j	_exception_entry	/* 0x50: Timer interrupt */
	.space	28			/* 0x54-0x6F: Reserved */
	j	_exception_entry	/* 0x70: Keyboard interrupt */
	.space	140			/* 0x74-0xFF: Reserved */

	.text
