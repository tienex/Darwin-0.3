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
 *	File:	architecture/mips/asm_help.h
 *
 *	MIPS assembly language helper macros.
 */

#ifndef _ARCH_MIPS_ASM_HELP_H_
#define _ARCH_MIPS_ASM_HELP_H_

#if defined(__ASSEMBLER__)

/*
 * Standard function entry and exit macros
 */

/*
 * LEAF - declare a leaf function (non-nested, no stack frame needed)
 */
#define	LEAF(name)			\
	.globl	name;			\
	.ent	name;			\
name:;					\
	.frame	sp, 0, ra

/*
 * XLEAF - declare an alternate entry point for a leaf function
 */
#define	XLEAF(name)			\
	.globl	name;			\
	.aent	name;			\
name:

/*
 * NESTED - declare a nested function (may call other functions, needs frame)
 */
#define	NESTED(name, framesize, returnreg)	\
	.globl	name;			\
	.ent	name;			\
name:;					\
	.frame	sp, framesize, returnreg

/*
 * XNESTED - declare an alternate entry point for a nested function
 */
#define	XNESTED(name)			\
	.globl	name;			\
	.aent	name;			\
name:

/*
 * END - mark the end of a function
 */
#define	END(name)			\
	.end	name

/*
 * EXPORT - export a symbol
 */
#define	EXPORT(name)			\
	.globl	name;			\
name:

/*
 * BSS - allocate space in BSS
 */
#define	BSS(name, size)			\
	.comm	name, size

/*
 * LBSS - allocate local space in BSS
 */
#define	LBSS(name, size)		\
	.lcomm	name, size

/*
 * RODATA - switch to read-only data section
 */
#define	RODATA				\
	.rdata

/*
 * DATA - switch to data section
 */
#define	DATA				\
	.data

/*
 * TEXT - switch to text section
 */
#define	TEXT				\
	.text

/*
 * String literal support
 */
#define	STRING(name, string)		\
	.rdata;				\
name:;					\
	.asciiz	string;			\
	.text

/*
 * Alignment directives
 */
#define	ALIGN_DATA	.align	3	/* 8-byte alignment */
#define	ALIGN_TEXT	.align	2	/* 4-byte alignment (instruction) */

/*
 * Macros for creating position-independent code
 */
#if defined(__mips64)
#define	PTR_LA		dla		/* load address (64-bit) */
#define	PTR_L		ld		/* load pointer (64-bit) */
#define	PTR_S		sd		/* store pointer (64-bit) */
#define	PTR_SUBU	dsubu		/* subtract pointer (64-bit) */
#define	PTR_ADDU	daddu		/* add pointer (64-bit) */
#define	PTR_SLL		dsll		/* shift left logical (64-bit) */
#define	PTR_SRL		dsrl		/* shift right logical (64-bit) */
#define	PTR_SRA		dsra		/* shift right arithmetic (64-bit) */
#define	PTR_SIZE	8		/* pointer size in bytes */
#define	PTR_SHIFT	3		/* log2(PTR_SIZE) */
#else
#define	PTR_LA		la		/* load address (32-bit) */
#define	PTR_L		lw		/* load pointer (32-bit) */
#define	PTR_S		sw		/* store pointer (32-bit) */
#define	PTR_SUBU	subu		/* subtract pointer (32-bit) */
#define	PTR_ADDU	addu		/* add pointer (32-bit) */
#define	PTR_SLL		sll		/* shift left logical (32-bit) */
#define	PTR_SRL		srl		/* shift right logical (32-bit) */
#define	PTR_SRA		sra		/* shift right arithmetic (32-bit) */
#define	PTR_SIZE	4		/* pointer size in bytes */
#define	PTR_SHIFT	2		/* log2(PTR_SIZE) */
#endif

/*
 * Cache operation macros
 */
#define	CACHE_OP(op, reg, offset)	\
	cache	op, offset(reg)

/*
 * NOP macros for delay slots and alignment
 */
#define	NOP		nop
#define	NOP4		nop; nop; nop; nop
#define	NOP8		NOP4; NOP4

/*
 * Conditional assembly based on ISA level
 */
#if defined(__mips64)
#define	IF_MIPS64(code)		code
#define	IF_MIPS32(code)
#else
#define	IF_MIPS64(code)
#define	IF_MIPS32(code)		code
#endif

#endif	/* __ASSEMBLER__ */

#endif /* _ARCH_MIPS_ASM_HELP_H_ */
