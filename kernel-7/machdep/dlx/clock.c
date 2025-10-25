/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX Clock/Timer Support
 *
 * Provides timer/clock management for DLX architecture using
 * memory-mapped timer device at 0xfff00010.
 */

#include <mach/mach_types.h>
#include <kern/clock.h>

#include <machdep/dlx/trap.h>

#define DLX_TIMER_ADDRESS	0xfff00010
#define HZ			100		/* 100 Hz timer (10ms ticks) */
#define TIMER_INTERVAL_US	(1000000 / HZ)	/* Microseconds per tick */

/* Time tracking */
static unsigned long ticks = 0;		/* Total ticks since boot */
static unsigned long seconds = 0;	/* Seconds since boot */
static int ticks_per_second = HZ;

/* Timer state */
static int timer_initialized = 0;

/*
 * Initialize clock/timer hardware
 */
void
clock_init(void)
{
	volatile unsigned int *timer_reg;

	if (timer_initialized)
		return;

	timer_reg = (volatile unsigned int *)DLX_TIMER_ADDRESS;

	/* Set timer interval (in microseconds) */
	*timer_reg = TIMER_INTERVAL_US;

	ticks = 0;
	seconds = 0;
	timer_initialized = 1;

	printf("Clock initialized: %d Hz (%d us per tick)\n", HZ, TIMER_INTERVAL_US);
}

/*
 * Clock interrupt handler
 * Called on every timer interrupt (TRAP_TIMER)
 */
void
hardclock(void *pc)
{
	volatile unsigned int *timer_reg;

	if (!timer_initialized)
		return;

	/* Increment tick counter */
	ticks++;

	/* Track seconds */
	if (ticks % ticks_per_second == 0) {
		seconds++;
	}

	/* Reprogram timer for next interrupt */
	timer_reg = (volatile unsigned int *)DLX_TIMER_ADDRESS;
	*timer_reg = TIMER_INTERVAL_US;

	/* Call scheduler clock routine */
	/* In full Darwin implementation:
	 * hertz_tick(pc);
	 * thread_quantum_update();
	 * etc.
	 */
}

/*
 * Get current time
 */
void
microtime(struct timeval *tvp)
{
	if (!tvp)
		return;

	tvp->tv_sec = seconds;
	tvp->tv_usec = (ticks % ticks_per_second) * TIMER_INTERVAL_US;
}

/*
 * Get uptime in ticks
 */
unsigned long
get_ticks(void)
{
	return ticks;
}

/*
 * Get uptime in seconds
 */
unsigned long
get_uptime_seconds(void)
{
	return seconds;
}

/*
 * Delay for specified number of microseconds
 * Simple busy-wait implementation
 */
void
microdelay(unsigned int usec)
{
	unsigned long start_ticks = ticks;
	unsigned long delay_ticks = (usec * HZ) / 1000000;

	if (delay_ticks == 0)
		delay_ticks = 1;

	/* Busy wait */
	while ((ticks - start_ticks) < delay_ticks)
		;
}

/*
 * Delay for specified number of milliseconds
 */
void
delay(unsigned int msec)
{
	microdelay(msec * 1000);
}

/*
 * Get timer frequency
 */
int
get_timer_hz(void)
{
	return HZ;
}
