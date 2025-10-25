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
 * IA64 Machine Parameters
 */

#ifndef _MACH_IA64_MACH_PARAM_H_
#define _MACH_IA64_MACH_PARAM_H_

/*
 * Page size is 16KB on IA64 (configurable, but 16KB is optimal)
 */
#ifndef	PAGE_SHIFT
#define PAGE_SHIFT		14
#endif

#ifndef	PAGE_SIZE
#define PAGE_SIZE		(1 << PAGE_SHIFT)
#endif

#ifndef	PAGE_MASK
#define PAGE_MASK		(PAGE_SIZE - 1)
#endif

#define IA64_PGBYTES		PAGE_SIZE
#define IA64_PGSHIFT		PAGE_SHIFT

/*
 * Maximum number of CPUs supported
 */
#define NCPUS			64

/*
 * Cache line size (typical for Itanium)
 */
#define CACHE_LINE_SIZE		128

#endif /* _MACH_IA64_MACH_PARAM_H_ */
