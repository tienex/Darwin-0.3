/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Real-Time Clock Driver
 */

#include <sys/param.h>
#include <sys/time.h>

/*
 * Initialize real-time clock
 */
void
rtc_init(void)
{
	/* Initialize RTC hardware */
}

/*
 * Read current time from RTC
 */
time_t
rtc_gettime(void)
{
	/* Read time from RTC */
	return 0;
}

/*
 * Set RTC time
 */
void
rtc_settime(time_t t)
{
	/* Set RTC time */
}
