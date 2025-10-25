/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Serial port driver for Alpha - 16550 UART
 *
 * This driver supports the standard 16550 UART found on AlphaStation
 * systems via the ISA bus. It provides both polled and interrupt-driven
 * operation for console and serial communication.
 */

#include "serial.h"
#include "platform.h"
#include <sys/types.h>

/* Serial port database */
static struct alpha_serial_port serial_ports[4] = {
    { COM1_PORT, COM1_IRQ, DEFAULT_BAUD, 0, 0, 0, 0 },  /* COM1 */
    { COM2_PORT, COM2_IRQ, DEFAULT_BAUD, 0, 0, 0, 0 },  /* COM2 */
    { COM3_PORT, COM3_IRQ, DEFAULT_BAUD, 0, 0, 0, 0 },  /* COM3 */
    { COM4_PORT, COM4_IRQ, DEFAULT_BAUD, 0, 0, 0, 0 }   /* COM4 */
};

/* Default console port (COM1) */
static int console_port = 0;

/*
 * Read a UART register
 */
static inline unsigned char
uart_read(unsigned short base, unsigned char reg)
{
    return inb(base + reg);
}

/*
 * Write a UART register
 */
static inline void
uart_write(unsigned short base, unsigned char reg, unsigned char val)
{
    outb(base + reg, val);
}

/*
 * Calculate divisor for baud rate
 */
static unsigned short
uart_divisor(unsigned int baud)
{
    return (unsigned short)(UART_BASE_FREQ / (16 * baud));
}

/*
 * Initialize a serial port
 */
void
alpha_serial_init(int port, unsigned int baud)
{
    struct alpha_serial_port *sp;
    unsigned short divisor;
    unsigned char lcr;

    if (port < 0 || port >= 4)
        return;

    sp = &serial_ports[port];
    sp->baud = baud;

    /* Disable interrupts */
    uart_write(sp->base, UART_IER, 0x00);

    /* Set DLAB to access divisor */
    uart_write(sp->base, UART_LCR, UART_LCR_DLAB);

    /* Set baud rate divisor */
    divisor = uart_divisor(baud);
    uart_write(sp->base, UART_DLL, divisor & 0xFF);
    uart_write(sp->base, UART_DLH, (divisor >> 8) & 0xFF);

    /* Configure: 8 data bits, 1 stop bit, no parity */
    lcr = UART_LCR_WLS_8;
    uart_write(sp->base, UART_LCR, lcr);
    sp->lcr = lcr;

    /* Enable and clear FIFOs, set trigger level to 14 bytes */
    sp->fcr = UART_FCR_ENABLE | UART_FCR_CLR_RX | UART_FCR_CLR_TX | UART_FCR_TRIG_14;
    uart_write(sp->base, UART_FCR, sp->fcr);

    /* Enable DTR, RTS, and OUT2 (required for interrupts) */
    sp->mcr = UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2;
    uart_write(sp->base, UART_MCR, sp->mcr);

    /* Clear any pending interrupts */
    uart_read(sp->base, UART_IIR);
    uart_read(sp->base, UART_LSR);
    uart_read(sp->base, UART_MSR);
    uart_read(sp->base, UART_RBR);

    sp->initialized = 1;
}

/*
 * Transmit a character (polled mode)
 */
void
alpha_serial_putc(int port, char c)
{
    struct alpha_serial_port *sp;
    int timeout;

    if (port < 0 || port >= 4)
        return;

    sp = &serial_ports[port];
    if (!sp->initialized)
        return;

    /* Handle newline -> CR+LF conversion */
    if (c == '\n')
        alpha_serial_putc(port, '\r');

    /* Wait for transmitter to be ready (timeout after ~100ms) */
    timeout = 100000;
    while (!(uart_read(sp->base, UART_LSR) & UART_LSR_THRE)) {
        if (--timeout == 0)
            return;
    }

    /* Transmit character */
    uart_write(sp->base, UART_THR, c);
}

/*
 * Receive a character (polled mode)
 */
