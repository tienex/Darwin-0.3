/*
 * x86-64 Interrupt Handling
 *
 * This file handles hardware interrupt dispatch and management
 * for x86-64 architecture.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <mach/machine.h>

/* Interrupt Request Lines */
#define INTR_NIRQ	256	/* Total IRQs (0-255) */
#define INTR_NIPL	16	/* Interrupt Priority Levels */

/* Hardware IRQs */
#define IRQ_TIMER	0	/* Programmable Interval Timer */
#define IRQ_KEYBOARD	1	/* Keyboard controller */
#define IRQ_CASCADE	2	/* Cascade from slave PIC */
#define IRQ_SERIAL2	3	/* Serial port 2 */
#define IRQ_SERIAL1	4	/* Serial port 1 */
#define IRQ_PARALLEL2	5	/* Parallel port 2 */
#define IRQ_FLOPPY	6	/* Floppy disk controller */
#define IRQ_PARALLEL1	7	/* Parallel port 1 */
#define IRQ_RTC		8	/* Real-time clock */
#define IRQ_MOUSE	12	/* PS/2 mouse */
#define IRQ_COPROCESSOR	13	/* Math coprocessor */
#define IRQ_ATA1	14	/* Primary ATA channel */
#define IRQ_ATA2	15	/* Secondary ATA channel */

/* Interrupt handler function pointer */
typedef void (*intr_handler_t)(int irq, void *arg);

/* Interrupt descriptor */
struct intr_dispatch {
	intr_handler_t	handler;
	void		*arg;
	int		irq;
	int		flags;
	const char	*name;
};

static struct intr_dispatch dispatch_table[INTR_NIRQ];
static unsigned long irq_mask = 0xFFFFFFFF;  /* All IRQs masked initially */

/*
 * Initialize interrupt subsystem
 */
void
intr_init(void)
{
	int i;

	/* Clear dispatch table */
	for (i = 0; i < INTR_NIRQ; i++) {
		dispatch_table[i].handler = NULL;
		dispatch_table[i].arg = NULL;
		dispatch_table[i].irq = i;
		dispatch_table[i].flags = 0;
		dispatch_table[i].name = NULL;
	}

	printf("Interrupt subsystem initialized (%d IRQs)\n", INTR_NIRQ);
}

/*
 * Register an interrupt handler
 */
int
intr_register(int irq, intr_handler_t handler, void *arg, const char *name)
{
	if (irq < 0 || irq >= INTR_NIRQ) {
		printf("intr_register: invalid IRQ %d\n", irq);
		return -1;
	}

	if (dispatch_table[irq].handler != NULL) {
		printf("intr_register: IRQ %d already registered\n", irq);
		return -1;
	}

	dispatch_table[irq].handler = handler;
	dispatch_table[irq].arg = arg;
	dispatch_table[irq].name = name;

	printf("Registered IRQ %d: %s\n", irq, name ? name : "unnamed");

	return 0;
}

/*
 * Unregister an interrupt handler
 */
void
intr_unregister(int irq)
{
	if (irq < 0 || irq >= INTR_NIRQ)
		return;

	dispatch_table[irq].handler = NULL;
	dispatch_table[irq].arg = NULL;
	dispatch_table[irq].name = NULL;
}

/*
 * Dispatch interrupt to registered handler
 */
void
intr_dispatch(int irq)
{
	struct intr_dispatch *intr;

	if (irq < 0 || irq >= INTR_NIRQ)
		return;

	intr = &dispatch_table[irq];

	if (intr->handler != NULL) {
		intr->handler(irq, intr->arg);
	} else {
		printf("Unhandled IRQ %d\n", irq);
	}

	/* Send EOI to interrupt controller */
	intr_eoi(irq);
}

/*
 * Send End-Of-Interrupt to APIC/PIC
 */
void
intr_eoi(int irq)
{
	/* For APIC: write to EOI register */
	/* For now, stub implementation */

	/* Local APIC EOI register at 0xFEE000B0 */
	/* Would write 0 to signal EOI */
}

/*
 * Enable interrupt
 */
void
intr_enable(int irq)
{
	if (irq < 0 || irq >= INTR_NIRQ)
		return;

	irq_mask &= ~(1UL << irq);
	/* Would program interrupt controller here */
}

/*
 * Disable interrupt
 */
void
intr_disable(int irq)
{
	if (irq < 0 || irq >= INTR_NIRQ)
		return;

	irq_mask |= (1UL << irq);
	/* Would program interrupt controller here */
}

/*
 * Enable all interrupts (STI)
 */
void
intr_enable_all(void)
{
	__asm__ volatile("sti");
}

/*
 * Disable all interrupts (CLI)
 */
void
intr_disable_all(void)
{
	__asm__ volatile("cli");
}

/*
 * Save interrupt state and disable
 */
unsigned long
intr_save(void)
{
	unsigned long flags;
	__asm__ volatile("pushfq; popq %0; cli" : "=r" (flags));
	return flags;
}

/*
 * Restore interrupt state
 */
void
intr_restore(unsigned long flags)
{
	__asm__ volatile("pushq %0; popfq" :: "r" (flags));
}

/*
 * Check if interrupts are enabled
 */
int
intr_enabled(void)
{
	unsigned long flags;
	__asm__ volatile("pushfq; popq %0" : "=r" (flags));
	return (flags & 0x200) != 0;  /* IF flag */
}

/*
 * Spurious interrupt handler
 */
void
intr_spurious(int irq)
{
	printf("Spurious interrupt on IRQ %d\n", irq);
}
