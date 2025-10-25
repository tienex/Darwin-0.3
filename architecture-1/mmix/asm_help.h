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
 *	File:	architecture/mmix/asm_help.h
 *
 *	This header file defines macros useful when writing assembly code
 *	for the MMIX processor.
 *
 *	MMIX calling conventions:
 *	- Arguments passed in $0-$7 (up to 8 arguments)
 *	- Return value in $0
 *	- Frame pointer in $253 (conventional, not required)
 *	- Stack pointer in $254 (conventional)
 *	- Return address saved by PUSHJ in rJ
 */

#ifndef	_ARCH_MMIX_ASM_HELP_H_
#define	_ARCH_MMIX_ASM_HELP_H_

#import	<architecture/mmix/reg_help.h>

#ifdef	__ASSEMBLER__

/* MMIX instructions must be aligned on 4-byte boundaries */
#define ALIGN						\
	.align	4

#define	ROUND_TO_STACK(len)				\
	(((len) + STACK_INCR - 1) / STACK_INCR * STACK_INCR)

#ifdef notdef
#define CALL_MCOUNT						\
	GET	$255,rJ						;\
	PUSHJ	$255,mcount					;\
	PUT	rJ,$255						;
#else
#define CALL_MCOUNT
#endif

/*
 * MMIX register conventions:
 * $0-$7:   Function arguments and temporaries
 * $8-$15:  Temporaries
 * $16-$23: Callee-saved registers
 * $24-$31: More temporaries
 * $253:    Frame pointer (FP)
 * $254:    Stack pointer (SP)
 * $255:    Temporary for system use
 */

/*
 * Prologue for functions that may call other functions.
 * Saves callee-saved registers and sets up a stack frame.
 */
#define NESTED_FUNCTION_PROLOGUE(localvarsize)			\
	.set	__framesize,ROUND_TO_STACK(localvarsize+64)	;\
	.set	__nested_function, 1				;\
	CALL_MCOUNT						\
	.if __framesize						;\
	  SUBU	$254,$254,__framesize				;\
	  STOU	$253,$254,0		/* Save FP */		;\
	  ADDU	$253,$254,__framesize	/* Set new FP */	;\
	.endif							;\
	STOU	$16,$254,8		/* Save callee regs */	;\
	STOU	$17,$254,16					;\
	STOU	$18,$254,24					;\
	STOU	$19,$254,32					;\
	STOU	$20,$254,40					;\
	STOU	$21,$254,48					;\
	STOU	$22,$254,56					;\
	STOU	$23,$254,64

/*
 * Prologue for functions that do not call other functions.
 * Does not save registers (function's responsibility).
 */
#define LEAF_FUNCTION_PROLOGUE(localvarsize)			\
	.set	__framesize,ROUND_TO_STACK(localvarsize)	;\
	.set	__nested_function, 0				;\
	CALL_MCOUNT						\
	.if __framesize						;\
	  SUBU	$254,$254,__framesize				;\
	  STOU	$253,$254,0					;\
	  ADDU	$253,$254,__framesize				;\
	.endif

/*
 * Epilogue for any function.
 */
#define FUNCTION_EPILOGUE					\
	.if __nested_function					;\
	  LDOU	$16,$254,8		/* Restore callee regs */;\
	  LDOU	$17,$254,16					;\
	  LDOU	$18,$254,24					;\
	  LDOU	$19,$254,32					;\
	  LDOU	$20,$254,40					;\
	  LDOU	$21,$254,48					;\
	  LDOU	$22,$254,56					;\
	  LDOU	$23,$254,64					;\
	.endif							;\
	.if __framesize						;\
	  LDOU	$253,$254,0		/* Restore FP */	;\
	  ADDU	$254,$254,__framesize				;\
	.endif							;\
	POP	0,0			/* Return */

/*
 * Macros for declaring procedures
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
 */

/*
 * IMPORT -- import symbol
 */
#define	IMPORT(name)					\
	.reference	name

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
 * LBSS -- declare local zero'ed storage
 */
#define	LBSS(name,size)					\
	.lcomm	name,size

/*
 * String definitions
 */
#define	STRING(name,string)				\
	.globl	name					;\
	.data						;\
name:	.asciz	string					;\
	.text

#define	STRINGD(name,string)				\
	.data						;\
name:	.asciz	string					;\
	.text

#endif	/* __ASSEMBLER__ */
#endif	/* _ARCH_MMIX_ASM_HELP_H_ */
