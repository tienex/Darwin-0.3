/*
 * x86-64 Kernel Initialization
 *
 * This file contains the main initialization routine for x86-64
 * Called from start.s after basic CPU setup
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <mach/machine.h>
#include <mach/machine/vm_types.h>
#include <mach/machine/vm_param.h>

/* External functions */
extern void gdt_init(void);
extern void idt_init(void);
extern void pmap_bootstrap(vm_offset_t);
extern void machine_startup(void);

/* Boot parameters */
static unsigned long boot_arg0, boot_arg1, boot_arg2;

/*
 * Main x86-64 initialization routine
 * Called from start.s after basic CPU setup is complete
 *
 * At this point:
 * - CPU is in 64-bit long mode
 * - Basic page tables are set up
 * - Stack is initialized
 * - Interrupts are disabled
 */
void
x86_64_init(unsigned long arg0, unsigned long arg1, unsigned long arg2)
{
	/* Save boot arguments */
	boot_arg0 = arg0;
	boot_arg1 = arg1;
	boot_arg2 = arg2;

	/* Print startup message */
	printf("\nDarwin/x86-64 kernel initializing...\n");
	printf("Boot arguments: %lx %lx %lx\n", arg0, arg1, arg2);

	/* Initialize CPU features */
	cpu_init();

	/* Bootstrap the page mapping system */
	printf("Bootstrapping pmap...\n");
	pmap_bootstrap(0);

	/* Initialize the physical memory manager */
	printf("Initializing memory management...\n");

	/* Initialize machine-dependent subsystems */
	printf("Starting machine-dependent initialization...\n");
	machine_startup();

	/* Initialize I/O system */
	printf("Initializing I/O subsystem...\n");

	/* Print system information */
	print_system_info();

	/* Start the kernel proper */
	printf("Transferring control to kernel main...\n");
	kernel_bootstrap();

	/* Should never return */
	panic("x86_64_init: kernel_bootstrap returned!");
}

/*
 * Initialize CPU features
 */
void
cpu_init(void)
{
	unsigned int cpuid_result[4];
	unsigned long cr0, cr4;

	printf("CPU initialization...\n");

	/* Check for required CPU features */
	/* CPUID leaf 1: Feature Information */
	cpuid(1, 0, (unsigned long)cpuid_result);

	/* Check for SSE support (bit 25 of EDX) */
	if (cpuid_result[3] & (1 << 25)) {
		printf("  SSE supported\n");

		/* Enable SSE in CR4 */
		__asm__ volatile("movq %%cr4, %0" : "=r" (cr4));
		cr4 |= (1 << 9) | (1 << 10);  /* OSFXSR and OSXMMEXCPT */
		__asm__ volatile("movq %0, %%cr4" :: "r" (cr4));
	}

	/* Check for SSE2 support (bit 26 of EDX) */
	if (cpuid_result[3] & (1 << 26)) {
		printf("  SSE2 supported\n");
	}

	/* Enable write protection in CR0 */
	__asm__ volatile("movq %%cr0, %0" : "=r" (cr0));
	cr0 |= (1 << 16);  /* WP bit */
	__asm__ volatile("movq %0, %%cr0" :: "r" (cr0));

	/* Set up syscall/sysret MSRs */
	setup_syscall_msrs();

	printf("CPU initialization complete\n");
}

/*
 * Set up MSRs for syscall/sysret instructions
 */
void
setup_syscall_msrs(void)
{
	unsigned long star, lstar, sfmask;

	/* STAR MSR (0xC0000081): CS/SS selectors for syscall/sysret */
	/* Bits 63-48: User CS/SS base (sysret)
	 * Bits 47-32: Kernel CS/SS base (syscall)
	 */
	star = ((0x1BUL << 48) | (0x08UL << 32));
	wrmsr64(0xC0000081, star);

	/* LSTAR MSR (0xC0000082): syscall entry point */
	extern void syscall_entry(void);
	lstar = (unsigned long)syscall_entry;
	wrmsr64(0xC0000082, lstar);

	/* SFMASK MSR (0xC0000084): RFLAGS mask for syscall */
	/* Clear IF (interrupts) during syscall entry */
	sfmask = 0x200;  /* IF flag */
	wrmsr64(0xC0000084, sfmask);

	/* Enable syscall/sysret in EFER MSR */
	unsigned long efer = rdmsr64(0xC0000080);
	efer |= (1 << 0);  /* SCE bit (System Call Extensions) */
	wrmsr64(0xC0000080, efer);

	printf("Syscall MSRs configured\n");
}

/*
 * Print system information
 */
void
print_system_info(void)
{
	unsigned int cpuid_result[4];
	char vendor[13];

	printf("\nSystem Information:\n");

	/* Get CPU vendor string */
	cpuid(0, 0, (unsigned long)cpuid_result);
	*(unsigned int *)(vendor + 0) = cpuid_result[1];  /* EBX */
	*(unsigned int *)(vendor + 4) = cpuid_result[3];  /* EDX */
	*(unsigned int *)(vendor + 8) = cpuid_result[2];  /* ECX */
	vendor[12] = '\0';
	printf("  CPU Vendor: %s\n", vendor);

	/* Get CPU features */
	cpuid(1, 0, (unsigned long)cpuid_result);
	unsigned int family = ((cpuid_result[0] >> 8) & 0xF) + ((cpuid_result[0] >> 20) & 0xFF);
	unsigned int model = ((cpuid_result[0] >> 4) & 0xF) | ((cpuid_result[0] >> 12) & 0xF0);
	unsigned int stepping = cpuid_result[0] & 0xF;
	printf("  CPU Family: %u, Model: %u, Stepping: %u\n", family, model, stepping);

	printf("  Page Size: %d bytes\n", X86_64_PGBYTES);
	printf("  Kernel Stack Size: %d bytes\n", KERNSTACK_SIZE);

	printf("\n");
}

/*
 * Kernel bootstrap - hand off to main kernel
 */
void
kernel_bootstrap(void)
{
	/* This would call the main kernel initialization */
	printf("Kernel bootstrap - ready to start scheduler\n");

	/* Enable interrupts */
	__asm__ volatile("sti");

	/* Idle loop for now */
	for (;;) {
		__asm__ volatile("hlt");
	}
}
