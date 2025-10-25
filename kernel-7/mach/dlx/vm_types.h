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
 * DLX VM Type Definitions
 * Based on ETH DLXSIM architecture
 */

#ifndef	_MACH_DLX_VM_TYPES_H_
#define	_MACH_DLX_VM_TYPES_H_

/*
 * DLX is a 32-bit architecture
 */
typedef unsigned int		natural_t;
typedef int			integer_t;

typedef natural_t		vm_offset_t;
typedef natural_t		vm_size_t;

/*
 * DLX page size is 4KB (4096 bytes)
 */
#define DLX_PGBYTES		4096		/* bytes per DLX page */
#define DLX_PGSHIFT		12		/* log2(DLX_PGBYTES) */

#define PAGE_SIZE		DLX_PGBYTES
#define PAGE_SHIFT		DLX_PGSHIFT
#define PAGE_MASK		(PAGE_SIZE - 1)

#define trunc_page(x)		((x) & ~PAGE_MASK)
#define round_page(x)		trunc_page((x) + PAGE_MASK)

#endif	/* _MACH_DLX_VM_TYPES_H_ */
