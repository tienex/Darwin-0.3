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

#ifndef _RISCV_LIMITS_H_
#define _RISCV_LIMITS_H_

#define	CHAR_BIT	8		/* number of bits in a char */
#define	MB_LEN_MAX	6		/* Allow 31 bit UTF2 */

#define	CLK_TCK		100		/* ticks per second */

/*
 * According to ANSI (section 2.2.4.2), the values below must be usable by
 * #if preprocessing directives.  Additionally, the expression must have the
 * same type as would an expression that is an object of the corresponding
 * type converted according to the integral promotions.  The subtraction for
 * INT_MIN and LONG_MIN is so the value is not unsigned; 2147483648 is an
 * unsigned int for 32-bit two's complement ANSI compilers (section 3.1.3.2).
 * These numbers work for pcc as well.  The UINT_MAX and ULONG_MAX values
 * are written as hex so that GCC will be quiet about large integer constants.
 */
#define	SCHAR_MAX	127		/* min value for a signed char */
#define	SCHAR_MIN	(-128)		/* max value for a signed char */

#define	UCHAR_MAX	255		/* max value for an unsigned char */
#define	CHAR_MAX	127		/* max value for a char */
#define	CHAR_MIN	(-128)		/* min value for a char */

#define	USHRT_MAX	65535		/* max value for an unsigned short */
#define	SHRT_MAX	32767		/* max value for a short */
#define	SHRT_MIN	(-32768)	/* min value for a short */

#define	UINT_MAX	0xffffffff	/* max value for an unsigned int */
#define	INT_MAX		2147483647	/* max value for an int */
#define	INT_MIN		(-2147483647-1)	/* min value for an int */

#if defined(__riscv_xlen) && __riscv_xlen == 64
#define	ULONG_MAX	0xffffffffffffffffUL	/* max value for an unsigned long */
#define	LONG_MAX	9223372036854775807L	/* max value for a long */
#define	LONG_MIN	(-9223372036854775807L-1)	/* min value for a long */
#else
#define	ULONG_MAX	0xffffffffUL	/* max value for an unsigned long */
#define	LONG_MAX	2147483647L	/* max value for a long */
#define	LONG_MIN	(-2147483647L-1)	/* min value for a long */
#endif

#if !defined(_ANSI_SOURCE)
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define	SSIZE_MAX	LONG_MAX	/* max value for a ssize_t */
#else
#define	SSIZE_MAX	INT_MAX		/* max value for a ssize_t */
#endif

#if !defined(_POSIX_SOURCE)
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define	SIZE_T_MAX	ULONG_MAX	/* max value for a size_t */
#else
#define	SIZE_T_MAX	UINT_MAX	/* max value for a size_t */
#endif

/* GCC requires that quad constants be written as expressions. */
#define	UQUAD_MAX	((u_quad_t)0-1)	/* max value for a uquad_t */
					/* max value for a quad_t */
#define	QUAD_MAX	((quad_t)(UQUAD_MAX >> 1))
#define	QUAD_MIN	(-QUAD_MAX-1)	/* min value for a quad_t */

#endif /* !_POSIX_SOURCE */
#endif /* !_ANSI_SOURCE */

#endif /* _RISCV_LIMITS_H_ */
