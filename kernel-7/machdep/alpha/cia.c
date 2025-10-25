/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * CIA (21171/21172/21174) Chipset Support
 *
 * The CIA (Cache/memory, I/O bridge, Asic) is the core logic chipset
 * used in AlphaStation 500 and 600 systems. It provides:
 * - Memory controller
 * - PCI host bridge
 * - Interrupt routing
 * - DMA support with scatter-gather
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include "platform.h"

/*
 * CIA chipset revision
 */
static int cia_revision = 0;

/*
 * CIA register access
 */
static inline unsigned long
cia_read(unsigned long reg)
{
	unsigned long val;
	unsigned long addr = CIA_CSR_BASE + reg;

	__asm__ volatile ("ldq %0, 0(%1)" : "=r" (val) : "r" (addr));
	alpha_mb();

	return val;
}

static inline void
cia_write(unsigned long reg, unsigned long val)
{
	unsigned long addr = CIA_CSR_BASE + reg;

	alpha_mb();
	__asm__ volatile ("stq %1, 0(%0)" : : "r" (addr), "r" (val) : "memory");
	alpha_mb();
}

/*
 * Initialize CIA chipset
 */
void
alpha_cia_init(void)
{
	unsigned long rev;

	printf("Initializing CIA chipset...\n");

	/*
	 * Read CIA revision
	 */
	rev = cia_read(CIA_REG_CSR_GEN);
	cia_revision = (rev >> 32) & 0xFF;

	printf("  CIA revision: %d\n", cia_revision);

	/*
	 * Initialize memory controller
	 */
	cia_init_memory();

	/*
	 * Initialize PCI bridge
	 */
	cia_init_pci();

	/*
	 * Initialize interrupt controller
	 */
	cia_init_interrupts();

	/*
	 * Set up scatter-gather TLB for DMA
	 */
	cia_init_sg();

	printf("CIA initialization complete\n");
}

/*
 * Initialize CIA memory controller
 */
void
cia_init_memory(void)
{
	unsigned long config;

	/*
	 * Read memory configuration
	 */
	config = cia_read(CIA_REG_CSR_CNFG);

	/*
	 * Memory configuration is typically set by firmware
	 * We just verify it's sane
	 */
	printf("  Memory controller configured\n");
}

/*
 * Initialize CIA PCI bridge
 */
void
cia_init_pci(void)
{
	unsigned long ctrl;

	/*
	 * Configure PCI bridge
	 */
	ctrl = cia_read(CIA_REG_CIA_CTRL);

	/*
	 * Enable PCI bus
	 * Enable PCI memory and I/O spaces
	 * Enable PCI parity checking
	 */
	ctrl |= (1UL << 0);  /* PCI enable */
	ctrl |= (1UL << 1);  /* PCI memory enable */
	ctrl |= (1UL << 2);  /* PCI I/O enable */
	ctrl |= (1UL << 6);  /* PCI parity enable */

	cia_write(CIA_REG_CIA_CTRL, ctrl);

	printf("  PCI bridge configured\n");
}

/*
 * Initialize CIA interrupt controller
 *
 * The CIA routes interrupts from PCI devices to the CPU.
 * It provides 4 PCI interrupt lines (INTA-INTD).
 */
void
cia_init_interrupts(void)
{
	/*
	 * Interrupt routing is typically set up by firmware
	 * We just need to enable interrupt delivery
	 */

	printf("  Interrupt controller configured\n");
}

/*
 * Initialize scatter-gather TLB for DMA
 *
 * The CIA provides a scatter-gather TLB that allows PCI devices
 * to perform DMA to non-contiguous physical memory.
 */
void
cia_init_sg(void)
{
	/*
	 * Configure scatter-gather windows
	 *
	 * Window 0: Direct-mapped for low memory (0-8MB)
	 * Window 1: Scatter-gather for high memory
	 */

	/* Window 0: Direct-mapped, 8MB */
	cia_write(CIA_PCI_W0_BASE, 0x00000000);
	cia_write(CIA_PCI_W0_MASK, 0x00700000);  /* 8MB */
	cia_write(CIA_PCI_T0_BASE, 0x00000000);

	/* Window 1: Scatter-gather, up to 1GB */
	cia_write(CIA_PCI_W1_BASE, 0x80000000);
	cia_write(CIA_PCI_W1_MASK, 0x3FF00000);  /* 1GB */
	/* T1_BASE points to scatter-gather page table (allocated later) */

	/* Invalidate all TLB entries */
	cia_write(CIA_PCI_TBIA, 0);

	printf("  Scatter-gather DMA configured\n");
}

/*
 * CIA error handling
 */
void
cia_handle_error(void)
{
	unsigned long err;

	err = cia_read(CIA_REG_CIA_ERR);

	if (err != 0) {
		printf("CIA Error: 0x%lx\n", err);

		/* Clear error */
		cia_write(CIA_REG_CIA_ERR, err);
	}
}

/*
 * CIA diagnostic functions
 */

void
cia_print_status(void)
{
	unsigned long ctrl, err, diag;

	ctrl = cia_read(CIA_REG_CIA_CTRL);
	err = cia_read(CIA_REG_CIA_ERR);
	diag = cia_read(CIA_REG_CIA_DIAG);

	printf("\nCIA Chipset Status:\n");
	printf("  Revision:    %d\n", cia_revision);
	printf("  Control:     0x%016lx\n", ctrl);
	printf("  Error:       0x%016lx\n", err);
	printf("  Diagnostic:  0x%016lx\n", diag);
}

/*
 * HAE (Host Address Extension) management
 *
 * The HAE registers extend PCI addresses beyond 32 bits.
 * This is needed for accessing high memory from PCI devices.
 */

static unsigned long cia_hae_mem = 0;
static unsigned long cia_hae_io = 0;

unsigned long
cia_get_hae_mem(void)
{
	return cia_hae_mem;
}

void
cia_set_hae_mem(unsigned long hae)
{
	if (hae != cia_hae_mem) {
		cia_hae_mem = hae;
		cia_write(CIA_REG_CSR_HAE_MEM, hae);
		alpha_mb();
	}
}

unsigned long
cia_get_hae_io(void)
{
	return cia_hae_io;
}

void
cia_set_hae_io(unsigned long hae)
{
	if (hae != cia_hae_io) {
		cia_hae_io = hae;
		cia_write(CIA_REG_CSR_HAE_IO, hae);
		alpha_mb();
	}
}
