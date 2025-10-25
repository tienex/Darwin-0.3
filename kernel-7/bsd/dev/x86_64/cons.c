/*
 * x86-64 console driver
 *
 * This file provides basic console I/O support for x86-64.
 *
 * STUB IMPLEMENTATION
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/tty.h>

/*
 * Console getc - get character from console
 */
int
cngetc(void)
{
	/* Stub implementation */
	return 0;
}

/*
 * Console putc - put character to console
 */
void
cnputc(int c)
{
	/* Stub implementation - would write to VGA or serial console */
}

/*
 * Console initialization
 */
void
cninit(void)
{
	/* Stub implementation */
}

/*
 * Console probe
 */
void
cnprobe(void)
{
	/* Stub implementation */
}
