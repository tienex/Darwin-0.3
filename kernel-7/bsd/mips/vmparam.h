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
 * MIPS virtual memory parameters
 */

#ifndef	_BSD_MIPS_VMPARAM_H_
#define	_BSD_MIPS_VMPARAM_H_

#include <mach/mips/vm_param.h>

/*
 * User process VM parameters
 */

#if defined(_MIPS64) || defined(__mips64)
/* MIPS64 address space layout */
#define USRTEXT		0x0000000000400000UL	/* Start of user text */
#define USRSTACK	0x0000800000000000UL	/* Start of user stack */
#define LOWPAGES	0			/* Low pages reserved */
#else
/* MIPS32 address space layout */
#define USRTEXT		0x00400000	/* Start of user text (4MB) */
#define USRSTACK	0x7FFFF000	/* Start of user stack */
#define LOWPAGES	0		/* Low pages reserved */
#endif

/*
 * Virtual memory statistics
 */
#define VM_METER	1

/*
 * Default maximum number of pages in an exec task
 */
#ifndef MAXTSIZ
#define MAXTSIZ		(64*1024*1024)		/* max text size (64MB) */
#endif

#ifndef DFLDSIZ
#define DFLDSIZ		(64*1024*1024)		/* initial data size (64MB) */
#endif

#ifndef MAXDSIZ
#define MAXDSIZ		(256*1024*1024)		/* max data size (256MB) */
#endif

#ifndef DFLSSIZ
#define DFLSSIZ		(8*1024*1024)		/* initial stack size (8MB) */
#endif

#ifndef MAXSSIZ
#define MAXSSIZ		(64*1024*1024)		/* max stack size (64MB) */
#endif

/*
 * PTEs per page table page
 */
#if defined(_MIPS64) || defined(__mips64)
#define NPTEPG		(MIPS_PGSIZE/8)		/* 512 PTEs for 64-bit */
#else
#define NPTEPG		(MIPS_PGSIZE/4)		/* 1024 PTEs for 32-bit */
#endif

/*
 * Kernel virtual memory parameters
 */

#ifndef KERNEL_VM_SIZE
#if defined(_MIPS64) || defined(__mips64)
#define KERNEL_VM_SIZE	(512*1024*1024)		/* 512MB kernel VM */
#else
#define KERNEL_VM_SIZE	(256*1024*1024)		/* 256MB kernel VM */
#endif
#endif

#endif	/* _BSD_MIPS_VMPARAM_H_ */
