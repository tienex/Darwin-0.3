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
 * MMIX Floating Point Unit
 *
 * MMIX uses IEEE 754 floating point with integrated FPU.
 * All floating point state is maintained in the rA (arithmetic status) register.
 */

#ifndef _ARCHITECTURE_MMIX_FPU_H_
#define _ARCHITECTURE_MMIX_FPU_H_

/*
 * MMIX rA register (arithmetic status register)
 * This register contains all floating point exception flags.
 */
typedef struct mmix_fp_status {
    unsigned long long		event_bits;	/* Exception event bits */

    /* Exception enable flags */
    unsigned int		enable_inexact		:1,
				enable_underflow	:1,
				enable_overflow		:1,
				enable_zerodivide	:1,
				enable_invalid		:1,
				enable_intoverflow	:1,

    /* Exception flag bits */
				flag_inexact		:1,
				flag_underflow		:1,
				flag_overflow		:1,
				flag_zerodivide		:1,
				flag_invalid		:1,
				flag_intoverflow	:1,

    /* Rounding mode */
				round_mode		:2,
#define MMIX_FP_RND_NEAR	0	/* Round to nearest */
#define MMIX_FP_RND_ZERO	1	/* Round toward zero */
#define MMIX_FP_RND_UP		2	/* Round toward +infinity */
#define MMIX_FP_RND_DOWN	3	/* Round toward -infinity */

				reserved		:14;
} mmix_fp_status_t;

/*
 * Floating point exception types
 */
#define MMIX_FP_INEXACT		0x01	/* Inexact result */
#define MMIX_FP_UNDERFLOW	0x02	/* Underflow */
#define MMIX_FP_OVERFLOW	0x04	/* Overflow */
#define MMIX_FP_ZERODIVIDE	0x08	/* Division by zero */
#define MMIX_FP_INVALID		0x10	/* Invalid operation */
#define MMIX_FP_INTOVERFLOW	0x20	/* Integer overflow */

/*
 * MMIX floating point state
 * Much simpler than x86 as FP is integrated into general registers.
 */
typedef struct mmix_fp_state {
    mmix_fp_status_t		status;		/* rA register contents */
    unsigned long long		rE;		/* Epsilon register */
} mmix_fp_state_t;

/*
 * IEEE 754 double precision format (used by MMIX)
 * All MMIX floating point is 64-bit IEEE 754 double precision.
 */
typedef struct mmix_fp_double {
    unsigned long long		mantissa	:52,
				exponent	:11,
				sign		:1;
} mmix_fp_double_t;

#endif /* _ARCHITECTURE_MMIX_FPU_H_ */
