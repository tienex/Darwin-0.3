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
 *	File:	architecture/riscv/asm_help.h
 *	Author:	Adapted for RISC-V
 *
 *	This header file defines macros useful when writing assembly code
 *	for the RISC-V processor family.
 */

#ifndef	_ARCH_RISCV_ASM_HELP_H_
#define	_ARCH_RISCV_ASM_HELP_H_

#import	<architecture/riscv/reg_help.h>

#ifdef	__ASSEMBLER__

/* Alignment directive - align to 4-byte boundary */
#define ALIGN						\
	.align	2

#define	ROUND_TO_STACK(len)				\
	(((len) + STACK_INCR - 1) / STACK_INCR * STACK_INCR)

#ifdef notdef
#define CALL_MCOUNT						\
	/* RISC-V mcount support - to be implemented */
#else
#define CALL_MCOUNT
#endif

/*
 * Prologue for functions that may call other functions.  Saves
 * registers and sets up a C frame.
 */
#define NESTED_FUNCTION_PROLOGUE(localvarsize)			\
	.set	__framesize,ROUND_TO_STACK(localvarsize)	;\
	.set	__nested_function, 1				;\
	CALL_MCOUNT						\
	.if __framesize						;\
	  addi	sp, sp, -(__framesize + 48)			;\
	  sd	ra, (__framesize + 40)(sp)			;\
	  sd	s0, (__framesize + 32)(sp)			;\
	  addi	s0, sp, __framesize + 48			;\
	.endif							;\
	sd	s1, 0(sp)					;\
	sd	s2, 8(sp)					;\
	sd	s3, 16(sp)					;\
	sd	s4, 24(sp)					;\
	sd	s5, 32(sp)

/*
 * Prologue for functions that do not call other functions.  Does not
 * save registers (this is the functions responsibility).  Does set
 * up a C frame.
 */
#define LEAF_FUNCTION_PROLOGUE(localvarsize)			\
	.set	__framesize,ROUND_TO_STACK(localvarsize)	;\
	.set	__nested_function, 0				;\
	CALL_MCOUNT						\
	.if __framesize						;\
	  addi	sp, sp, -(__framesize + 16)			;\
	  sd	ra, (__framesize + 8)(sp)			;\
	  sd	s0, __framesize(sp)				;\
	  addi	s0, sp, __framesize + 16			;\
	.endif

/*
 * Epilogue for any function.
 */
#define FUNCTION_EPILOGUE					\
	.if __nested_function					;\
	  ld	s5, 32(sp)					;\
	  ld	s4, 24(sp)					;\
	  ld	s3, 16(sp)					;\
	  ld	s2, 8(sp)					;\
	  ld	s1, 0(sp)					;\
	.endif							;\
	.if __framesize						;\
	  ld	s0, __framesize(sp)				;\
	  ld	ra, (__framesize + 8)(sp)			;\
	  addi	sp, sp, __framesize + 16			;\
	.endif							;\
	ret

/*
 * Macros for declaring procedures
 *
 * Use of these macros allows ctags to have a predictable way
 * to find various types of declarations.  They also simplify
 * inserting appropriate symbol table information.
 */

/*
 * TEXT -- declare start of text segment
 */
#define	TEXT						\
	.text

/*
 * DATA -- declare start of data segment
 */
#define DATA						\
	.data

/*
 * LEAF -- declare global leaf procedure
 * NOTE: Control SHOULD NOT FLOW into a LEAF!  A LEAF should only
 * be jumped to.  (A leaf may do an align.)  Use a LABEL() if you
 * need control to flow into the label.
 */
#define	LEAF(name, localvarsize)			\
	.globl	name					;\
	ALIGN						;\
name:							;\
	LEAF_FUNCTION_PROLOGUE(localvarsize)

/*
 * X_LEAF -- declare alternate global label for leaf
 */
#define	X_LEAF(name, value)				\
	.globl	name					;\
	.set	name,value

/*
 * P_LEAF -- declare private leaf procedure
 */
#define	P_LEAF(name, localvarsize)			\
	ALIGN						;\
name:							;\
	LEAF_FUNCTION_PROLOGUE(localvarsize)

/*
 * LABEL -- declare a global code label
 * MUST be used (rather than LEAF, NESTED, etc) if control
 * "flows into" the label.
 */
#define	LABEL(name)					\
	.globl	name					;\
name:

/*
 * NESTED -- declare procedure that invokes other procedures
 */
#define	NESTED(name, localvarsize)			\
	.globl	name					;\
	ALIGN						;\
name:							;\
	NESTED_FUNCTION_PROLOGUE(localvarsize)

/*
 * X_NESTED -- declare alternate global label for nested proc
 */
#define	X_NESTED(name, value)				\
	.globl	name					;\
	.set	name,value

/*
 * P_NESTED -- declare private nested procedure
 */
#define	P_NESTED(name, localvarsize)			\
	ALIGN						;\
name:							;\
	NESTED_FUNCTION_PROLOGUE(localvarsize)

/*
 * END -- mark end of procedure
 */
#define	END(name)					\
	FUNCTION_EPILOGUE

/*
 * Storage definition macros
 * The main purpose of these is to allow an easy handle for ctags
 */

/*
 * IMPORT -- import symbol
 */
#define	IMPORT(name)					\
	.extern	name

/*
 * ABS -- declare global absolute symbol
 */
#define	ABS(name, value)				\
	.globl	name					;\
	.set	name,value

/*
 * P_ABS -- declare private absolute symbol
 */
#define	P_ABS(name, value)				\
	.set	name,value

/*
 * EXPORT -- declare global label for data
 */
#define	EXPORT(name)					\
	.globl	name					;\
name:

/*
 * BSS -- declare global zero'ed storage
 */
#define	BSS(name,size)					\
	.comm	name,size

/*
 * P_BSS -- declare private zero'ed storage
 */
#define	P_BSS(name,size)				\
	.lcomm	name,size

/*
 * External symbol references for RISC-V
 */
#if defined(__DYNAMIC__)
/* PIC support for RISC-V - to be implemented as needed */
#define CALL_EXTERN(func)				\
	call	func

#define BRANCH_EXTERN(func)				\
	j	func

#else
#define CALL_EXTERN(func)	call	func
#define BRANCH_EXTERN(func)	j	func
#endif

#endif	/* __ASSEMBLER__ */

#endif	/* _ARCH_RISCV_ASM_HELP_H_ */
