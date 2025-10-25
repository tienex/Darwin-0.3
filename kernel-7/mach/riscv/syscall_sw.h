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

#ifdef	KERNEL_PRIVATE

#ifndef	_MACH_RISCV_SYSCALL_SW_H_
#define _MACH_RISCV_SYSCALL_SW_H_

/*
 * System call conventions for RISC-V:
 * - Syscall number in a7
 * - Arguments in a0-a6
 * - Return value in a0
 * - Use ecall instruction to trap into kernel
 */

#define kernel_trap(trap_name, trap_number, number_args)	\
.globl _##trap_name						;\
_##trap_name:							;\
	li	a7, trap_number					;\
	ecall							;\
	ret

#endif	/* _MACH_RISCV_SYSCALL_SW_H_ */

#endif	/* KERNEL_PRIVATE */
