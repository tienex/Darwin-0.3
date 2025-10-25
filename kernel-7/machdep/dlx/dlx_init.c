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
 * DLX Initialization Code
 */

#include <mach/mach_types.h>
#include <kern/cpu_number.h>
#include <machdep/dlx/pmap.h>

/*
 * Storage for kernel pmap
 */
struct pmap kernel_pmap_store;

/*
 * Machine-dependent startup code
 */
void
machine_startup(void)
{
	/* Initialize exception handlers */
	extern void dlx_exception_init(void);
	dlx_exception_init();

	/* Print startup message */
	printf("DLX Architecture Support initialized\n");
	printf("MMU Mode: ");
	switch (dlx_mmu_mode) {
	case DLX_MMU_MODE_PAGE_TABLES_ONLY:
		printf("Page Tables Only\n");
		break;
	case DLX_MMU_MODE_TLB_ONLY:
		printf("TLB Only\n");
		break;
	case DLX_MMU_MODE_BOTH:
		printf("Page Tables + TLB (hybrid)\n");
		break;
	default:
		printf("Unknown\n");
	}
	printf("TLB Entries: %d\n", DLX_TLB_ENTRIES);
	printf("Page Size: %d bytes\n", PAGE_SIZE);
}

/*
 * Machine-dependent initialization
 */
void
machine_init(void)
{
	/* Nothing additional needed */
}

/*
 * Set up CPU
 */
void
cpu_bootstrap(void)
{
	/* Initialize this CPU */
}

/*
 * Get CPU information
 */
void
cpu_machine_init(void)
{
	/* Set up CPU-specific features */
}
