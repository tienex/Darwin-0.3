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
 * RISC-V Family: Trap and exception handlers.
 */

#include <mach/mach_types.h>
#include <mach/exception.h>
#include <kern/syscall_sw.h>
#include <vm/vm_kern.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/user.h>

/* RISC-V exception causes from specification */
#define CAUSE_INST_ACCESS_FAULT		1
#define CAUSE_ILLEGAL_INST		2
#define CAUSE_BREAKPOINT		3
#define CAUSE_LOAD_MISALIGN		4
#define CAUSE_LOAD_ACCESS_FAULT		5
#define CAUSE_STORE_MISALIGN		6
#define CAUSE_STORE_ACCESS_FAULT	7
#define CAUSE_ECALL_U			8
#define CAUSE_ECALL_S			9
#define CAUSE_ECALL_M			11
#define CAUSE_INST_PAGE_FAULT		12
#define CAUSE_LOAD_PAGE_FAULT		13
#define CAUSE_STORE_PAGE_FAULT		15

/*
 * User trap handler
 */
void
user_trap(unsigned long cause, unsigned long tval, void *state)
{
	int _exception = 0, code = 0, subcode = 0;
	thread_t thread = current_thread();

	switch (cause) {
	case CAUSE_INST_ACCESS_FAULT:
		_exception = EXC_BAD_ACCESS;
		code = EXC_RISCV_INST_ACCESS_FAULT;
		subcode = tval;
		break;

	case CAUSE_ILLEGAL_INST:
		_exception = EXC_BAD_INSTRUCTION;
		code = EXC_RISCV_ILLEGAL_INST;
		subcode = tval;
		break;

	case CAUSE_BREAKPOINT:
		_exception = EXC_BREAKPOINT;
		code = EXC_RISCV_BREAKPOINT;
		break;

	case CAUSE_LOAD_MISALIGN:
	case CAUSE_STORE_MISALIGN:
		_exception = EXC_ARITHMETIC;
		code = (cause == CAUSE_LOAD_MISALIGN) ?
			EXC_RISCV_LOAD_MISALIGN : EXC_RISCV_STORE_MISALIGN;
		subcode = tval;
		break;

	case CAUSE_LOAD_ACCESS_FAULT:
	case CAUSE_STORE_ACCESS_FAULT:
		_exception = EXC_SOFTWARE;
		code = (cause == CAUSE_LOAD_ACCESS_FAULT) ?
			EXC_RISCV_LOAD_ACCESS_FAULT : EXC_RISCV_STORE_ACCESS_FAULT;
		subcode = tval;
		break;

	case CAUSE_ECALL_U:
		_exception = EXC_EMULATION;
		code = EXC_RISCV_ECALL_U;
		break;

	case CAUSE_INST_PAGE_FAULT:
	case CAUSE_LOAD_PAGE_FAULT:
	case CAUSE_STORE_PAGE_FAULT:
		_exception = EXC_BAD_ACCESS;
		code = (cause == CAUSE_INST_PAGE_FAULT) ? EXC_RISCV_INST_PAGE_FAULT :
		       (cause == CAUSE_LOAD_PAGE_FAULT) ? EXC_RISCV_LOAD_PAGE_FAULT :
		       EXC_RISCV_STORE_PAGE_FAULT;
		subcode = tval;
		break;

	default:
		printf("Unknown user trap: cause=%lx, tval=%lx\n", cause, tval);
		_exception = EXC_BAD_INSTRUCTION;
		code = cause;
		subcode = tval;
		break;
	}

	if (_exception) {
		exception_triage(_exception, code, subcode);
	}
}

/*
 * Kernel trap handler
 */
void
kernel_trap(unsigned long cause, unsigned long tval, void *state)
{
	switch (cause) {
	case CAUSE_INST_PAGE_FAULT:
	case CAUSE_LOAD_PAGE_FAULT:
	case CAUSE_STORE_PAGE_FAULT:
		/* Handle kernel page fault */
		printf("Kernel page fault: cause=%lx, addr=%lx\n", cause, tval);
		panic("kernel page fault");
		break;

	case CAUSE_BREAKPOINT:
		/* Kernel debugger breakpoint */
		printf("Kernel breakpoint at %lx\n", tval);
		break;

	default:
		printf("Unknown kernel trap: cause=%lx, tval=%lx\n", cause, tval);
		panic("kernel trap");
		break;
	}
}

/*
 * Main trap entry point
 * Called from assembly trap handler in locore.s
 */
void
trap_handler(unsigned long cause, unsigned long tval, void *regs, int from_user)
{
	if (from_user) {
		user_trap(cause, tval, regs);
	} else {
		kernel_trap(cause, tval, regs);
	}
}

/*
 * System call handler
 */
void
syscall_handler(void *state)
{
	/* System call handling - to be implemented */
	printf("syscall_handler: not yet implemented\n");
}
