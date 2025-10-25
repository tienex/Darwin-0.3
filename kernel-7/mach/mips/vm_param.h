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

#ifndef	_MACH_MIPS_VM_PARAM_H_
#define _MACH_MIPS_VM_PARAM_H_

#import <sys/types.h>

#define BYTE_SIZE	8	/* byte size in bits */

/* Page size definitions for MIPS */
#define MIPS_PGBYTES	4096	/* bytes per MIPS page */
#define MIPS_PGSHIFT	12	/* number of bits to shift for pages */
#define MIPS_PGALIGN	12      /* power of two for page alignment */

/* MIPS32 and MIPS64 address space definitions */
#if defined(_MIPS64) || defined(__mips64)
/* MIPS64 address space */
#define VM_MIN_ADDRESS		((vm_offset_t) 0x0000000000000000UL)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x0000fffffffff000UL)
#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0xffffffff80000000UL)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xfffffffffffff000UL)
#else
/* MIPS32 address space */
#define VM_MIN_ADDRESS		((vm_offset_t) 0)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x7ffff000)
#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x80000000)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xfffff000)
#endif

/* Kernel text base - typically in kseg0 for MIPS */
#if defined(_MIPS64) || defined(__mips64)
#define KERNELBASE_TEXT	0xffffffff80000000UL
#else
#define KERNELBASE_TEXT	0x80000000
#endif

#define mips_round_page(x)	((((unsigned long)(x)) + MIPS_PGBYTES - 1) & \
					~(MIPS_PGBYTES-1))
#define mips_trunc_page(x)	(((unsigned long)(x)) & ~(MIPS_PGBYTES-1))

/*
 * Kernel and interrupt stack sizes
 */
#define KERNSTACK_SIZE		(4 * MIPS_PGBYTES)
#define INTSTACK_SIZE		(8 * MIPS_PGBYTES)

/*
 * Maximum alignment required by any data type for this architecture.
 * MIPS requires 8-byte alignment for doubles (MIPS32) and 16-byte for MIPS64.
 */
#if defined(_MIPS64) || defined(__mips64)
#define MAX_DATA_ALIGNMENT      16
#else
#define MAX_DATA_ALIGNMENT      8
#endif

#endif	/* _MACH_MIPS_VM_PARAM_H_ */
