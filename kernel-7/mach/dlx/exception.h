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
 * DLX Exception Types
 */

#ifndef	_MACH_DLX_EXCEPTION_H_
#define	_MACH_DLX_EXCEPTION_H_

/*
 * DLX Exception codes
 */
#define EXC_DLX_TLB_MISS_LOAD	1	/* TLB miss on load */
#define EXC_DLX_TLB_MISS_STORE	2	/* TLB miss on store */
#define EXC_DLX_TLB_MOD		3	/* TLB modification exception */
#define EXC_DLX_TLB_LOAD	4	/* TLB exception on load */
#define EXC_DLX_TLB_STORE	5	/* TLB exception on store */

#define EXC_TYPES_COUNT		6	/* Number of exception types */

#endif	/* _MACH_DLX_EXCEPTION_H_ */
