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

#ifndef	_MIPS_TYPES_H_
#define	_MIPS_TYPES_H_

/*
 * Basic integral types for MIPS.
 * Varies between MIPS32 and MIPS64.
 */

#if defined(_MIPS64) || defined(__mips64)
/* MIPS64: 64-bit longs and pointers */
typedef	long			register_t;
typedef unsigned long		u_register_t;
#else
/* MIPS32: 32-bit longs and pointers */
typedef	int			register_t;
typedef unsigned int		u_register_t;
#endif

#endif	/* _MIPS_TYPES_H_ */
