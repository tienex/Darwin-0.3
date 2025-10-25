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
 * IA64 Assembly Macros
 * Defines standard assembly conventions for IA64 architecture
 */

/* IA64 function arguments via registers */
#define S_ARG0	r32
#define S_ARG1	r33
#define S_ARG2	r34
#define S_ARG3	r35
#define S_ARG4	r36
#define S_ARG5	r37
#define S_ARG6	r38
#define S_ARG7	r39

/* Stack frame setup/teardown using register stack */
#define FRAME	alloc loc0 = ar.pfs, 0, 3, 0, 0
#define EMARF	mov ar.pfs = loc0; br.ret.sptk.many rp

/* Return values */
#define RET0	r8
#define RET1	r9
#define RET2	r10
#define RET3	r11

/* Symbol naming */
#define EXT(x)		_##x
#define LBb(x,n)	n##b
#define LBf(x,n)	n##f

#define ALIGN 4
#define	LCL(x)	x

#define LB(x,n) n

#define String	.ascii
#define Value	.word

#define Times(a,b) (a*b)
#define Divide(a,b) (a/b)

/* Function entry macros */
#ifdef GPROF
#define MCOUNT		.data; LB(x, 9): .long 0; .text; mov r14=LBb(x, 9); br.call.sptk.many rp=mcount
#define	ENTRY(x)	.globl EXT(x); .align ALIGN; .proc EXT(x); EXT(x): ; \
			alloc loc0=ar.pfs,0,3,1,0; mov loc1=rp; MCOUNT; mov rp=loc1; mov ar.pfs=loc0
#define	ASENTRY(x) 	.globl x; .align ALIGN; .proc x; x: ; \
			alloc loc0=ar.pfs,0,3,1,0; mov loc1=rp; MCOUNT; mov rp=loc1; mov ar.pfs=loc0
#else	/* GPROF */
#define MCOUNT
#define	ENTRY(x)	.globl EXT(x); .align ALIGN; .proc EXT(x); EXT(x):
#define	ASENTRY(x)	.globl x; .align ALIGN; .proc x; x:
#endif	/* GPROF */

#define	Entry(x)	.globl EXT(x); .align ALIGN; .proc EXT(x); EXT(x):
#define	DATA(x)		.globl EXT(x); .align ALIGN; EXT(x):
#define END(x)		.endp EXT(x)

/* IA64-specific defines */
#define KERNEL_STACK_SIZE	16384	/* 16KB kernel stacks */
#define BUNDLE_SIZE		16	/* IA64 instruction bundles are 16 bytes */

/* Register usage conventions
 * r0      - always 0
 * r1      - global pointer (gp)
 * r2-r3   - scratch
 * r4-r7   - preserved
 * r8-r11  - return values
 * r12     - stack pointer (sp)
 * r13     - thread pointer (tp)
 * r14-r31 - scratch
 * r32-r39 - incoming arguments (out0-out7)
 * r40+    - local registers (via register stack)
 *
 * Register Stack Engine manages r32+
 */

/* Processor Status Register bits */
#define PSR_BE	1	/* Big-endian */
#define PSR_UP	2	/* User Performance monitor */
#define PSR_AC	3	/* Alignment Check */
#define PSR_MFL	4	/* Lower floating-point registers written */
#define PSR_MFH	5	/* Upper floating-point registers written */
#define PSR_IC	13	/* Interruption Collection */
#define PSR_I	14	/* Interrupt enable */
#define PSR_PK	15	/* Protection Key enable */
#define PSR_DT	17	/* Data address Translation */
#define PSR_DFL	18	/* Disabled Floating-point Low register set */
#define PSR_DFH	19	/* Disabled Floating-point High register set */
#define PSR_SP	20	/* Secure Performance monitors */
#define PSR_PP	21	/* Privileged Performance monitor */
#define PSR_DI	22	/* Disable Instruction set transition */
#define PSR_SI	23	/* Secure Interval timer */
#define PSR_DB	24	/* Debug Breakpoint fault */
#define PSR_LP	25	/* Lower Privilege transfer trap */
#define PSR_TB	26	/* Taken Branch trap */
#define PSR_RT	27	/* Register stack translation */
#define PSR_IS	34	/* Instruction Set (0=IA-64, 1=x86) */
#define PSR_IT	36	/* Instruction address Translation */
#define PSR_ID	37	/* Instruction Debug fault disable */
#define PSR_DA	38	/* Disable Data Access and dirty-bit faults */
#define PSR_DD	39	/* Data Debug fault disable */
#define PSR_SS	40	/* Single Step */
#define PSR_RI	41	/* Restart Instruction (2 bits, 41-42) */
#define PSR_ED	43	/* Exception Deferral */
#define PSR_BN	44	/* Register Bank (0 or 1) */
#define PSR_IA	45	/* Disable Instruction Access-bit faults */

