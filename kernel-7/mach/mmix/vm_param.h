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
 * MMIX Virtual Memory Parameters - Kernel Level
 */

#ifndef	_MACH_MMIX_VM_PARAM_H_
#define _MACH_MMIX_VM_PARAM_H_

#import <sys/types.h>

#define BYTE_SIZE	8	/* byte size in bits */

/*
 * MMIX page size is 8KB (8192 bytes)
 */
#define MMIX_PGBYTES	8192	/* bytes per MMIX page */
#define MMIX_PGSHIFT	13	/* number of bits to shift for pages */
#define MMIX_PGALIGN	13      /* power of two for page alignment */

/*
 * Virtual address space layout for MMIX (64-bit)
 *
 * User space:   0x0000000000000000 - 0x7FFFFFFFFFFFFFFF
 * Kernel space: 0x8000000000000000 - 0xFFFFFFFFFFFFFFFF
 */
#define VM_MIN_ADDRESS		((vm_offset_t) 0x0000000000000000ULL)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x7FFFFFFFFFFFF000ULL)

/*
 * Kernel text typically starts at a low address
 */
#define KERNELBASE_TEXT		0x8000000000100000ULL

/*
 * Page rounding macros
 */
#define mmix_round_page(x)	((((unsigned long long)(x)) + MMIX_PGBYTES - 1) & \
					~(MMIX_PGBYTES-1))
#define mmix_trunc_page(x)	(((unsigned long long)(x)) & ~(MMIX_PGBYTES-1))

/*
 * Kernel address space
 */
#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x8000000000000000ULL)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xFFFFFFFFFFFFFFFFULL)

/*
 * Kernel stack sizes
 * MMIX has a large address space, so we can afford larger stacks
 */
#define KERNSTACK_SIZE		(8 * MMIX_PGBYTES)	/* 64 KB */
#define INTSTACK_SIZE		(16 * MMIX_PGBYTES)	/* 128 KB */

/*
 * Maximum alignment required by any data type for this architecture.
 * MMIX prefers octabyte (8-byte) alignment, but can use 16-byte
 * alignment for paired operations.
 */
#define MAX_DATA_ALIGNMENT      16  /* 16 byte alignment for efficiency */

/*
 * MMIX-specific VM constants
 */

/*
 * Number of segment table entries
 * MMIX uses a large flat address space
 */
#define MMIX_SEG_MAX		256

/*
 * Cache characteristics
 * These are nominal values; actual hardware may vary
 */
#define MMIX_CACHE_LINE		64	/* Cache line size in bytes */
#define MMIX_CACHE_SIZE		(512*1024)	/* 512 KB L1 cache */

/*
 * TLB characteristics
 */
#define MMIX_TLB_ENTRIES	256	/* Number of TLB entries */

#endif	/* _MACH_MMIX_VM_PARAM_H_ */
