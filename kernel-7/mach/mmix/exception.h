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
 *	File:	mach/mmix/exception.h
 *
 *	MMIX exception codes
 */

#ifndef	_MACH_MMIX_EXCEPTION_H_
#define _MACH_MMIX_EXCEPTION_H_

#define	EXC_TYPES_COUNT	10	/* incl. illegal exception 0 */

#define EXCEPTION_CODE_MAX 2    /* elements in vector (code+subcode) */

/*
 *	EXC_BAD_INSTRUCTION
 */
#define EXC_MMIX_INVALID_SYSCALL	1    /* invalid syscall number */
#define EXC_MMIX_PRIVINST		2    /* privileged instruction */
#define EXC_MMIX_TRACE			3    /* trace/single-step */
#define EXC_MMIX_ILLEGAL_INST		4    /* illegal instruction */
#define EXC_MMIX_TRIP			5    /* TRIP instruction */

/*
 *	EXC_BAD_ACCESS
 *	Note: do not conflict with kern_return_t values returned by vm_fault
 */
#define EXC_MMIX_VM_PROT_READ		0x101 /* error reading syscall args */
#define EXC_MMIX_BADSPACE		0x102 /* bad space referenced */
#define EXC_MMIX_UNALIGNED		0x103 /* unaligned data reference */
#define EXC_MMIX_PROTECTION_FAULT	0x104 /* protection fault */

/*
 *	EXC_ARITHMETIC
 *
 *	MMIX has extensive arithmetic exception support
 *	through its rA (arithmetic status) register.
 */
#define EXC_MMIX_OVERFLOW		1    /* integer overflow */
#define EXC_MMIX_ZERO_DIVIDE		2    /* integer divide by zero */
#define EXC_MMIX_FLT_INEXACT		3    /* IEEE inexact exception */
#define EXC_MMIX_FLT_ZERO_DIVIDE	4    /* IEEE zero divide */
#define EXC_MMIX_FLT_UNDERFLOW		5    /* IEEE floating underflow */
#define EXC_MMIX_FLT_OVERFLOW		6    /* IEEE floating overflow */
#define EXC_MMIX_FLT_NOT_A_NUMBER	7    /* IEEE not a number */
#define EXC_MMIX_FLT_INVALID		8    /* IEEE invalid operation */

/*
 *	EXC_SOFTWARE
 */
#define EXC_MMIX_MIGRATE		0x10100		/* Time to migrate */

/*
 *	EXC_BREAKPOINT
 */
#define EXC_MMIX_BREAKPOINT		1    /* breakpoint trap */

/*
 *	MMIX-specific interrupts
 *	MMIX has a comprehensive interrupt system managed through
 *	the rK (interrupt mask) and rQ (interrupt request) registers.
 */
#define EXC_MMIX_INTERRUPT_POWER	0x01	/* Power failure */
#define EXC_MMIX_INTERRUPT_MEM		0x02	/* Memory parity error */
#define EXC_MMIX_INTERRUPT_NONEX	0x04	/* Nonexistent memory */
#define EXC_MMIX_INTERRUPT_REBOOT	0x08	/* Reboot */

/*
 *	machine dependent exception masks
 */
#define	EXC_MASK_MACHINE	0

/* from MACH 3.0 mach_kernel/mach/exception.h */
#include <mach/mmix/vm_types.h>
#ifndef __ASSEMBLER__
typedef integer_t	exception_data_type_t;
#endif /* __ASSEMBLER__ */

#endif	/* _MACH_MMIX_EXCEPTION_H_ */
