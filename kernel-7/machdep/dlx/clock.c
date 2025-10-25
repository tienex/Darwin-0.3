/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX Clock/Timer Support
 */

#include <mach/mach_types.h>
#include <kern/clock.h>

#include <machdep/dlx/trap.h>

#define DLX_TIMER_ADDRESS	0xfff00010
#define HZ			100		/* 100 Hz timer */

static unsigned long ticks = 0;

/*
 * Initialize clock
 */
void
clock_init(void)
{
	unsigned int timer_val = 1000000 / HZ;  /* Microseconds */

	/* Set timer */
	*(volatile unsigned int *)DLX_TIMER_ADDRESS = timer_val;

	printf("Clock initialized: %d Hz\n", HZ);
}

/*
 * Clock interrupt handler
 */
void
hardclock(void *pc)
{
	ticks++;

	/* Call scheduler clock routine */
	/* hertz_tick(pc); */
}

/*
 * Get time
 */
void
microtime(struct timeval *tvp)
{
	tvp->tv_sec = ticks / HZ;
	tvp->tv_usec = (ticks % HZ) * (1000000 / HZ);
}
