/*
 * MMIX clock/timer interrupt handling
 */

#include <kern/clock.h>
#include <mach/machine.h>

void
mmix_timer_interrupt(void)
{
	/* Handle timer interrupt from rI register */
	/* Reload rI with new interval */
	/* Update system time */
	/* Call generic clock interrupt handler */
}

void
machine_clock_init(void)
{
	/* Initialize MMIX interval timer (rI register) */
	/* Set initial interval value */
	/* Enable timer interrupts in rK */
}
