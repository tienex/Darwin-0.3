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
 * RISC-V trap definitions
 */

#ifndef	_MACHDEP_RISCV_TRAP_H_
#define _MACHDEP_RISCV_TRAP_H_

/*
 * RISC-V exception causes (from RISC-V Privileged Spec)
 */
#define CAUSE_MISALIGNED_FETCH     0
#define CAUSE_FETCH_ACCESS         1
#define CAUSE_ILLEGAL_INSTRUCTION  2
#define CAUSE_BREAKPOINT           3
#define CAUSE_MISALIGNED_LOAD      4
#define CAUSE_LOAD_ACCESS          5
#define CAUSE_MISALIGNED_STORE     6
#define CAUSE_STORE_ACCESS         7
#define CAUSE_USER_ECALL           8
#define CAUSE_SUPERVISOR_ECALL     9
#define CAUSE_HYPERVISOR_ECALL     10
#define CAUSE_MACHINE_ECALL        11
#define CAUSE_FETCH_PAGE_FAULT     12
#define CAUSE_LOAD_PAGE_FAULT      13
#define CAUSE_STORE_PAGE_FAULT     15

/* Interrupt bit in scause register */
#define CAUSE_INTERRUPT_BIT        (1UL << (__riscv_xlen - 1))

/*
 * RISC-V interrupt causes
 */
#define IRQ_S_SOFT                 1
#define IRQ_M_SOFT                 3
#define IRQ_S_TIMER                5
#define IRQ_M_TIMER                7
#define IRQ_S_EXT                  9
#define IRQ_M_EXT                  11

/*
 * Trap frame structure - saved on kernel stack on trap entry
 */
struct trapframe {
	unsigned long pc;       /* Program counter (sepc) */
	unsigned long ra;       /* Return address (x1) */
	unsigned long sp;       /* Stack pointer (x2) */
	unsigned long gp;       /* Global pointer (x3) */
	unsigned long tp;       /* Thread pointer (x4) */
	unsigned long t0;       /* Temporary (x5) */
	unsigned long t1;       /* Temporary (x6) */
	unsigned long t2;       /* Temporary (x7) */
	unsigned long s0;       /* Saved register (x8/fp) */
	unsigned long s1;       /* Saved register (x9) */
	unsigned long a0;       /* Argument/return (x10) */
	unsigned long a1;       /* Argument/return (x11) */
	unsigned long a2;       /* Argument (x12) */
	unsigned long a3;       /* Argument (x13) */
	unsigned long a4;       /* Argument (x14) */
	unsigned long a5;       /* Argument (x15) */
	unsigned long a6;       /* Argument (x16) */
	unsigned long a7;       /* Argument (x17) */
	unsigned long s2;       /* Saved register (x18) */
	unsigned long s3;       /* Saved register (x19) */
	unsigned long s4;       /* Saved register (x20) */
	unsigned long s5;       /* Saved register (x21) */
	unsigned long s6;       /* Saved register (x22) */
	unsigned long s7;       /* Saved register (x23) */
	unsigned long s8;       /* Saved register (x24) */
	unsigned long s9;       /* Saved register (x25) */
	unsigned long s10;      /* Saved register (x26) */
	unsigned long s11;      /* Saved register (x27) */
	unsigned long t3;       /* Temporary (x28) */
	unsigned long t4;       /* Temporary (x29) */
	unsigned long t5;       /* Temporary (x30) */
	unsigned long t6;       /* Temporary (x31) */
	unsigned long cause;    /* Exception cause (scause) */
	unsigned long tval;     /* Trap value (stval/sbadaddr) */
	unsigned long status;   /* Status register (sstatus) */
};

#define TRAPFRAME_SIZE  (34 * 8)  /* 34 registers * 8 bytes (RV64) */

#ifndef __ASSEMBLER__

/* Trap handler prototypes */
void trap_handler(unsigned long cause, unsigned long tval,
                  struct trapframe *tf, int from_user);
void user_trap(unsigned long cause, unsigned long tval, struct trapframe *tf);
void kernel_trap(unsigned long cause, unsigned long tval, struct trapframe *tf);
void syscall_handler(struct trapframe *tf);

#endif /* !__ASSEMBLER__ */

#endif	/* _MACHDEP_RISCV_TRAP_H_ */
