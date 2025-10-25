/*
 * x86-64 I/O Primitives
 *
 * This file provides I/O port access functions and primitives.
 * These wrap the low-level assembly I/O instructions.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <mach/machine.h>

/*
 * External assembly functions from locore.s
 */
extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
extern unsigned int inl(unsigned short port);
extern void outb(unsigned short port, unsigned char val);
extern void outw(unsigned short port, unsigned short val);
extern void outl(unsigned short port, unsigned int val);

/*
 * Read byte from I/O port
 */
unsigned char
io_inb(unsigned short port)
{
	return inb(port);
}

/*
 * Read word from I/O port
 */
unsigned short
io_inw(unsigned short port)
{
	return inw(port);
}

/*
 * Read long from I/O port
 */
unsigned int
io_inl(unsigned short port)
{
	return inl(port);
}

/*
 * Write byte to I/O port
 */
void
io_outb(unsigned short port, unsigned char val)
{
	outb(port, val);
}

/*
 * Write word to I/O port
 */
void
io_outw(unsigned short port, unsigned short val)
{
	outw(port, val);
}

/*
 * Write long to I/O port
 */
void
io_outl(unsigned short port, unsigned int val)
{
	outl(port, val);
}

/*
 * Read multiple bytes from I/O port
 */
void
io_insb(unsigned short port, void *addr, int count)
{
	unsigned char *buf = addr;
	int i;

	for (i = 0; i < count; i++)
		buf[i] = inb(port);
}

/*
 * Read multiple words from I/O port
 */
void
io_insw(unsigned short port, void *addr, int count)
{
	unsigned short *buf = addr;
	int i;

	for (i = 0; i < count; i++)
		buf[i] = inw(port);
}

/*
 * Read multiple longs from I/O port
 */
void
io_insl(unsigned short port, void *addr, int count)
{
	unsigned int *buf = addr;
	int i;

	for (i = 0; i < count; i++)
		buf[i] = inl(port);
}

/*
 * Write multiple bytes to I/O port
 */
void
io_outsb(unsigned short port, const void *addr, int count)
{
	const unsigned char *buf = addr;
	int i;

	for (i = 0; i < count; i++)
		outb(port, buf[i]);
}

/*
 * Write multiple words to I/O port
 */
void
io_outsw(unsigned short port, const void *addr, int count)
{
	const unsigned short *buf = addr;
	int i;

	for (i = 0; i < count; i++)
		outw(port, buf[i]);
}

/*
 * Write multiple longs to I/O port
 */
void
io_outsl(unsigned short port, const void *addr, int count)
{
	const unsigned int *buf = addr;
	int i;

	for (i = 0; i < count; i++)
		outl(port, buf[i]);
}

/*
 * I/O delay (for slow devices)
 */
void
io_delay(void)
{
	/* Write to port 0x80 (diagnostic port) for ~1us delay */
	outb(0x80, 0);
}

/*
 * Memory-mapped I/O operations
 */

/*
 * Read byte from memory-mapped I/O
 */
unsigned char
mmio_readb(volatile void *addr)
{
	return *(volatile unsigned char *)addr;
}

/*
 * Read word from memory-mapped I/O
 */
unsigned short
mmio_readw(volatile void *addr)
{
	return *(volatile unsigned short *)addr;
}

/*
 * Read long from memory-mapped I/O
 */
unsigned int
mmio_readl(volatile void *addr)
{
	return *(volatile unsigned int *)addr;
}

/*
 * Read quad from memory-mapped I/O
 */
unsigned long
mmio_readq(volatile void *addr)
{
	return *(volatile unsigned long *)addr;
}

/*
 * Write byte to memory-mapped I/O
 */
void
mmio_writeb(volatile void *addr, unsigned char val)
{
	*(volatile unsigned char *)addr = val;
}

/*
 * Write word to memory-mapped I/O
 */
void
mmio_writew(volatile void *addr, unsigned short val)
{
	*(volatile unsigned short *)addr = val;
}

/*
 * Write long to memory-mapped I/O
 */
void
mmio_writel(volatile void *addr, unsigned int val)
{
	*(volatile unsigned int *)addr = val;
}

/*
 * Write quad to memory-mapped I/O
 */
void
mmio_writeq(volatile void *addr, unsigned long val)
{
	*(volatile unsigned long *)addr = val;
}

/*
 * I/O port permission bitmap operations
 * (for allowing user-space I/O)
 */

/*
 * Allow I/O port access for user process
 */
int
io_allow_port(unsigned short port)
{
	/* Would modify I/O permission bitmap in TSS */
	/* Stub implementation */
	return 0;
}

/*
 * Deny I/O port access for user process
 */
int
io_deny_port(unsigned short port)
{
	/* Would modify I/O permission bitmap in TSS */
	/* Stub implementation */
	return 0;
}
