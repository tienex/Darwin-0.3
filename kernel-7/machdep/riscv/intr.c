/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * RISC-V interrupt handling routines
 *
 * This module manages the Platform-Level Interrupt Controller (PLIC)
 * and Core-Local Interrupt Controller (CLINT) on RISC-V systems.
 */

#include <mach/mach_types.h>
#include <mach/boolean.h>
#include <kern/spl.h>
#include <machdep/riscv/interrupts.h>
#include <machdep/riscv/trap.h>

/*
 * Interrupt table
 */
static intr_entry_t intr_table[MAX_INTERRUPTS];
static int intr_initialized = 0;

/*
 * PLIC and CLINT base addresses (platform-specific)
 * These should be configured based on device tree or platform config
 */
#define PLIC_BASE       0x0C000000UL
#define CLINT_BASE      0x02000000UL

/*
 * PLIC register offsets
 */
#define PLIC_PRIORITY(irq)      (PLIC_BASE + 0x000000 + (irq) * 4)
#define PLIC_PENDING(irq)       (PLIC_BASE + 0x001000 + ((irq) / 32) * 4)
#define PLIC_ENABLE(hart, irq)  (PLIC_BASE + 0x002000 + (hart) * 0x80 + ((irq) / 32) * 4)
#define PLIC_THRESHOLD(hart)    (PLIC_BASE + 0x200000 + (hart) * 0x1000)
#define PLIC_CLAIM(hart)        (PLIC_BASE + 0x200004 + (hart) * 0x1000)

/*
 * CLINT register offsets
 */
#define CLINT_MSIP(hart)        (CLINT_BASE + 0x0000 + (hart) * 4)
#define CLINT_MTIMECMP(hart)    (CLINT_BASE + 0x4000 + (hart) * 8)
#define CLINT_MTIME             (CLINT_BASE + 0xBFF8)

/*
 * Initialize interrupt subsystem
 */
void
intr_init(void)
{
	int i;

	/* Clear interrupt table */
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		intr_table[i].handler = NULL;
		intr_table[i].arg = NULL;
		intr_table[i].irq = i;
		intr_table[i].enabled = FALSE;
	}

	/* Disable all interrupts at PLIC */
	for (i = 1; i < MAX_INTERRUPTS; i++) {
		intr_disable(i);
	}

	/* Set threshold to 0 (allow all priorities) */
	/* This is platform-specific and would need proper MMIO access */

	intr_initialized = 1;
}

/*
 * Establish an interrupt handler
 */
int
intr_establish(int irq, intr_handler_t handler, void *arg)
{
	if (irq < 0 || irq >= MAX_INTERRUPTS)
		return -1;

	if (!intr_initialized)
		intr_init();

	intr_table[irq].handler = handler;
	intr_table[irq].arg = arg;
	intr_table[irq].enabled = TRUE;

	/* Enable interrupt at PLIC */
	intr_enable(irq);

	return 0;
}

/*
 * Remove an interrupt handler
 */
void
intr_disestablish(int irq)
{
	if (irq < 0 || irq >= MAX_INTERRUPTS)
		return;

	/* Disable interrupt at PLIC */
	intr_disable(irq);

	intr_table[irq].handler = NULL;
	intr_table[irq].arg = NULL;
	intr_table[irq].enabled = FALSE;
}

/*
 * Enable an interrupt
 */
void
intr_enable(int irq)
{
	if (irq < 0 || irq >= MAX_INTERRUPTS)
		return;

	intr_table[irq].enabled = TRUE;

	/*
	 * Enable in PLIC - platform-specific
	 * This would require proper MMIO access to PLIC registers
	 * For now, this is a stub implementation
	 */
}

/*
 * Disable an interrupt
 */
void
intr_disable(int irq)
{
	if (irq < 0 || irq >= MAX_INTERRUPTS)
		return;

	intr_table[irq].enabled = FALSE;

	/*
	 * Disable in PLIC - platform-specific
	 * This would require proper MMIO access to PLIC registers
	 * For now, this is a stub implementation
	 */
}

/*
 * Handle external interrupt (from PLIC)
 */
void
intr_handler_external(void)
{
	int irq;
	intr_handler_t handler;
	void *arg;

	/*
	 * Claim interrupt from PLIC
	 * This is platform-specific and would need proper MMIO access
	 */
	irq = 0;  /* Would read from PLIC_CLAIM register */

	if (irq > 0 && irq < MAX_INTERRUPTS) {
		handler = intr_table[irq].handler;
		arg = intr_table[irq].arg;

		if (handler && intr_table[irq].enabled) {
			/* Call the handler */
			(*handler)(arg);
		}

		/*
		 * Complete interrupt at PLIC
		 * Write IRQ number back to PLIC_CLAIM register
		 */
	}
}

/*
 * Handle timer interrupt (from CLINT)
 */
void
intr_handler_timer(void)
{
	/*
	 * Handle timer interrupt
	 * This would typically call the clock handler
	 */

	/*
	 * Clear timer interrupt by setting mtimecmp to a future value
	 * This is platform-specific
	 */
}

/*
 * Handle software interrupt (from CLINT)
 */
void
intr_handler_software(void)
{
	/*
	 * Handle software interrupt (IPI on multi-core systems)
	 * Clear by writing 0 to MSIP register
	 */
}
