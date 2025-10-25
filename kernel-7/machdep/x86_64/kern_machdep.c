/*
 * x86-64 Kernel Machine-Dependent Code
 *
 * This file contains miscellaneous kernel machine-dependent utilities
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <mach/machine.h>
#include <mach/machine/vm_types.h>

/*
 * External functions
 */
extern unsigned long rdtsc(void);
extern void cpuid(unsigned long leaf, unsigned long subleaf, unsigned long *result);

/*
 * Reboot the system
 */
void
boot(int howto)
{
	printf("Rebooting system (howto=0x%x)...\n", howto);

	/* Disable interrupts */
	__asm__ volatile("cli");

	/* Method 1: Triple fault (most reliable) */
	/* Load invalid IDT and trigger interrupt */
	struct {
		unsigned short limit;
		unsigned long base;
	} __attribute__((packed)) null_idt = { 0, 0 };

	__asm__ volatile("lidt %0" :: "m" (null_idt));
	__asm__ volatile("int $3");  /* Trigger breakpoint */

	/* Method 2: Keyboard controller reset */
	outb(0x64, 0xFE);

	/* Method 3: ACPI reset (if available) */
	/* Would write to ACPI reset register */

	/* Halt if all else fails */
	for (;;)
		__asm__ volatile("hlt");
}

/*
 * Halt the system
 */
void
halt(void)
{
	printf("System halted.\n");

	__asm__ volatile("cli");
	for (;;)
		__asm__ volatile("hlt");
}

/*
 * Panic - halt on fatal error
 */
void
panic(const char *fmt, ...)
{
	va_list args;

	printf("\nKernel panic: ");
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	printf("\n");

	/* Print stack trace if available */
	/* backtrace(); */

	/* Halt system */
	__asm__ volatile("cli");
	for (;;)
		__asm__ volatile("hlt");
}

/*
 * Get CPU information
 */
void
cpu_info(void)
{
	unsigned int cpuid_result[4];
	char vendor[13];

	/* Get CPU vendor */
	cpuid(0, 0, (unsigned long)cpuid_result);
	*(unsigned int *)(vendor + 0) = cpuid_result[1];
	*(unsigned int *)(vendor + 4) = cpuid_result[3];
	*(unsigned int *)(vendor + 8) = cpuid_result[2];
	vendor[12] = '\0';

	printf("CPU: %s\n", vendor);

	/* Get features */
	cpuid(1, 0, (unsigned long)cpuid_result);
	printf("  Family %u, Model %u, Stepping %u\n",
	       ((cpuid_result[0] >> 8) & 0xF) + ((cpuid_result[0] >> 20) & 0xFF),
	       ((cpuid_result[0] >> 4) & 0xF) | ((cpuid_result[0] >> 12) & 0xF0),
	       cpuid_result[0] & 0xF);

	/* Feature flags */
	if (cpuid_result[3] & (1 << 0))
		printf("  FPU ");
	if (cpuid_result[3] & (1 << 4))
		printf("TSC ");
	if (cpuid_result[3] & (1 << 5))
		printf("MSR ");
	if (cpuid_result[3] & (1 << 6))
		printf("PAE ");
	if (cpuid_result[3] & (1 << 9))
		printf("APIC ");
	if (cpuid_result[3] & (1 << 13))
		printf("PGE ");
	if (cpuid_result[3] & (1 << 25))
		printf("SSE ");
	if (cpuid_result[3] & (1 << 26))
		printf("SSE2 ");
	if (cpuid_result[2] & (1 << 0))
		printf("SSE3 ");
	if (cpuid_result[2] & (1 << 9))
		printf("SSSE3 ");
	if (cpuid_result[2] & (1 << 19))
		printf("SSE4.1 ");
	if (cpuid_result[2] & (1 << 20))
		printf("SSE4.2 ");
	printf("\n");
}

/*
 * Identify CPU type
 */
cpu_type_t
cpu_type(void)
{
	return CPU_TYPE_X86_64;
}

/*
 * Identify CPU subtype
 */
