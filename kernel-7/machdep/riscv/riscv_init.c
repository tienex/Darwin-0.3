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
 * RISC-V kernel initialization
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <mach/mach_types.h>
#include <vm/vm_kern.h>

extern void pmap_bootstrap(vm_offset_t load_start);
extern void machine_startup(void);

/*
 * Machine-specific initialization
 * Called from start.s
 */
void
_riscv_init(void)
{
	/* Initialize console for early debugging */
	printf("RISC-V Darwin kernel initializing...\n");

	/* Bootstrap virtual memory */
	pmap_bootstrap(0x80000000);

	/* Continue to machine-independent startup */
	machine_startup();

	/* Should never return */
	panic("_riscv_init: machine_startup returned");
}

/*
 * Machine-specific startup
 */
void
machine_startup(void)
{
	/* Set up machine info */
	machine_info.major_version = 1;
	machine_info.minor_version = 0;
	machine_info.max_cpus = 1;
	machine_info.avail_cpus = 1;
	machine_info.memory_size = 0;  /* To be filled in */

	printf("RISC-V kernel ready\n");
}

/*
 * Halt the processor
 */
void
halt_cpu(void)
{
	printf("CPU halted\n");
	for (;;) {
		__asm__ volatile("wfi");
	}
}

/*
 * Clear BSS section
 */
void
riscv_clear_bss(void)
{
	extern char _bss_start[], _bss_end[];
	char *p;

	for (p = _bss_start; p < _bss_end; p++)
		*p = 0;
}
