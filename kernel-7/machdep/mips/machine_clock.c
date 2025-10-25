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
 * MIPS:	Machine clock and timer support
 *
 * MIPS CPUs have built-in CP0 Count/Compare registers:
 *  - Count increments at constant rate (typically half CPU frequency)
 *  - Compare register triggers interrupt when Count == Compare
 *  - Used for periodic scheduler tick and timing
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <kern/clock.h>
#include <kern/cpu_number.h>
#include <kern/spl.h>
#include <machdep/mips/trap.h>

/*
 * Clock parameters
 */
#define MIPS_HZ		100		/* 100 ticks per second = 10ms */

/*
 * CPU frequency and Count register increment rate
 * These should be detected at boot time
 * For now, assume 200MHz CPU with Count at CPU_freq/2
 */
static unsigned long	cpu_frequency = 200000000;	/* 200 MHz */
static unsigned long	count_frequency = 100000000;	/* 100 MHz (CPU/2) */
static unsigned long	tick_increment;			/* Count ticks per HZ */

/*
 * Time tracking
 */
static unsigned long	ticks = 0;		/* Total ticks since boot */

/*
 * CP0 Count/Compare register access
 */

static inline unsigned long
mips_read_count(void)
{
	unsigned long count;
	__asm__ volatile("mfc0 %0, $9" : "=r" (count));
	return count;
}

static inline void
mips_write_compare(unsigned long val)
{
	__asm__ volatile("mtc0 %0, $11" : : "r" (val));
}

static inline unsigned long
mips_read_compare(void)
{
	unsigned long compare;
	__asm__ volatile("mfc0 %0, $11" : "=r" (compare));
	return compare;
}

/*
 * Initialize clock hardware
 */
void
mips_clock_init(void)
{
	unsigned long count;

	/* Calculate tick increment */
	tick_increment = count_frequency / MIPS_HZ;

	/* Read current Count value */
	count = mips_read_count();

	/* Set Compare for first tick */
	mips_write_compare(count + tick_increment);

	/* Enable timer interrupt in SR (bit 15 = IM7) */
	unsigned int sr = mips_get_sr();
	sr |= 0x8000;		/* Enable timer interrupt */
	mips_set_sr(sr);

	printf("MIPS clock initialized: CPU %d MHz, Count %d MHz, tick %d us\n",
	       cpu_frequency / 1000000,
	       count_frequency / 1000000,
	       tick_increment * 1000000 / count_frequency);
}

/*
 * Clock interrupt handler
 * Called from trap.c when timer interrupt occurs
 */
void
mips_clock_interrupt(void)
{
	unsigned long count, compare;

	/* Read current Count and Compare */
	count = mips_read_count();
	compare = mips_read_compare();

	/* Set next Compare value */
	/* Add tick_increment to current compare to maintain accuracy */
	mips_write_compare(compare + tick_increment);

	/* Increment tick counter */
	ticks++;

	/* Call generic clock handler */
	hardclock_tick();
}

/*
 * Return current time in microseconds since boot
 */
unsigned long
mips_get_timebase(void)
{
	unsigned long count = mips_read_count();

	/* Convert Count to microseconds */
	return (count * 1000000UL) / count_frequency;
}

/*
 * Delay for specified number of microseconds
 */
void
delay(unsigned int usec)
{
	unsigned long start, delta, count;

	/* Calculate Count ticks needed */
	delta = ((unsigned long)usec * count_frequency) / 1000000;

	start = mips_read_count();
	do {
		count = mips_read_count();
	} while ((count - start) < delta);
}

/*
 * Microsecond delay (alias for delay)
 */
void
microtime_delay(unsigned int usec)
{
	delay(usec);
}

/*
 * Get current tick count
 */
unsigned long
get_ticks(void)
{
	return ticks;
}

/*
 * Calibrate CPU frequency
 * Should be called early in boot with a known time source
 */
void
mips_calibrate_cpu_frequency(void)
{
	/* This would measure CPU frequency using an external timer */
	/* For now, we use the default values set above */
	/* Real implementation would:
	 * 1. Read Count at time T0
	 * 2. Wait known interval (e.g., 1ms using RTC)
	 * 3. Read Count at time T1
	 * 4. Calculate: count_freq = (T1 - T0) / interval
	 */
}

/*
 * Set CPU frequency (for boot-time configuration)
 */
void
mips_set_cpu_frequency(unsigned long freq)
{
	cpu_frequency = freq;
	count_frequency = freq / 2;	/* Typically Count runs at CPU/2 */

	/* Recalculate tick increment */
	if (tick_increment != 0)
		tick_increment = count_frequency / MIPS_HZ;
}

/*
 * Get CPU frequency
 */
unsigned long
mips_get_cpu_frequency(void)
{
	return cpu_frequency;
}

/*
 * Real-time clock support (if hardware RTC present)
 */

/*
 * Read hardware real-time clock
 * Returns time in seconds since epoch
 */
time_t
read_rtc(void)
{
	/* This would read from hardware RTC chip */
	/* For now, return 0 (epoch) */
	return 0;
}

/*
 * Write hardware real-time clock
 */
void
write_rtc(time_t time)
{
	/* This would write to hardware RTC chip */
}

/*
 * Initialize real-time clock
 */
void
rtc_init(void)
{
	/* Initialize hardware RTC if present */
	/* Set time zone, leap year, etc. */
}

/*
 * High-resolution timer support
 */

/*
 * Read high-resolution timestamp
 * Returns Count register value
 */
uint64_t
mips_read_timebase_64(void)
{
#if defined(_MIPS64) || defined(__mips64)
	uint64_t count;
	__asm__ volatile("dmfc0 %0, $9" : "=r" (count));
	return count;
#else
	return (uint64_t)mips_read_count();
#endif
}

/*
 * Convert timebase to nanoseconds
 */
uint64_t
mips_timebase_to_ns(uint64_t timebase)
{
	return (timebase * 1000000000ULL) / count_frequency;
}

/*
 * Convert nanoseconds to timebase
 */
uint64_t
mips_ns_to_timebase(uint64_t ns)
{
	return (ns * count_frequency) / 1000000000ULL;
}

/*
 * Scheduler quantum support
 */

/*
 * Set quantum for thread scheduling
 * Called by scheduler to set thread time slice
 */
void
mips_set_quantum(unsigned int ms)
{
	/* Convert milliseconds to Count ticks */
	unsigned long quantum_ticks = (ms * count_frequency) / 1000;

	/* Store for scheduler use */
	/* In real implementation, would set per-CPU quantum */
}

/*
 * Performance monitoring counter support
 * MIPS has optional performance counters in CP0
 */

/*
 * Read performance counter
 */
unsigned long
mips_read_perfcnt(int counter)
{
	unsigned long count;

	switch (counter) {
	case 0:
		__asm__ volatile("mfc0 %0, $25, 0" : "=r" (count));
		break;
	case 1:
		__asm__ volatile("mfc0 %0, $25, 1" : "=r" (count));
		break;
	default:
		count = 0;
	}

	return count;
}

/*
 * Configure performance counter
 */
void
mips_config_perfcnt(int counter, unsigned int event)
{
	/* Configure performance counter to count specific event */
	/* Events: cycles, instructions, cache misses, etc. */
	/* Implementation depends on specific MIPS CPU */
}

/*
 * External declarations for functions used by this module
 */
extern unsigned int mips_get_sr(void);
extern void mips_set_sr(unsigned int);
extern void hardclock_tick(void);	/* Generic clock handler */
