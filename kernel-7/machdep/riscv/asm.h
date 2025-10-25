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
 * RISC-V assembly language macros and definitions
 */

#ifndef _MACHDEP_RISCV_ASM_H_
#define _MACHDEP_RISCV_ASM_H_

/*
 * Register size definitions
 */
#if defined(__riscv_xlen) && __riscv_xlen == 64
#define SZREG   8
#define LGREG   3
#define REGLOAD  ld
#define REGSTORE sd
#else
#define SZREG   4
#define LGREG   2
#define REGLOAD  lw
#define REGSTORE sw
#endif

/*
 * Entry/exit macros for assembly functions
 */
#ifdef __ASSEMBLER__

#define ENTRY(name) \
	.globl name; \
	.align 2; \
	name:

#define END(name) \
	.size name, . - name

#define WEAK(name) \
	.weak name; \
	.align 2; \
	name:

/*
 * Load/store register macros with proper sizes
 */
#define REG_S   REGSTORE
#define REG_L   REGLOAD

/*
 * Align to cache line
 */
#define ALIGN_CACHE .align 6

/*
 * Function entry with frame pointer setup
 */
#define NESTED(name, framesize, ra_offset) \
	ENTRY(name); \
	addi sp, sp, -framesize; \
	REG_S ra, ra_offset(sp)

/*
 * Function exit with frame pointer cleanup
 */
#define RESTORE_AND_RETURN(framesize, ra_offset) \
	REG_L ra, ra_offset(sp); \
	addi sp, sp, framesize; \
	ret

#endif /* __ASSEMBLER__ */

#endif /* _MACHDEP_RISCV_ASM_H_ */
