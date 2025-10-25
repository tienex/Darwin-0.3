/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * ISA Bus Support for AlphaStation
 */

#ifndef _MACHDEP_ALPHA_ISA_H_
#define _MACHDEP_ALPHA_ISA_H_

/* ISA IRQ numbers */
#define ISA_IRQ_TIMER       0   /* System timer (i8254 PIT) */
#define ISA_IRQ_KEYBOARD    1   /* Keyboard controller */
#define ISA_IRQ_CASCADE     2   /* Cascade from slave PIC */
#define ISA_IRQ_SERIAL2     3   /* COM2 */
#define ISA_IRQ_SERIAL1     4   /* COM1 */
#define ISA_IRQ_PARALLEL    7   /* Parallel port */
#define ISA_IRQ_RTC         8   /* Real-time clock */
#define ISA_IRQ_MOUSE       12  /* PS/2 mouse */
#define ISA_IRQ_FPU         13  /* FPU (not used on Alpha) */
#define ISA_IRQ_IDE_PRIMARY 14  /* Primary IDE */
#define ISA_IRQ_IDE_SECONDARY 15 /* Secondary IDE */

/* Function prototypes */
void alpha_isa_init(void);
void alpha_pic_init(void);
void alpha_dma_init(void);
void alpha_isa_enable_irq(int irq);
void alpha_isa_disable_irq(int irq);
void alpha_isa_eoi(int irq);
void alpha_isa_interrupt_handler(void);
void alpha_isa_interrupt(int irq);
void alpha_isa_print_irq_mask(void);

#endif /* _MACHDEP_ALPHA_ISA_H_ */
