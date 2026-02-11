/* device.c - DLX I/O Device Simulation
 * Copyright (C) 1999 Apple Computer, Inc.
 */

#include "dlx.h"
#include <time.h>

/* Initialize devices */
void dlx_devices_init(dlx_sim_t *sim) {
    sim->devices.timer = 0;
    sim->devices.kbd_head = 0;
    sim->devices.kbd_tail = 0;
    memset(sim->devices.kbd_buffer, 0, sizeof(sim->devices.kbd_buffer));
}

/* Read from memory-mapped device */
uint32_t dlx_device_read(dlx_sim_t *sim, uint32_t addr) {
    switch (addr) {
    case TIMER_ADDR:
        /* Return cycle count as timer value */
        return (uint32_t)sim->cpu.cycles;

    case KBD_DATA_ADDR:
        /* Read from keyboard buffer */
        if (sim->devices.kbd_head != sim->devices.kbd_tail) {
            char c = sim->devices.kbd_buffer[sim->devices.kbd_tail];
            sim->devices.kbd_tail = (sim->devices.kbd_tail + 1) % 256;
            return (uint32_t)c;
        }
        return 0;

    case KBD_STATUS_ADDR:
        /* Return 1 if keyboard data available */
        return (sim->devices.kbd_head != sim->devices.kbd_tail) ? 1 : 0;

    default:
        if (sim->verbose) {
            fprintf(stderr, "Read from unknown device: 0x%08x\n", addr);
        }
        return 0;
    }
}

/* Write to memory-mapped device */
void dlx_device_write(dlx_sim_t *sim, uint32_t addr, uint32_t value) {
    switch (addr) {
    case TIMER_ADDR:
        /* Writing to timer resets it */
        sim->devices.timer = value;
        break;

    case 0xFFF00000:
        /* Console output */
        putchar(value & 0xFF);
        fflush(stdout);
        break;

    case 0xFFF00004:
        /* Simulator control */
        if (value == 0) {
            /* Halt simulation */
            sim->cpu.running = 0;
            if (sim->verbose) {
                printf("\nSimulation halted by program\n");
            }
        }
        break;

    default:
        if (sim->verbose) {
            fprintf(stderr, "Write to unknown device: 0x%08x = 0x%08x\n",
                    addr, value);
        }
        break;
    }
}

/* Simulate keyboard input (for interactive mode) */
void dlx_kbd_putchar(dlx_sim_t *sim, char c) {
    int next_head = (sim->devices.kbd_head + 1) % 256;
    if (next_head != sim->devices.kbd_tail) {
        sim->devices.kbd_buffer[sim->devices.kbd_head] = c;
        sim->devices.kbd_head = next_head;
    }
}
