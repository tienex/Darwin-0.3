/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Machine-Dependent Functions
 */

#include <mach/mach_types.h>
#include <mach/machine.h>
#include <kern/cpu_number.h>
#include <kern/cpu_data.h>
#include <architecture/alpha/cpu.h>

/*
 * Machine-dependent startup code
 */
void
machine_startup(void)
{
	/*
	 * Perform any final machine-dependent initialization
	 */
}

/*
 * Halt all CPUs
 */
void
halt_all_cpus(boolean_t reboot)
{
	if (reboot) {
		/* Reboot the system */
		__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_halt));
	} else {
		/* Halt the system */
		__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_halt));
	}

	/* Should not return */
	for (;;) ;
}

/*
 * Halt a single CPU
 */
void
halt_cpu(void)
{
	__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_halt));
}

/*
 * Machine-dependent initialization for BSD
 */
void
machine_init(void)
{
	/*
	 * Initialize machine-dependent BSD subsystems
	 */
}

/*
 * Set up CPU information for Mach
 */
void
machine_info(void)
{
	machine_info_t *mi = &machine_info;

	mi->major_version = KERNEL_MAJOR_VERSION;
	mi->minor_version = KERNEL_MINOR_VERSION;
	mi->max_cpus = NCPUS;
	mi->avail_cpus = 1;  /* Updated as CPUs come online */
	mi->memory_size = 0;  /* Updated by memory sizing code */
}

/*
 * Slave CPU initialization
 */
void
slave_main(int cpu)
{
	/*
	 * Initialize this CPU
	 */

	/*
	 * Set up CPU data structure
	 */

	/*
	 * Enter the scheduler
	 */
	/* cpu_launch_first_thread(); */
}

/*
 * Processor-specific operations
 */

/*
 * Start a processor
 */
kern_return_t
cpu_start(int cpu)
{
	/*
	 * Send interrupt to wake up the specified CPU
	 */
	return KERN_SUCCESS;
}

/*
 * Stop a processor (not supported on Alpha)
 */
kern_return_t
cpu_stop(int cpu)
{
	return KERN_FAILURE;
}

/*
 * Get CPU info
 */
kern_return_t
cpu_info(processor_flavor_t flavor, int cpu, processor_info_t info,
	 unsigned int *count)
{
	switch (flavor) {
	case PROCESSOR_BASIC_INFO:
		/* Fill in basic processor info */
		return KERN_SUCCESS;

	default:
		return KERN_INVALID_ARGUMENT;
	}
}

/*
 * Context switching support
 */

/*
 * Switch to new thread
 */
void
machine_switch_context(thread_t old, thread_t new)
{
	/*
	 * Save old thread state
	 */

	/*
	 * Restore new thread state
	 */

	/*
	 * Switch address space if needed
	 */
}

/*
 * Load context for new thread
 */
void
machine_load_context(thread_t thread)
{
	/*
	 * Load complete context for thread
	 */
}

/*
 * Set up initial thread state
 */
void
machine_thread_set_state(thread_t thread, void *state)
{
	/*
	 * Set thread state from user-provided state
	 */
}

/*
 * Get thread state
 */
void
machine_thread_get_state(thread_t thread, void *state)
{
	/*
	 * Copy thread state to user buffer
	 */
}

/*
 * Idle CPU
 */
void
machine_idle(void)
{
	/*
	 * Put CPU in low-power state
	 * Alpha doesn't have a standard idle instruction,
	 * so we just enable interrupts and wait
	 */
	__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_swpipl), "r" (0));

	/* Wait for interrupt */
	for (;;) {
		/* Interrupts will wake us up */
	}
}
