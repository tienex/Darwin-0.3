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
 * MIPS:	For setjmp/longjmp (kernel version).
 *
 * Kernel label_t structure matches the jmp_buf layout:
 * 12 words for s0-s7, s8/fp, sp, ra, gp
 */

#ifndef _BSD_MIPS_LABEL_T_H_
#define _BSD_MIPS_LABEL_T_H_

#if defined(_MIPS64) || defined(__mips64)
typedef struct label_t {
	long	val[12];
} label_t;
#else
typedef struct label_t {
	int	val[12];
} label_t;
#endif

#endif	/* _BSD_MIPS_LABEL_T_H_ */
