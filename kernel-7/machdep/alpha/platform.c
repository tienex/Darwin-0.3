/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Platform Support - AlphaStation 500/600
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/pal.h>
#include "platform.h"

struct alpha_platform current_platform;

/*
 * Platform database
 */
static const struct alpha_platform platforms[] = {
	/* AlphaStation 500 (Maverick) */
	{
		.type = ALPHA_PLATFORM_ALPHASTATION_500,
		.name = "AlphaStation 500",
		.model = "Maverick",
		.chipset = ALPHA_CHIPSET_CIA,
		.cpu_type = ALPHA_IMPL_EV5,
		.memory_base = AS500_MEMORY_BASE,
		.memory_size = 0,  /* Detected from HWRPB */
		.io_base = AS500_PCI_IO_BASE,
		.pci_config_base = AS500_PCI_CONFIG_BASE,
		.pci_mem_base = AS500_PCI_MEM_BASE,
		.pci_io_base = AS500_PCI_IO_BASE,
		.num_pci_buses = 1,
		.isa_present = 1,
		.rtc_port = AS500_RTC_PORT,
	},

	/* AlphaStation 600 (Alcor) */
	{
		.type = ALPHA_PLATFORM_ALPHASTATION_600,
		.name = "AlphaStation 600",
		.model = "Alcor",
		.chipset = ALPHA_CHIPSET_CIA,
		.cpu_type = ALPHA_IMPL_EV5,
		.memory_base = AS600_MEMORY_BASE,
		.memory_size = 0,  /* Detected from HWRPB */
		.io_base = AS600_PCI_IO_BASE,
		.pci_config_base = AS600_PCI_CONFIG_BASE,
		.pci_mem_base = AS600_PCI_MEM_BASE,
		.pci_io_base = AS600_PCI_IO_BASE,
		.num_pci_buses = 1,
		.isa_present = 1,
		.rtc_port = AS600_RTC_PORT,
	},

	/* Sentinel */
	{ .type = ALPHA_PLATFORM_UNKNOWN }
};

/*
 * Detect platform from HWRPB
 */
int
alpha_platform_detect(void)
{
	extern struct hwrpb *alpha_get_hwrpb(void);
	struct hwrpb *hwrpb = alpha_get_hwrpb();
	unsigned long systype;
	int i;

	if (hwrpb == NULL) {
		printf("Warning: No HWRPB available, cannot detect platform\n");
		current_platform.type = ALPHA_PLATFORM_UNKNOWN;
		return -1;
	}

	/*
	 * System type is in the HWRPB
	 * Note: In real HWRPB, this would be in a specific field
	 * For now, we'll use a simplified detection
	 */
	systype = 20;  /* Default to AlphaStation 500 for development */

	/*
	 * Match system type to platform
	 */
	for (i = 0; platforms[i].type != ALPHA_PLATFORM_UNKNOWN; i++) {
		if ((systype == HWRPB_SYSTYPE_ALPHASTATION_500 &&
		     platforms[i].type == ALPHA_PLATFORM_ALPHASTATION_500) ||
		    (systype == HWRPB_SYSTYPE_ALPHASTATION_600 &&
		     platforms[i].type == ALPHA_PLATFORM_ALPHASTATION_600)) {
			current_platform = platforms[i];
			return 0;
		}
	}

	/* Default to AlphaStation 500 if not detected */
	current_platform = platforms[0];
	return 0;
}

/*
 * Initialize platform-specific hardware
 */
void
alpha_platform_init(void)
{
	printf("Platform: %s (%s)\n", current_platform.name, current_platform.model);

	/*
	 * Initialize chipset
	 */
	switch (current_platform.chipset) {
	case ALPHA_CHIPSET_CIA:
		alpha_cia_init();
		break;

	default:
		printf("Warning: Unknown chipset %d\n", current_platform.chipset);
		break;
	}

	/*
	 * Initialize PCI bus
	 */
	if (current_platform.num_pci_buses > 0) {
		alpha_pci_init();
	}

	/*
	 * Initialize ISA bus
	 */
	if (current_platform.isa_present) {
		alpha_isa_init();
	}
}

