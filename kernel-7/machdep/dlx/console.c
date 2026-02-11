/*
 * DLX Console Driver
 *
 * Memory-mapped console device for DLX architecture.
 * Provides character I/O through DLXSIM's keyboard device.
 */

#include <mach/dlx/vm_types.h>
#include <mach/dlx/boolean.h>
#include "trap.h"

/*
 * DLX console/keyboard I/O addresses (memory-mapped)
 */
#define DLX_KBD_PUTCHAR		0xfff00100	/* Write char to output */
#define DLX_KBD_NCHARSOUT	0xfff00120	/* Number of chars in output buffer */
#define DLX_KBD_GETCHAR		0xfff00180	/* Read char from input */
#define DLX_KBD_NCHARSIN	0xfff001a0	/* Number of chars in input buffer */
#define DLX_KBD_INTR		0xfff001c0	/* Interrupt control */

/* Console state */
static int console_initialized = 0;

/*
 * Initialize console device
 */
void
console_init(void)
{
	volatile unsigned int *kbd_intr;

	if (console_initialized)
		return;

	/* Enable keyboard interrupts */
	kbd_intr = (volatile unsigned int *)DLX_KBD_INTR;
	*kbd_intr = 1;	/* Enable interrupts */

	console_initialized = 1;
}

/*
 * Write a character to console
 * Returns 0 on success, -1 on error
 */
int
console_putchar(int c)
{
	volatile unsigned int *putchar_reg;
	volatile unsigned int *ncharsout_reg;
	int timeout = 1000000;

	if (!console_initialized)
		console_init();

	putchar_reg = (volatile unsigned int *)DLX_KBD_PUTCHAR;
	ncharsout_reg = (volatile unsigned int *)DLX_KBD_NCHARSOUT;

	/* Wait for output buffer to have space (with timeout) */
	while (*ncharsout_reg >= 128 && timeout-- > 0)
		;

	if (timeout <= 0)
		return -1;	/* Timeout */

	/* Write character */
	*putchar_reg = (unsigned int)c;

	return 0;
}

/*
 * Read a character from console (non-blocking)
 * Returns character on success, -1 if no char available
 */
int
console_getchar(void)
{
	volatile unsigned int *getchar_reg;
	volatile unsigned int *ncharsin_reg;

	if (!console_initialized)
		console_init();

	getchar_reg = (volatile unsigned int *)DLX_KBD_GETCHAR;
	ncharsin_reg = (volatile unsigned int *)DLX_KBD_NCHARSIN;

	/* Check if any characters available */
	if (*ncharsin_reg == 0)
		return -1;	/* No char available */

	/* Read and return character */
	return (int)(*getchar_reg & 0xff);
}

/*
 * Check if input characters are available
 * Returns number of characters in input buffer
 */
int
console_chars_avail(void)
{
	volatile unsigned int *ncharsin_reg;

	if (!console_initialized)
		return 0;

	ncharsin_reg = (volatile unsigned int *)DLX_KBD_NCHARSIN;
	return (int)*ncharsin_reg;
}

/*
 * Write a string to console
 */
void
console_puts(const char *s)
{
	if (!s)
		return;

	while (*s) {
		if (*s == '\n')
			console_putchar('\r');	/* CR before LF */
		console_putchar(*s++);
	}
}

/*
 * Console interrupt handler
 * Called from trap.c when keyboard interrupt occurs
 */
void
console_interrupt(void)
{
	int c;

	/* Read all available characters */
	while ((c = console_getchar()) != -1) {
		/* For now, just echo the character */
		/* In a full implementation, this would buffer input
		 * for use by read() system calls */
		console_putchar(c);
	}
}

/*
 * Simple printf implementation for kernel debugging
 * Supports: %d, %x, %s, %c, %%
 */
static void
print_number(unsigned int num, int base, int is_signed)
{
	char buf[32];
	int i = 0;
	int is_negative = 0;
	const char *digits = "0123456789abcdef";

	if (is_signed && (int)num < 0) {
		is_negative = 1;
		num = -(int)num;
	}

	if (num == 0) {
		console_putchar('0');
		return;
	}

	while (num > 0) {
		buf[i++] = digits[num % base];
		num /= base;
	}

	if (is_negative)
		console_putchar('-');

	while (i > 0)
		console_putchar(buf[--i]);
}

void
printf(const char *fmt, ...)
{
	const char *p;
	unsigned int *args;
	int arg_index = 0;

	if (!console_initialized)
		console_init();

	if (!fmt)
		return;

	/* Simple varargs handling - args are on stack after fmt */
	args = ((unsigned int *)&fmt) + 1;

	for (p = fmt; *p; p++) {
		if (*p != '%') {
			if (*p == '\n')
				console_putchar('\r');
			console_putchar(*p);
			continue;
		}

		p++;  /* Skip '%' */
		switch (*p) {
		case 'd':	/* Decimal */
			print_number(args[arg_index++], 10, 1);
			break;
		case 'u':	/* Unsigned decimal */
			print_number(args[arg_index++], 10, 0);
			break;
		case 'x':	/* Hexadecimal */
		case 'X':
			console_puts("0x");
			print_number(args[arg_index++], 16, 0);
			break;
		case 'p':	/* Pointer */
			console_puts("0x");
			print_number(args[arg_index++], 16, 0);
			break;
		case 's':	/* String */
			console_puts((const char *)args[arg_index++]);
			break;
		case 'c':	/* Character */
			console_putchar((int)args[arg_index++]);
			break;
		case '%':	/* Literal % */
			console_putchar('%');
			break;
		default:
			console_putchar('%');
			console_putchar(*p);
			break;
		}
	}
}
