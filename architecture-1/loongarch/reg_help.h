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
 *	File:	architecture/loongarch/reg_help.h
 *
 *	LoongArch-specific macros and inlines for defining machine registers.
 */

#ifndef _ARCH_LOONGARCH_REG_HELP_H_
#define _ARCH_LOONGARCH_REG_HELP_H_

#if defined(__ASSEMBLER__)
/*
 * General Purpose Register Usage Aliases
 */
#define	zero		r0		// always zero
#define	ra		r1		// return address
#define	tp		r2		// thread pointer
#define	sp		r3		// stack pointer
#define	a0		r4		// argument 0, return value 0
#define	a1		r5		// argument 1, return value 1
#define	a2		r6		// argument 2
#define	a3		r7		// argument 3
#define	a4		r8		// argument 4
#define	a5		r9		// argument 5
#define	a6		r10		// argument 6
#define	a7		r11		// argument 7
#define	t0		r12		// temporary 0
#define	t1		r13		// temporary 1
#define	t2		r14		// temporary 2
#define	t3		r15		// temporary 3
#define	t4		r16		// temporary 4
#define	t5		r17		// temporary 5
#define	t6		r18		// temporary 6
#define	t7		r19		// temporary 7
#define	t8		r20		// temporary 8
#define	fp		r22		// frame pointer
#define	s0		r23		// saved register 0
#define	s1		r24		// saved register 1
#define	s2		r25		// saved register 2
#define	s3		r26		// saved register 3
#define	s4		r27		// saved register 4
#define	s5		r28		// saved register 5
#define	s6		r29		// saved register 6
#define	s7		r30		// saved register 7
#define	s8		r31		// saved register 8

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
 * Stack pointer must always be a multiple of 16
 */
#define	STACK_INCR	16
#define	ROUND_FRAME(x)	((((unsigned)(x)) + STACK_INCR - 1) & ~(STACK_INCR-1))

#endif /* _ARCH_LOONGARCH_REG_HELP_H_ */
