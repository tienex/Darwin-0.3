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
 * MIPS exception and trap handling
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/signal.h>

#include <mach/exception.h>
#include <machdep/mips/trap.h>
#include <machdep/mips/pcb.h>
#include <machdep/mips/pmap.h>

/* Exception statistics */
static unsigned int trap_stats[T_NEXC];

/* Exception names for debugging */
static const char *trap_names[T_NEXC] = {
	"Interrupt",			/* 0 */
	"TLB modification",		/* 1 */
	"TLB load/fetch",		/* 2 */
	"TLB store",			/* 3 */
	"Address error (load)",		/* 4 */
	"Address error (store)",	/* 5 */
	"Bus error (instruction)",	/* 6 */
	"Bus error (data)",		/* 7 */
	"System call",			/* 8 */
	"Breakpoint",			/* 9 */
	"Reserved instruction",		/* 10 */
	"Coprocessor unusable",		/* 11 */
	"Arithmetic overflow",		/* 12 */
	"Trap",				/* 13 */
	"Reserved",			/* 14 */
	"Floating point exception",	/* 15 */
	"Reserved",			/* 16 */
	"Reserved",			/* 17 */
	"Reserved",			/* 18 */
	"Reserved",			/* 19 */
	"Reserved",			/* 20 */
	"Reserved",			/* 21 */
	"Reserved",			/* 22 */
	"Watchpoint",			/* 23 */
	"Machine check",		/* 24 */
};

/*
 * Main trap handler
 * Called from assembly exception handler with saved state
 */
void
trap(struct mips_saved_state *state)
{
	unsigned int exccode;
	unsigned int cause;
	int user_mode;

	/* Get exception code from Cause register */
	cause = state->cause;
	exccode = (cause >> 2) & 0x1F;

	/* Check if we were in user mode */
	user_mode = (state->status & 0x00000010) ? 0 : 1;

	/* Update statistics */
	if (exccode < T_NEXC) {
		trap_stats[exccode]++;
	}

	/* Handle exception based on type */
	switch (exccode) {
	case T_INT:
		/* Interrupt */
		mips_interrupt_handler(state);
		break;

	case T_TLB_MOD:
	case T_TLB_LD:
	case T_TLB_ST:
		/* TLB exception */
		mips_tlb_exception(state, exccode);
		break;

	case T_ADDR_ERR_LD:
	case T_ADDR_ERR_ST:
		/* Address error */
		printf("MIPS address error at PC=0x%llx, BadVAddr=0x%llx\n",
		       (unsigned long long)state->epc,
		       (unsigned long long)state->badvaddr);
		if (user_mode) {
			/* Send signal to user process */
			/* TODO: Implement signal sending */
		} else {
			panic("Kernel address error");
		}
		break;

	case T_BUS_ERR_IF:
	case T_BUS_ERR_LD:
		/* Bus error */
		printf("MIPS bus error at PC=0x%llx\n",
		       (unsigned long long)state->epc);
		if (!user_mode) {
			panic("Kernel bus error");
		}
		break;

	case T_SYSCALL:
		/* System call */
		mips_syscall_handler(state);
		break;

	case T_BREAK:
		/* Breakpoint */
		printf("MIPS breakpoint at PC=0x%llx\n",
		       (unsigned long long)state->epc);
		/* TODO: Handle debugger breakpoint */
		break;

	case T_RES_INST:
		/* Reserved instruction */
		printf("MIPS reserved instruction at PC=0x%llx\n",
		       (unsigned long long)state->epc);
		if (user_mode) {
			/* TODO: Send SIGILL to user process */
		} else {
			panic("Kernel reserved instruction");
		}
		break;

	case T_COP_UNUSABLE:
		/* Coprocessor unusable */
		mips_cop_unusable(state);
		break;

	case T_OVFLOW:
		/* Arithmetic overflow */
		printf("MIPS overflow at PC=0x%llx\n",
		       (unsigned long long)state->epc);
		if (user_mode) {
			/* TODO: Send SIGFPE to user process */
		}
		break;

	case T_TRAP:
		/* Trap instruction */
		printf("MIPS trap at PC=0x%llx\n",
		       (unsigned long long)state->epc);
		break;

	case T_FPE:
		/* Floating point exception */
		printf("MIPS FPE at PC=0x%llx\n",
		       (unsigned long long)state->epc);
		/* TODO: Handle FPU exception */
		break;

	case T_WATCH:
		/* Watchpoint */
		printf("MIPS watchpoint at PC=0x%llx\n",
		       (unsigned long long)state->epc);
		break;

	default:
		/* Unknown exception */
		printf("MIPS unknown exception %d at PC=0x%llx\n",
		       exccode, (unsigned long long)state->epc);
		if (!user_mode) {
			panic("Unknown kernel exception");
		}
		break;
	}
}

/*
 * Handle interrupt
 */
void
mips_interrupt_handler(struct mips_saved_state *state)
{
	unsigned int cause, status, pending;

	cause = state->cause;
	status = state->status;

	/* Get pending interrupts (Cause.IP & Status.IM) */
	pending = (cause & status & 0xFF00) >> 8;

	/* Handle interrupts in priority order */
	/* TODO: Implement interrupt dispatch */

	printf("MIPS interrupt: pending=0x%02x\n", pending);
}

/*
 * Handle TLB exception
 */
void
mips_tlb_exception(struct mips_saved_state *state, unsigned int exccode)
{
	/* TODO: Implement TLB refill */
	printf("MIPS TLB exception: code=%d, BadVAddr=0x%llx\n",
	       exccode, (unsigned long long)state->badvaddr);

	/* For now, panic on kernel TLB miss */
	if (!(state->status & 0x00000010)) {
		panic("Kernel TLB exception");
	}
}

/*
 * Handle coprocessor unusable exception
 */
void
mips_cop_unusable(struct mips_saved_state *state)
{
	unsigned int cause;
	unsigned int cop;

	cause = state->cause;
	cop = (cause >> 28) & 0x3;

	if (cop == 1) {
		/* FPU unusable - enable it */
		state->status |= 0x20000000;	/* Set CU1 bit */
	} else {
		printf("MIPS coprocessor %d unusable\n", cop);
	}
}

/*
 * Handle system call
 */
void
mips_syscall_handler(struct mips_saved_state *state)
{
	/* TODO: Implement system call dispatch */
	printf("MIPS syscall: v0=%lld\n",
	       (unsigned long long)state->v0);

	/* Advance PC past syscall instruction */
	state->epc += 4;
}

/*
 * Print trap statistics
 */
void
trap_dump_stats(void)
{
	int i;

	printf("MIPS trap statistics:\n");
	for (i = 0; i < T_NEXC; i++) {
		if (trap_stats[i] > 0) {
			printf("  %2d: %-25s %10u\n", i,
			       (i < sizeof(trap_names)/sizeof(trap_names[0])) ?
			       trap_names[i] : "Unknown",
			       trap_stats[i]);
		}
	}
}
