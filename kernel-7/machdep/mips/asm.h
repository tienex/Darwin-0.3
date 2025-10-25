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

#ifndef	_MIPS_ASM_H_
#define	_MIPS_ASM_H_

/* MIPS register definitions for assembly code */

/* Argument registers - standard MIPS ABI */
#define ARG0 a0		/* $4 */
#define ARG1 a1		/* $5 */
#define ARG2 a2		/* $6 */
#define ARG3 a3		/* $7 */

/* Return value registers */
#define RETVAL0 v0	/* $2 */
#define RETVAL1 v1	/* $3 */

/* Temporary registers */
#define tmp0	t0	/* $8 */
#define tmp1	t1	/* $9 */
#define tmp2	t2	/* $10 */
#define tmp3	t3	/* $11 */

/* MIPS CP0 (Coprocessor 0) register numbers */
#define CP0_INDEX	0	/* TLB Index */
#define CP0_RANDOM	1	/* TLB Random */
#define CP0_ENTRYLO0	2	/* TLB EntryLo0 */
#define CP0_ENTRYLO1	3	/* TLB EntryLo1 */
#define CP0_CONTEXT	4	/* TLB Context */
#define CP0_PAGEMASK	5	/* TLB PageMask */
#define CP0_WIRED	6	/* TLB Wired */
#define CP0_BADVADDR	8	/* Bad Virtual Address */
#define CP0_COUNT	9	/* Count register */
#define CP0_ENTRYHI	10	/* TLB EntryHi */
#define CP0_COMPARE	11	/* Compare register */
#define CP0_STATUS	12	/* Status register */
#define CP0_CAUSE	13	/* Cause register */
#define CP0_EPC		14	/* Exception Program Counter */
#define CP0_PRID	15	/* Processor Revision Identifier */
#define CP0_CONFIG	16	/* Configuration register */
#define CP0_LLADDR	17	/* Load Linked Address */
#define CP0_WATCHLO	18	/* Watchpoint Low */
#define CP0_WATCHHI	19	/* Watchpoint High */
#define CP0_XCONTEXT	20	/* Extended Context (MIPS64) */
#define CP0_ECC		26	/* Error Correction Code */
#define CP0_CACHEERR	27	/* Cache Error */
#define CP0_TAGLO	28	/* Cache TagLo */
#define CP0_TAGHI	29	/* Cache TagHi */
#define CP0_ERROREPC	30	/* Error Exception Program Counter */

/* Status register (CP0_STATUS) bit definitions */
#define ST_CU3		0x80000000	/* Coprocessor 3 usable */
#define ST_CU2		0x40000000	/* Coprocessor 2 usable */
#define ST_CU1		0x20000000	/* Coprocessor 1 (FPU) usable */
#define ST_CU0		0x10000000	/* Coprocessor 0 usable */
#define ST_RP		0x08000000	/* Reduced power */
#define ST_FR		0x04000000	/* Additional FP registers */
#define ST_RE		0x02000000	/* Reverse endian */
#define ST_BEV		0x00400000	/* Boot exception vectors */
#define ST_TS		0x00200000	/* TLB shutdown */
#define ST_SR		0x00100000	/* Soft reset */
#define ST_NMI		0x00080000	/* Non-maskable interrupt */
#define ST_IM		0x0000FF00	/* Interrupt mask */
#define ST_IM7		0x00008000	/* Interrupt 7 */
#define ST_IM6		0x00004000	/* Interrupt 6 */
#define ST_IM5		0x00002000	/* Interrupt 5 */
#define ST_IM4		0x00001000	/* Interrupt 4 */
#define ST_IM3		0x00000800	/* Interrupt 3 */
#define ST_IM2		0x00000400	/* Interrupt 2 */
#define ST_IM1		0x00000200	/* Interrupt 1 */
#define ST_IM0		0x00000100	/* Interrupt 0 */
#define ST_KX		0x00000080	/* Kernel 64-bit addressing */
#define ST_SX		0x00000040	/* Supervisor 64-bit addressing */
#define ST_UX		0x00000020	/* User 64-bit addressing */
#define ST_KSU		0x00000018	/* Kernel/Supervisor/User mode */
#define ST_ERL		0x00000004	/* Error level */
#define ST_EXL		0x00000002	/* Exception level */
#define ST_IE		0x00000001	/* Interrupt enable */

/* Cause register (CP0_CAUSE) bit definitions */
#define CAUSE_BD	0x80000000	/* Branch delay */
#define CAUSE_CE	0x30000000	/* Coprocessor error */
#define CAUSE_IV	0x00800000	/* Interrupt vector */
#define CAUSE_WP	0x00400000	/* Watchpoint */
#define CAUSE_IP	0x0000FF00	/* Interrupt pending */
#define CAUSE_EXCCODE	0x0000007C	/* Exception code */

/* Exception codes */
#define EXC_INT		0	/* Interrupt */
#define EXC_MOD		1	/* TLB modification */
#define EXC_TLBL	2	/* TLB load/fetch */
#define EXC_TLBS	3	/* TLB store */
#define EXC_ADEL	4	/* Address error (load/fetch) */
#define EXC_ADES	5	/* Address error (store) */
#define EXC_IBE		6	/* Bus error (instruction) */
#define EXC_DBE		7	/* Bus error (data) */
#define EXC_SYS		8	/* Syscall */
#define EXC_BP		9	/* Breakpoint */
#define EXC_RI		10	/* Reserved instruction */
#define EXC_CPU		11	/* Coprocessor unusable */
#define EXC_OV		12	/* Arithmetic overflow */
#define EXC_TR		13	/* Trap */
#define EXC_FPE		15	/* Floating point exception */
#define EXC_WATCH	23	/* Watchpoint */
#define EXC_MCHECK	24	/* Machine check */

/* Frame alignment */
#ifndef __LANGUAGE_ASSEMBLY
#define ALIGNMENT 8
#endif /* __LANGUAGE_ASSEMBLY */

/* Breakpoint instruction */
#define BREAKPOINT_TRAP break	0

/* KDB support */
#define MACH_KDB 0

#endif	/* _MIPS_ASM_H_ */
