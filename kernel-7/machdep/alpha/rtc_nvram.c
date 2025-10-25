/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * RTC (Real-Time Clock) and NVRAM Support
 *
 * AlphaStation 500/600 use a standard MC146818A-compatible RTC chip
 * accessed via ISA I/O ports 0x70 (address) and 0x71 (data).
 */

#include <mach/mach_types.h>
#include <sys/time.h>
#include "platform.h"

/*
 * RTC register definitions
 */
#define RTC_REG_SECONDS		0x00
#define RTC_REG_MINUTES		0x02
#define RTC_REG_HOURS		0x04
#define RTC_REG_DAY_OF_WEEK	0x06
#define RTC_REG_DAY_OF_MONTH	0x07
#define RTC_REG_MONTH		0x08
#define RTC_REG_YEAR		0x09
#define RTC_REG_CENTURY		0x32	/* Century (if supported) */

#define RTC_REG_STATUS_A	0x0A
#define RTC_REG_STATUS_B	0x0B
#define RTC_REG_STATUS_C	0x0C
#define RTC_REG_STATUS_D	0x0D

/* Status Register A */
#define RTC_SRA_UIP		0x80	/* Update in progress */

/* Status Register B */
#define RTC_SRB_24HR		0x02	/* 24-hour mode */
#define RTC_SRB_BINARY		0x04	/* Binary mode (vs BCD) */
#define RTC_SRB_UIE		0x10	/* Update-ended interrupt */
#define RTC_SRB_AIE		0x20	/* Alarm interrupt */
#define RTC_SRB_PIE		0x40	/* Periodic interrupt */
#define RTC_SRB_SET		0x80	/* Set mode (inhibit updates) */

/*
 * NVRAM area (bytes 14-127 in RTC chip)
 */
#define NVRAM_START		0x0E
#define NVRAM_SIZE		114

/*
 * Read RTC register
 */
static unsigned char
rtc_read(unsigned char reg)
{
	outb(reg, ISA_PORT_RTC_ADDR);
	return inb(ISA_PORT_RTC_DATA);
}

/*
 * Write RTC register
 */
static void
rtc_write(unsigned char reg, unsigned char val)
{
	outb(reg, ISA_PORT_RTC_ADDR);
	outb(val, ISA_PORT_RTC_DATA);
}

/*
 * Wait for RTC update to complete
 */
static void
rtc_wait_update(void)
{
	int timeout = 10000;

	while ((rtc_read(RTC_REG_STATUS_A) & RTC_SRA_UIP) && timeout-- > 0)
		/* Wait */ ;
}

/*
 * Convert BCD to binary
 */
