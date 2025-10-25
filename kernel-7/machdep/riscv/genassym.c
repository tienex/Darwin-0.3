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
 * genassym.c for RISC-V
 *
 * This file is used to produce an assembly file which, intermingled with
 * unuseful assembly code, has all the necessary definitions emitted.
 * This assembly file is then postprocessed with sed to extract only these
 * definitions and thus the final assyms.s is created.
 *
 * This convoluted means is necessary since the structure alignment
 * and packing may be different between the host machine and the
 * target so we are forced into using the cross compiler to generate
 * the values, but we cannot run anything on the target machine.
 */

#include <stddef.h>
#include <mach/mach_types.h>
#include <mach/riscv/thread_status.h>

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/vmparam.h>
#include <sys/dir.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/mbuf.h>
#include <sys/msgbuf.h>

#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/task.h>

#include <machdep/riscv/thread.h>
#include <machdep/riscv/trap.h>

/* Process Control Block offsets */
int _PCB_FLOAT_STATE = offsetof(struct pcb, fs);
int _PCB_SAVED_STATE = offsetof(struct pcb, ss);
int _PCB_EXCEPTION_STATE = offsetof(struct pcb, es);
int _PCB_KSP = offsetof(struct pcb, ksp);
int _PCB_CTHREAD_SELF = offsetof(struct pcb, cthread_self);
int _PCB_FLAGS = offsetof(struct pcb, flags);

/* PCB size */
int _PCB_SIZE = sizeof(struct pcb);

/* Floating point state offsets */
int _PCB_FS_F0 = offsetof(struct pcb, fs.fpregs[0]);
int _PCB_FS_F1 = offsetof(struct pcb, fs.fpregs[1]);
int _PCB_FS_F2 = offsetof(struct pcb, fs.fpregs[2]);
int _PCB_FS_F3 = offsetof(struct pcb, fs.fpregs[3]);
int _PCB_FS_F4 = offsetof(struct pcb, fs.fpregs[4]);
int _PCB_FS_F5 = offsetof(struct pcb, fs.fpregs[5]);
int _PCB_FS_F6 = offsetof(struct pcb, fs.fpregs[6]);
int _PCB_FS_F7 = offsetof(struct pcb, fs.fpregs[7]);
int _PCB_FS_F8 = offsetof(struct pcb, fs.fpregs[8]);
int _PCB_FS_F9 = offsetof(struct pcb, fs.fpregs[9]);
int _PCB_FS_F10 = offsetof(struct pcb, fs.fpregs[10]);
int _PCB_FS_F11 = offsetof(struct pcb, fs.fpregs[11]);
int _PCB_FS_F12 = offsetof(struct pcb, fs.fpregs[12]);
int _PCB_FS_F13 = offsetof(struct pcb, fs.fpregs[13]);
int _PCB_FS_F14 = offsetof(struct pcb, fs.fpregs[14]);
int _PCB_FS_F15 = offsetof(struct pcb, fs.fpregs[15]);
int _PCB_FS_F16 = offsetof(struct pcb, fs.fpregs[16]);
int _PCB_FS_F17 = offsetof(struct pcb, fs.fpregs[17]);
int _PCB_FS_F18 = offsetof(struct pcb, fs.fpregs[18]);
int _PCB_FS_F19 = offsetof(struct pcb, fs.fpregs[19]);
int _PCB_FS_F20 = offsetof(struct pcb, fs.fpregs[20]);
int _PCB_FS_F21 = offsetof(struct pcb, fs.fpregs[21]);
int _PCB_FS_F22 = offsetof(struct pcb, fs.fpregs[22]);
int _PCB_FS_F23 = offsetof(struct pcb, fs.fpregs[23]);
int _PCB_FS_F24 = offsetof(struct pcb, fs.fpregs[24]);
int _PCB_FS_F25 = offsetof(struct pcb, fs.fpregs[25]);
int _PCB_FS_F26 = offsetof(struct pcb, fs.fpregs[26]);
int _PCB_FS_F27 = offsetof(struct pcb, fs.fpregs[27]);
int _PCB_FS_F28 = offsetof(struct pcb, fs.fpregs[28]);
int _PCB_FS_F29 = offsetof(struct pcb, fs.fpregs[29]);
int _PCB_FS_F30 = offsetof(struct pcb, fs.fpregs[30]);
int _PCB_FS_F31 = offsetof(struct pcb, fs.fpregs[31]);
int _PCB_FS_FCSR = offsetof(struct pcb, fs.fcsr);

