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
 * IA64 (Itanium) Family:	Virtual memory constants.
 */

#ifndef	_MACH_IA64_VM_PARAM_H_
#define _MACH_IA64_VM_PARAM_H_

#import <sys/types.h>

#define BYTE_SIZE	8	/* byte size in bits */
#define BYTE_MSF	0

/*
 * IA64 page sizes:
 * - Variable page size support (4KB, 8KB, 16KB, 64KB, 256KB, 1MB, 4MB, 16MB, 256MB)
 * - We use 16KB as default for better TLB efficiency
 */
#define IA64_PGBYTES	16384	/* bytes per IA64 page (16KB) */
#define IA64_PGSHIFT	14	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define ia64_btop(x)		(((unsigned long)(x)) >> IA64_PGSHIFT)
#define ia64_ptob(x)		(((unsigned long)(x)) << IA64_PGSHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page bytes)
 */

#define ia64_round_page(x)	((((unsigned long)(x)) + IA64_PGBYTES - 1) & \
					~(IA64_PGBYTES-1))
#define ia64_trunc_page(x)	(((unsigned long)(x)) & ~(IA64_PGBYTES-1))

/*
 * IA64 address space layout:
 * - 64-bit address space divided into 8 regions (3-bit region number)
 * - Each region is 2^61 bytes (2 EB)
 * - Region 0-4: User space
 * - Region 5: Kernel virtual memory
 * - Region 6: Kernel identity-mapped memory
 * - Region 7: Kernel percpu data
 */

#define IA64_REGION_SHIFT	61
#define IA64_REGION_MASK	0xE000000000000000UL

#define IA64_REGION_0		0x0000000000000000UL	/* User region 0 */
#define IA64_REGION_1		0x2000000000000000UL	/* User region 1 */
#define IA64_REGION_2		0x4000000000000000UL	/* User region 2 */
#define IA64_REGION_3		0x6000000000000000UL	/* User region 3 */
#define IA64_REGION_4		0x8000000000000000UL	/* User region 4 */
#define IA64_REGION_5		0xA000000000000000UL	/* Kernel virtual */
#define IA64_REGION_6		0xC000000000000000UL	/* Kernel identity */
#define IA64_REGION_7		0xE000000000000000UL	/* Kernel percpu */

#define VM_MIN_ADDRESS		((vm_offset_t) 0x0000000000000000UL)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x6000000000000000UL)  /* End of region 2 */

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0xA000000000000000UL)  /* Region 5 */
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xC000000000000000UL)  /* End of region 5 */

#define KERNSTACK_SIZE		(4*IA64_PGBYTES)	/* 64KB kernel stacks */
#define INTSTACK_SIZE		(2*IA64_PGBYTES)	/* 32KB interrupt stacks */

/*
 *	Conversion between IA64 pages and VM pages
 */

#define trunc_ia64_to_vm(p)	(atop(trunc_page(ia64_ptob(p))))
#define round_ia64_to_vm(p)	(atop(round_page(ia64_ptob(p))))
#define vm_to_ia64(p)		(ia64_btop(ptoa(p)))

/*
 * Maximum alignment required by any data type for this architecture.
 * IA64 requires 16-byte alignment for some operations (128-bit loads/stores)
 */
#define	MAX_DATA_ALIGNMENT	16		/* 16 bytes */

/*
 * IA64 TLB parameters
 */
#define IA64_TR_ENTRIES		8	/* Translation Registers per type */
#define IA64_TC_ENTRIES		96	/* Translation Cache entries (typical) */

/*
 * IA64 Register Stack Engine parameters
 */
#define IA64_RBS_SIZE		(64*1024)	/* 64KB backing store */

#endif	/* _MACH_IA64_VM_PARAM_H_ */
