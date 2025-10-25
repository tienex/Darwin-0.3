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
 * RISC-V Family:	Machine dependent exception codes.
 */

/*
 * EXC_BAD_ACCESS
 */
#define EXC_RISCV_INST_PAGE_FAULT	12
#define EXC_RISCV_LOAD_PAGE_FAULT	13
#define EXC_RISCV_STORE_PAGE_FAULT	15

/*
 * EXC_BAD_INSTRUCTION
 */
#define EXC_RISCV_ILLEGAL_INST		2
#define EXC_RISCV_BREAKPOINT		3

/*
 * EXC_ARITHMETIC
 */
#define EXC_RISCV_LOAD_MISALIGN		4
#define EXC_RISCV_STORE_MISALIGN	6

/*
 * EXC_EMULATION
 */
#define EXC_RISCV_ECALL_U		8
#define EXC_RISCV_ECALL_S		9
#define EXC_RISCV_ECALL_M		11

/*
 * EXC_SOFTWARE
 */
#define EXC_RISCV_INST_ACCESS_FAULT	1
#define EXC_RISCV_LOAD_ACCESS_FAULT	5
#define EXC_RISCV_STORE_ACCESS_FAULT	7

/*
 * EXC_BREAKPOINT
 */
/* EXC_RISCV_BREAKPOINT is defined above (3) */
