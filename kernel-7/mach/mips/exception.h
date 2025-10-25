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
 * MIPS Family: Machine dependent exception codes.
 */

/*
 * EXC_BAD_ACCESS
 */
#define EXC_MIPS_TLB_MOD		1	/* TLB modification exception */
#define EXC_MIPS_TLB_LOAD		2	/* TLB exception (load or instruction fetch) */
#define EXC_MIPS_TLB_STORE		3	/* TLB exception (store) */
#define EXC_MIPS_ADDR_LOAD		4	/* Address error (load or instruction fetch) */
#define EXC_MIPS_ADDR_STORE		5	/* Address error (store) */

/*
 * EXC_BAD_INSTRUCTION
 */
#define EXC_MIPS_IBE			6	/* Bus error (instruction fetch) */
#define EXC_MIPS_DBE			7	/* Bus error (data reference) */
#define EXC_MIPS_RESERVED_INSTR		10	/* Reserved instruction */
#define EXC_MIPS_COPROCESSOR		11	/* Coprocessor unusable */

/*
 * EXC_ARITHMETIC
 */
#define EXC_MIPS_OVERFLOW		12	/* Arithmetic overflow */
#define EXC_MIPS_FPE			15	/* Floating point exception */

/*
 * EXC_BREAKPOINT
 */
#define EXC_MIPS_BREAKPOINT		9	/* Breakpoint */

/*
 * EXC_SOFTWARE
 */
#define EXC_MIPS_SYSCALL		8	/* System call */
#define EXC_MIPS_TRAP			13	/* Trap instruction */

/*
 * MIPS R6 specific exceptions
 */
#define EXC_MIPS_MDMX			22	/* MDMX unusable (MIPS64) */
#define EXC_MIPS_WATCH			23	/* Watch exception */
#define EXC_MIPS_MCHECK			24	/* Machine check */
#define EXC_MIPS_THREAD			25	/* Thread exception (MT ASE) */
#define EXC_MIPS_DSP			26	/* DSP ASE state disabled */
#define EXC_MIPS_CACHEERR		30	/* Cache error */
