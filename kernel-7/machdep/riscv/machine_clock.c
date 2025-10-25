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
 * RISC-V machine clock support
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>

/*
 * Number of microseconds per tick
 */
#define TICK	(1000000 / hz)

/*
 * Initialize the machine clock
 */
void
machine_clock_init(void)
{
	/* Initialize timer interrupt */
	/* Set up timer to fire at hz rate */
}

/*
 * Start the clock
 */
void
startrtclock(void)
{
	/* Start the real-time clock */
}

/*
 * Delay for a number of microseconds
 */
void
delay(int usec)
{
	/* Busy-wait delay using cycle counter */
	/* RISC-V can use rdtime instruction */
	volatile int i;
	for (i = 0; i < usec * 10; i++)
		;
}

/*
 * Microseconds to clock ticks
 */
int
microsec_to_ticks(int microsec)
{
	return (microsec / TICK);
}
