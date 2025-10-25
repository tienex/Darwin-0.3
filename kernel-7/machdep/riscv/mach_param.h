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
 * RISC-V machine parameters
 */

#ifndef _MACHDEP_RISCV_MACH_PARAM_H_
#define _MACHDEP_RISCV_MACH_PARAM_H_

/*
 * Page size
 */
#define PAGE_SHIFT      12
#define PAGE_SIZE       (1 << PAGE_SHIFT)
#define PAGE_MASK       (PAGE_SIZE - 1)

/*
 * Kernel stack size
 */
#define KERNEL_STACK_SIZE       (4 * PAGE_SIZE)

/*
 * Interrupt stack size
 */
#define INTSTACK_SIZE           (4 * PAGE_SIZE)

/*
 * Number of CPUs
 */
#ifndef NCPUS
#define NCPUS   1
#endif

/*
 * Cache line size (typical for RISC-V)
 */
#define CACHE_LINE_SIZE         64

/*
 * Byte order
 */
#define BYTE_ORDER      LITTLE_ENDIAN

#endif /* _MACHDEP_RISCV_MACH_PARAM_H_ */
