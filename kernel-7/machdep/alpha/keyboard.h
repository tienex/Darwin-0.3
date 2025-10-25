/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Keyboard controller driver for Alpha - 8042 PS/2 Controller
 */

#ifndef _MACHDEP_ALPHA_KEYBOARD_H_
#define _MACHDEP_ALPHA_KEYBOARD_H_

/* 8042 PS/2 Controller ports */
#define KBD_DATA_PORT       0x60    /* Data port (read/write) */
#define KBD_STATUS_PORT     0x64    /* Status port (read) */
#define KBD_CMD_PORT        0x64    /* Command port (write) */

/* Status register bits */
#define KBD_STAT_OBF        0x01    /* Output buffer full */
#define KBD_STAT_IBF        0x02    /* Input buffer full */
#define KBD_STAT_SYS        0x04    /* System flag */
#define KBD_STAT_CMD        0x08    /* Command/data (1=command, 0=data) */
#define KBD_STAT_UNLOCKED   0x10    /* Keyboard not locked */
#define KBD_STAT_AUXOBF     0x20    /* Auxiliary output buffer full (mouse) */
#define KBD_STAT_TIMEOUT    0x40    /* Timeout error */
#define KBD_STAT_PARITY     0x80    /* Parity error */

/* 8042 Commands */
#define KBD_CMD_READ_CMD    0x20    /* Read command byte */
#define KBD_CMD_WRITE_CMD   0x60    /* Write command byte */
#define KBD_CMD_SELF_TEST   0xAA    /* Controller self-test */
#define KBD_CMD_KBD_TEST    0xAB    /* Keyboard interface test */
#define KBD_CMD_KBD_DIS     0xAD    /* Disable keyboard */
#define KBD_CMD_KBD_EN      0xAE    /* Enable keyboard */
#define KBD_CMD_READ_IN     0xC0    /* Read input port */
#define KBD_CMD_READ_OUT    0xD0    /* Read output port */
#define KBD_CMD_WRITE_OUT   0xD1    /* Write output port */
#define KBD_CMD_READ_TEST   0xE0    /* Read test inputs */
#define KBD_CMD_AUX_DIS     0xA7    /* Disable auxiliary device (mouse) */
#define KBD_CMD_AUX_EN      0xA8    /* Enable auxiliary device */
#define KBD_CMD_AUX_TEST    0xA9    /* Test auxiliary interface */
#define KBD_CMD_WRITE_AUX   0xD4    /* Write to auxiliary device */

/* Command byte bits */
#define KBD_CB_KBD_INT      0x01    /* Keyboard interrupt enable */
#define KBD_CB_AUX_INT      0x02    /* Auxiliary interrupt enable */
#define KBD_CB_SYS          0x04    /* System flag */
#define KBD_CB_KBD_DIS      0x10    /* Keyboard disable */
#define KBD_CB_AUX_DIS      0x20    /* Auxiliary disable */
#define KBD_CB_XLATE        0x40    /* Scan code translation */

/* Output port bits */
#define KBD_OUT_RESET       0x01    /* System reset (0=reset) */
#define KBD_OUT_A20         0x02    /* A20 gate */
#define KBD_OUT_AUX_DATA    0x04    /* Auxiliary data */
#define KBD_OUT_AUX_CLK     0x08    /* Auxiliary clock */
#define KBD_OUT_KBD_OBF     0x10    /* Keyboard output buffer full */
#define KBD_OUT_AUX_OBF     0x20    /* Auxiliary output buffer full */
#define KBD_OUT_KBD_CLK     0x40    /* Keyboard clock */
#define KBD_OUT_KBD_DATA    0x80    /* Keyboard data */

