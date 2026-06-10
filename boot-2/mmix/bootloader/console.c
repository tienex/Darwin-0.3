/*
 * MMIX Bootloader Console Driver
 * Copyright (c) 1999-2025 Apple Computer, Inc.
 *
 * Simple console output via emulator memory-mapped device
 */

#include "boot.h"

/* Console device registers */
static volatile unsigned long long *console_out =
	(volatile unsigned long long *)MMIX_CONSOLE_OUT;
static volatile unsigned long long *console_in =
	(volatile unsigned long long *)MMIX_CONSOLE_IN;

/*
 * Initialize console
 */
void boot_console_init(void)
{
	/* Clear any pending data */
	*console_out = 0;
}

/*
 * Output a single character
 */
void boot_console_putc(char c)
{
	/* Convert \n to \r\n for proper console output */
	if (c == '\n') {
		*console_out = '\r';
	}

	*console_out = (unsigned long long)c;
}

/*
 * Output a string
 */
void boot_console_puts(const char *s)
{
	while (*s) {
		boot_console_putc(*s++);
	}
}

/*
 * Read a character (blocking)
 */
int boot_console_getc(void)
{
	unsigned long long c;

	/* Wait for character */
	do {
		c = *console_in;
	} while (c == 0);

	return (int)(c & 0xFF);
}
