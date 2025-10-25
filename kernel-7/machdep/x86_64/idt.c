/*
 * x86-64 Interrupt Descriptor Table (IDT) Management
 *
 * The IDT contains descriptors for all interrupt and exception handlers.
 * In x86-64, IDT entries are 16 bytes each (vs 8 bytes in 32-bit mode).
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <mach/machine.h>

/*
 * IDT entry structure (16 bytes in 64-bit mode)
 */
struct idt_entry {
	unsigned short	offset_low;	/* Offset bits 0-15 */
	unsigned short	selector;	/* Code segment selector */
	unsigned char	ist;		/* Interrupt Stack Table offset */
	unsigned char	type_attr;	/* Type and attributes */
	unsigned short	offset_mid;	/* Offset bits 16-31 */
	unsigned int	offset_high;	/* Offset bits 32-63 */
	unsigned int	reserved;	/* Reserved, must be 0 */
} __attribute__((packed));

/*
 * IDT with 256 entries (0-255)
 * 0-31: CPU exceptions
 * 32-255: Interrupts (hardware, software, system calls)
 */
#define IDT_ENTRIES 256
struct idt_entry idt[IDT_ENTRIES] __attribute__((aligned(16)));

/*
 * Exception/interrupt handler function pointers (from locore.s)
 */
extern void divide_error(void);
extern void debug_trap(void);
extern void nmi(void);
extern void breakpoint(void);
extern void overflow(void);
extern void bounds_check(void);
extern void invalid_opcode(void);
extern void device_not_available(void);
extern void double_fault(void);
extern void invalid_tss(void);
extern void segment_not_present(void);
extern void stack_fault(void);
extern void general_protection(void);
extern void page_fault(void);
extern void fpu_error(void);
extern void alignment_check(void);
extern void machine_check(void);
extern void simd_error(void);
extern void syscall_entry(void);

/*
 * Gate types
 */
#define IDT_TYPE_INTERRUPT	0x8E	/* Interrupt gate: DPL=0, Present */
#define IDT_TYPE_TRAP		0x8F	/* Trap gate: DPL=0, Present */
#define IDT_TYPE_SYSCALL	0xEE	/* Interrupt gate: DPL=3, Present */

/*
 * Set an IDT entry
 *
 * num: IDT entry number (0-255)
 * offset: Address of handler function
 * selector: Code segment selector (typically 0x08 for kernel code)
 * type_attr: Gate type and attributes
 * ist: Interrupt Stack Table index (0 = don't use IST)
 */
static void
idt_set_gate(int num, unsigned long offset, unsigned short selector,
             unsigned char type_attr, unsigned char ist)
{
	idt[num].offset_low = offset & 0xFFFF;
	idt[num].offset_mid = (offset >> 16) & 0xFFFF;
	idt[num].offset_high = (offset >> 32) & 0xFFFFFFFF;

	idt[num].selector = selector;
	idt[num].ist = ist & 0x07;
	idt[num].type_attr = type_attr;
	idt[num].reserved = 0;
}

/*
 * Initialize the IDT
 */
void
idt_init(void)
{
	int i;

	/* Clear IDT */
	bzero(idt, sizeof(idt));

	/* Set up exception handlers (0-19) */
	idt_set_gate(0, (unsigned long)divide_error, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(1, (unsigned long)debug_trap, 0x08, IDT_TYPE_TRAP, 0);
	idt_set_gate(2, (unsigned long)nmi, 0x08, IDT_TYPE_INTERRUPT, 1); /* Use IST1 for NMI */
	idt_set_gate(3, (unsigned long)breakpoint, 0x08, IDT_TYPE_TRAP, 0);
	idt_set_gate(4, (unsigned long)overflow, 0x08, IDT_TYPE_TRAP, 0);
	idt_set_gate(5, (unsigned long)bounds_check, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(6, (unsigned long)invalid_opcode, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(7, (unsigned long)device_not_available, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(8, (unsigned long)double_fault, 0x08, IDT_TYPE_INTERRUPT, 2); /* Use IST2 for double fault */
	/* 9: Coprocessor segment overrun (legacy, not used) */
	idt_set_gate(10, (unsigned long)invalid_tss, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(11, (unsigned long)segment_not_present, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(12, (unsigned long)stack_fault, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(13, (unsigned long)general_protection, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(14, (unsigned long)page_fault, 0x08, IDT_TYPE_INTERRUPT, 0);
	/* 15: Reserved */
	idt_set_gate(16, (unsigned long)fpu_error, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(17, (unsigned long)alignment_check, 0x08, IDT_TYPE_INTERRUPT, 0);
	idt_set_gate(18, (unsigned long)machine_check, 0x08, IDT_TYPE_INTERRUPT, 3); /* Use IST3 for MCE */
	idt_set_gate(19, (unsigned long)simd_error, 0x08, IDT_TYPE_INTERRUPT, 0);

	/* Entries 20-31: Reserved for future CPU exceptions */

	/* Entry 32-255: Available for hardware/software interrupts */
	/* These would be set up by device drivers */

	/* Example: System call gate at vector 0x80 (like Linux/BSD) */
	/* idt_set_gate(0x80, (unsigned long)syscall_entry, 0x08, IDT_TYPE_SYSCALL, 0); */

	printf("x86-64 IDT initialized at %p (%d entries)\n", idt, IDT_ENTRIES);
}

/*
 * Install a custom interrupt handler
 * Used by device drivers to register interrupt handlers
 */
void
idt_install_handler(int vector, void (*handler)(void), int ist)
{
	if (vector < 0 || vector >= IDT_ENTRIES) {
		printf("idt_install_handler: invalid vector %d\n", vector);
		return;
	}

	idt_set_gate(vector, (unsigned long)handler, 0x08, IDT_TYPE_INTERRUPT, ist);
}

/*
 * Get IDT base and limit (called from assembly)
 */
extern unsigned short _idt_limit;
extern unsigned long _idt_base;

unsigned short _idt_limit = sizeof(idt) - 1;
unsigned long _idt_base = (unsigned long)idt;
