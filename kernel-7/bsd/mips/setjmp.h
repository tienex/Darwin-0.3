/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * MIPS setjmp buffer definition
 */

#ifndef _BSD_MIPS_SETJMP_H_
#define _BSD_MIPS_SETJMP_H_

/*
 * jmp_buf layout for MIPS:
 * 0-7:   s0-s7 (callee-saved registers)
 * 8:     s8/fp (frame pointer)
 * 9:     sp (stack pointer)
 * 10:    ra (return address)
 * 11:    gp (global pointer)
 * Total: 12 words (48 bytes for MIPS32, 96 bytes for MIPS64)
 */

#if defined(_MIPS64) || defined(__mips64)
#define _JBLEN	12	/* Size of jmp_buf in longs (64-bit) */
typedef long jmp_buf[_JBLEN];
#else
#define _JBLEN	12	/* Size of jmp_buf in ints (32-bit) */
typedef int jmp_buf[_JBLEN];
#endif

/*
 * For signal-saving setjmp/longjmp
 */
#if defined(_MIPS64) || defined(__mips64)
#define _SIGJBLEN	(12 + 2)	/* jmp_buf + signal mask */
typedef long sigjmp_buf[_SIGJBLEN];
#else
#define _SIGJBLEN	(12 + 2)	/* jmp_buf + signal mask */
typedef int sigjmp_buf[_SIGJBLEN];
#endif

#endif /* _BSD_MIPS_SETJMP_H_ */
