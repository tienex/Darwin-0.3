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
 *	File:	architecture/riscv/reg_help.h
 *	Author:	Adapted for RISC-V
 *
 *	This header file defines cpp macros useful for defining
 *	machine register and doing machine-level operations.
 */

#ifndef	_ARCH_RISCV_REG_HELP_H_
#define	_ARCH_RISCV_REG_HELP_H_

/* Bitfield definition aid */
#define	BITS_WIDTH(msb, lsb)	((msb)-(lsb)+1)
#define	BIT_WIDTH(pos)		(1)	/* mostly to record the position */

/* Mask creation */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define	MKMASK(width, offset)	(((unsigned long)-1)>>(64-(width))<<(offset))
#define	BITSMASK(msb, lsb)	MKMASK(BITS_WIDTH(msb, lsb), lsb & 0x3f)
#define	BITMASK(pos)		MKMASK(BIT_WIDTH(pos), pos & 0x3f)
#else
#define	MKMASK(width, offset)	(((unsigned)-1)>>(32-(width))<<(offset))
#define	BITSMASK(msb, lsb)	MKMASK(BITS_WIDTH(msb, lsb), lsb & 0x1f)
#define	BITMASK(pos)		MKMASK(BIT_WIDTH(pos), pos & 0x1f)
#endif

/* Register addresses */
#if	__ASSEMBLER__
# define	REG_ADDR(type, addr)	(addr)
#else	/* __ASSEMBLER__ */
# define	REG_ADDR(type, addr)	(*(volatile type *)(addr))
#endif	/* __ASSEMBLER__ */

/* Cast a register to be an unsigned */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define	CONTENTS(foo)	(*(unsigned long *) &(foo))
#else
#define	CONTENTS(foo)	(*(unsigned *) &(foo))
#endif

/* Stack pointer must always be a multiple of 16 for RISC-V ABI */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define	STACK_INCR	16
#else
#define	STACK_INCR	16
#endif
#define	ROUND_FRAME(x)	((((unsigned long)(x)) + STACK_INCR - 1) & ~(STACK_INCR-1))

/* STRINGIFY -- perform all possible substitutions, then stringify */
#define	__STR(x)	#x		/* just a helper macro */
#define	STRINGIFY(x)	__STR(x)

/*
 * REG_PAIR_DEF -- define a register pair
 * Register pairs are appropriately aligned to allow access via
 * double-word loads and stores.
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
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define	REG_PAIR_DEF(type0, name0, type1, name1)		\
	struct {						\
		type0	name0 __attribute__(( aligned(16) ));	\
		type1	name1;					\
	} name0##_##name1
#else
#define	REG_PAIR_DEF(type0, name0, type1, name1)		\
	struct {						\
		type0	name0 __attribute__(( aligned(8) ));	\
		type1	name1;					\
	} name0##_##name1
#endif

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

#endif	/* _ARCH_RISCV_REG_HELP_H_ */
