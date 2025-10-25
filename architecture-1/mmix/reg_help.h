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
 *	File:	architecture/mmix/reg_help.h
 *
 *	This header file defines cpp macros useful for defining
 *	machine register and doing machine-level operations.
 */

#ifndef	_ARCH_MMIX_REG_HELP_H_
#define	_ARCH_MMIX_REG_HELP_H_

/* Bitfield definition aid */
#define	BITS_WIDTH(msb, lsb)	((msb)-(lsb)+1)
#define	BIT_WIDTH(pos)		(1)	/* mostly to record the position */

/* Mask creation */
#define	MKMASK(width, offset)	(((unsigned long long)-1)>>(64-(width))<<(offset))
#define	BITSMASK(msb, lsb)	MKMASK(BITS_WIDTH(msb, lsb), lsb & 0x3f)
#define	BITMASK(pos)		MKMASK(BIT_WIDTH(pos), pos & 0x3f)

/* 32-bit mask creation for instruction fields */
#define	MKMASK32(width, offset)	(((unsigned int)-1)>>(32-(width))<<(offset))
#define	BITSMASK32(msb, lsb)	MKMASK32(BITS_WIDTH(msb, lsb), lsb & 0x1f)
#define	BITMASK32(pos)		MKMASK32(BIT_WIDTH(pos), pos & 0x1f)

/* Register addresses */
#if	__ASSEMBLER__
# define	REG_ADDR(type, addr)	(addr)
#else	/* __ASSEMBLER__ */
# define	REG_ADDR(type, addr)	(*(volatile type *)(addr))
#endif	/* __ASSEMBLER__ */

/* Cast a register to be an unsigned */
#define	CONTENTS(foo)	(*(unsigned long long *) &(foo))

/* Stack pointer must always be a multiple of 8 (MMIX is 64-bit) */
#define	STACK_INCR	8
#define	ROUND_FRAME(x)	((((unsigned long long)(x)) + STACK_INCR - 1) & ~(STACK_INCR-1))

/* STRINGIFY -- perform all possible substitutions, then stringify */
#define	__STR(x)	#x		/* just a helper macro */
#define	STRINGIFY(x)	__STR(x)

/*
 * REG_PAIR_DEF -- define a register pair
 * For MMIX, pairs are 128-bit aligned (two 64-bit registers).
 *
 * Usage:
 *	struct foo {
 *		REG_PAIR_DEF(
 *			bar_t *,	barp,
 *			afu_t,		afu
 *		);
 *	};
 *
 * Access to individual entries of the pair is via the REG_PAIR
 * macro (below).
 */
#define	REG_PAIR_DEF(type0, name0, type1, name1)		\
	struct {						\
		type0	name0 __attribute__(( aligned(16) ));	\
		type1	name1;					\
	} name0##_##name1

/*
 * REG_PAIR -- Macro to define names for accessing individual registers
 * of register pairs.
 *
 * Usage:
 *	arg0 is first element of pair
 *	arg1 is second element of pair
 *	arg2 is desired element of pair
 * eg:
 *	#define	foo_barp	REG_PAIR(barp, afu, afu)
 */
#define	REG_PAIR(name0, name1, the_name)			\
	name0##_##name1.the_name

/*
 * MMIX register number helpers
 */
#define MMIX_REG_NUM(r)		((r) & 0xFF)		/* Extract register number */
#define MMIX_IS_GLOBAL(r)	((r) >= rG_value)	/* Check if global register */
#define MMIX_IS_LOCAL(r)	((r) < rL_value && (r) >= rG_value) /* Local register */
#define MMIX_IS_SPECIAL(r)	((r) >= 32)		/* Special register */

/*
 * Special register access helpers
 */
#define MMIX_GET_SPECIAL(reg)	/* Inline assembly for GET instruction */ \
	({ unsigned long long __val;					\
	   __asm__ volatile ("get %0," #reg : "=r" (__val));		\
	   __val; })

#define MMIX_PUT_SPECIAL(reg, val)	/* Inline assembly for PUT instruction */ \
	do {								\
	   __asm__ volatile ("put " #reg ",%0" : : "r" (val));		\
	} while(0)

#endif	/* _ARCH_MMIX_REG_HELP_H_ */
