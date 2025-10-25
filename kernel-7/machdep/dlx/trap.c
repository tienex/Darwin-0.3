/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * Complete DLX Trap and Exception Handling
 */

#include <mach/mach_types.h>
#include <mach/exception.h>

#include <kern/thread.h>
#include <kern/task.h>

#include <vm/vm_fault.h>
#include <vm/vm_kern.h>

#include <machdep/dlx/pmap.h>
#include <machdep/dlx/trap.h>

extern unsigned int dlx_status_register;

/*
 * Page Fault Handler
 */
void
dlx_pagefault_handler(vm_offset_t va, int is_write)
{
	kern_return_t kr;
	vm_map_t map;
	vm_prot_t prot;
	pt_entry_t *pte;
	pmap_t pmap;

	/* Check if page tables are enabled */
	if (!(dlx_status_register & DLX_STATUS_PAGE_TABLE)) {
		printf("Page fault with page tables disabled at 0x%x\n", va);
		panic("dlx_pagefault_handler: page tables not enabled");
	}

	/* Get current map */
	if (va >= VM_MIN_KERNEL_ADDRESS) {
		map = kernel_map;
		pmap = kernel_pmap;
	} else {
		thread_t thread = current_thread();
		if (thread == THREAD_NULL || thread->task == TASK_NULL) {
			panic("Page fault in user space with no current thread");
		}
		map = thread->task->map;
		pmap = vm_map_pmap(map);
	}

	/* Check page table first */
	pte = pmap_pte(pmap, va);

	if (pte && (*pte & DLX_PTE_VALID)) {
		/* Page table entry exists - update TLB if needed */
		if (dlx_status_register & DLX_STATUS_TLB) {
			/* Set reference/dirty bits */
			*pte |= DLX_PTE_REFERENCE;
			if (is_write)
				*pte |= DLX_PTE_DIRTY;

			/* Load into TLB */
			dlx_tlb_insert(va, *pte, DLX_L2_PAGE_SIZE_BITS);
		}
		return;
	}

	/* Real page fault - call VM system */
	prot = VM_PROT_READ | (is_write ? VM_PROT_WRITE : 0);
	kr = vm_fault(map, trunc_page(va), prot, FALSE, FALSE, NULL, 0);

	if (kr != KERN_SUCCESS) {
		if (va >= VM_MIN_KERNEL_ADDRESS) {
			panic("Kernel page fault at 0x%x: %d", va, kr);
		} else {
			/* User page fault - send exception to task */
			printf("User page fault at 0x%x: %d\n", va, kr);
			thread_exception_return();
		}
	}

	/* After successful vm_fault, PTE should be valid */
	pte = pmap_pte(pmap, va);
	if (pte && (*pte & DLX_PTE_VALID)) {
		*pte |= DLX_PTE_REFERENCE;
		if (is_write)
			*pte |= DLX_PTE_DIRTY;

		if (dlx_status_register & DLX_STATUS_TLB) {
			dlx_tlb_insert(va, *pte, DLX_L2_PAGE_SIZE_BITS);
		}
	}
}

/*
 * TLB Fault Handler
 */
void
dlx_tlbfault_handler(vm_offset_t va, int is_write)
{
	pt_entry_t *pte;
	pmap_t pmap;

	/* Check if TLB is enabled */
	if (!(dlx_status_register & DLX_STATUS_TLB)) {
		panic("TLB fault with TLB disabled at 0x%x", va);
	}

	/* Get current pmap */
	if (va >= VM_MIN_KERNEL_ADDRESS) {
		pmap = kernel_pmap;
	} else {
		thread_t thread = current_thread();
		if (thread == THREAD_NULL || thread->task == TASK_NULL) {
			panic("TLB fault in user space with no current thread");
		}
		pmap = vm_map_pmap(thread->task->map);
	}

	/* Look up page table entry */
	if (dlx_status_register & DLX_STATUS_PAGE_TABLE) {
		pte = pmap_pte(pmap, va);

		if (pte && (*pte & DLX_PTE_VALID)) {
			/* Update reference/dirty bits */
			*pte |= DLX_PTE_REFERENCE;
			if (is_write)
				*pte |= DLX_PTE_DIRTY;

			/* Load into TLB */
			dlx_tlb_insert(va, *pte, DLX_L2_PAGE_SIZE_BITS);
			return;
		}
	}

	/* No valid PTE - this is a page fault */
	dlx_pagefault_handler(va, is_write);
}

/*
 * General Exception Handler
 */
void
dlx_exception_handler(unsigned int status, vm_offset_t fault_addr)
{
	unsigned int trapno = status & 0xFF;

	switch (trapno) {
	case TRAP_ILLEGALINST:
		printf("Illegal instruction at 0x%x\n", fault_addr);
		panic("Illegal instruction");
		break;

	case TRAP_ADDRESS:
		printf("Address error at 0x%x\n", fault_addr);
		panic("Address error");
		break;

	case TRAP_ACCESS:
		printf("Access violation at 0x%x\n", fault_addr);
		panic("Access violation");
		break;

	case TRAP_OVERFLOW:
		printf("Arithmetic overflow\n");
		panic("Overflow");
		break;

	case TRAP_DIV0:
		printf("Divide by zero\n");
		panic("Divide by zero");
		break;

	case TRAP_PRIVILEGE:
		printf("Privilege violation at 0x%x\n", fault_addr);
		panic("Privilege violation");
		break;

	case TRAP_FORMAT:
		printf("Malformed instruction at 0x%x\n", fault_addr);
		panic("Format error");
		break;

	case TRAP_TIMER:
		/* Timer interrupt - call scheduler */
		hardclock(NULL);
		break;

	case TRAP_KBD:
		/* Keyboard interrupt - call console handler */
		printf("Keyboard interrupt\n");
		break;

	default:
		printf("Unknown exception %d at 0x%x\n", trapno, fault_addr);
		panic("Unknown exception");
	}
}

/*
 * System Call Handler
 */
void
syscall_handler(void)
{
	thread_t thread = current_thread();

	if (thread == THREAD_NULL || thread->task == TASK_NULL) {
		panic("System call with no current thread");
	}

	/* Get syscall number from saved state */
	/* Call mach_trap_table handler */
	/* Return to user */

	printf("System call handler\n");
}
