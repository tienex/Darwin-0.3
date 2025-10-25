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
 *	File:	mach/mmix/syscall_sw.h
 *	Author:	MMIX Architecture Port
 *
 *	This header file defines system call macros
 *	for the MMIX architecture.
 *
 */

#ifndef	_MACH_MMIX_SYSCALL_SW_H_
#define	_MACH_MMIX_SYSCALL_SW_H_

#import <architecture/mmix/asm_help.h>

/*
 * Mach system call traps.
 *
 * The first 8 system call args are passed by user-code in
 * $0 - $7. (For more than 8 args, additional args should be
 * passed in $16-$23 or on stack.)
 *
 * The system call number is in $255.
 *
 * NOTE: The kernel does not preserve caller-saves across system calls.
 * MMIX uses TRAP instruction to enter kernel mode.
 */

/*
 * kernel_trap_args_N -- Put the arguments in the right places.
 * Args are passed in $0 - $7, system call number in $255.
 */
#define	kernel_trap_args_0
#define	kernel_trap_args_1
#define	kernel_trap_args_2
#define	kernel_trap_args_3
#define	kernel_trap_args_4
#define	kernel_trap_args_5
#define	kernel_trap_args_6
#define	kernel_trap_args_7

/*
 * simple_kernel_trap -- Mach system calls with 8 or less args
 * Args are passed in $0 - $7, system call number in $255.
 * Use TRAP instruction to enter kernel.
 */
#define simple_kernel_trap(trap_name, trap_number)	\
	.globl	_##trap_name				@\
_##trap_name:						@\
	SETL	$255,trap_number			 @\
	TRAP	0,0,0					 @\
	POP	1,0					 @\
	END(trap_name)

#define kernel_trap_0(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_1(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_2(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_3(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_4(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_5(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_6(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_7(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_8(trap_name,trap_number)		 \
        simple_kernel_trap(trap_name,trap_number)

#define kernel_trap_9(trap_name,trap_number)		 \
        simple_kernel_trap(trap_name,trap_number)

#define kernel_trap(trap_name,trap_number,nargs)	 \
	kernel_trap_##nargs(trap_name,trap_number)

#endif	/* _MACH_MMIX_SYSCALL_SW_H_ */
