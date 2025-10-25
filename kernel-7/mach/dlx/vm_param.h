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
 * DLX VM Parameters
 * Based on ETH DLXSIM architecture
 */

#ifndef	_MACH_DLX_VM_PARAM_H_
#define	_MACH_DLX_VM_PARAM_H_

#include <mach/machine/vm_types.h>

/*
 * DLX Virtual Memory Layout
 *
 * 0x00000000 - 0x7FFFFFFF : User space (2GB)
 * 0x80000000 - 0xFFFFFFFF : Kernel space (2GB)
 */

#define VM_MIN_ADDRESS		((vm_offset_t) 0)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x80000000)

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x80000000)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xFFFFFFFF)

/*
 * Machine-dependent configuration parameters
 */
#define KERNEL_STACK_SIZE	(4 * PAGE_SIZE)
#define INTSTACK_SIZE		(4 * PAGE_SIZE)

#endif	/* _MACH_DLX_VM_PARAM_H_ */
