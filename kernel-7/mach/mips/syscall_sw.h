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
 *	File:	mach/mips/syscall_sw.h
 *
 *	This header file defines system call macros
 *	for the MIPS family processors.
 */

#ifndef	_MACH_MIPS_SYSCALL_SW_H_
#define	_MACH_MIPS_SYSCALL_SW_H_

#import <architecture/mips/asm_help.h>

/*
 * Mach system call traps.
 *
 * The first 4 system call args are passed by user-code in a0-a3.
 * Additional args (if any) are passed on the stack.
 *
 * The system call number is in v0.
 *
 * MIPS uses the 'syscall' instruction to trap to the kernel.
 * The return value is in v0, with v1 as a secondary return register.
 */

/*
 * kernel_trap_args_N -- Put the arguments in the right places.
 * Args are passed in a0 - a3, with additional args on stack.
 * System call number in v0.
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
 * simple_kernel_trap -- Mach system calls
 * Args are passed in a0 - a3, system call number in v0.
 * Do a "syscall" instruction to enter kernel.
 */
#define simple_kernel_trap(trap_name, trap_number)	\
	LEAF(trap_name)					 @\
	li	v0, trap_number				 @\
	syscall						 @\
	jr	ra					 @\
	nop						 @\
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

#endif	/* _MACH_MIPS_SYSCALL_SW_H_ */
