/*
 * x86-64 Machine Clock Support
 *
 * This file handles timer/clock management for x86-64,
 * including the PIT, TSC, and APIC timer.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <mach/machine.h>

/* PIT (Programmable Interval Timer) ports */
#define PIT_CH0		0x40	/* Channel 0 data port */
#define PIT_CH1		0x41	/* Channel 1 data port */
#define PIT_CH2		0x42	/* Channel 2 data port */
#define PIT_MODE	0x43	/* Mode/Command register */

/* PIT frequency */
#define PIT_FREQ	1193182	/* Hz */
#define HZ		100	/* Ticks per second */

/* RTC (Real-Time Clock) ports */
#define RTC_INDEX	0x70
#define RTC_DATA	0x71

/* External I/O functions */
extern void outb(unsigned short port, unsigned char val);
extern unsigned char inb(unsigned short port);
extern unsigned long rdtsc(void);

/* Clock state */
static unsigned long ticks = 0;
static unsigned long tsc_freq = 0;

/*
 * Initialize the machine clock
 */
void
machine_clock_init(void)
{
	unsigned short divisor;

	printf("Initializing machine clock...\n");

	/* Set up PIT for periodic interrupts at HZ */
	divisor = PIT_FREQ / HZ;

	/* Channel 0, Mode 2 (rate generator), binary mode */
	outb(PIT_MODE, 0x34);

	/* Set divisor (LSB then MSB) */
	outb(PIT_CH0, divisor & 0xFF);
	outb(PIT_CH0, divisor >> 8);

	/* Calibrate TSC */
	tsc_calibrate();

	printf("Clock initialized: %d Hz, TSC freq: %lu MHz\n",
	       HZ, tsc_freq / 1000000);
}

/*
 * Calibrate the TSC (Time Stamp Counter)
 */
void
tsc_calibrate(void)
{
	unsigned long start_tsc, end_tsc;
	int i;

	/* Simple calibration using PIT */
	/* Wait for PIT to tick a few times and measure TSC */

	start_tsc = rdtsc();

	/* Busy wait for a short period */
	for (i = 0; i < 1000000; i++)
		__asm__ volatile("pause");

	end_tsc = rdtsc();

	/* Rough estimate */
	tsc_freq = (end_tsc - start_tsc) * 100;

	/* In a real implementation, would use more accurate calibration */
}

/*
 * Get TSC frequency in Hz
 */
unsigned long
tsc_get_frequency(void)
{
	return tsc_freq;
}

/*
 * Clock interrupt handler
 * Called HZ times per second
 */
void
clock_interrupt(void)
{
	ticks++;

	/* Update system time */
	/* Would call hardclock() here */

	/* Send EOI to interrupt controller */
	/* intr_eoi(IRQ_TIMER); */
}

/*
 * Get number of clock ticks since boot
 */
unsigned long
get_ticks(void)
{
	return ticks;
}

/*
 * Microsecond delay using TSC
 */
void
microdelay(unsigned int usecs)
{
	unsigned long start, end, cycles;

	if (tsc_freq == 0) {
		/* Fallback to simple loop if TSC not calibrated */
		volatile int i;
		for (i = 0; i < usecs * 1000; i++)
			;
		return;
	}

	cycles = (tsc_freq / 1000000) * usecs;
	start = rdtsc();
	end = start + cycles;

	while (rdtsc() < end)
		__asm__ volatile("pause");
}

/*
 * Millisecond delay
 */
void
mdelay(unsigned int msecs)
{
	microdelay(msecs * 1000);
}

/*
 * Read the RTC (Real-Time Clock)
 */
unsigned char
rtc_read(unsigned char reg)
{
	outb(RTC_INDEX, reg);
	return inb(RTC_DATA);
}

/*
 * Write to the RTC
 */
void
rtc_write(unsigned char reg, unsigned char val)
{
	outb(RTC_INDEX, reg);
	outb(RTC_DATA, val);
}

/*
 * Get current time from RTC
 */
void
rtc_get_time(struct tm *tm)
{
	/* BCD to binary conversion */
	#define BCD2BIN(val) (((val) & 0x0F) + ((val) >> 4) * 10)

	/* Read RTC registers */
	tm->tm_sec = BCD2BIN(rtc_read(0x00));
	tm->tm_min = BCD2BIN(rtc_read(0x02));
	tm->tm_hour = BCD2BIN(rtc_read(0x04));
	tm->tm_mday = BCD2BIN(rtc_read(0x07));
	tm->tm_mon = BCD2BIN(rtc_read(0x08)) - 1;
	tm->tm_year = BCD2BIN(rtc_read(0x09)) + 100;  /* Years since 1900 */

	#undef BCD2BIN
}

/*
 * Initialize APIC timer (for SMP systems)
 */
void
apic_timer_init(void)
{
	/* Stub implementation */
	/* Would program Local APIC timer */
}

/*
 * Read current time in microseconds
 */
unsigned long
microtime(void)
{
	unsigned long usecs;

	if (tsc_freq > 0) {
		usecs = (rdtsc() * 1000000) / tsc_freq;
	} else {
		usecs = ticks * (1000000 / HZ);
	}

	return usecs;
}

/*
 * Get monotonic time in nanoseconds
 */
unsigned long long
nanotime(void)
{
	unsigned long long nsecs;

	if (tsc_freq > 0) {
		nsecs = (rdtsc() * 1000000000ULL) / tsc_freq;
	} else {
		nsecs = ticks * (1000000000ULL / HZ);
	}

	return nsecs;
}

/*
 * Set alarm for profiling/statistics
 */
void
set_alarm(unsigned int msecs)
{
	/* Would program timer for one-shot interrupt */
}

/*
 * Cancel alarm
 */
void
cancel_alarm(void)
{
	/* Would cancel one-shot timer */
}
