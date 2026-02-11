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
 * DLX VM Machine-Dependent Code
 */

#include <mach/mach_types.h>
#include <mach/vm_param.h>

#include <kern/thread.h>
#include <kern/task.h>

#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>

#include <machdep/dlx/pmap.h>

/*
 * Finish the address space of a new thread
 */
void
thread_set_child(thread_t child, int pid)
{
	/* Nothing special needed for DLX */
}

/*
 * Set up the context for a new thread
 */
void
thread_dup(thread_t parent, thread_t child)
{
	/* Copy thread context */
	/* In real implementation, would copy register state */
}

/*
 * Move pages from one physical page to another
 */
void
pmap_copy_page(vm_offset_t src, vm_offset_t dst)
{
	bcopy((void *)src, (void *)dst, PAGE_SIZE);

	/* Flush cache if necessary */
	/* DLX may need cache coherency operations here */
}

/*
 * Zero a physical page
 */
void
pmap_zero_page(vm_offset_t phys)
{
	bzero((void *)phys, PAGE_SIZE);
}

/*
 * Copy user data
 */
int
copyinstr(const void *from, void *to, size_t maxlen, size_t *lencopied)
{
	/* Simple implementation - would need proper user/kernel checks */
	size_t len = strlen((const char *)from) + 1;
	if (len > maxlen)
		len = maxlen;

	bcopy(from, to, len);

	if (lencopied)
		*lencopied = len;

	return 0;
}

/*
 * Copy from user space
 */
int
copyin(const void *from, void *to, size_t len)
{
	/* Simple implementation - would need proper user/kernel checks */
	bcopy(from, to, len);
	return 0;
}

/*
 * Copy to user space
 */
int
copyout(const void *from, void *to, size_t len)
{
	/* Simple implementation - would need proper user/kernel checks */
	bcopy(from, to, len);
	return 0;
}

/*
 * Allocate a kernel stack for a thread
 */
vm_offset_t
kernel_stack_alloc(vm_size_t stack_size)
{
	vm_offset_t stack;

	if (kmem_alloc_wired(kernel_map, &stack, stack_size) != KERN_SUCCESS)
		panic("kernel_stack_alloc: cannot allocate stack");

	return stack;
}

/*
 * Free a kernel stack
 */
void
kernel_stack_free(vm_offset_t stack, vm_size_t stack_size)
{
	kmem_free(kernel_map, stack, stack_size);
}
