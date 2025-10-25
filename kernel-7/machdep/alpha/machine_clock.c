/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Machine Clock Support
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>

/*
 * Initialize machine clock
 */
void
machine_clock_init(void)
{
	/* Initialize clock hardware */
}

/*
 * Clock interrupt handler
 */
void
rtclock_intr(void)
{
	/* Handle clock interrupt */
}
