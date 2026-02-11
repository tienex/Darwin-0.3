/*
 * DLX Interrupt Controller
 *
 * Manages interrupt handling for DLX architecture.
 * DLX uses status register bits for interrupt masking.
 */

#include <mach/dlx/vm_types.h>
#include <mach/dlx/boolean.h>
#include "trap.h"
#include "cpu.h"

/* Interrupt statistics */
static unsigned long interrupt_counts[64];	/* Count per interrupt type */
static unsigned long total_interrupts = 0;

/* Interrupt handler table */
typedef void (*interrupt_handler_t)(void *arg);

struct interrupt_entry {
	interrupt_handler_t handler;
	void *arg;
	const char *name;
};

static struct interrupt_entry interrupt_table[64];

/*
 * Initialize interrupt controller
 */
void
interrupt_init(void)
{
	int i;

	/* Clear statistics */
	for (i = 0; i < 64; i++) {
		interrupt_counts[i] = 0;
		interrupt_table[i].handler = NULL;
		interrupt_table[i].arg = NULL;
		interrupt_table[i].name = NULL;
	}

	total_interrupts = 0;

	printf("Interrupt controller initialized\n");
}

/*
 * Register interrupt handler
 */
int
interrupt_register(int irq, interrupt_handler_t handler, void *arg, const char *name)
{
	if (irq < 0 || irq >= 64)
		return -1;

	if (interrupt_table[irq].handler != NULL)
		return -1;	/* Already registered */

	interrupt_table[irq].handler = handler;
	interrupt_table[irq].arg = arg;
	interrupt_table[irq].name = name;

	return 0;
}

/*
 * Unregister interrupt handler
 */
int
interrupt_unregister(int irq)
{
	if (irq < 0 || irq >= 64)
		return -1;

	interrupt_table[irq].handler = NULL;
	interrupt_table[irq].arg = NULL;
	interrupt_table[irq].name = NULL;

	return 0;
}

/*
 * Dispatch interrupt to registered handler
 * Returns 0 if handled, -1 if no handler
 */
int
interrupt_dispatch(int irq)
{
	if (irq < 0 || irq >= 64)
		return -1;

	/* Update statistics */
	interrupt_counts[irq]++;
	total_interrupts++;

	/* Call handler if registered */
	if (interrupt_table[irq].handler != NULL) {
		interrupt_table[irq].handler(interrupt_table[irq].arg);
		return 0;
	}

	/* No handler registered */
	printf("Unhandled interrupt: %d\n", irq);
	return -1;
}

/*
 * Enable interrupts globally
 */
void
interrupt_enable(void)
{
	unsigned int status;

	/* Read current status register */
	asm volatile("movs2i %0, r0" : "=r" (status));

	/* Clear interrupt mask (enable all interrupts) */
	status &= ~DLX_STATUS_INTRMASK;

	/* Write back status register */
	asm volatile("movi2s r0, %0" :: "r" (status));
}

/*
 * Disable interrupts globally
 */
void
interrupt_disable(void)
{
	unsigned int status;

	/* Read current status register */
	asm volatile("movs2i %0, r0" : "=r" (status));

	/* Set interrupt mask (disable all interrupts) */
	status |= DLX_STATUS_INTRMASK;

	/* Write back status register */
	asm volatile("movi2s r0, %0" :: "r" (status));
}

/*
 * Set interrupt priority level (SPL)
 * Returns previous level
 */
int
splx(int level)
{
	unsigned int status, old_level;

	/* Read current status register */
	asm volatile("movs2i %0, r0" : "=r" (status));

	/* Extract old interrupt mask */
	old_level = status & DLX_STATUS_INTRMASK;

	/* Set new interrupt mask */
	status = (status & ~DLX_STATUS_INTRMASK) | (level & DLX_STATUS_INTRMASK);

	/* Write back status register */
	asm volatile("movi2s r0, %0" :: "r" (status));

	return old_level;
}

/*
 * Raise priority to block all interrupts
 */
int
splhigh(void)
{
	return splx(SPLOFF);
}

/*
 * Lower priority to allow all interrupts
 */
int
spl0(void)
{
	return splx(SPL0);
}

/*
 * Get interrupt statistics
 */
void
interrupt_stats(int irq, unsigned long *count)
{
	if (irq < 0 || irq >= 64 || count == NULL)
		return;

	*count = interrupt_counts[irq];
}

/*
 * Print interrupt statistics
 */
void
interrupt_print_stats(void)
{
	int i;

	printf("\nInterrupt Statistics:\n");
	printf("Total interrupts: %lu\n", total_interrupts);
	printf("\nBy IRQ:\n");

	for (i = 0; i < 64; i++) {
		if (interrupt_counts[i] > 0) {
			printf("  IRQ %2d: %10lu", i, interrupt_counts[i]);
			if (interrupt_table[i].name)
				printf(" (%s)", interrupt_table[i].name);
			printf("\n");
		}
	}
}

/*
 * Check if interrupt is pending
 * In DLX, this would typically be determined by the trap mechanism
 */
int
interrupt_pending(int irq)
{
	/* DLX doesn't have a standard pending interrupt register
	 * Interrupts are delivered via traps
	 * Return 0 (not pending) for now
	 */
	return 0;
}

/*
 * Get current interrupt level (SPL)
 */
int
interrupt_get_level(void)
{
	unsigned int status;

	/* Read current status register */
	asm volatile("movs2i %0, r0" : "=r" (status));

	return status & DLX_STATUS_INTRMASK;
}