static unsigned char
bcd_to_bin(unsigned char bcd)
{
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/*
 * Convert binary to BCD
 */
static unsigned char
bin_to_bcd(unsigned char bin)
{
	return ((bin / 10) << 4) | (bin % 10);
}

/*
 * Initialize RTC
 */
void
alpha_rtc_init(void)
{
	unsigned char status_b;

	printf("Initializing RTC...\n");

	/* Wait for any pending update */
	rtc_wait_update();

	/* Read status register B */
	status_b = rtc_read(RTC_REG_STATUS_B);

	/* Set 24-hour mode and binary mode */
	status_b |= RTC_SRB_24HR | RTC_SRB_BINARY;

	/* Disable interrupts for now */
	status_b &= ~(RTC_SRB_UIE | RTC_SRB_AIE | RTC_SRB_PIE);

	rtc_write(RTC_REG_STATUS_B, status_b);

	printf("  RTC configured (24-hour, binary mode)\n");
}

/*
 * Read current time from RTC
 */
void
alpha_rtc_gettime(struct tm *tm)
{
	unsigned char status_b;
	int is_binary;

	/* Wait for update to complete */
	rtc_wait_update();

	/* Check if RTC is in binary or BCD mode */
	status_b = rtc_read(RTC_REG_STATUS_B);
	is_binary = (status_b & RTC_SRB_BINARY) != 0;

	/* Read time */
	tm->tm_sec = rtc_read(RTC_REG_SECONDS);
	tm->tm_min = rtc_read(RTC_REG_MINUTES);
	tm->tm_hour = rtc_read(RTC_REG_HOURS);
	tm->tm_mday = rtc_read(RTC_REG_DAY_OF_MONTH);
	tm->tm_mon = rtc_read(RTC_REG_MONTH);
	tm->tm_year = rtc_read(RTC_REG_YEAR);

	/* Convert BCD to binary if needed */
	if (!is_binary) {
		tm->tm_sec = bcd_to_bin(tm->tm_sec);
		tm->tm_min = bcd_to_bin(tm->tm_min);
		tm->tm_hour = bcd_to_bin(tm->tm_hour);
		tm->tm_mday = bcd_to_bin(tm->tm_mday);
		tm->tm_mon = bcd_to_bin(tm->tm_mon);
		tm->tm_year = bcd_to_bin(tm->tm_year);
	}

	/* Adjust month (RTC uses 1-12, tm uses 0-11) */
	tm->tm_mon--;

	/* Adjust year (RTC stores 2-digit year) */
	if (tm->tm_year < 70)
		tm->tm_year += 100;  /* 2000+ */

	/* Calculate day of week (0=Sunday) */
	/* Simplified - would use proper algorithm in production */
	tm->tm_wday = 0;

	/* Day of year */
	tm->tm_yday = 0;  /* Would calculate properly */

	/* Daylight saving time */
	tm->tm_isdst = 0;
}

/*
 * Set RTC time
 */
void
alpha_rtc_settime(const struct tm *tm)
{
	unsigned char status_b;
	int year;

	/* Wait for update to complete */
	rtc_wait_update();

	/* Enter SET mode to inhibit updates */
	status_b = rtc_read(RTC_REG_STATUS_B);
	rtc_write(RTC_REG_STATUS_B, status_b | RTC_SRB_SET);

	/* Adjust year */
	year = tm->tm_year;
	if (year >= 100)
		year -= 100;  /* 2000+ -> 00+ */

	/* Write time (assuming binary mode) */
	rtc_write(RTC_REG_SECONDS, tm->tm_sec);
	rtc_write(RTC_REG_MINUTES, tm->tm_min);
	rtc_write(RTC_REG_HOURS, tm->tm_hour);
	rtc_write(RTC_REG_DAY_OF_MONTH, tm->tm_mday);
	rtc_write(RTC_REG_MONTH, tm->tm_mon + 1);  /* tm uses 0-11 */
	rtc_write(RTC_REG_YEAR, year);

	/* Exit SET mode */
	rtc_write(RTC_REG_STATUS_B, status_b);
}

/*
 * NVRAM operations
 */

/*
 * Read byte from NVRAM
 */
unsigned char
alpha_nvram_read(unsigned char offset)
{
	if (offset >= NVRAM_SIZE)
		return 0xFF;

	return rtc_read(NVRAM_START + offset);
}

/*
 * Write byte to NVRAM
 */
void
alpha_nvram_write(unsigned char offset, unsigned char value)
{
	if (offset >= NVRAM_SIZE)
		return;

	rtc_write(NVRAM_START + offset, value);
}

/*
 * Read block from NVRAM
 */
void
alpha_nvram_read_block(unsigned char offset, unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len && (offset + i) < NVRAM_SIZE; i++) {
		buf[i] = rtc_read(NVRAM_START + offset + i);
	}
}

/*
 * Write block to NVRAM
 */
void
alpha_nvram_write_block(unsigned char offset, const unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len && (offset + i) < NVRAM_SIZE; i++) {
		rtc_write(NVRAM_START + offset + i, buf[i]);
	}
}

/*
 * Calculate checksum of NVRAM
 */
unsigned char
alpha_nvram_checksum(void)
{
	unsigned char sum = 0;
	int i;

	for (i = 0; i < NVRAM_SIZE - 1; i++) {
		sum += rtc_read(NVRAM_START + i);
	}

	return sum;
}

/*
 * Verify NVRAM checksum
 */
int
alpha_nvram_verify(void)
{
	unsigned char calculated = alpha_nvram_checksum();
	unsigned char stored = rtc_read(NVRAM_START + NVRAM_SIZE - 1);

	return (calculated == stored);
}

/*
 * Update NVRAM checksum
 */
void
alpha_nvram_update_checksum(void)
{
	unsigned char sum = alpha_nvram_checksum();
	rtc_write(NVRAM_START + NVRAM_SIZE - 1, sum);
}

/*
 * Print current RTC time
 */
void
alpha_rtc_print(void)
{
	struct tm tm;
	const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
				"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	alpha_rtc_gettime(&tm);

	printf("\nCurrent Time: %s %s %2d %02d:%02d:%02d %d\n",
	       days[tm.tm_wday],
	       months[tm.tm_mon],
	       tm.tm_mday,
	       tm.tm_hour,
	       tm.tm_min,
	       tm.tm_sec,
	       1900 + tm.tm_year);
}

/* Stub structure if not defined */
#ifndef _SYS_TIME_H_
struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};
#endif
