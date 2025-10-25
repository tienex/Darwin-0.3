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
#include <vm/vm_kern.h>

/*
 * Finish a fork operation
 */
void
cpu_fork(
	struct proc	*p1,
	struct proc	*p2
)
{
	/* Set up child process registers */
}

/*
 * Set user registers for exec
 */
void
setregs(
	struct proc	*p,
	u_long		entry,
	u_long		stack
)
{
	/* Set up initial user registers */
}

/*
 * Move pages from one kernel address to another
 */
void
pagemove(
	caddr_t		from,
	caddr_t		to,
	int		size
)
{
	/* Move pages in kernel space */
	bcopy(from, to, size);
}

/*
 * Map user I/O request into kernel space
 */
int
vmapbuf(struct buf *bp)
{
	/* Map buffer into kernel virtual address space */
	return 0;
}

/*
 * Unmap user I/O request from kernel space
 */
void
vunmapbuf(struct buf *bp)
{
	/* Unmap buffer from kernel virtual address space */
}

/*
 * Force reset the processor by using an invalid address
 */
void
boot(int howto)
{
	/* Reboot the system */
	for (;;) {
		/* Wait for interrupt or reset */
		__asm__ volatile("wfi");
	}
}
