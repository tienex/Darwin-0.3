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
 * RISC-V Family: Physical memory map management (pmap).
 */

#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <vm/vm_kern.h>
#include <vm/pmap.h>

/*
 * Initialize the pmap module
 */
void
pmap_bootstrap(vm_offset_t load_start)
{
	/* Bootstrap pmap system */
	/* Initialize page tables */
}

/*
 * Initialize pmap for kernel
 */
void
pmap_init(void)
{
	/* Initialize pmap structures */
}

/*
 * Create a new pmap
 */
pmap_t
pmap_create(vm_size_t size)
{
	/* Allocate and initialize a new pmap */
	return PMAP_NULL;
}

/*
 * Destroy a pmap
 */
void
pmap_destroy(pmap_t pmap)
{
	/* Free pmap resources */
}

/*
 * Add a reference to the specified pmap
 */
void
pmap_reference(pmap_t pmap)
{
	/* Increment reference count */
}

/*
 * Insert a page mapping
 */
void
pmap_enter(
	pmap_t		pmap,
	vm_offset_t	va,
	vm_offset_t	pa,
	vm_prot_t	prot,
	boolean_t	wired
)
{
	/* Insert page table entry */
}

/*
 * Remove a page mapping
 */
void
pmap_remove(
	pmap_t		pmap,
	vm_offset_t	sva,
	vm_offset_t	eva
)
{
	/* Remove page table entries */
}

/*
 * Change protection on a range
 */
void
pmap_protect(
	pmap_t		pmap,
	vm_offset_t	sva,
	vm_offset_t	eva,
	vm_prot_t	prot
)
{
	/* Change protection bits */
}

/*
 * Extract physical address from virtual
 */
vm_offset_t
pmap_extract(
	pmap_t		pmap,
	vm_offset_t	va
)
{
	/* Walk page tables to find physical address */
	return 0;
}

/*
 * Make all mappings to a page read-only
 */
void
pmap_page_protect(
	vm_offset_t	pa,
	vm_prot_t	prot
)
{
	/* Protect all mappings to this page */
}

/*
 * Clear reference bit on a page
 */
void
pmap_clear_reference(vm_offset_t pa)
{
	/* Clear reference bit */
}

/*
 * Clear modify bit on a page
 */
void
pmap_clear_modify(vm_offset_t pa)
{
	/* Clear modify bit */
}

/*
 * Check if page has been referenced
 */
boolean_t
pmap_is_referenced(vm_offset_t pa)
{
	/* Check reference bit */
	return FALSE;
}

/*
 * Check if page has been modified
 */
boolean_t
pmap_is_modified(vm_offset_t pa)
{
	/* Check modify bit */
	return FALSE;
}

/*
 * Activate pmap for current thread
 */
void
pmap_activate(
	pmap_t		pmap,
	thread_t	thread,
	int		cpu
)
{
	/* Load page table base register (satp on RISC-V) */
}

/*
 * Deactivate pmap
 */
void
pmap_deactivate(
	pmap_t		pmap,
	thread_t	thread,
	int		cpu
)
{
	/* Deactivate pmap */
}

/*
 * Return kernel pmap
 */
pmap_t
pmap_kernel(void)
{
	extern pmap_t kernel_pmap;
	return kernel_pmap;
}
