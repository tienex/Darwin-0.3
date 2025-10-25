/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Keyboard controller driver for Alpha - 8042 PS/2 Controller
 *
 * This driver supports the Intel 8042 PS/2 keyboard controller found
 * on AlphaStation systems via the ISA bus. It provides keyboard input
 * and A20 gate control.
 */

#include "keyboard.h"
#include "platform.h"
#include <sys/types.h>

/* Global keyboard state */
static struct alpha_keyboard kbd;

/* Scan code to ASCII translation table (US keyboard, scan code set 1) */
static const unsigned char scancode_to_ascii[] = {
    0,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',   /* 00-07 */
    '7', '8',  '9',  '0',  '-',  '=',  '\b', '\t',  /* 08-0F */
    'q', 'w',  'e',  'r',  't',  'y',  'u',  'i',   /* 10-17 */
    'o', 'p',  '[',  ']',  '\n', 0,    'a',  's',   /* 18-1F */
    'd', 'f',  'g',  'h',  'j',  'k',  'l',  ';',   /* 20-27 */
    '\'', '`', 0,    '\\', 'z',  'x',  'c',  'v',   /* 28-2F */
    'b', 'n',  'm',  ',',  '.',  '/',  0,    '*',   /* 30-37 */
    0,   ' ',  0,    0,    0,    0,    0,    0,     /* 38-3F */
};

static const unsigned char scancode_to_ascii_shift[] = {
    0,   0x1B, '!',  '@',  '#',  '$',  '%',  '^',   /* 00-07 */
    '&', '*',  '(',  ')',  '_',  '+',  '\b', '\t',  /* 08-0F */
    'Q', 'W',  'E',  'R',  'T',  'Y',  'U',  'I',   /* 10-17 */
    'O', 'P',  '{',  '}',  '\n', 0,    'A',  'S',   /* 18-1F */
    'D', 'F',  'G',  'H',  'J',  'K',  'L',  ':',   /* 20-27 */
    '"', '~',  0,    '|',  'Z',  'X',  'C',  'V',   /* 28-2F */
    'B', 'N',  'M',  '<',  '>',  '?',  0,    '*',   /* 30-37 */
    0,   ' ',  0,    0,    0,    0,    0,    0,     /* 38-3F */
};

/*
 * Read keyboard status register
 */
unsigned char
alpha_keyboard_read_status(void)
{
    return inb(KBD_STATUS_PORT);
}

/*
 * Read data from keyboard
 */
unsigned char
alpha_keyboard_read_data(void)
{
    return inb(KBD_DATA_PORT);
}

/*
 * Write data to keyboard
 */
void
alpha_keyboard_write_data(unsigned char data)
{
    outb(KBD_DATA_PORT, data);
}

/*
 * Send command to controller
 */
void
alpha_keyboard_send_cmd(unsigned char cmd)
{
    outb(KBD_CMD_PORT, cmd);
}

/*
 * Wait for input buffer to be empty
 */
int
alpha_keyboard_wait_input(void)
{
    int timeout = KBD_TIMEOUT;

    while (timeout--) {
        if (!(alpha_keyboard_read_status() & KBD_STAT_IBF))
            return 0;
    }
    return -1;  /* Timeout */
}

/*
 * Wait for output buffer to be full
 */
int
alpha_keyboard_wait_output(void)
{
    int timeout = KBD_TIMEOUT;

    while (timeout--) {
        if (alpha_keyboard_read_status() & KBD_STAT_OBF)
            return 0;
    }
    return -1;  /* Timeout */
}

/*
 * Flush output buffer
 */
void
alpha_keyboard_flush(void)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (!(alpha_keyboard_read_status() & KBD_STAT_OBF))
            break;
        alpha_keyboard_read_data();
    }
}

/*
 * Write to keyboard with ACK
 */
static int
keyboard_write_ack(unsigned char data)
{
    int timeout = 3;    /* Retry up to 3 times */
    unsigned char response;

    while (timeout--) {
        if (alpha_keyboard_wait_input() < 0)
            return -1;

        alpha_keyboard_write_data(data);

        if (alpha_keyboard_wait_output() < 0)
            return -1;

        response = alpha_keyboard_read_data();

        if (response == KBD_RESP_ACK)
            return 0;

        if (response != KBD_RESP_RESEND)
            return -1;
    }

    return -1;
}

/*
 * Enable A20 gate
 */
void
alpha_keyboard_enable_a20(void)
{
    unsigned char out;

    /* Wait for input buffer */
    alpha_keyboard_wait_input();

    /* Read output port */
    alpha_keyboard_send_cmd(KBD_CMD_READ_OUT);
    alpha_keyboard_wait_output();
    out = alpha_keyboard_read_data();

    /* Set A20 bit */
    out |= KBD_OUT_A20;

    /* Write output port */
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_WRITE_OUT);
    alpha_keyboard_wait_input();
    alpha_keyboard_write_data(out);
    alpha_keyboard_wait_input();
}

