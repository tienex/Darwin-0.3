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
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 *	File:	mmix/mach_param.h
 *
 *	MMIX machine dependent parameters
 */

#ifndef	_MMIX_MACH_PARAM_H_
#define _MMIX_MACH_PARAM_H_

/*
 * MMIX uses 8KB pages
 */
#define MMIX_PGBYTES	8192		/* bytes per MMIX page */
#define MMIX_PGSHIFT	13		/* log2(MMIX_PGBYTES) */

#define PAGE_SIZE	MMIX_PGBYTES
#define PAGE_SHIFT	MMIX_PGSHIFT
#define PAGE_MASK	(PAGE_SIZE-1)

/*
 * Kernel virtual memory layout
 */
#define VM_MIN_ADDRESS		((vm_offset_t) 0)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x7FFFFFFFFFFFFFFFULL)
#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x8000000000000000ULL)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xFFFFFFFFFFFFFFFFULL)

/*
 * User/kernel address space split at 0x8000000000000000
 */
#define USRSTACK	0x8000000000000000ULL

/*
 * Kernel stack size - 64KB (8 pages)
 */
#define KERNEL_STACK_SIZE	(8 * PAGE_SIZE)

/*
 * Interrupt stack size - 128KB (16 pages)
 */
#define INTSTACK_SIZE		(16 * PAGE_SIZE)

#endif	/* _MMIX_MACH_PARAM_H_ */
