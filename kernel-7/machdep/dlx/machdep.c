/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX Machine-Dependent Code
 */

#include <mach/mach_types.h>
#include <mach/machine.h>
#include <kern/cpu_data.h>

#include <machdep/dlx/pmap.h>

/*
 * Machine type
 */
cpu_type_t	cpu_type = CPU_TYPE_DLX;
cpu_subtype_t	cpu_subtype = CPU_SUBTYPE_DLX_ALL;

/*
 * Machine initialization
 */
void
machine_init(void)
{
	printf("DLX machine initialization\n");
}

void
machine_startup(void)
{
	printf("DLX Architecture Support initialized\n");
	printf("Page Size: %d bytes\n", PAGE_SIZE);
	printf("TLB Entries: %d\n", DLX_TLB_ENTRIES);
	printf("MMU Mode: ");

	if (dlx_status_register & DLX_STATUS_PAGE_TABLE)
		printf("Page Tables ");
	if (dlx_status_register & DLX_STATUS_TLB)
		printf("+ TLB");
	printf("\n");
}

void
slave_machine_init(void)
{
	/* For SMP support */
}

void
cpu_machine_init(void)
{
	/* CPU-specific initialization */
}

void
machine_conf(void)
{
	/* Machine configuration */
}

/*
 * Halt CPU
 */
void
halt_cpu(void)
{
	while (1) {
		/* Execute halt instruction */
		asm volatile("trap #0x300");  /* EXIT trap */
	}
}

void
halt_all_cpus(boolean_t reboot)
{
	printf("Halting system...\n");
	halt_cpu();
}