/* Keyboard commands */
#define KBD_KBD_LED         0xED    /* Set LEDs */
#define KBD_KBD_ECHO        0xEE    /* Echo */
#define KBD_KBD_SCANCODE    0xF0    /* Get/set scan code set */
#define KBD_KBD_ID          0xF2    /* Read keyboard ID */
#define KBD_KBD_RATE        0xF3    /* Set typematic rate/delay */
#define KBD_KBD_ENABLE      0xF4    /* Enable scanning */
#define KBD_KBD_DISABLE     0xF5    /* Disable scanning */
#define KBD_KBD_DEFAULT     0xF6    /* Set default parameters */
#define KBD_KBD_RESET       0xFF    /* Reset keyboard */

/* Keyboard responses */
#define KBD_RESP_ACK        0xFA    /* Acknowledge */
#define KBD_RESP_RESEND     0xFE    /* Resend */
#define KBD_RESP_ERROR      0xFC    /* Error */
#define KBD_RESP_ECHO       0xEE    /* Echo response */
#define KBD_RESP_BAT_OK     0xAA    /* BAT completed successfully */
#define KBD_RESP_BAT_FAIL   0xFC    /* BAT failed */
#define KBD_RESP_ID1        0xAB    /* Keyboard ID byte 1 (MF2) */
#define KBD_RESP_ID2        0x83    /* Keyboard ID byte 2 (MF2) */

/* LED bits */
#define KBD_LED_SCROLL      0x01    /* Scroll Lock */
#define KBD_LED_NUM         0x02    /* Num Lock */
#define KBD_LED_CAPS        0x04    /* Caps Lock */

/* Special scan codes */
#define KBD_SC_EXTENDED     0xE0    /* Extended scan code prefix */
#define KBD_SC_RELEASED     0xF0    /* Key release prefix (set 2) */
#define KBD_SC_PAUSE        0xE1    /* Pause key prefix */

/* Timeout values */
#define KBD_TIMEOUT         100000  /* General timeout counter */
#define KBD_INIT_TIMEOUT    1000000 /* Initialization timeout */

/* Keyboard buffer size */
#define KBD_BUFFER_SIZE     64

/* Keyboard state structure */
struct alpha_keyboard {
    unsigned char leds;             /* Current LED state */
    unsigned char scan_set;         /* Current scan code set (1, 2, or 3) */
    unsigned char typematic_rate;   /* Typematic rate */
    unsigned char typematic_delay;  /* Typematic delay */
    int initialized;                /* Initialization flag */

    /* Input buffer (circular) */
    unsigned char buffer[KBD_BUFFER_SIZE];
    int buffer_head;
    int buffer_tail;
    int buffer_count;

    /* State flags */
    unsigned char extended;         /* Extended key flag */
    unsigned char released;         /* Key release flag */
    unsigned char shift;            /* Shift pressed */
    unsigned char ctrl;             /* Ctrl pressed */
    unsigned char alt;              /* Alt pressed */
    unsigned char caps_lock;        /* Caps Lock state */
    unsigned char num_lock;         /* Num Lock state */
    unsigned char scroll_lock;      /* Scroll Lock state */
};

/* Function prototypes */
int alpha_keyboard_init(void);
void alpha_keyboard_reset(void);
int alpha_keyboard_read(void);
int alpha_keyboard_poll(void);
void alpha_keyboard_write(unsigned char data);
unsigned char alpha_keyboard_read_status(void);
void alpha_keyboard_send_cmd(unsigned char cmd);
unsigned char alpha_keyboard_read_data(void);
void alpha_keyboard_write_data(unsigned char data);
int alpha_keyboard_wait_input(void);
int alpha_keyboard_wait_output(void);
void alpha_keyboard_set_leds(unsigned char leds);
void alpha_keyboard_set_rate(unsigned char rate, unsigned char delay);
void alpha_keyboard_interrupt(void);
int alpha_keyboard_getc(void);
void alpha_keyboard_flush(void);

/* A20 gate control */
void alpha_keyboard_enable_a20(void);
void alpha_keyboard_disable_a20(void);

#endif /* _MACHDEP_ALPHA_KEYBOARD_H_ */