/*
 * Disable A20 gate
 */
void
alpha_keyboard_disable_a20(void)
{
    unsigned char out;

    /* Wait for input buffer */
    alpha_keyboard_wait_input();

    /* Read output port */
    alpha_keyboard_send_cmd(KBD_CMD_READ_OUT);
    alpha_keyboard_wait_output();
    out = alpha_keyboard_read_data();

    /* Clear A20 bit */
    out &= ~KBD_OUT_A20;

    /* Write output port */
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_WRITE_OUT);
    alpha_keyboard_wait_input();
    alpha_keyboard_write_data(out);
    alpha_keyboard_wait_input();
}

/*
 * Set keyboard LEDs
 */
void
alpha_keyboard_set_leds(unsigned char leds)
{
    if (keyboard_write_ack(KBD_KBD_LED) < 0)
        return;

    if (keyboard_write_ack(leds) < 0)
        return;

    kbd.leds = leds;
}

/*
 * Set typematic rate and delay
 */
void
alpha_keyboard_set_rate(unsigned char rate, unsigned char delay)
{
    unsigned char param;

    /* Encode rate and delay */
    param = (rate & 0x1F) | ((delay & 0x03) << 5);

    if (keyboard_write_ack(KBD_KBD_RATE) < 0)
        return;

    if (keyboard_write_ack(param) < 0)
        return;

    kbd.typematic_rate = rate;
    kbd.typematic_delay = delay;
}

/*
 * Reset keyboard
 */
void
alpha_keyboard_reset(void)
{
    unsigned char response;

    /* Send reset command */
    if (keyboard_write_ack(KBD_KBD_RESET) < 0)
        return;

    /* Wait for BAT completion (up to 1 second) */
    if (alpha_keyboard_wait_output() < 0)
        return;

    response = alpha_keyboard_read_data();
    if (response != KBD_RESP_BAT_OK) {
        /* BAT failed */
        return;
    }
}

/*
 * Initialize keyboard controller and keyboard
 */
int
alpha_keyboard_init(void)
{
    unsigned char cmd_byte;
    unsigned char response;

    /* Initialize state */
    kbd.leds = 0;
    kbd.scan_set = 1;
    kbd.typematic_rate = 0x0B;  /* ~10 chars/sec */
    kbd.typematic_delay = 1;    /* 500ms */
    kbd.buffer_head = 0;
    kbd.buffer_tail = 0;
    kbd.buffer_count = 0;
    kbd.extended = 0;
    kbd.released = 0;
    kbd.shift = 0;
    kbd.ctrl = 0;
    kbd.alt = 0;
    kbd.caps_lock = 0;
    kbd.num_lock = 0;
    kbd.scroll_lock = 0;

    /* Flush output buffer */
    alpha_keyboard_flush();

    /* Disable keyboard and mouse */
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_KBD_DIS);
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_AUX_DIS);

    /* Flush again */
    alpha_keyboard_flush();

    /* Read command byte */
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_READ_CMD);
    alpha_keyboard_wait_output();
    cmd_byte = alpha_keyboard_read_data();

    /* Modify command byte: enable keyboard interrupt, disable mouse */
    cmd_byte |= KBD_CB_KBD_INT;         /* Enable keyboard interrupt */
    cmd_byte &= ~KBD_CB_KBD_DIS;        /* Enable keyboard */
    cmd_byte |= KBD_CB_AUX_DIS;         /* Disable mouse */
    cmd_byte |= KBD_CB_XLATE;           /* Enable scan code translation */

    /* Write command byte */
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_WRITE_CMD);
    alpha_keyboard_wait_input();
    alpha_keyboard_write_data(cmd_byte);

    /* Controller self-test */
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_SELF_TEST);
    alpha_keyboard_wait_output();
    response = alpha_keyboard_read_data();
    if (response != 0x55) {
        /* Self-test failed */
        return -1;
    }

    /* Write command byte again (self-test may reset it) */
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_WRITE_CMD);
    alpha_keyboard_wait_input();
    alpha_keyboard_write_data(cmd_byte);

    /* Enable keyboard */
    alpha_keyboard_wait_input();
    alpha_keyboard_send_cmd(KBD_CMD_KBD_EN);

    /* Reset keyboard */
    alpha_keyboard_reset();

    /* Set LEDs */
    alpha_keyboard_set_leds(0);

    /* Set typematic rate */
    alpha_keyboard_set_rate(kbd.typematic_rate, kbd.typematic_delay);

    kbd.initialized = 1;

    return 0;
}