/*
 * Get platform name
 */
const char *
alpha_platform_name(void)
{
	return current_platform.name;
}

/*
 * Platform power management
 */

void
platform_poweroff(void)
{
	printf("Powering off...\n");

	/*
	 * Platform-specific poweroff sequence
	 * Typically involves ACPI or chipset-specific registers
	 */

	/* Use PALcode halt as fallback */
	extern void pal_unix_halt(void);
	pal_unix_halt();
}

void
platform_reboot(void)
{
	printf("Rebooting...\n");

	/*
	 * Reboot via keyboard controller (classic PC method)
	 */
	outb(0xFE, 0x64);

	/* If that fails, halt */
	extern void pal_unix_halt(void);
	pal_unix_halt();
}

void
platform_halt(void)
{
	printf("System halted.\n");

	extern void pal_unix_halt(void);
	pal_unix_halt();
}

/*
 * I/O port access
 *
 * Alpha uses memory-mapped I/O, with a specific address range
 * for ISA I/O port emulation.
 */

static inline unsigned long
port_to_addr(unsigned short port)
{
	return current_platform.pci_io_base + port;
}

unsigned char
inb(unsigned short port)
{
	unsigned long addr = port_to_addr(port);
	unsigned char val;

	__asm__ volatile ("ldbu %0, 0(%1)" : "=r" (val) : "r" (addr));
	alpha_mb();

	return val;
}

void
outb(unsigned char value, unsigned short port)
{
	unsigned long addr = port_to_addr(port);

	alpha_mb();
	__asm__ volatile ("stb %1, 0(%0)" : : "r" (addr), "r" (value) : "memory");
	alpha_mb();
}

unsigned short
inw(unsigned short port)
{
	unsigned long addr = port_to_addr(port);
	unsigned short val;

	__asm__ volatile ("ldwu %0, 0(%1)" : "=r" (val) : "r" (addr));
	alpha_mb();

	return val;
}

void
outw(unsigned short value, unsigned short port)
{
	unsigned long addr = port_to_addr(port);

	alpha_mb();
	__asm__ volatile ("stw %1, 0(%0)" : : "r" (addr), "r" (value) : "memory");
	alpha_mb();
}

unsigned int
inl(unsigned short port)
{
	unsigned long addr = port_to_addr(port);
	unsigned int val;

	__asm__ volatile ("ldl %0, 0(%1)" : "=r" (val) : "r" (addr));
	alpha_mb();

	return val;
}

void
outl(unsigned int value, unsigned short port)
{
	unsigned long addr = port_to_addr(port);

	alpha_mb();
	__asm__ volatile ("stl %1, 0(%0)" : : "r" (addr), "r" (value) : "memory");
	alpha_mb();
}

/*
 * PCI configuration space access
 */

unsigned int
pci_config_read(int bus, int dev, int func, int reg)
{
	unsigned long addr;
	unsigned int val;

	/*
	 * PCI configuration address format:
	 * Bits 0-1:   Register alignment (0)
	 * Bits 2-7:   Register number
	 * Bits 8-10:  Function number
	 * Bits 11-15: Device number
	 * Bits 16-23: Bus number
	 */
	addr = current_platform.pci_config_base |
	       (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xFC);

	__asm__ volatile ("ldl %0, 0(%1)" : "=r" (val) : "r" (addr));
	alpha_mb();

	return val;
}

void
pci_config_write(int bus, int dev, int func, int reg, unsigned int value)
{
	unsigned long addr;

	addr = current_platform.pci_config_base |
	       (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xFC);

	alpha_mb();
	__asm__ volatile ("stl %1, 0(%0)" : : "r" (addr), "r" (value) : "memory");
	alpha_mb();
}

/*
 * Stubs for external functions
 */
void alpha_cia_init(void) { /* Implemented in cia.c */ }
void alpha_pci_init(void) { /* Implemented in pci.c */ }
void alpha_isa_init(void) { /* Implemented in isa.c */ }
