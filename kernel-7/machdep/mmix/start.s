/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999-2025 Apple Computer, Inc.  All Rights
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
 * MMIX Kernel Entry Point
 *
 * This is the first code executed when the bootloader transfers control
 * to the kernel. We are entered with:
 *
 *   $0 = pointer to boot_args structure
 *   MMU enabled with identity mapping + kernel mapping
 *   Running at kernel virtual address (0x8000000000100000)
 *   Interrupts disabled
 *
 * We must:
 *   1. Save boot_args pointer
 *   2. Initialize special registers
 *   3. Set up kernel stack
 *   4. Initialize BSS if needed
 *   5. Call mmix_init() to start kernel initialization
 */

#include <machdep/mmix/asm.h>
#include <mach/mmix/vm_param.h>

	.file	"start.s"

/*
 * Kernel stacks - aligned on page boundaries
 */

	.data
	.align	MMIX_PGSHIFT		/* Align on 8KB page boundary */

	/* Interrupt stack */
	.globl	intstack
intstack:
	.space	INTSTACK_SIZE		/* 16KB interrupt stack */

	.align	3
	.globl	intstack_top_ss
intstack_top_ss:
	.quad	intstack + INTSTACK_SIZE - SS_SIZE

	/* GDB debugger stack */
	.globl	gdbstack
gdbstack:
	.space	KERNSTACK_SIZE		/* 16KB debugger stack */

	.align	3
	.globl	gdbstack_top_ss
gdbstack_top_ss:
	.quad	gdbstack + KERNSTACK_SIZE - SS_SIZE

	.globl	gdbstackptr
gdbstackptr:
	.quad	gdbstack + KERNSTACK_SIZE - SS_SIZE

	/* Saved boot arguments pointer */
	.align	3
	.globl	mmix_boot_args
mmix_boot_args:
	.quad	0

/*
 * Kernel entry point
 *
 * Called by bootloader with:
 *   $0 = boot_args pointer
 */

	.text
	.align	4			/* 16-byte alignment */

	.globl	_start
	.globl	start
_start:
start:
	/*
	 * Save boot arguments pointer
	 * $0 contains pointer from bootloader
	 */
	GETA	$1,mmix_boot_args
	STOU	$0,$1,0

	/*
	 * Initialize MMIX special registers
	 */

	/* rG = 32 (32 global registers $0-$31) */
	SET	$0,32
	PUT	rG,$0

	/* rL = 16 (use $0-$15 as locals, rest for temps) */
	SET	$0,16
	PUT	rL,$0

	/* rK = 0 (disable all interrupts initially) */
	SET	$0,0
	PUT	rK,$0

	/* rT = 0 (clear trap address) */
	SET	$0,0
	PUT	rT,$0

	/* rV = 0 (clear virtual translation register) */
	SET	$0,0
	PUT	rV,$0

	/* rI = 0 (clear interval counter) */
	SET	$0,0
	PUT	rI,$0

	/*
	 * Clear floating point status
	 */
	SET	$0,0
	PUT	rA,$0		/* Clear rA (arithmetic status) */

	/*
	 * Set up kernel stack
	 * SP ($254) = top of interrupt stack
	 */
	GETA	$254,intstack
	SETL	$1,INTSTACK_SIZE
	ADDU	$254,$254,$1

	/* Frame pointer ($253) = stack pointer */
	SET	$253,$254

	/*
	 * Clear all general purpose registers $1-$252
	 * (Keep $253=FP, $254=SP, $255=return)
	 * Start with locals $16-$252
	 */
	SET	$1,0
	SET	$2,0
	SET	$3,0
	SET	$4,0
	SET	$5,0
	SET	$6,0
	SET	$7,0
	SET	$8,0
	SET	$9,0
	SET	$10,0
	SET	$11,0
	SET	$12,0
	SET	$13,0
	SET	$14,0
	SET	$15,0

	/* Clear callee-saved registers $16-$23 */
	SET	$16,0
	SET	$17,0
	SET	$18,0
	SET	$19,0
	SET	$20,0
	SET	$21,0
	SET	$22,0
	SET	$23,0

	/*
	 * Initialize BSS section
	 * (Set all uninitialized data to zero)
	 */
	GETA	$1,bss_start
	GETA	$2,bss_end
	SET	$3,0
1:
	CMP	$4,$1,$2
	BNN	$4,2f		/* Branch if $1 >= $2 */
	STOU	$3,$1,0
	ADDU	$1,$1,8		/* Advance by 8 bytes */
	JMP	1b
2:

	/*
	 * Call mmix_init(boot_args)
	 *
	 * Pass boot_args pointer as first argument
	 */
	GETA	$1,mmix_boot_args
	LDOU	$0,$1,0		/* $0 = boot_args pointer */

	PUSHJ	$255,mmix_init

	/*
	 * mmix_init() should never return, but if it does,
	 * loop forever
	 */
halt_loop:
	JMP	halt_loop

/*
 * BSS section boundaries
 * These symbols are defined by the linker
 */
	.globl	bss_start
	.globl	bss_end

/*
 * Stack size definitions
 */
#define INTSTACK_SIZE	(16*1024)	/* 16KB interrupt stack */
#define KERNSTACK_SIZE	(16*1024)	/* 16KB kernel stack */
#define SS_SIZE		512		/* Saved state structure size */

/*
 * Function: mmix_init
 *
 * External C function that performs kernel initialization
 * Defined in machdep.c
 */
	.globl	mmix_init
