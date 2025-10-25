/*
 * x86-64 Global Descriptor Table (GDT) Management
 *
 * In x86-64 long mode, segmentation is largely disabled, but we still
 * need a minimal GDT for:
 * - Code segment descriptors (kernel and user)
 * - Data segment descriptors (kernel and user)
 * - Task State Segment (TSS) descriptor
 *
 * Segment limits and bases are ignored in 64-bit mode (except FS/GS).
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <mach/machine.h>

/*
 * GDT entry structure (8 bytes for code/data, 16 bytes for system segments)
 */
struct gdt_entry {
	unsigned short	limit_low;
	unsigned short	base_low;
	unsigned char	base_mid;
	unsigned char	access;
	unsigned char	granularity;
	unsigned char	base_high;
} __attribute__((packed));

/*
 * System segment descriptor (TSS, LDT) in 64-bit mode is 16 bytes
 */
struct gdt_entry_sys {
	unsigned short	limit_low;
	unsigned short	base_low;
	unsigned char	base_mid;
	unsigned char	access;
	unsigned char	granularity;
	unsigned char	base_high;
	unsigned int	base_upper;
	unsigned int	reserved;
} __attribute__((packed));

/*
 * GDT structure
 * Entry 0: Null descriptor (required)
 * Entry 1: Kernel code segment (0x08)
 * Entry 2: Kernel data segment (0x10)
 * Entry 3: User code segment (0x18)
 * Entry 4: User data segment (0x20)
 * Entry 5-6: TSS descriptor (16 bytes, takes 2 entries)
 */
#define GDT_ENTRIES 8
struct gdt_entry gdt[GDT_ENTRIES] __attribute__((aligned(16)));

/*
 * TSS (Task State Segment) structure for x86-64
 * Used primarily for interrupt stack switching
 */
struct tss_64 {
	unsigned int	reserved0;
	unsigned long	rsp0;		/* Stack pointer for ring 0 */
	unsigned long	rsp1;		/* Stack pointer for ring 1 */
	unsigned long	rsp2;		/* Stack pointer for ring 2 */
	unsigned long	reserved1;
	unsigned long	ist[7];		/* Interrupt Stack Table */
	unsigned long	reserved2;
	unsigned short	reserved3;
	unsigned short	iomap_base;
} __attribute__((packed));

struct tss_64 tss __attribute__((aligned(16)));

/*
 * Set a GDT entry
 */
static void
gdt_set_entry(int num, unsigned long base, unsigned long limit,
              unsigned char access, unsigned char gran)
{
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_mid = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
	gdt[num].access = access;
}

/*
 * Set a system segment descriptor (TSS)
 */
static void
gdt_set_sys_entry(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran)
{
	struct gdt_entry_sys *sys = (struct gdt_entry_sys *)&gdt[num];

	sys->base_low = (base & 0xFFFF);
	sys->base_mid = (base >> 16) & 0xFF;
	sys->base_high = (base >> 24) & 0xFF;
	sys->base_upper = (base >> 32);

	sys->limit_low = (limit & 0xFFFF);
	sys->granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
	sys->access = access;
	sys->reserved = 0;
}

/*
 * Initialize the GDT
 */
void
gdt_init(void)
{
	/* Clear GDT */
	bzero(gdt, sizeof(gdt));

	/* Entry 0: Null descriptor */
	gdt_set_entry(0, 0, 0, 0, 0);

	/* Entry 1: Kernel code segment (0x08)
	 * Base = 0, Limit = 0 (ignored in 64-bit mode)
	 * Access = 0x9A (Present, Ring 0, Code, Executable, Readable)
	 * Gran = 0xA0 (64-bit, not 32-bit)
	 */
	gdt_set_entry(1, 0, 0, 0x9A, 0xA0);

	/* Entry 2: Kernel data segment (0x10)
	 * Base = 0, Limit = 0 (ignored in 64-bit mode)
	 * Access = 0x92 (Present, Ring 0, Data, Writable)
	 * Gran = 0xC0 (4KB granularity)
	 */
	gdt_set_entry(2, 0, 0, 0x92, 0xC0);

	/* Entry 3: User code segment (0x18 + 3 = 0x1B)
	 * Access = 0xFA (Present, Ring 3, Code, Executable, Readable)
	 * Gran = 0xA0 (64-bit)
	 */
	gdt_set_entry(3, 0, 0, 0xFA, 0xA0);

	/* Entry 4: User data segment (0x20 + 3 = 0x23)
	 * Access = 0xF2 (Present, Ring 3, Data, Writable)
	 * Gran = 0xC0 (4KB granularity)
	 */
	gdt_set_entry(4, 0, 0, 0xF2, 0xC0);

	/* Initialize TSS */
	bzero(&tss, sizeof(tss));
	tss.iomap_base = sizeof(tss);

	/* Entry 5-6: TSS descriptor (16 bytes)
	 * Access = 0x89 (Present, Ring 0, 64-bit TSS Available)
	 * Gran = 0x00
	 */
	gdt_set_sys_entry(5, (unsigned long)&tss, sizeof(tss) - 1, 0x89, 0x00);

	printf("x86-64 GDT initialized at %p\n", gdt);
}

/*
 * Set TSS stack pointer for ring 0
 */
void
tss_set_rsp0(unsigned long rsp0)
{
	tss.rsp0 = rsp0;
}

/*
 * Get GDT base and limit (called from assembly)
 */
extern unsigned short _gdt_limit;
extern unsigned long _gdt_base;

unsigned short _gdt_limit = sizeof(gdt) - 1;
unsigned long _gdt_base = (unsigned long)gdt;