/* Thread state offsets */
int _SS_PC = offsetof(struct riscv_thread_state, pc);
int _SS_RA = offsetof(struct riscv_thread_state, ra);
int _SS_SP = offsetof(struct riscv_thread_state, sp);
int _SS_GP = offsetof(struct riscv_thread_state, gp);
int _SS_TP = offsetof(struct riscv_thread_state, tp);
int _SS_T0 = offsetof(struct riscv_thread_state, t0);
int _SS_T1 = offsetof(struct riscv_thread_state, t1);
int _SS_T2 = offsetof(struct riscv_thread_state, t2);
int _SS_S0 = offsetof(struct riscv_thread_state, s0);
int _SS_S1 = offsetof(struct riscv_thread_state, s1);
int _SS_A0 = offsetof(struct riscv_thread_state, a0);
int _SS_A1 = offsetof(struct riscv_thread_state, a1);
int _SS_A2 = offsetof(struct riscv_thread_state, a2);
int _SS_A3 = offsetof(struct riscv_thread_state, a3);
int _SS_A4 = offsetof(struct riscv_thread_state, a4);
int _SS_A5 = offsetof(struct riscv_thread_state, a5);
int _SS_A6 = offsetof(struct riscv_thread_state, a6);
int _SS_A7 = offsetof(struct riscv_thread_state, a7);
int _SS_S2 = offsetof(struct riscv_thread_state, s2);
int _SS_S3 = offsetof(struct riscv_thread_state, s3);
int _SS_S4 = offsetof(struct riscv_thread_state, s4);
int _SS_S5 = offsetof(struct riscv_thread_state, s5);
int _SS_S6 = offsetof(struct riscv_thread_state, s6);
int _SS_S7 = offsetof(struct riscv_thread_state, s7);
int _SS_S8 = offsetof(struct riscv_thread_state, s8);
int _SS_S9 = offsetof(struct riscv_thread_state, s9);
int _SS_S10 = offsetof(struct riscv_thread_state, s10);
int _SS_S11 = offsetof(struct riscv_thread_state, s11);
int _SS_T3 = offsetof(struct riscv_thread_state, t3);
int _SS_T4 = offsetof(struct riscv_thread_state, t4);
int _SS_T5 = offsetof(struct riscv_thread_state, t5);
int _SS_T6 = offsetof(struct riscv_thread_state, t6);

/* Trap frame offsets */
int _TF_PC = offsetof(struct trapframe, pc);
int _TF_RA = offsetof(struct trapframe, ra);
int _TF_SP = offsetof(struct trapframe, sp);
int _TF_GP = offsetof(struct trapframe, gp);
int _TF_TP = offsetof(struct trapframe, tp);
int _TF_T0 = offsetof(struct trapframe, t0);
int _TF_T1 = offsetof(struct trapframe, t1);
int _TF_T2 = offsetof(struct trapframe, t2);
int _TF_S0 = offsetof(struct trapframe, s0);
int _TF_S1 = offsetof(struct trapframe, s1);
int _TF_A0 = offsetof(struct trapframe, a0);
int _TF_A1 = offsetof(struct trapframe, a1);
int _TF_A2 = offsetof(struct trapframe, a2);
int _TF_A3 = offsetof(struct trapframe, a3);
int _TF_A4 = offsetof(struct trapframe, a4);
int _TF_A5 = offsetof(struct trapframe, a5);
int _TF_A6 = offsetof(struct trapframe, a6);
int _TF_A7 = offsetof(struct trapframe, a7);
int _TF_S2 = offsetof(struct trapframe, s2);
int _TF_S3 = offsetof(struct trapframe, s3);
int _TF_S4 = offsetof(struct trapframe, s4);
int _TF_S5 = offsetof(struct trapframe, s5);
int _TF_S6 = offsetof(struct trapframe, s6);
int _TF_S7 = offsetof(struct trapframe, s7);
int _TF_S8 = offsetof(struct trapframe, s8);
int _TF_S9 = offsetof(struct trapframe, s9);
int _TF_S10 = offsetof(struct trapframe, s10);
int _TF_S11 = offsetof(struct trapframe, s11);
int _TF_T3 = offsetof(struct trapframe, t3);
int _TF_T4 = offsetof(struct trapframe, t4);
int _TF_T5 = offsetof(struct trapframe, t5);
int _TF_T6 = offsetof(struct trapframe, t6);
int _TF_CAUSE = offsetof(struct trapframe, cause);
int _TF_TVAL = offsetof(struct trapframe, tval);
int _TF_STATUS = offsetof(struct trapframe, status);
int _TF_SIZE = sizeof(struct trapframe);

/* Thread structure offsets */
int _THREAD_PCB = offsetof(struct thread, pcb);
int _THREAD_RECOVER = offsetof(struct thread, recover);
int _THREAD_TASK = offsetof(struct thread, task);
int _THREAD_AST = offsetof(struct thread, ast);
int _THREAD_KERNEL_STACK = offsetof(struct thread, kernel_stack);

/* Process structure offsets */
int _P_PRI = offsetof(struct proc, p_priority);
int _P_STAT = offsetof(struct proc, p_stat);
int _P_SIG = offsetof(struct proc, p_siglist);
int _P_FLAG = offsetof(struct proc, p_flag);

/* Process states */
int _SSLEEP = SSLEEP;
int _SRUN = SRUN;

/* AST values */
int _AST_ZILCH = AST_ZILCH;

/* Rusage offset */
int _RU_MINFLT = offsetof(struct rusage, ru_minflt);

/* User profiling offsets */
int _PR_BASE = offsetof(struct uprof, pr_base);
int _PR_SIZE = offsetof(struct uprof, pr_size);
int _PR_OFF = offsetof(struct uprof, pr_off);
int _PR_SCALE = offsetof(struct uprof, pr_scale);

/* uthread offsets */
int _U_AR0 = offsetof(struct uthread, uu_ar0);

/* Kernel return values */
int _KERN_FAILURE = KERN_FAILURE;
int _KERN_SUCCESS = KERN_SUCCESS;

/* VM constants */
int _VM_MIN_KERNEL_ADDRESS = VM_MIN_KERNEL_ADDRESS;
int _VM_MAX_KERNEL_ADDRESS = VM_MAX_KERNEL_ADDRESS;

/* Page size */
int _PAGE_SIZE = PAGE_SIZE;
int _PAGE_SHIFT = PAGE_SHIFT;
