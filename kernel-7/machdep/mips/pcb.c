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
 * MIPS Process Control Block (PCB) management
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/user.h>

#include <mach/mips/thread_status.h>
#include <machdep/mips/pcb.h>
#include <machdep/mips/pmap.h>

/*
 * Initialize a new PCB
 */
void
pcb_init(pcb_t pcb)
{
	/* Zero out the entire PCB */
	bzero((caddr_t)pcb, sizeof(struct mips_pcb));

	/* Initialize flags */
	pcb->flags = 0;

	/* Allocate ASID */
	/* TODO: Implement ASID allocation */
	pcb->asid = 0;
}

/*
 * Terminate PCB
 */
void
pcb_terminate(pcb_t pcb)
{
	/* Free ASID */
	/* TODO: Implement ASID deallocation */

	/* Clear FPU ownership if applicable */
	if (pcb->flags & PCB_FPU_OWNER) {
		/* TODO: Clear FPU state */
		pcb->flags &= ~PCB_FPU_OWNER;
	}
}

/*
 * Save user state to PCB
 */
void
pcb_user_to_save(pcb_t pcb, struct mips_thread_state *user_state)
{
	/* Save general purpose registers */
	pcb->s0 = user_state->regs[16];
	pcb->s1 = user_state->regs[17];
	pcb->s2 = user_state->regs[18];
	pcb->s3 = user_state->regs[19];
	pcb->s4 = user_state->regs[20];
	pcb->s5 = user_state->regs[21];
	pcb->s6 = user_state->regs[22];
	pcb->s7 = user_state->regs[23];
	pcb->s8 = user_state->regs[30];	/* fp */

	pcb->gp = user_state->regs[28];
	pcb->sp = user_state->regs[29];
	pcb->ra = user_state->regs[31];
	pcb->pc = user_state->pc;
}

/*
 * Restore user state from PCB
 */
void
pcb_save_to_user(pcb_t pcb, struct mips_thread_state *user_state)
{
	/* Restore general purpose registers */
	user_state->regs[16] = pcb->s0;
	user_state->regs[17] = pcb->s1;
	user_state->regs[18] = pcb->s2;
	user_state->regs[19] = pcb->s3;
	user_state->regs[20] = pcb->s4;
	user_state->regs[21] = pcb->s5;
	user_state->regs[22] = pcb->s6;
	user_state->regs[23] = pcb->s7;
	user_state->regs[30] = pcb->s8;	/* fp */

	user_state->regs[28] = pcb->gp;
	user_state->regs[29] = pcb->sp;
	user_state->regs[31] = pcb->ra;
	user_state->pc = pcb->pc;
}

/*
 * Context switch - save current PCB and restore new PCB
 */
void
pcb_switch(pcb_t old_pcb, pcb_t new_pcb)
{
	/* Save old context */
	if (old_pcb != NULL) {
		/* Save callee-saved registers */
		/* This would be done in assembly */
	}

	/* Load new context */
	if (new_pcb != NULL) {
		/* Restore callee-saved registers */
		/* This would be done in assembly */

		/* Switch ASID if changed */
		/* TODO: Update TLB ASID */
	}
}

/*
 * Allocate and initialize a new PCB for a thread
 */
pcb_t
pcb_create(void)
{
	pcb_t pcb;

	/* Allocate PCB */
	pcb = (pcb_t)kalloc(sizeof(struct mips_pcb));
	if (pcb == NULL) {
		return NULL;
	}

	/* Initialize PCB */
	pcb_init(pcb);

	return pcb;
}

/*
 * Free a PCB
 */
void
pcb_destroy(pcb_t pcb)
{
	if (pcb != NULL) {
		pcb_terminate(pcb);
		kfree((caddr_t)pcb, sizeof(struct mips_pcb));
	}
}

/*
 * Save FPU state to PCB
 */
void
pcb_save_fpu(pcb_t pcb)
{
	/* TODO: Save FPU registers to pcb->pcb_fpregs */
	pcb->flags |= PCB_FPU_USED;
}

/*
 * Restore FPU state from PCB
 */
void
pcb_restore_fpu(pcb_t pcb)
{
	/* TODO: Restore FPU registers from pcb->pcb_fpregs */
	pcb->flags |= PCB_FPU_OWNER;
}

/*
 * Enable FPU for current thread
 */
void
pcb_enable_fpu(void)
{
	unsigned int status;

	/* Read Status register */
	__asm__ __volatile__(
		"mfc0 %0, $12\n\t"
		: "=r" (status)
	);

	/* Set CU1 bit to enable FPU */
	status |= 0x20000000;

	/* Write Status register */
	__asm__ __volatile__(
		"mtc0 %0, $12\n\t"
		"nop\n\t"
		:: "r" (status)
	);
}

/*
 * Disable FPU for current thread
 */
void
pcb_disable_fpu(void)
{
	unsigned int status;

	/* Read Status register */
	__asm__ __volatile__(
		"mfc0 %0, $12\n\t"
		: "=r" (status)
	);

	/* Clear CU1 bit to disable FPU */
	status &= ~0x20000000;

	/* Write Status register */
	__asm__ __volatile__(
		"mtc0 %0, $12\n\t"
		"nop\n\t"
		:: "r" (status)
	);
}
