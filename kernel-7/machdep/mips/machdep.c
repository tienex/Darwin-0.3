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
 * MIPS machine-dependent functions
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/reboot.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/msgbuf.h>
#include <sys/mbuf.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/sysctl.h>

#include <mach/machine.h>
#include <mach/mips/thread_status.h>
#include <machdep/mips/pmap.h>
#include <machdep/mips/pcb.h>

/*
 * Machine-dependent startup code
 */
void
machine_startup(void)
{
	/* Initialize machine-specific subsystems */
	printf("MIPS machine startup\n");
}

/*
 * Initialize machine-dependent state
 */
void
machine_init(void)
{
	/* Initialize CPU-specific features */
	printf("MIPS machine init\n");
}

/*
 * Machine-dependent boot information
 */
void
machine_bootinfo(void)
{
	printf("Darwin MIPS ISA I-R6 (32/64-bit, LE/BE)\n");
}

/*
 * Halt the CPU
 */
void
machine_halt(void)
{
	printf("Halting MIPS CPU...\n");

	/* Disable interrupts */
	__asm__ __volatile__(
		"di\n\t"		/* Disable interrupts */
		"sync\n\t"
		::: "memory"
	);

	/* Spin forever */
	for (;;) {
		__asm__ __volatile__("wait");
	}
}

/*
 * Reboot the machine
 */
void
machine_reboot(int howto)
{
	printf("Rebooting MIPS system...\n");

	/* Disable interrupts */
	__asm__ __volatile__(
		"di\n\t"
		"sync\n\t"
		::: "memory"
	);

	/* Platform-specific reboot would go here */
	/* For now, just halt */
	machine_halt();
}

/*
 * Return boot arguments
 */
char *
machine_boot_args(void)
{
	return "";
}

/*
 * Read CP0 Status register
 */
unsigned int
mips_read_status(void)
{
	unsigned int status;

	__asm__ __volatile__(
		"mfc0 %0, $12\n\t"	/* Read CP0 Status */
		: "=r" (status)
	);

	return status;
}

/*
 * Write CP0 Status register
 */
void
mips_write_status(unsigned int status)
{
	__asm__ __volatile__(
		"mtc0 %0, $12\n\t"	/* Write CP0 Status */
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		:: "r" (status)
	);
}

/*
 * Read CP0 Cause register
 */
unsigned int
mips_read_cause(void)
{
	unsigned int cause;

	__asm__ __volatile__(
		"mfc0 %0, $13\n\t"	/* Read CP0 Cause */
		: "=r" (cause)
	);

	return cause;
}

/*
 * Enable interrupts
 */
void
mips_enable_interrupts(void)
{
	unsigned int status;

	status = mips_read_status();
	status |= 0x00000001;	/* Set IE bit */
	mips_write_status(status);
}

/*
 * Disable interrupts
 */
unsigned int
mips_disable_interrupts(void)
{
	unsigned int status, old_status;

	old_status = mips_read_status();
	status = old_status & ~0x00000001;	/* Clear IE bit */
	mips_write_status(status);

	return old_status;
}

/*
 * Restore interrupt state
 */
void
mips_restore_interrupts(unsigned int status)
{
	mips_write_status(status);
}

/*
 * Get CPU identification
 */
void
mips_get_cpuid(void)
{
	unsigned int prid;

	__asm__ __volatile__(
		"mfc0 %0, $15\n\t"	/* Read CP0 PRId */
		: "=r" (prid)
	);

	printf("MIPS CPU PRId: 0x%08x\n", prid);
	printf("  Company: 0x%02x\n", (prid >> 16) & 0xFF);
	printf("  Processor ID: 0x%02x\n", (prid >> 8) & 0xFF);
	printf("  Revision: 0x%02x\n", prid & 0xFF);
}

/*
 * Delay for specified microseconds
 */
void
delay(int usec)
{
	/* Simple busy-wait delay */
	/* TODO: Implement proper timing using CP0 Count register */
	volatile int i, j;

	for (i = 0; i < usec; i++) {
		for (j = 0; j < 100; j++) {
			__asm__ __volatile__("nop");
		}
	}
}
