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
 * MIPS:	MD VM operations.
 *
 * Machine-dependent virtual memory support functions.
 */

#import <sys/param.h>
#import <sys/buf.h>
#import <sys/errno.h>

#import <kern/thread.h>

#import <vm/vm_kern.h>

#import <machdep/mips/pmap.h>

/*
 * Move pages from one kernel virtual address to another.
 * Both addresses are assumed to reside in the kernel pmap,
 * and size must be a multiple of the page size.
 */
void
pagemove(caddr_t from, caddr_t to, int size)
{
	register pt_entry_t *fpte, *tpte;
	vm_offset_t from_addr = (vm_offset_t)from;
	vm_offset_t to_addr = (vm_offset_t)to;

	if (size % MIPS_PGBYTES)
		panic("pagemove");

	while (size > 0) {
		fpte = pmap_pte(kernel_pmap, from_addr);
		tpte = pmap_pte(kernel_pmap, to_addr);

		if (fpte != PT_ENTRY_NULL && tpte != PT_ENTRY_NULL) {
			*tpte = *fpte;
			*fpte = 0;
		}

		from_addr += MIPS_PGBYTES;
		to_addr += MIPS_PGBYTES;
		size -= MIPS_PGBYTES;
	}

	/* Flush TLB to ensure changes are visible */
	mips_tlb_flush();
}

/*
 * kernacc - check kernel access to kernel memory.
 *
 * Verifies that the kernel can access the specified range of
 * kernel virtual memory with the given protection.
 */
boolean_t
kernacc(
	vm_offset_t 	base,
	unsigned int	len,
	int		op
)
{
	vm_offset_t	end = base + len;
	pt_entry_t	*pte;

	base = trunc_page(base);

	while (base < end) {
		pte = pmap_pte(kernel_pmap, base);
		if (pte == PT_ENTRY_NULL)
			return (FALSE);

		if (!(*pte & MIPS_PTE_VALID))
			return (FALSE);

		/* Check write protection if write access requested */
		if (op == B_WRITE && !(*pte & MIPS_PTE_DIRTY))
			return (FALSE);

		base += PAGE_SIZE;
	}

	return (TRUE);
}

/*
 * useracc - check user access to user memory.
 *
 * Verifies that the current task can access the specified range
 * of user virtual memory with the given protection.
 */
boolean_t
useracc(
	vm_offset_t	base,
	unsigned int	len,
	int		op
)
{
	vm_offset_t	end = base + len;
	pt_entry_t	*pte;
	pmap_t		pmap = current_pmap();

	if (pmap == PMAP_NULL)
		return (FALSE);

	base = trunc_page(base);

	while (base < end) {
		pte = pmap_pte(pmap, base);
		if (pte == PT_ENTRY_NULL)
			return (FALSE);

		if (!(*pte & MIPS_PTE_VALID))
			return (FALSE);

		/* Check write protection if write access requested */
		if (op == B_WRITE && !(*pte & MIPS_PTE_DIRTY))
			return (FALSE);

		base += PAGE_SIZE;
	}

	return (TRUE);
}

/*
 * vslock - lock user pages in memory
 */
void
vslock(
	vm_offset_t	addr,
	unsigned int	len
)
{
	vm_map_pageable(current_task()->map, trunc_page(addr),
		round_page(addr + len), FALSE);
}

/*
 * vsunlock - unlock user pages
 */
void
vsunlock(
	vm_offset_t	addr,
	unsigned int	len,
	int		dirtied
)
{
	vm_map_pageable(current_task()->map, trunc_page(addr),
		round_page(addr + len), TRUE);
}
