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
 * MIPS:	Kernel machine-dependent initialization and control
 *
 * Functions for kernel initialization, halt, and reboot.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/reboot.h>

#include <kern/thread.h>
#include <kern/task.h>

#include <vm/vm_kern.h>

/*
 * Halt the system
 */
void
halt_all_cpus(boolean_t reboot)
{
	unsigned int sr;

	/* Disable interrupts */
	sr = mips_get_sr();
	sr &= ~0x00000001;	/* Clear IE bit */
	mips_set_sr(sr);

	if (reboot) {
		printf("Rebooting system...\n");
		/* Hardware reset would go here */
		/* For now, enter wait state */
		for (;;)
			__asm__ volatile("wait");
	} else {
		printf("System halted.\n");
		/* Enter wait state */
		for (;;)
			__asm__ volatile("wait");
	}
}

/*
 * Halt a single CPU (for multiprocessor systems)
 */
void
halt_cpu(void)
{
	/* Put this CPU into wait state */
	unsigned int sr = mips_get_sr();
	sr &= ~0x00000001;	/* Disable interrupts */
	mips_set_sr(sr);

	for (;;)
		__asm__ volatile("wait");
}

/*
 * CPU detection and initialization
 */
void
cpu_init(void)
{
	unsigned int prid;
	unsigned int config;

	/* Read PRId register */
	__asm__ volatile("mfc0 %0, $15" : "=r" (prid));

	/* Read Config register */
	__asm__ volatile("mfc0 %0, $16" : "=r" (config));

	printf("MIPS CPU: PRId=0x%08x Config=0x%08x\n", prid, config);

	/* Decode CPU type */
	switch ((prid >> 8) & 0xff) {
	case 0x00:
		printf("CPU: MIPS R2000/R3000\n");
		break;
	case 0x01:
		printf("CPU: MIPS R6000\n");
		break;
	case 0x02:
	case 0x03:
		printf("CPU: MIPS R4000/R4400\n");
		break;
	case 0x04:
		printf("CPU: MIPS R5000\n");
		break;
	case 0x09:
		printf("CPU: MIPS R10000\n");
		break;
	default:
		printf("CPU: Unknown MIPS processor\n");
	}

	/* Initialize FPU if present */
	if (config & 0x1) {
		/* FPU is present */
		printf("FPU detected\n");
		/* Enable FPU in SR */
		unsigned int sr = mips_get_sr();
		sr |= 0x20000000;	/* Set CU1 bit */
		mips_set_sr(sr);
	}
}

/*
 * Initialize kernel stacks
 */
void
init_kstack(void)
{
	/* Set up kernel stack for each CPU */
	/* Already done by bootstrap */
}

/*
 * Machine check exception handler
 */
void
machine_check(void)
{
	unsigned int epc, cause, badvaddr;

	__asm__ volatile("mfc0 %0, $14" : "=r" (epc));
	__asm__ volatile("mfc0 %0, $13" : "=r" (cause));
	__asm__ volatile("mfc0 %0, $8" : "=r" (badvaddr));

	printf("Machine check exception:\n");
	printf("  EPC      = 0x%08x\n", epc);
	printf("  Cause    = 0x%08x\n", cause);
	printf("  BadVAddr = 0x%08x\n", badvaddr);

	panic("Machine check");
}

/*
 * External declarations
 */
extern unsigned int mips_get_sr(void);
extern void mips_set_sr(unsigned int);
