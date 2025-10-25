/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Serial port driver for Alpha - 16550 UART
 */

#ifndef _MACHDEP_ALPHA_SERIAL_H_
#define _MACHDEP_ALPHA_SERIAL_H_

/* 16550 UART Register offsets */
#define UART_RBR        0       /* Receive Buffer Register (read) */
#define UART_THR        0       /* Transmit Holding Register (write) */
#define UART_DLL        0       /* Divisor Latch Low (DLAB=1) */
#define UART_DLH        1       /* Divisor Latch High (DLAB=1) */
#define UART_IER        1       /* Interrupt Enable Register */
#define UART_IIR        2       /* Interrupt Identification Register (read) */
#define UART_FCR        2       /* FIFO Control Register (write) */
#define UART_LCR        3       /* Line Control Register */
#define UART_MCR        4       /* Modem Control Register */
#define UART_LSR        5       /* Line Status Register */
#define UART_MSR        6       /* Modem Status Register */
#define UART_SCR        7       /* Scratch Register */

/* IER bits */
#define UART_IER_RDI    0x01    /* Received Data Available Interrupt */
#define UART_IER_THRI   0x02    /* Transmitter Holding Register Empty */
#define UART_IER_RLSI   0x04    /* Receiver Line Status Interrupt */
#define UART_IER_MSI    0x08    /* Modem Status Interrupt */

/* IIR bits */
#define UART_IIR_NO_INT 0x01    /* No interrupts pending */
#define UART_IIR_ID     0x0E    /* Interrupt ID mask */
#define UART_IIR_MSI    0x00    /* Modem status interrupt */
#define UART_IIR_THRI   0x02    /* Transmitter holding register empty */
#define UART_IIR_RDI    0x04    /* Received data available */
#define UART_IIR_RLSI   0x06    /* Receiver line status interrupt */
#define UART_IIR_CTI    0x0C    /* Character timeout */

/* FCR bits */
#define UART_FCR_ENABLE 0x01    /* Enable FIFOs */
#define UART_FCR_CLR_RX 0x02    /* Clear RX FIFO */
#define UART_FCR_CLR_TX 0x04    /* Clear TX FIFO */
#define UART_FCR_DMA    0x08    /* DMA mode select */
#define UART_FCR_TRIG_1 0x00    /* Trigger level 1 byte */
#define UART_FCR_TRIG_4 0x40    /* Trigger level 4 bytes */
#define UART_FCR_TRIG_8 0x80    /* Trigger level 8 bytes */
#define UART_FCR_TRIG_14 0xC0   /* Trigger level 14 bytes */

/* LCR bits */
#define UART_LCR_WLS_5  0x00    /* 5 bit character length */
#define UART_LCR_WLS_6  0x01    /* 6 bit character length */
#define UART_LCR_WLS_7  0x02    /* 7 bit character length */
#define UART_LCR_WLS_8  0x03    /* 8 bit character length */
#define UART_LCR_STB    0x04    /* Number of stop bits (0=1, 1=1.5/2) */
#define UART_LCR_PEN    0x08    /* Parity enable */
#define UART_LCR_EPS    0x10    /* Even parity select */
#define UART_LCR_STKP   0x20    /* Stick parity */
#define UART_LCR_SBRK   0x40    /* Set break */
#define UART_LCR_DLAB   0x80    /* Divisor latch access bit */

/* MCR bits */
#define UART_MCR_DTR    0x01    /* Data Terminal Ready */
#define UART_MCR_RTS    0x02    /* Request to Send */
#define UART_MCR_OUT1   0x04    /* Out 1 */
#define UART_MCR_OUT2   0x08    /* Out 2 (enables interrupts) */
#define UART_MCR_LOOP   0x10    /* Loopback mode */

/* LSR bits */
#define UART_LSR_DR     0x01    /* Data ready */
#define UART_LSR_OE     0x02    /* Overrun error */
#define UART_LSR_PE     0x04    /* Parity error */
#define UART_LSR_FE     0x08    /* Framing error */
#define UART_LSR_BI     0x10    /* Break interrupt */
#define UART_LSR_THRE   0x20    /* Transmit holding register empty */
#define UART_LSR_TEMT   0x40    /* Transmitter empty */
#define UART_LSR_FIFOERR 0x80   /* FIFO error */

/* MSR bits */
#define UART_MSR_DCTS   0x01    /* Delta CTS */
#define UART_MSR_DDSR   0x02    /* Delta DSR */
#define UART_MSR_TERI   0x04    /* Trailing edge RI */
#define UART_MSR_DDCD   0x08    /* Delta DCD */
#define UART_MSR_CTS    0x10    /* Clear to Send */
#define UART_MSR_DSR    0x20    /* Data Set Ready */
#define UART_MSR_RI     0x40    /* Ring Indicator */
#define UART_MSR_DCD    0x80    /* Data Carrier Detect */

/* Standard baud rates */
#define UART_BAUD_115200 1
#define UART_BAUD_57600  2
#define UART_BAUD_38400  3
#define UART_BAUD_19200  6
#define UART_BAUD_9600   12
#define UART_BAUD_4800   24
#define UART_BAUD_2400   48
#define UART_BAUD_1200   96

/* Serial port configuration */
#define UART_BASE_FREQ  1843200 /* Base frequency for divisor calculation */
#define DEFAULT_BAUD    9600

/* Serial port addresses on ISA bus */
#define COM1_PORT       0x3F8
#define COM2_PORT       0x2F8
#define COM3_PORT       0x3E8
#define COM4_PORT       0x2E8

/* IRQ lines */
#define COM1_IRQ        4
#define COM2_IRQ        3
#define COM3_IRQ        4
#define COM4_IRQ        3

/* Serial port structure */
struct alpha_serial_port {
    unsigned short base;        /* I/O port base address */
    unsigned char irq;          /* IRQ number */
    unsigned int baud;          /* Baud rate */
    unsigned char lcr;          /* Line control settings */
    unsigned char mcr;          /* Modem control settings */
    unsigned char fcr;          /* FIFO control settings */
    int initialized;            /* Initialization flag */
};

/* Function prototypes */
void alpha_serial_init(int port, unsigned int baud);
void alpha_serial_putc(int port, char c);
int alpha_serial_getc(int port);
int alpha_serial_poll(int port);
void alpha_serial_write(int port, const char *str, int len);
int alpha_serial_read(int port, char *buf, int len);
void alpha_serial_set_baud(int port, unsigned int baud);
void alpha_serial_interrupt(int irq);

/* Console interface */
void alpha_serial_console_init(void);
void alpha_serial_console_putc(char c);
int alpha_serial_console_getc(void);
int alpha_serial_console_poll(void);

#endif /* _MACHDEP_ALPHA_SERIAL_H_ */
