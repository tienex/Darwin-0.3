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
 * MIPS:	Definition of Status register (CP0 register 12).
 */

#if	KERNEL_PRIVATE

#ifndef _BSD_MIPS_PSL_H_
#define _BSD_MIPS_PSL_H_

/*
 * MIPS CP0 Status Register bit definitions
 */
#define SR_IE		0x00000001	/* Interrupt Enable */
#define SR_EXL		0x00000002	/* Exception Level */
#define SR_ERL		0x00000004	/* Error Level */
#define SR_KSU_MASK	0x00000018	/* Mode bits */
#define SR_KSU_USER	0x00000010	/* User mode */
#define SR_KSU_SUPER	0x00000008	/* Supervisor mode */
#define SR_KSU_KERNEL	0x00000000	/* Kernel mode */
#define SR_UX		0x00000020	/* 64-bit user mode (MIPS III+) */
#define SR_SX		0x00000040	/* 64-bit supervisor mode */
#define SR_KX		0x00000080	/* 64-bit kernel mode */
#define SR_IM_MASK	0x0000FF00	/* Interrupt Mask (8 levels) */
#define SR_IM_SHIFT	8
#define SR_CU0		0x10000000	/* Coprocessor 0 usable */
#define SR_CU1		0x20000000	/* Coprocessor 1 usable (FPU) */
#define SR_CU2		0x40000000	/* Coprocessor 2 usable */
#define SR_CU3		0x80000000	/* Coprocessor 3 usable */

/*
 * User mode status bits
 */
#define SR_USERSET	(SR_IE | SR_KSU_USER)
#define SR_USERCLR	(SR_EXL | SR_ERL | SR_CU0 | SR_CU2 | SR_CU3)

/*
 * PSL compatibility definitions
 */
#define PSL_T		SR_EXL		/* Trace/single-step equivalent */

#endif	/* _BSD_MIPS_PSL_H_ */

#endif	/* KERNEL_PRIVATE */
