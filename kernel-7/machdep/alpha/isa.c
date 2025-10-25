/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * ISA Bus Support for AlphaStation 500/600
 *
 * The ISA bus on Alpha systems is handled through PCI-to-ISA bridges,
 * typically integrated into the core chipset.
 */

#include <mach/mach_types.h>
#include "platform.h"

/*
 * ISA interrupt routing
 *
 * ISA interrupts are mapped to PCI interrupts through the chipset.
 * AlphaStation 500/600 use the following mapping:
 */
#define ISA_IRQ_TIMER		0	/* System timer */
#define ISA_IRQ_KEYBOARD	1	/* Keyboard */
#define ISA_IRQ_CASCADE		2	/* Cascade from slave PIC */
#define ISA_IRQ_SERIAL2		3	/* COM2 */
#define ISA_IRQ_SERIAL1		4	/* COM1 */
#define ISA_IRQ_PARALLEL	7	/* Parallel port */
#define ISA_IRQ_RTC		8	/* Real-time clock */
#define ISA_IRQ_MOUSE		12	/* PS/2 mouse */
#define ISA_IRQ_FPU		13	/* FPU (not used on Alpha) */
#define ISA_IRQ_IDE_PRIMARY	14	/* Primary IDE */
#define ISA_IRQ_IDE_SECONDARY	15	/* Secondary IDE */

/*
 * 8259 PIC (Programmable Interrupt Controller) ports
 */
#define PIC_MASTER_CMD		0x20
#define PIC_MASTER_DATA		0x21
#define PIC_SLAVE_CMD		0xA0
#define PIC_SLAVE_DATA		0xA1

/* PIC commands */
#define PIC_CMD_EOI		0x20	/* End of interrupt */
#define PIC_CMD_ICW1		0x11	/* Initialization command word 1 */

/*
 * DMA controller ports
 */
#define DMA1_STATUS		0x08
#define DMA1_CMD		0x08
#define DMA1_REQUEST		0x09
#define DMA1_MASK		0x0A
#define DMA1_MODE		0x0B
#define DMA1_CLEAR		0x0C
#define DMA1_TEMP		0x0D
#define DMA1_RESET		0x0D
#define DMA1_CLEAR_MASK		0x0E
#define DMA1_WRITE_MASK		0x0F

/*
 * Initialize ISA bus
 */
void
alpha_isa_init(void)
{
	printf("Initializing ISA bus...\n");

	/*
	 * Initialize 8259 PIC
	 */
	alpha_pic_init();

	/*
	 * Initialize DMA controller
	 */
	alpha_dma_init();

	/*
	 * Initialize RTC
	 */
	extern void alpha_rtc_init(void);
	alpha_rtc_init();

	printf("ISA bus initialization complete\n");
}

/*
 * Initialize 8259 Programmable Interrupt Controller
 */
void
alpha_pic_init(void)
{
	/* Initialize master PIC */
	outb(PIC_CMD_ICW1, PIC_MASTER_CMD);	/* ICW1: cascade mode */
	outb(0x20, PIC_MASTER_DATA);		/* ICW2: interrupt vector base */
	outb(0x04, PIC_MASTER_DATA);		/* ICW3: slave on IRQ2 */
	outb(0x01, PIC_MASTER_DATA);		/* ICW4: 8086 mode */

	/* Initialize slave PIC */
	outb(PIC_CMD_ICW1, PIC_SLAVE_CMD);	/* ICW1: cascade mode */
	outb(0x28, PIC_SLAVE_DATA);		/* ICW2: interrupt vector base */
	outb(0x02, PIC_SLAVE_DATA);		/* ICW3: slave ID */
	outb(0x01, PIC_SLAVE_DATA);		/* ICW4: 8086 mode */

	/* Mask all interrupts initially */
	outb(0xFF, PIC_MASTER_DATA);
	outb(0xFF, PIC_SLAVE_DATA);

	printf("  8259 PIC configured\n");
}

/*
 * Enable ISA IRQ
 */
void
alpha_isa_enable_irq(int irq)
{
	unsigned char mask;

	if (irq < 8) {
		/* Master PIC */
		mask = inb(PIC_MASTER_DATA);
		mask &= ~(1 << irq);
		outb(mask, PIC_MASTER_DATA);
	} else {
		/* Slave PIC */
		mask = inb(PIC_SLAVE_DATA);
		mask &= ~(1 << (irq - 8));
		outb(mask, PIC_SLAVE_DATA);

		/* Also enable cascade IRQ */
		mask = inb(PIC_MASTER_DATA);
		mask &= ~(1 << ISA_IRQ_CASCADE);
		outb(mask, PIC_MASTER_DATA);
	}
}

/*
 * Disable ISA IRQ
 */
void
alpha_isa_disable_irq(int irq)
{
	unsigned char mask;

	if (irq < 8) {
		/* Master PIC */
		mask = inb(PIC_MASTER_DATA);
		mask |= (1 << irq);
		outb(mask, PIC_MASTER_DATA);
	} else {
		/* Slave PIC */
		mask = inb(PIC_SLAVE_DATA);
		mask |= (1 << (irq - 8));
		outb(mask, PIC_SLAVE_DATA);
	}
}

/*
 * Send EOI (End Of Interrupt) to PIC
 */
void
alpha_isa_eoi(int irq)
{
	if (irq >= 8) {
		/* Send EOI to slave PIC */
		outb(PIC_CMD_EOI, PIC_SLAVE_CMD);
	}

	/* Always send EOI to master PIC */
	outb(PIC_CMD_EOI, PIC_MASTER_CMD);
}

/*
 * Initialize DMA controller
 */
void
alpha_dma_init(void)
{
	/* Reset DMA controller */
	outb(0, DMA1_RESET);

	/* Clear mask register */
	outb(0, DMA1_CLEAR_MASK);

	printf("  DMA controller configured\n");
}

/*
 * ISA interrupt handler
 */
void
alpha_isa_interrupt(int irq)
{
	/* Handle ISA interrupt */

	switch (irq) {
	case ISA_IRQ_TIMER:
		/* Handle timer interrupt */
		break;

	case ISA_IRQ_KEYBOARD:
		/* Handle keyboard interrupt */
		break;

	case ISA_IRQ_RTC:
		/* Handle RTC interrupt */
		break;

	default:
		/* Unknown interrupt */
		break;
	}

	/* Send EOI */
	alpha_isa_eoi(irq);
}

/*
 * Print ISA interrupt mask
 */
void
alpha_isa_print_irq_mask(void)
{
	unsigned char master_mask, slave_mask;

	master_mask = inb(PIC_MASTER_DATA);
	slave_mask = inb(PIC_SLAVE_DATA);

	printf("\nISA Interrupt Mask:\n");
	printf("  Master PIC (IRQ 0-7):  0x%02X\n", master_mask);
	printf("  Slave PIC (IRQ 8-15):  0x%02X\n", slave_mask);
}
