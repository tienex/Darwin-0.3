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
 *	File:	architecture/mips/reg_help.h
 *
 *	MIPS-specific macros and inlines for defining machine registers.
 */

#ifndef _ARCH_MIPS_REG_HELP_H_
#define _ARCH_MIPS_REG_HELP_H_

#if defined(__ASSEMBLER__)
/*
 * MIPS General Purpose Register Usage Aliases
 * Following standard MIPS ABI conventions
 */
#define	zero		$0		// always zero
#define	at		$1		// assembler temporary
#define	v0		$2		// return value 0
#define	v1		$3		// return value 1
#define	a0		$4		// argument 0
#define	a1		$5		// argument 1
#define	a2		$6		// argument 2
#define	a3		$7		// argument 3
#define	t0		$8		// temporary 0 (caller-saved)
#define	t1		$9		// temporary 1 (caller-saved)
#define	t2		$10		// temporary 2 (caller-saved)
#define	t3		$11		// temporary 3 (caller-saved)
#define	t4		$12		// temporary 4 (caller-saved)
#define	t5		$13		// temporary 5 (caller-saved)
#define	t6		$14		// temporary 6 (caller-saved)
#define	t7		$15		// temporary 7 (caller-saved)
#define	s0		$16		// saved 0 (callee-saved)
#define	s1		$17		// saved 1 (callee-saved)
#define	s2		$18		// saved 2 (callee-saved)
#define	s3		$19		// saved 3 (callee-saved)
#define	s4		$20		// saved 4 (callee-saved)
#define	s5		$21		// saved 5 (callee-saved)
#define	s6		$22		// saved 6 (callee-saved)
#define	s7		$23		// saved 7 (callee-saved)
#define	t8		$24		// temporary 8 (caller-saved)
#define	t9		$25		// temporary 9 (caller-saved)
#define	k0		$26		// kernel reserved 0
#define	k1		$27		// kernel reserved 1
#define	gp		$28		// global pointer (callee-saved)
#define	sp		$29		// stack pointer (callee-saved)
#define	fp		$30		// frame pointer (callee-saved)
#define	s8		$30		// saved 8 (same as fp)
#define	ra		$31		// return address

/*
 * Conversion of GPR aliases to register numbers
 */
#define	GPR_ZERO	0		// always zero
#define	GPR_AT		1		// assembler temporary
#define	GPR_V0		2		// return value 0
#define	GPR_V1		3		// return value 1
#define	GPR_A0		4		// argument 0
#define	GPR_A1		5		// argument 1
#define	GPR_A2		6		// argument 2
#define	GPR_A3		7		// argument 3
#define	GPR_T0		8		// temporary 0
#define	GPR_T1		9		// temporary 1
#define	GPR_T2		10		// temporary 2
#define	GPR_T3		11		// temporary 3
#define	GPR_T4		12		// temporary 4
#define	GPR_T5		13		// temporary 5
#define	GPR_T6		14		// temporary 6
#define	GPR_T7		15		// temporary 7
#define	GPR_S0		16		// saved 0
#define	GPR_S1		17		// saved 1
#define	GPR_S2		18		// saved 2
#define	GPR_S3		19		// saved 3
#define	GPR_S4		20		// saved 4
#define	GPR_S5		21		// saved 5
#define	GPR_S6		22		// saved 6
#define	GPR_S7		23		// saved 7
#define	GPR_T8		24		// temporary 8
#define	GPR_T9		25		// temporary 9
#define	GPR_K0		26		// kernel reserved 0
#define	GPR_K1		27		// kernel reserved 1
#define	GPR_GP		28		// global pointer
#define	GPR_SP		29		// stack pointer
#define	GPR_FP		30		// frame pointer
#define	GPR_S8		30		// saved 8 (same as fp)
#define	GPR_RA		31		// return address

/*
 * MIPS Floating Point Register names
 * MIPS has 32 FP registers, used as singles or paired as doubles
 */
