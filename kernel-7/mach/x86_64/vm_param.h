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
 * x86_64 Family:	Virtual memory constants.
 */

#ifndef	_MACH_X86_64_VM_PARAM_H_
#define _MACH_X86_64_VM_PARAM_H_

#import <sys/types.h>

#define BYTE_SIZE	8	/* byte size in bits */
#define BYTE_MSF	0

#define X86_64_PGBYTES	4096	/* bytes per x86-64 page */
#define X86_64_PGSHIFT	12	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define x86_64_btop(x)		(((unsigned long)(x)) >> X86_64_PGSHIFT)
#define x86_64_ptob(x)		(((unsigned long)(x)) << X86_64_PGSHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define x86_64_round_page(x)	((((unsigned long)(x)) + X86_64_PGBYTES - 1) & \
					~(X86_64_PGBYTES-1))
#define x86_64_trunc_page(x)	(((unsigned long)(x)) & ~(X86_64_PGBYTES-1))

/*
 * User/kernel address space split
 * On x86-64, canonical addresses use only lower 48 bits
 * User space: 0x0000000000000000 - 0x00007FFFFFFFFFFF
 * Kernel space: 0xFFFF800000000000 - 0xFFFFFFFFFFFFFFFF
 */
#define VM_MIN_ADDRESS		((vm_offset_t) 0x0000000000000000UL)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x00007FFFFFFFFFFFUL)

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0xFFFF800000000000UL)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xFFFFFFFFFFFFFFFFUL)

#define KERNSTACK_SIZE		(4*X86_64_PGBYTES)	/* 16KB kernel stack */
#define INTSTACK_SIZE		(2*X86_64_PGBYTES)	/* 8KB interrupt stack */

/*
 *	Conversion between x86-64 pages and VM pages
 */

#define trunc_x86_64_to_vm(p)	(atop(trunc_page(x86_64_ptob(p))))
#define round_x86_64_to_vm(p)	(atop(round_page(x86_64_ptob(p))))
#define vm_to_x86_64(p)		(x86_64_btop(ptoa(p)))

/*
 * Maximum alignment required by any data type for this architecture.
 * (Use 8 bytes for 64-bit alignment)
 */
#define	MAX_DATA_ALIGNMENT	8		/* 8 bytes */

#endif	/* _MACH_X86_64_VM_PARAM_H_ */
