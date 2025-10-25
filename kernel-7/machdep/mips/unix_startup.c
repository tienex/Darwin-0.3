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
 * MIPS:	BSD Unix subsystem startup
 *
 * Initialize BSD subsystem for MIPS architecture.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/reboot.h>
#include <sys/conf.h>
#include <sys/msgbuf.h>

#include <kern/thread.h>
#include <kern/task.h>

#include <vm/vm_kern.h>
#include <vm/vm_page.h>

#include <machdep/mips/pmap.h>

/*
 * External functions
 */
extern void mips_clock_init(void);
extern void rtc_init(void);
extern void configure(void);

/*
 * Machine-dependent startup code
 */
void
machine_startup(void)
{
	/*
	 * Initialize machine-specific subsystems
	 */

	/* Initialize clock */
	mips_clock_init();

	/* Initialize real-time clock if present */
	rtc_init();

	printf("MIPS machine startup complete\n");
}

/*
 * Slave CPU startup (for multiprocessor systems)
 */
void
slave_machine_init(void)
{
	/* Initialize slave CPU */
	/* Set up per-CPU data structures */
	/* Initialize local timer */

	printf("Slave CPU initialized\n");
}

/*
 * Set up initial process 0
 */
void
mips_init_process0(void)
{
	thread_t		thread;
	task_t			task;
	struct proc		*p;

	/* Get process 0 structures */
	p = (struct proc *)&proc0;
	task = kernel_task;
	thread = current_thread();

	/* Set up process 0 as kernel task */
	p->task = task;
	p->p_flag = P_INMEM | P_SYSTEM;
	p->p_stat = SRUN;
	p->p_nice = NZERO;

	/* No special MIPS setup needed */
}

/*
 * Machine-dependent reboot preparation
 */
void
machine_reboot(int how)
{
	/* Disable interrupts */
	unsigned int sr = mips_get_sr();
	sr &= ~0x00000001;	/* Clear IE bit */
	mips_set_sr(sr);

	/* Sync disks if clean reboot */
	if ((how & RB_NOSYNC) == 0) {
		printf("Syncing disks...\n");
		sync(current_proc(), (void *)NULL, (int *)NULL);
	}

	/* Hardware-specific reboot */
	if (how & RB_HALT) {
		printf("Halting...\n");
		/* Enter infinite loop */
		for (;;)
			__asm__ volatile("wait");
	} else {
		printf("Rebooting...\n");
		/* Trigger hardware reset */
		/* Implementation depends on board */
		/* For now, infinite loop */
		for (;;)
			__asm__ volatile("wait");
	}
}

/*
 * Configure root device
 */
void
setroot(void)
{
	/* Determine root device */
	/* For now, assume first configured disk */

	/* Would normally scan devices and match with boot arguments */
}

/*
 * Autoconfiguration
 * Probe and attach devices
 */
void
configure(void)
{
	/* Configure interrupt controller */
	/* Probe system bus */
	/* Attach console */
	/* Attach block devices */
	/* Attach network interfaces */

	printf("Device autoconfiguration complete\n");

	/* Set root device */
	setroot();
}

/*
 * External declarations
 */
extern unsigned int mips_get_sr(void);
extern void mips_set_sr(unsigned int);
extern void sync(struct proc *, void *, int *);
extern struct proc proc0;
extern task_t kernel_task;