#define	f0		$f0		// FP return value / temporary
#define	f1		$f1		// FP temp (odd)
#define	f2		$f2		// FP return value / temporary
#define	f3		$f3		// FP temp (odd)
#define	f4		$f4		// FP temporary
#define	f5		$f5		// FP temp (odd)
#define	f6		$f6		// FP temporary
#define	f7		$f7		// FP temp (odd)
#define	f8		$f8		// FP temporary
#define	f9		$f9		// FP temp (odd)
#define	f10		$f10		// FP temporary
#define	f11		$f11		// FP temp (odd)
#define	f12		$f12		// FP argument 0
#define	f13		$f13		// FP arg 0 (odd)
#define	f14		$f14		// FP argument 1
#define	f15		$f15		// FP arg 1 (odd)
#define	f16		$f16		// FP temporary
#define	f17		$f17		// FP temp (odd)
#define	f18		$f18		// FP temporary
#define	f19		$f19		// FP temp (odd)
#define	f20		$f20		// FP saved (callee-saved)
#define	f21		$f21		// FP saved (odd)
#define	f22		$f22		// FP saved (callee-saved)
#define	f23		$f23		// FP saved (odd)
#define	f24		$f24		// FP saved (callee-saved)
#define	f25		$f25		// FP saved (odd)
#define	f26		$f26		// FP saved (callee-saved)
#define	f27		$f27		// FP saved (odd)
#define	f28		$f28		// FP saved (callee-saved)
#define	f29		$f29		// FP saved (odd)
#define	f30		$f30		// FP saved (callee-saved)
#define	f31		$f31		// FP saved (odd)

/*
 * Conversion of FPR aliases to register numbers
 */
#define	FPR_F0		0
#define	FPR_F1		1
#define	FPR_F2		2
#define	FPR_F3		3
#define	FPR_F4		4
#define	FPR_F5		5
#define	FPR_F6		6
#define	FPR_F7		7
#define	FPR_F8		8
#define	FPR_F9		9
#define	FPR_F10		10
#define	FPR_F11		11
#define	FPR_F12		12
#define	FPR_F13		13
#define	FPR_F14		14
#define	FPR_F15		15
#define	FPR_F16		16
#define	FPR_F17		17
#define	FPR_F18		18
#define	FPR_F19		19
#define	FPR_F20		20
#define	FPR_F21		21
#define	FPR_F22		22
#define	FPR_F23		23
#define	FPR_F24		24
#define	FPR_F25		25
#define	FPR_F26		26
#define	FPR_F27		27
#define	FPR_F28		28
#define	FPR_F29		29
#define	FPR_F30		30
#define	FPR_F31		31

#endif	/* __ASSEMBLER__ */

/* Bitfield definition aid */
#define	BITS_WIDTH(msb, lsb)	((msb)-(lsb)+1)
#define	BIT_WIDTH(pos)		(1)	/* mostly to record the position */

/* Mask creation */
#define	MKMASK(width, offset)	(((unsigned)-1)>>(32-(width))<<(offset))
#define	BITSMASK(msb, lsb)	MKMASK(BITS_WIDTH(msb, lsb), lsb & 0x1f)
#define	BITMASK(pos)		MKMASK(BIT_WIDTH(pos), pos & 0x1f)

/* Register addresses */
#if	__ASSEMBLER__
# define	REG_ADDR(type, addr)	(addr)
#else	/* ! __ASSEMBLER__ */
# define	REG_ADDR(type, addr)	(*(volatile type *)(addr))
#endif	/* __ASSEMBLER__ */

/* Cast a register to be an unsigned */
#define	CONTENTS(foo)	(*(unsigned *) &(foo))

/* STRINGIFY -- perform all possible substitutions, then stringify */
#define	__STR(x)	#x		/* just a helper macro */
#define	STRINGIFY(x)	__STR(x)

/*
 * Stack pointer must always be a multiple of 8 (MIPS32) or 16 (MIPS64)
 */
#if defined(_MIPS64) || defined(__mips64)
#define	STACK_INCR	16
#else
#define	STACK_INCR	8
#endif
#define	ROUND_FRAME(x)	((((unsigned)(x)) + STACK_INCR - 1) & ~(STACK_INCR-1))

#endif /* _ARCH_MIPS_REG_HELP_H_ */
