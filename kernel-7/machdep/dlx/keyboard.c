/*
 * DLX Keyboard Driver
 *
 * Provides buffered keyboard input with interrupt support.
 * Integrates with console.c for raw I/O operations.
 */

#include <mach/dlx/vm_types.h>
#include <mach/dlx/boolean.h>
#include "trap.h"

#define KBD_BUFFER_SIZE		256	/* Input buffer size */

/* Keyboard input buffer (ring buffer) */
static unsigned char kbd_buffer[KBD_BUFFER_SIZE];
static int kbd_head = 0;		/* Write position */
static int kbd_tail = 0;		/* Read position */
static int kbd_count = 0;		/* Number of chars in buffer */
static int kbd_initialized = 0;

/* Forward declarations */
extern int console_getchar(void);
extern void console_init(void);

/*
 * Initialize keyboard driver
 */
void
keyboard_init(void)
{
	if (kbd_initialized)
		return;

	/* Initialize console (includes keyboard hardware) */
	console_init();

	/* Initialize buffer */
	kbd_head = 0;
	kbd_tail = 0;
	kbd_count = 0;

	kbd_initialized = 1;
}

/*
 * Add character to keyboard buffer
 * Returns 0 on success, -1 if buffer full
 */
static int
keyboard_buffer_put(unsigned char c)
{
	if (kbd_count >= KBD_BUFFER_SIZE)
		return -1;	/* Buffer full */

	kbd_buffer[kbd_head] = c;
	kbd_head = (kbd_head + 1) % KBD_BUFFER_SIZE;
	kbd_count++;

	return 0;
}

/*
 * Get character from keyboard buffer
 * Returns character on success, -1 if buffer empty
 */
static int
keyboard_buffer_get(void)
{
	unsigned char c;

	if (kbd_count == 0)
		return -1;	/* Buffer empty */

	c = kbd_buffer[kbd_tail];
	kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
	kbd_count--;

	return (int)c;
}

/*
 * Check if keyboard buffer has data
 */
int
keyboard_has_data(void)
{
	return kbd_count > 0;
}

/*
 * Read character from keyboard (blocking)
 * Waits until a character is available
 */
int
keyboard_read(void)
{
	int c;

	if (!kbd_initialized)
		keyboard_init();

	/* Wait for character */
	while ((c = keyboard_buffer_get()) == -1) {
		/* In a real implementation, this would sleep
		 * and be woken by interrupt handler */
	}

	return c;
}

/*
 * Read character from keyboard (non-blocking)
 * Returns -1 if no character available
 */
int
keyboard_read_nonblock(void)
{
	if (!kbd_initialized)
		keyboard_init();

	return keyboard_buffer_get();
}

/*
 * Read line from keyboard (blocking)
 * Reads until newline or max_len reached
 * Returns number of characters read
 */
int
keyboard_readline(char *buf, int max_len)
{
	int i = 0;
	int c;

	if (!kbd_initialized)
		keyboard_init();

	if (!buf || max_len <= 0)
		return 0;

	while (i < max_len - 1) {
		c = keyboard_read();

		/* Handle special characters */
		if (c == '\n' || c == '\r') {
			buf[i] = '\0';
			return i;
		} else if (c == '\b' || c == 127) {  /* Backspace or DEL */
			if (i > 0) {
				i--;
				/* Echo backspace */
				extern int console_putchar(int c);
				console_putchar('\b');
				console_putchar(' ');
				console_putchar('\b');
			}
		} else if (c >= 32 && c < 127) {  /* Printable ASCII */
			buf[i++] = (char)c;
			/* Echo character */
			extern int console_putchar(int c);
			console_putchar(c);
		}
	}

	buf[i] = '\0';
	return i;
}

/*
 * Keyboard interrupt handler
 * Called from trap.c when keyboard interrupt occurs (TRAP_KBD)
 */
void
keyboard_interrupt(void)
{
	int c;

	if (!kbd_initialized)
		keyboard_init();

	/* Read all available characters from hardware */
	while ((c = console_getchar()) != -1) {
		/* Buffer the character */
		if (keyboard_buffer_put((unsigned char)c) == -1) {
			/* Buffer full - drop character */
			/* In a real implementation, might want to signal error */
		}
	}

	/* In a full implementation, would wake up sleeping readers here */
}

/*
 * Flush keyboard buffer
 */
void
keyboard_flush(void)
{
	kbd_head = 0;
	kbd_tail = 0;
	kbd_count = 0;
}

/*
 * Get keyboard buffer statistics
 */
void
keyboard_stats(int *count, int *size)
{
	if (count)
		*count = kbd_count;
	if (size)
		*size = KBD_BUFFER_SIZE;
}