int
alpha_serial_getc(int port)
{
    struct alpha_serial_port *sp;
    unsigned char lsr;

    if (port < 0 || port >= 4)
        return -1;

    sp = &serial_ports[port];
    if (!sp->initialized)
        return -1;

    /* Wait for data to be available */
    while (!alpha_serial_poll(port))
        ;

    /* Check for errors */
    lsr = uart_read(sp->base, UART_LSR);
    if (lsr & (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI)) {
        /* Error occurred - read and discard */
        uart_read(sp->base, UART_RBR);
        return -1;
    }

    /* Read character */
    return uart_read(sp->base, UART_RBR);
}

/*
 * Poll for received data
 */
int
alpha_serial_poll(int port)
{
    struct alpha_serial_port *sp;

    if (port < 0 || port >= 4)
        return 0;

    sp = &serial_ports[port];
    if (!sp->initialized)
        return 0;

    return (uart_read(sp->base, UART_LSR) & UART_LSR_DR) ? 1 : 0;
}

/*
 * Write a string to the serial port
 */
void
alpha_serial_write(int port, const char *str, int len)
{
    int i;

    for (i = 0; i < len; i++)
        alpha_serial_putc(port, str[i]);
}

/*
 * Read from the serial port (non-blocking)
 */
int
alpha_serial_read(int port, char *buf, int len)
{
    int i;
    int c;

    for (i = 0; i < len; i++) {
        if (!alpha_serial_poll(port))
            break;

        c = alpha_serial_getc(port);
        if (c < 0)
            break;

        buf[i] = (char)c;
    }

    return i;
}

/*
 * Change baud rate
 */
void
alpha_serial_set_baud(int port, unsigned int baud)
{
    struct alpha_serial_port *sp;
    unsigned short divisor;
    unsigned char lcr;

    if (port < 0 || port >= 4)
        return;

    sp = &serial_ports[port];
    if (!sp->initialized)
        return;

    /* Save current LCR */
    lcr = sp->lcr;

    /* Set DLAB to access divisor */
    uart_write(sp->base, UART_LCR, lcr | UART_LCR_DLAB);

    /* Set new divisor */
    divisor = uart_divisor(baud);
    uart_write(sp->base, UART_DLL, divisor & 0xFF);
    uart_write(sp->base, UART_DLH, (divisor >> 8) & 0xFF);

    /* Restore LCR */
    uart_write(sp->base, UART_LCR, lcr);

    sp->baud = baud;
}

/*
 * Serial interrupt handler
 */
void
alpha_serial_interrupt(int irq)
{
    struct alpha_serial_port *sp;
    unsigned char iir, lsr;
    int port;
    int c;

    /* Find which port triggered the interrupt */
    for (port = 0; port < 4; port++) {
        sp = &serial_ports[port];
        if (sp->initialized && sp->irq == irq)
            break;
    }

    if (port >= 4)
        return;

    /* Handle all pending interrupts */
    while (1) {
        iir = uart_read(sp->base, UART_IIR);

        /* No more interrupts pending */
        if (iir & UART_IIR_NO_INT)
            break;

        /* Process interrupt based on type */
        switch (iir & UART_IIR_ID) {
        case UART_IIR_RDI:      /* Received data available */
        case UART_IIR_CTI:      /* Character timeout */
            /* Read all available characters */
            while (uart_read(sp->base, UART_LSR) & UART_LSR_DR) {
                c = uart_read(sp->base, UART_RBR);
                /* TODO: Add to input buffer or call handler */
            }
            break;

        case UART_IIR_THRI:     /* Transmitter holding register empty */
            /* TODO: Send next character from output buffer */
            break;

        case UART_IIR_RLSI:     /* Receiver line status interrupt */
            /* Read LSR to clear the interrupt */
            lsr = uart_read(sp->base, UART_LSR);
            /* TODO: Handle errors */
            break;

        case UART_IIR_MSI:      /* Modem status interrupt */
            /* Read MSR to clear the interrupt */
            uart_read(sp->base, UART_MSR);
            break;
        }
    }
}

/*
 * Initialize console on COM1
 */
void
alpha_serial_console_init(void)
{
    console_port = 0;
    alpha_serial_init(console_port, 9600);
}

/*
 * Console output character
 */
void
alpha_serial_console_putc(char c)
{
    alpha_serial_putc(console_port, c);
}

/*
 * Console input character
 */
int
alpha_serial_console_getc(void)
{
    return alpha_serial_getc(console_port);
}

/*
 * Console poll for input
 */
int
alpha_serial_console_poll(void)
{
    return alpha_serial_poll(console_port);
}
