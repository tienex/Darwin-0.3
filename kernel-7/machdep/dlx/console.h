/*
 * DLX Console and Keyboard Driver Interface
 */

#ifndef _MACHDEP_DLX_CONSOLE_H_
#define _MACHDEP_DLX_CONSOLE_H_

/*
 * Console functions
 */
void console_init(void);
int console_putchar(int c);
int console_getchar(void);
int console_chars_avail(void);
void console_puts(const char *s);
void console_interrupt(void);

/*
 * Keyboard functions
 */
void keyboard_init(void);
int keyboard_read(void);
int keyboard_read_nonblock(void);
int keyboard_readline(char *buf, int max_len);
void keyboard_interrupt(void);
void keyboard_flush(void);
int keyboard_has_data(void);
void keyboard_stats(int *count, int *size);

/*
 * Printf for kernel debugging
 */
void printf(const char *fmt, ...);

#endif /* _MACHDEP_DLX_CONSOLE_H_ */
