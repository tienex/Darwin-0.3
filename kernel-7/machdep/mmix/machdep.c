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
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 *	File:	mmix/machdep.c
 *
 *	Machine-dependent routines for MMIX architecture
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signalvar.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/reboot.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/msgbuf.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/sysctl.h>

#include <kern/task.h>
#include <kern/thread.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>

/*
 * Early kernel initialization
 *
 * Called from start.s with boot_args pointer
 */
void
mmix_init(struct boot_args *args)
{
	extern void exception_init(void);
	extern void pmap_bootstrap(unsigned long long, vm_offset_t *);
	extern void machine_startup(void);

	/* Save boot arguments */
	/* TODO: Store args in global variable */

	/* Initialize exception handling */
	exception_init();

	/* Initialize virtual memory */
	vm_offset_t first_avail = args->phys_base + 0x01000000; /* 16MB */
	pmap_bootstrap(args->phys_mem_size, &first_avail);

	/* Continue with machine-independent initialization */
	machine_startup();
}

/*
 * Machine-specific initialization
 */
void
machine_startup(void)
{
	/* MMIX-specific startup code */
	printf("MMIX Darwin kernel starting...\n");

	/* Initialize console */
	/* Initialize devices */
	/* Start scheduler */
}

/*
 * Stub for machine-dependent system control
 */
int
cpu_sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
           void *newp, size_t newlen, struct proc *p)
{
	return (EOPNOTSUPP);
}

/*
 * Stub for sending a signal to a process
 */
void
sendsig(catcher, sig, mask, code)
	sig_t catcher;
	int sig, mask;
	unsigned code;
{
	/* Signal delivery implementation */
}
