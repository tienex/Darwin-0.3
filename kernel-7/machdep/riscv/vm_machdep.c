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
 * RISC-V VM machine-dependent code
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <kern/thread.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <machdep/riscv/thread.h>

/*
 * Finish a fork operation - set up child process state
 */
void
cpu_fork(struct proc *p1, struct proc *p2)
{
	thread_t parent_thread, child_thread;
	struct pcb *parent_pcb, *child_pcb;
	struct riscv_thread_state *child_state;

	/* Get parent and child threads */
	parent_thread = (thread_t)p1->task->thread_list.next;
	child_thread = (thread_t)p2->task->thread_list.next;

	if (!parent_thread || !child_thread)
		return;

	/* Ensure PCBs exist */
	if (!child_thread->pcb)
		pcb_init(child_thread);

	parent_pcb = parent_thread->pcb;
	child_pcb = child_thread->pcb;

	if (!parent_pcb || !child_pcb)
		return;

	/* Copy parent's register state to child */
	child_pcb->ss = parent_pcb->ss;
	child_pcb->fs = parent_pcb->fs;
	child_pcb->es = parent_pcb->es;
	child_pcb->flags = parent_pcb->flags & ~(PCB_FPVALID);

	/* Child returns 0 from fork() - set a0 (return value register) to 0 */
	child_state = &child_pcb->ss;
	child_state->a0 = 0;
	child_state->a1 = 1;  /* Second return value (is_child flag) */

	/* Child gets parent's PC to return to same point */
	/* PC is already copied from parent */
}

/*
 * Set user registers for exec - set up initial program state
 */
void
setregs(struct proc *p, u_long entry, u_long stack)
{
	thread_t thread;
	struct pcb *pcb;
	struct riscv_thread_state *state;

	/* Get thread for this process */
	thread = (thread_t)p->task->thread_list.next;
	if (!thread)
		return;

	/* Ensure PCB exists */
	if (!thread->pcb)
		pcb_init(thread);

	pcb = thread->pcb;
	if (!pcb)
		return;

	/* Clear all user register state */
	state = &pcb->ss;
	bzero(state, sizeof(struct riscv_thread_state));

	/* Set up entry point and stack */
	state->pc = entry;
	state->sp = stack;

	/* Set up initial arguments (argc, argv, envp passed in registers) */
	/* These will be set up by exec code - we just initialize to 0 */
	state->a0 = 0;  /* argc */
	state->a1 = 0;  /* argv */
	state->a2 = 0;  /* envp */

	/* Clear floating point state */
	bzero(&pcb->fs, sizeof(struct riscv_float_state));
	pcb->flags &= ~PCB_FPVALID;

	/* Clear exception state */
	bzero(&pcb->es, sizeof(struct riscv_exception_state));
}

/*
 * Move pages from one kernel virtual address to another
 */
void
pagemove(caddr_t from, caddr_t to, int size)
{
	vm_offset_t pa;
	vm_offset_t from_va = (vm_offset_t)from;
	vm_offset_t to_va = (vm_offset_t)to;

	/* Move pages by remapping - more efficient than copying */
	while (size > 0) {
		/* Get physical address of source page */
		pa = pmap_extract(kernel_pmap, from_va);

		if (pa) {
			/* Unmap source */
			pmap_remove(kernel_pmap, from_va, from_va + PAGE_SIZE);

			/* Map to destination */
			pmap_enter(kernel_pmap, to_va, pa,
			          VM_PROT_READ | VM_PROT_WRITE, FALSE);
		}

		from_va += PAGE_SIZE;
		to_va += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
}

/*
 * Map user I/O request into kernel virtual address space
 */
int
vmapbuf(struct buf *bp)
{
	vm_offset_t kva, uva;
	vm_offset_t pa;
	vm_size_t size, off;

	if ((bp->b_flags & B_PHYS) == 0)
		panic("vmapbuf: not B_PHYS");

	/* Get user address and size */
	uva = (vm_offset_t)bp->b_un.b_addr;
	size = bp->b_bcount;
	off = uva & PAGE_MASK;

	/* Round to page boundaries */
	uva = trunc_page(uva);
	size = round_page(off + size);

	/* Allocate kernel virtual address space */
	kva = kmem_alloc_pageable(kernel_map, size);
	if (kva == 0)
		return ENOMEM;

	/* Save original address */
	bp->b_saveaddr = bp->b_un.b_addr;

	/* Set buffer address to kernel mapping */
	bp->b_un.b_addr = (caddr_t)(kva + off);

	/* Map user pages into kernel space */
	while (size > 0) {
		/* Get physical address of user page */
		pa = pmap_extract(vm_map_pmap(bp->b_proc->task->map), uva);

		if (pa == 0)
			panic("vmapbuf: null page frame");

		/* Map into kernel space */
		pmap_enter(kernel_pmap, kva, pa,
		          VM_PROT_READ | VM_PROT_WRITE, FALSE);

		kva += PAGE_SIZE;
		uva += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	return 0;
}

/*
 * Unmap user I/O request from kernel virtual address space
 */
void
vunmapbuf(struct buf *bp)
{
	vm_offset_t kva;
	vm_size_t size, off;

	if ((bp->b_flags & B_PHYS) == 0)
		panic("vunmapbuf: not B_PHYS");

	/* Get kernel address */
	kva = (vm_offset_t)bp->b_un.b_addr;
	off = kva & PAGE_MASK;
	kva = trunc_page(kva);
	size = round_page(bp->b_bcount + off);

	/* Unmap from kernel space */
	pmap_remove(kernel_pmap, kva, kva + size);

	/* Free kernel virtual address space */
	kmem_free(kernel_map, kva, size);

	/* Restore original address */
	bp->b_un.b_addr = bp->b_saveaddr;
	bp->b_saveaddr = 0;
}

/*
 * Force reset the processor
 */
void
boot(int howto)
{
	/* Disable interrupts */
	__asm__ __volatile__("csrci sstatus, 0x2");

	/* Print reboot message */
	printf("Rebooting...\n");

	/* Wait a moment for output to flush */
	delay(1000000);

	/* Try SBI shutdown/reboot if available */
	/* SBI_SHUTDOWN = 8, SBI_RESET = 9 */
	/* This is a platform-specific operation */

	/* If that doesn't work, loop forever */
	for (;;) {
		__asm__ __volatile__("wfi");  /* Wait for interrupt */
	}
}

/*
 * CPU idle routine
 */
void
cpu_idle(void)
{
	/* Wait for interrupt (low-power mode) */
	__asm__ __volatile__("wfi");
}

/*
 * Get current CPU number
 */
int
cpu_number(void)
{
	/* Read hart ID from tp register or mhartid CSR */
	/* For now, assume single CPU */
	return 0;
}

/*
 * Set user stack pointer
 */
void
set_user_sp(thread_t thread, unsigned long sp)
{
	if (thread && thread->pcb) {
		thread->pcb->ss.sp = sp;
	}
}

/*
 * Get user stack pointer
 */
unsigned long
get_user_sp(thread_t thread)
{
	if (thread && thread->pcb) {
		return thread->pcb->ss.sp;
	}
	return 0;
}
