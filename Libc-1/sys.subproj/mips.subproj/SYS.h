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
 *	File:	SYS.h
 *
 *	Definition of the user side of the UNIX system call interface
 *	for MIPS.
 *
 *	System calls return with error indication in v0:
 *		v0 == -1: Error (errno in v0)
 *		v0 >= 0:  Success (return value in v0)
 */

#define KERNEL_PRIVATE	1

/*
 * Header files.
 */
#include <architecture/mips/asm_help.h>
#include <mach/mips/syscall_sw.h>
#include <mach/mips/exception.h>
#include <sys/syscall.h>

/*
 * Macros for MIPS system call wrappers
 */

/*
 * SYSCALL - define a system call with error handling
 *
 * MIPS syscall convention:
 * - syscall number in v0
 * - arguments in a0-a3, rest on stack
 * - return value in v0
 * - error flag: v0 == -1 indicates error
 */
#define	SYSCALL(name, nargs)			\
	.globl	cerror;				\
LEAF(name);					\
	li	v0, SYS_##name;			\
	syscall;				\
	beqz	a3, 1f;				\
	nop;					\
	la	t9, cerror;			\
	jr	t9;				\
	nop;					\
1:

#define	SYSCALL_NONAME(name, nargs)		\
	.globl	cerror;				\
	li	v0, SYS_##name;			\
	syscall;				\
	beqz	a3, 1f;				\
	nop;					\
	la	t9, cerror;			\
	jr	t9;				\
	nop;					\
1:

#define	PSEUDO(pseudo, name, nargs)		\
LEAF(pseudo);					\
	SYSCALL_NONAME(name, nargs)

/*
 * RSYSCALL - simple system call with just a return
 */
#define	RSYSCALL(name, nargs)			\
	SYSCALL(name, nargs);			\
	jr	ra;				\
	nop;					\
	END(name)

/*
 * ENTRY - define a function entry point
 */
#ifndef ENTRY
#define ENTRY(name)				\
	.globl	name;				\
	.ent	name;				\
name:
#endif

/*
 * END - mark end of function
 */
#ifndef END
#define END(name)				\
	.end	name
#endif
