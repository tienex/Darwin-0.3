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
 * MMIX virtual memory parameters
 */

#ifndef	_BSD_MMIX_VMPARAM_H_
#define	_BSD_MMIX_VMPARAM_H_ 1

#import <sys/resource.h>

/*
 * MMIX user stack starts at 0x8000000000000000
 * (top of lower half of 64-bit address space)
 */
#define	USRSTACK	0x8000000000000000ULL

/*
 * Virtual memory related constants, all in bytes
 * MMIX has a 64-bit address space, so we can be more generous.
 */
#ifndef DFLDSIZ
#define	DFLDSIZ		(128*1024*1024)		/* initial data size limit (128MB) */
#endif
#ifndef MAXDSIZ
#define	MAXDSIZ		(RLIM_INFINITY)		/* max data size */
#endif
#ifndef	DFLSSIZ
#define	DFLSSIZ		(8*1024*1024)		/* initial stack size limit (8MB) */
#endif
#ifndef	MAXSSIZ
#define	MAXSSIZ		(1024*1024*1024)	/* max stack size (1GB) */
#endif
#ifndef	DFLCSIZ
#define DFLCSIZ		(0)			/* initial core size limit */
#endif
#ifndef	MAXCSIZ
#define MAXCSIZ		(RLIM_INFINITY)		/* max core size */
#endif

/*
 * MMIX-specific VM parameters
 */
#define	VM_MIN_ADDRESS		((vm_offset_t) 0)
#define	VM_MAX_ADDRESS		((vm_offset_t) 0x7FFFFFFFFFFFFFFFULL)
#define	VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x8000000000000000ULL)
#define	VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xFFFFFFFFFFFFFFFFULL)

/*
 * Page table organization
 * MMIX uses 8KB pages
 */
#define	MMIX_PGBYTES		8192		/* bytes per MMIX page */
#define	MMIX_PGSHIFT		13		/* log2(MMIX_PGBYTES) */

#ifndef	KERNEL
#define	NBPG			MMIX_PGBYTES
#define	PGSHIFT			MMIX_PGSHIFT
#endif

#endif	/* _BSD_MMIX_VMPARAM_H_ */
