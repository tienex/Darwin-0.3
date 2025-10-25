/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Machine Clock Support - i8254 PIT Driver
 *
 * The Intel 8254 Programmable Interval Timer provides the system clock
 * interrupt on AlphaStation systems via the ISA bus.
 */

#include <mach/mach_types.h>
#include <kern/clock.h>
#include <architecture/alpha/cpu.h>
#include "platform.h"
#include "isa.h"

/* i8254 PIT ports */
#define PIT_CHANNEL0    0x40    /* Channel 0 data port (system timer) */
#define PIT_CHANNEL1    0x41    /* Channel 1 data port (refresh - unused) */
#define PIT_CHANNEL2    0x42    /* Channel 2 data port (speaker) */
#define PIT_COMMAND     0x43    /* Mode/Command register */

/* PIT command register bits */
#define PIT_SEL0        0x00    /* Select channel 0 */
#define PIT_SEL1        0x40    /* Select channel 1 */
#define PIT_SEL2        0x80    /* Select channel 2 */
#define PIT_LATCH       0x00    /* Latch count command */
#define PIT_LSB         0x10    /* Read/write LSB only */
#define PIT_MSB         0x20    /* Read/write MSB only */
#define PIT_BOTH        0x30    /* Read/write LSB then MSB */

/* PIT operating modes */
#define PIT_MODE0       0x00    /* Interrupt on terminal count */
#define PIT_MODE1       0x02    /* Hardware retriggerable one-shot */
#define PIT_MODE2       0x04    /* Rate generator */
#define PIT_MODE3       0x06    /* Square wave generator */
#define PIT_MODE4       0x08    /* Software triggered strobe */
#define PIT_MODE5       0x0A    /* Hardware triggered strobe */

/* PIT constants */
#define PIT_FREQUENCY   1193182 /* PIT input frequency (Hz) */
#define HZ              100     /* Desired tick rate (100 Hz = 10ms) */
#define LATCH_COUNT     (PIT_FREQUENCY / HZ)

/* Clock state */
static unsigned long clock_ticks = 0;
static unsigned long clock_freq = HZ;

/*
 * Read current PIT counter value
 */
static unsigned short
pit_read_counter(void)
{
    unsigned char low, high;

    /* Latch counter 0 */
    outb(PIT_COMMAND, PIT_SEL0 | PIT_LATCH);

    /* Read LSB then MSB */
    low = inb(PIT_CHANNEL0);
    high = inb(PIT_CHANNEL0);

    return ((unsigned short)high << 8) | low;
}

/*
 * Set PIT divisor for channel 0
 */
static void
pit_set_divisor(unsigned short divisor)
{
    /* Set command: Channel 0, both bytes, mode 2 (rate generator) */
    outb(PIT_COMMAND, PIT_SEL0 | PIT_BOTH | PIT_MODE2);

    /* Write divisor: LSB then MSB */
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

/*
 * Initialize machine clock
 */
void
machine_clock_init(void)
{
    /* Disable interrupts during setup */
    unsigned long ipl = pal_unix_swpipl(ALPHA_IPL_HIGH);

    /* Set up PIT channel 0 for periodic interrupts */
    pit_set_divisor(LATCH_COUNT);

    /* Enable timer interrupt (IRQ 0) */
    alpha_isa_enable_irq(ISA_IRQ_TIMER);

    /* Restore interrupt level */
    pal_unix_swpipl(ipl);

    printf("Clock initialized: %d Hz (PIT divisor %d)\n", HZ, LATCH_COUNT);
}

/*
 * Clock interrupt handler
 *
 * Called from ISA interrupt handler when timer IRQ (IRQ 0) fires.
 */
void
rtclock_intr(void)
{
    /* Increment tick counter */
    clock_ticks++;

    /*
     * Call the generic clock interrupt handler
     * This updates system time and handles timer expirations
     */
    /* hertz_tick(USER_MODE(state), state->pc); */

    /*
     * On SMP systems, we might need to send IPIs to other CPUs
     * to update their local clocks
     */
#ifdef SMP
    /* smp_send_timer_broadcast(); */
#endif
}

/*
 * Get current tick count
 */
unsigned long
machine_clock_ticks(void)
{
    return clock_ticks;
}

/*
 * Get clock frequency
 */
unsigned long
machine_clock_frequency(void)
{
    return clock_freq;
}

/*
 * Delay for approximately the specified number of microseconds
 */
void
machine_delay_us(unsigned int microseconds)
{
    unsigned long start_ticks, end_ticks, elapsed;
    unsigned long required_ticks;

    /* Calculate required ticks */
    required_ticks = (microseconds * clock_freq) / 1000000;

    /* Use PIT for short delays */
    if (microseconds < 10000) {
        /* Use PIT counter for accurate short delays */
        unsigned short start, end, count;

        start = pit_read_counter();
        count = (unsigned short)((microseconds * (PIT_FREQUENCY / 1000000)) & 0xFFFF);

        do {
            end = pit_read_counter();
            if (end <= start)
                elapsed = start - end;
            else
                elapsed = (LATCH_COUNT - end) + start;
        } while (elapsed < count);
    } else {
        /* Use tick counter for longer delays */
        start_ticks = clock_ticks;
        end_ticks = start_ticks + required_ticks;

        while (clock_ticks < end_ticks)
            ;
    }
}

/*
 * Calibrate delay loop
 */
void
machine_clock_calibrate(void)
{
    /* Could implement more sophisticated calibration here */
    /* For now, we rely on PIT frequency being accurate */
}
