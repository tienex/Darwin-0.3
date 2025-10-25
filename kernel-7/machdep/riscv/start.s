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
 * RISC-V startup code
 */

.section .text.start
.globl _start

_start:
	/* Set up stack pointer */
	la	sp, _stack_top

	/* Clear BSS */
	la	a0, _bss_start
	la	a1, _bss_end
1:
	bge	a0, a1, 2f
	sd	zero, 0(a0)
	addi	a0, a0, 8
	j	1b
2:
	/* Jump to kernel initialization */
	call	_riscv_init

	/* Should never reach here */
3:
	wfi
	j	3b

.section .bss
.align 16
_stack_bottom:
	.space 16384
_stack_top:
