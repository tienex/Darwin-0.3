/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * MMIX:	Exception and interrupt frames.
 *
 * MMIX uses special registers to save state on exceptions/interrupts.
 */

#ifndef _ARCHITECTURE_MMIX_FRAME_H_
#define _ARCHITECTURE_MMIX_FRAME_H_

/*
 * MMIX exception frame
 *
 * When an exception occurs, MMIX automatically saves state in
 * the rW, rX, rY, rZ registers (and rWW, rXX, rYY, rZZ for traps).
 */
typedef struct mmix_except_frame {
    unsigned long long		rW;	/* Where-interrupted (PC) */
    unsigned long long		rX;	/* Execution register (instruction) */
    unsigned long long		rY;	/* Y operand */
    unsigned long long		rZ;	/* Z operand */
    unsigned long long		rB;	/* Bootstrap register (return address) */
} mmix_except_frame_t;

/*
 * MMIX trap frame
 *
 * Traps are forced exceptions (like system calls).
 */
typedef struct mmix_trap_frame {
    unsigned long long		rWW;	/* Where-interrupted (trap) */
    unsigned long long		rXX;	/* Execution register (trap) */
    unsigned long long		rYY;	/* Y operand (trap) */
    unsigned long long		rZZ;	/* Z operand (trap) */
    unsigned long long		rBB;	/* Bootstrap register (trap) */
} mmix_trap_frame_t;

/*
 * MMIX call frame
 *
 * Used by PUSHJ and PUSHGO instructions.
 * The register stack grows down from rG (global threshold).
 */
typedef struct mmix_call_frame {
    unsigned long long		rJ;	/* Return address */
    unsigned long long		saved_regs[32];	/* Local registers */
    unsigned long long		rL;	/* Local threshold */
} mmix_call_frame_t;

/*
 * MMIX stack frame
 *
 * Standard C function call stack frame.
 */
typedef struct mmix_stack_frame {
    unsigned long long		saved_fp;	/* Previous frame pointer */
    unsigned long long		return_addr;	/* Return address */
    unsigned long long		args[8];	/* Function arguments */
    unsigned long long		locals[1];	/* Local variables (variable size) */
} mmix_stack_frame_t;

/*
 * Exception types from rX register
 */
#define MMIX_EX_TYPE(x)		(((x) >> 56) & 0xFF)

#define MMIX_EX_POWER_FAILURE	0x00	/* Power failure */
#define MMIX_EX_MEM_PARITY	0x01	/* Memory parity error */
#define MMIX_EX_NONEXIST_MEM	0x02	/* Nonexistent memory */
#define MMIX_EX_REBOOT		0x03	/* Reboot */
#define MMIX_EX_PAGE_FAULT	0x04	/* Page fault */
#define MMIX_EX_PROT_VIOLATION	0x05	/* Protection violation */
#define MMIX_EX_PRIV_INST	0x06	/* Privileged instruction */
#define MMIX_EX_ILLEGAL_INST	0x07	/* Illegal instruction */
#define MMIX_EX_DIV_CHECK	0x08	/* Division check */
#define MMIX_EX_FP_EXCEPTION	0x09	/* Floating point exception */
#define MMIX_EX_INT_OVERFLOW	0x0A	/* Integer overflow */
#define MMIX_EX_BREAKPOINT	0x0B	/* Breakpoint */
#define MMIX_EX_TRIP		0x0C	/* TRIP instruction */
#define MMIX_EX_FORCED_TRAP	0x0D	/* Forced trap */
#define MMIX_EX_DYNAMIC_TRAP	0x0E	/* Dynamic trap */

/*
 * Privilege levels
 */
#define MMIX_PRIV_USER		0	/* User mode */
#define MMIX_PRIV_KERNEL	1	/* Kernel mode */

#endif /* _ARCHITECTURE_MMIX_FRAME_H_ */
