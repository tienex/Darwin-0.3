/*
 * MMIX Bootloader MMU Initialization
 * Copyright (c) 1999-2025 Apple Computer, Inc.
 *
 * Set up initial page tables for kernel boot
 */

#include "boot.h"

/* Simple page table entry structure */
struct pte {
	unsigned long long phys_addr;	/* Physical address (bits 13-63) */
	unsigned int flags;		/* Page flags */
	unsigned int reserved;
};

/* Page table flags */
#define PTE_VALID	0x01		/* Page is valid */
#define PTE_WRITABLE	0x02		/* Page is writable */
#define PTE_USER	0x04		/* User-accessible */
#define PTE_NOCACHE	0x08		/* Non-cacheable */

/* Page table storage (statically allocated) */
static struct pte page_table[4096] __attribute__((aligned(PAGE_SIZE)));

/*
 * Initialize MMU and create identity mapping for bootloader
 */
void boot_mmu_init(void)
{
	int i;

	boot_console_puts("Setting up page tables...\n");

	/* Clear page table */
	for (i = 0; i < 4096; i++) {
		page_table[i].phys_addr = 0;
		page_table[i].flags = 0;
		page_table[i].reserved = 0;
	}

	/* Map first 256 MB of physical memory to both:
	 * - 0x0000000000000000 (identity mapping for bootloader)
	 * - 0x8000000000000000 (kernel virtual space)
	 */

	/* Identity mapping: 0x00000000 -> 0x00000000 */
	for (i = 0; i < 256; i++) {
		unsigned long long phys = (unsigned long long)i * 1024 * 1024;
		boot_mmu_map(phys, phys, 1024 * 1024,
		             PTE_VALID | PTE_WRITABLE);
	}

	/* Kernel mapping: 0x8000000000000000 -> 0x00000000 */
	for (i = 0; i < 256; i++) {
		unsigned long long virt = 0x8000000000000000ULL +
		                          (unsigned long long)i * 1024 * 1024;
		unsigned long long phys = (unsigned long long)i * 1024 * 1024;
		boot_mmu_map(virt, phys, 1024 * 1024,
		             PTE_VALID | PTE_WRITABLE);
	}

	/* Map console device */
	boot_mmu_map(MMIX_CONSOLE_BASE, MMIX_CONSOLE_BASE, PAGE_SIZE,
	             PTE_VALID | PTE_WRITABLE | PTE_NOCACHE);

	/* Map disk device */
	boot_mmu_map(MMIX_DISK_BASE, MMIX_DISK_BASE, PAGE_SIZE * 2,
	             PTE_VALID | PTE_WRITABLE | PTE_NOCACHE);

	/* Store page table base address in boot args */
	extern struct mmix_boot_args boot_args;
	boot_args.page_table_base = (unsigned long long)page_table;

	boot_console_puts("MMU initialized\n");
}

/*
 * Create virtual to physical mapping
 *
 * virt: Virtual address
 * phys: Physical address
 * size: Size of mapping in bytes
 * flags: Page flags
 */
void boot_mmu_map(unsigned long long virt, unsigned long long phys,
                  unsigned long long size, unsigned int flags)
{
	unsigned long long vpage = virt >> PAGE_SHIFT;
	unsigned long long ppage = phys >> PAGE_SHIFT;
	unsigned long long pages = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	unsigned long long i;

	for (i = 0; i < pages; i++) {
		unsigned int index = (vpage + i) & 0xFFF; /* Simple direct mapping */

		if (index < 4096) {
			page_table[index].phys_addr = (ppage + i) << PAGE_SHIFT;
			page_table[index].flags = flags;
		}
	}
}