cpu_subtype_t
cpu_subtype(void)
{
	return CPU_SUBTYPE_X86_64_ALL;
}

/*
 * Get CPU family
 */
int
cpu_family(void)
{
	unsigned int cpuid_result[4];

	cpuid(1, 0, (unsigned long)cpuid_result);
	return ((cpuid_result[0] >> 8) & 0xF) + ((cpuid_result[0] >> 20) & 0xFF);
}

/*
 * Get CPU model
 */
int
cpu_model(void)
{
	unsigned int cpuid_result[4];

	cpuid(1, 0, (unsigned long)cpuid_result);
	return ((cpuid_result[0] >> 4) & 0xF) | ((cpuid_result[0] >> 12) & 0xF0);
}

/*
 * Get CPU stepping
 */
int
cpu_stepping(void)
{
	unsigned int cpuid_result[4];

	cpuid(1, 0, (unsigned long)cpuid_result);
	return cpuid_result[0] & 0xF;
}

/*
 * Check if CPU feature is supported
 */
int
cpu_has_feature(unsigned int feature)
{
	unsigned int cpuid_result[4];

	cpuid(1, 0, (unsigned long)cpuid_result);

	/* EDX features (bits 0-31) */
	if (feature < 32)
		return (cpuid_result[3] & (1 << feature)) != 0;

	/* ECX features (bits 32-63) */
	else if (feature < 64)
		return (cpuid_result[2] & (1 << (feature - 32))) != 0;

	return 0;
}

/* CPU feature flags */
#define CPU_FEATURE_FPU		0
#define CPU_FEATURE_TSC		4
#define CPU_FEATURE_MSR		5
#define CPU_FEATURE_PAE		6
#define CPU_FEATURE_APIC	9
#define CPU_FEATURE_SEP		11	/* SYSENTER/SYSEXIT */
#define CPU_FEATURE_PGE		13
#define CPU_FEATURE_CMOV	15
#define CPU_FEATURE_MMX		23
#define CPU_FEATURE_FXSR	24
#define CPU_FEATURE_SSE		25
#define CPU_FEATURE_SSE2	26
#define CPU_FEATURE_SSE3	32
#define CPU_FEATURE_SSSE3	41
#define CPU_FEATURE_SSE41	51
#define CPU_FEATURE_SSE42	52

/*
 * Initialize CPU features
 */
void
cpu_feature_init(void)
{
	/* Enable features as needed */
	if (cpu_has_feature(CPU_FEATURE_SSE)) {
		/* SSE is enabled in x86_64_init.c */
	}
}

/*
 * Get physical memory size
 */
unsigned long
mem_size(void)
{
	/* Would read from bootloader or BIOS/UEFI */
	/* Stub: return 256MB */
	return 256 * 1024 * 1024;
}

/*
 * Print memory map
 */
void
print_memory_map(void)
{
	printf("Memory Map:\n");
	printf("  Physical memory: %lu MB\n", mem_size() / (1024 * 1024));
	printf("  Kernel space: 0x%lx - 0x%lx\n",
	       VM_MIN_KERNEL_ADDRESS, VM_MAX_KERNEL_ADDRESS);
	printf("  User space: 0x%lx - 0x%lx\n",
	       VM_MIN_ADDRESS, VM_MAX_ADDRESS);
}

/*
 * Debug: print register state
 */
void
print_regs(void *regs_ptr)
{
	/* Would print all CPU registers */
	printf("Register dump:\n");
	/* ... */
}

/*
 * Performance counter support
 */
unsigned long
read_pmc(int counter)
{
	unsigned long low, high;

	__asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));
	return (high << 32) | low;
}

/*
 * Serialize instruction execution
 */
void
serialize(void)
{
	unsigned int cpuid_result[4];

	/* CPUID serializes execution */
	cpuid(0, 0, (unsigned long)cpuid_result);
}

/*
 * Memory fence
 */
void
memory_fence(void)
{
	__asm__ volatile("mfence" ::: "memory");
}

/*
 * Compiler barrier
 */
void
barrier(void)
{
	__asm__ volatile("" ::: "memory");
}
