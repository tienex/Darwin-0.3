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
 * RISC-V Family:	Virtual memory constants.
 */

#ifndef	_MACH_RISCV_VM_PARAM_H_
#define _MACH_RISCV_VM_PARAM_H_

#import <sys/types.h>

#define BYTE_SIZE	8	/* byte size in bits */
#define BYTE_MSF	0

#define RISCV_PGBYTES	4096	/* bytes per RISC-V page */
#define RISCV_PGSHIFT	12	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define riscv_btop(x)		(((unsigned long)(x)) >> RISCV_PGSHIFT)
#define riscv_ptob(x)		(((unsigned long)(x)) << RISCV_PGSHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define riscv_round_page(x)	((((unsigned long)(x)) + RISCV_PGBYTES - 1) & \
					~(RISCV_PGBYTES-1))
#define riscv_trunc_page(x)	(((unsigned long)(x)) & ~(RISCV_PGBYTES-1))

#if defined(__riscv_xlen) && __riscv_xlen == 64
/* RV64 memory layout */
#define VM_MIN_ADDRESS		((vm_offset_t) 0x0000000000000000UL)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x0000004000000000UL)

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0xFFFFFFE000000000UL)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xFFFFFFFF80000000UL)
#else
/* RV32 memory layout */
#define VM_MIN_ADDRESS		((vm_offset_t) 0x00000000)
#define VM_MAX_ADDRESS		((vm_offset_t) 0xC0000000)

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0xC0000000)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xFFFFFFFF)
#endif

#define KERNSTACK_SIZE		(2*RISCV_PGBYTES)
#define INTSTACK_SIZE		(1*RISCV_PGBYTES)

/*
 *	Conversion between RISC-V pages and VM pages
 */

#define trunc_riscv_to_vm(p)	(atop(trunc_page(riscv_ptob(p))))
#define round_riscv_to_vm(p)	(atop(round_page(riscv_ptob(p))))
#define vm_to_riscv(p)		(riscv_btop(ptoa(p)))

/*
 * Maximum alignment required by any data type for this architecture.
 */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define	MAX_DATA_ALIGNMENT	8		/* 8 bytes */
#else
#define	MAX_DATA_ALIGNMENT	4		/* 4 bytes */
#endif

#endif	/* _MACH_RISCV_VM_PARAM_H_ */
