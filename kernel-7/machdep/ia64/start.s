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
 * IA64 Kernel Startup Code
 *
 * This is the initial entry point for the IA64 Darwin kernel.
 * Called by the bootloader with:
 *   r32 = boot info pointer
 *   r33 = memory descriptor pointer
 */

#include "asm.h"

	.text
	.align BUNDLE_SIZE
	.global _start
	.proc _start

_start:
	/*
	 * Save boot parameters
	 */
	alloc	loc0 = ar.pfs, 0, 8, 1, 0
	mov	loc1 = rp		/* Save return pointer */
	mov	loc2 = gp		/* Save global pointer */

	mov	loc3 = r32		/* Save boot info */
	mov	loc4 = r33		/* Save memory descriptor */

	/*
	 * Initialize the BSP (Bootstrap Processor) stack
	 */
	movl	r12 = __bsp_stack		/* Load stack pointer */
	addl	r12 = KERNEL_STACK_SIZE, r12	/* Point to top of stack */

	/*
	 * Clear register stack frame markers
	 */
	mov	ar.rsc = 0		/* Disable RSE */
	;;
	movl	r14 = __bsp_rbs		/* Load backing store */
	;;
	mov	ar.bspstore = r14	/* Set backing store pointer */
	;;
	mov	ar.rsc = 3		/* Re-enable RSE (eager mode) */

	/*
	 * Initialize PSR for kernel mode
	 * - Disable interrupts
	 * - Enable instruction and data translation
	 * - Set IA-64 mode (not x86)
	 */
	movl	r14 = 0x0000001000000000	/* PSR.ic = 0, PSR.i = 0 */
	mov	cr.ipsr = r14

	/*
	 * Set up exception vectors
	 */
	movl	r14 = ia64_exception_vectors
	mov	cr.iva = r14		/* Interruption Vector Address */

	/*
	 * Call main kernel initialization
	 */
	mov	out0 = loc3		/* Pass boot info */
	br.call.sptk.many rp = _ia64_init

	/*
	 * Should never return, but if we do, halt
	 */
halt_loop:
	hint	@pause
	br.sptk	halt_loop
	;;

	.endp _start

/*
 * BSP stack and register backing store
 * Allocated in BSS
 */
	.bss
	.align 16
	.global __bsp_stack
__bsp_stack:
	.skip	KERNEL_STACK_SIZE

	.align 16
	.global __bsp_rbs
__bsp_rbs:
	.skip	65536		/* 64KB register backing store */
