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
 * DLX Exception Handlers
 * Handles TLB misses and other MMU-related exceptions
 */

#include <mach/mach_types.h>
#include <mach/exception.h>

#include <kern/thread.h>
#include <kern/task.h>

#include <vm/vm_fault.h>
#include <vm/vm_kern.h>

#include <machdep/dlx/pmap.h>

/*
 * DLX exception vector table
 * In real hardware, these would be at fixed addresses
 */

/*
 * General exception handler
 * Dispatches to specific handlers based on exception type
 */
void
dlx_exception_handler(int exception_type, vm_offset_t bad_vaddr,
		      int is_write, void *context)
{
	kern_return_t kr;
	vm_map_t map;
	vm_prot_t prot;

	switch (exception_type) {
	case EXC_DLX_TLB_MISS_LOAD:
		/* TLB miss on load - try to refill from page table */
		dlx_tlb_miss_handler(bad_vaddr, 0);

		/* If still not in TLB, it's a real page fault */
		map = current_thread()->map;
		if (map == VM_MAP_NULL)
			map = kernel_map;

		prot = VM_PROT_READ;
		kr = vm_fault(map, trunc_page(bad_vaddr), prot, FALSE, FALSE, NULL, 0);

		if (kr != KERN_SUCCESS) {
			panic("dlx_exception_handler: page fault at 0x%x failed: %d",
			      bad_vaddr, kr);
		}
		break;

	case EXC_DLX_TLB_MISS_STORE:
		/* TLB miss on store - try to refill from page table */
		dlx_tlb_miss_handler(bad_vaddr, 1);

		/* If still not in TLB, it's a real page fault */
		map = current_thread()->map;
		if (map == VM_MAP_NULL)
			map = kernel_map;

		prot = VM_PROT_READ | VM_PROT_WRITE;
		kr = vm_fault(map, trunc_page(bad_vaddr), prot, FALSE, FALSE, NULL, 0);

		if (kr != KERN_SUCCESS) {
			panic("dlx_exception_handler: page fault at 0x%x failed: %d",
			      bad_vaddr, kr);
		}
		break;

	case EXC_DLX_TLB_MOD:
		/* TLB modification (write to read-only page) */
		dlx_tlb_mod_handler(bad_vaddr);

		/* If not resolved, it's a protection violation */
		map = current_thread()->map;
		if (map == VM_MAP_NULL)
			map = kernel_map;

		prot = VM_PROT_WRITE;
		kr = vm_fault(map, trunc_page(bad_vaddr), prot, FALSE, FALSE, NULL, 0);

		if (kr != KERN_SUCCESS) {
			panic("dlx_exception_handler: protection fault at 0x%x failed: %d",
			      bad_vaddr, kr);
		}
		break;

	case EXC_DLX_TLB_LOAD:
	case EXC_DLX_TLB_STORE:
		/* TLB exception - invalid entry or permission violation */
		map = current_thread()->map;
		if (map == VM_MAP_NULL)
			map = kernel_map;

		prot = (exception_type == EXC_DLX_TLB_LOAD) ?
		       VM_PROT_READ : (VM_PROT_READ | VM_PROT_WRITE);

		kr = vm_fault(map, trunc_page(bad_vaddr), prot, FALSE, FALSE, NULL, 0);

		if (kr != KERN_SUCCESS) {
			panic("dlx_exception_handler: TLB fault at 0x%x failed: %d",
			      bad_vaddr, kr);
		}
		break;

	default:
		panic("dlx_exception_handler: unknown exception type %d at 0x%x",
		      exception_type, bad_vaddr);
	}
}

/*
 * Initialize exception handlers
 */
void
dlx_exception_init(void)
{
	/* Set up exception vectors */
	/* In real hardware, would install handlers at fixed addresses */
}