/*
 * Poll for keyboard data
 */
int
alpha_keyboard_poll(void)
{
    if (!kbd.initialized)
        return 0;

    return (alpha_keyboard_read_status() & KBD_STAT_OBF) ? 1 : 0;
}

/*
 * Read raw scan code from keyboard
 */
int
alpha_keyboard_read(void)
{
    unsigned char status;
    unsigned char scancode;

    if (!kbd.initialized)
        return -1;

    /* Check if data is available */
    status = alpha_keyboard_read_status();
    if (!(status & KBD_STAT_OBF))
        return -1;

    /* Read scan code */
    scancode = alpha_keyboard_read_data();

    return scancode;
}

/*
 * Keyboard interrupt handler
 */
void
alpha_keyboard_interrupt(void)
{
    int scancode;

    /* Read scan code */
    scancode = alpha_keyboard_read();
    if (scancode < 0)
        return;

    /* Add to buffer if not full */
    if (kbd.buffer_count < KBD_BUFFER_SIZE) {
        kbd.buffer[kbd.buffer_tail] = (unsigned char)scancode;
        kbd.buffer_tail = (kbd.buffer_tail + 1) % KBD_BUFFER_SIZE;
        kbd.buffer_count++;
    }
}

/*
 * Get translated ASCII character from keyboard
 */
int
alpha_keyboard_getc(void)
{
    unsigned char scancode;
    unsigned char ascii;
    int make_code;

    if (!kbd.initialized)
        return -1;

    /* Wait for data in buffer */
    while (kbd.buffer_count == 0) {
        /* In interrupt-driven mode, would sleep here */
        /* In polled mode, read directly */
        if (!alpha_keyboard_poll())
            continue;

        alpha_keyboard_interrupt();
    }

    /* Get scan code from buffer */
    scancode = kbd.buffer[kbd.buffer_head];
    kbd.buffer_head = (kbd.buffer_head + 1) % KBD_BUFFER_SIZE;
    kbd.buffer_count--;

    /* Check for extended code */
    if (scancode == KBD_SC_EXTENDED) {
        kbd.extended = 1;
        return -1;  /* Need next code */
    }

    /* Check for make/break (bit 7 = 1 means break) */
    make_code = !(scancode & 0x80);
    scancode &= 0x7F;

    /* Handle special keys */
    switch (scancode) {
    case 0x2A:  /* Left Shift */
    case 0x36:  /* Right Shift */
        kbd.shift = make_code;
        return -1;

    case 0x1D:  /* Ctrl */
        kbd.ctrl = make_code;
        return -1;

    case 0x38:  /* Alt */
        kbd.alt = make_code;
        return -1;

    case 0x3A:  /* Caps Lock */
        if (make_code) {
            kbd.caps_lock = !kbd.caps_lock;
            alpha_keyboard_set_leds((kbd.caps_lock ? KBD_LED_CAPS : 0) |
                                   (kbd.num_lock ? KBD_LED_NUM : 0) |
                                   (kbd.scroll_lock ? KBD_LED_SCROLL : 0));
        }
        return -1;

    case 0x45:  /* Num Lock */
        if (make_code) {
            kbd.num_lock = !kbd.num_lock;
            alpha_keyboard_set_leds((kbd.caps_lock ? KBD_LED_CAPS : 0) |
                                   (kbd.num_lock ? KBD_LED_NUM : 0) |
                                   (kbd.scroll_lock ? KBD_LED_SCROLL : 0));
        }
        return -1;

    case 0x46:  /* Scroll Lock */
        if (make_code) {
            kbd.scroll_lock = !kbd.scroll_lock;
            alpha_keyboard_set_leds((kbd.caps_lock ? KBD_LED_CAPS : 0) |
                                   (kbd.num_lock ? KBD_LED_NUM : 0) |
                                   (kbd.scroll_lock ? KBD_LED_SCROLL : 0));
        }
        return -1;
    }

    /* Only process make codes for regular keys */
    if (!make_code)
        return -1;

    /* Translate scan code to ASCII */
    if (scancode >= sizeof(scancode_to_ascii))
        return -1;

    if (kbd.shift || (kbd.caps_lock && scancode >= 0x10 && scancode <= 0x32))
        ascii = scancode_to_ascii_shift[scancode];
    else
        ascii = scancode_to_ascii[scancode];

    if (ascii == 0)
        return -1;

    /* Handle Ctrl combinations */
    if (kbd.ctrl && ascii >= 'a' && ascii <= 'z')
        ascii = ascii - 'a' + 1;  /* Ctrl-A = 1, etc. */
    else if (kbd.ctrl && ascii >= 'A' && ascii <= 'Z')
        ascii = ascii - 'A' + 1;

    return ascii;
}
