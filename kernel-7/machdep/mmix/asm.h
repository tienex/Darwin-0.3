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
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _MMIX_ASM_H_
#define _MMIX_ASM_H_

/*
 * Assembly language macros for MMIX kernel code
 */

#define __ASSEMBLER__

/* Function entry/exit macros */
#define ENTRY(name) \
	.globl	name; \
	.align	4; \
name:

#define END(name) \
	.size name, .-name

/* MMIX register definitions for assembly */
#define SP	$254		/* Stack pointer */
#define FP	$253		/* Frame pointer */
#define RV	$0		/* Return value */

/* Saved registers (callee-saved) */
#define S0	$16
#define S1	$17
#define S2	$18
#define S3	$19
#define S4	$20
#define S5	$21
#define S6	$22
#define S7	$23

/* Argument registers */
#define A0	$0
#define A1	$1
#define A2	$2
#define A3	$3
#define A4	$4
#define A5	$5
#define A6	$6
#define A7	$7

/* Temporary registers */
#define T0	$8
#define T1	$9
#define T2	$10
#define T3	$11

/* Data alignment */
#define ALIGN_DATA	.align 3	/* 8-byte alignment */
#define ALIGN_TEXT	.align 2	/* 4-byte alignment for instructions */

/* Standard function prologue/epilogue */
#define SAVE_CONTEXT \
	SUBU	SP,SP,64; \
	STOU	FP,SP,0; \
	ADDU	FP,SP,64; \
	STOU	S0,SP,8; \
	STOU	S1,SP,16; \
	STOU	S2,SP,24; \
	STOU	S3,SP,32; \
	STOU	S4,SP,40; \
	STOU	S5,SP,48; \
	STOU	S6,SP,56

#define RESTORE_CONTEXT \
	LDOU	S6,SP,56; \
	LDOU	S5,SP,48; \
	LDOU	S4,SP,40; \
	LDOU	S3,SP,32; \
	LDOU	S2,SP,24; \
	LDOU	S1,SP,16; \
	LDOU	S0,SP,8; \
	LDOU	FP,SP,0; \
	ADDU	SP,SP,64

#endif /* _MMIX_ASM_H_ */
