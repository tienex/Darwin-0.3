/*
 * MMIX Bootloader Main
 * Copyright (c) 1999-2025 Apple Computer, Inc.
 *
 * Main bootloader logic for MMIX emulator
 */

#include "boot.h"

/* Global boot arguments */
static struct mmix_boot_args boot_args;

/* Memory size (passed from boot_entry) */
extern unsigned long long boot_memory_size;

/*
 * Boot main entry point
 * Called from boot.s after basic initialization
 */
void boot_main(void)
{
	boot_console_init();
	boot_console_puts("\n");
	boot_console_puts("Darwin MMIX Bootloader v1.0\n");
	boot_console_puts("Copyright (c) 1999-2025 Apple Computer, Inc.\n");
	boot_console_puts("\n");

	/* Initialize subsystems */
	boot_console_puts("Initializing disk...\n");
	boot_disk_init();

	boot_console_puts("Initializing MMU...\n");
	boot_mmu_init();

	/* Set up boot arguments */
	boot_args.version = (MMIX_BOOT_VERSION_MAJOR << 16) |
	                    (MMIX_BOOT_VERSION_MINOR << 8) |
	                    MMIX_BOOT_VERSION_PATCH;
	boot_args.phys_mem_size = 256 * 1024 * 1024; /* 256 MB default */
	boot_args.virt_base = KERNEL_VIRT_BASE;
	boot_args.phys_base = KERNEL_LOAD_ADDR;
	boot_args.console_base = MMIX_CONSOLE_BASE;
	boot_args.disk_base = MMIX_DISK_BASE;

	/* Copy kernel path */
	const char *kernel_path = "/mach_kernel";
	unsigned long long i;
	for (i = 0; kernel_path[i] && i < 255; i++) {
		boot_args.boot_file[i] = kernel_path[i];
	}
	boot_args.boot_file[i] = '\0';

	/* Load kernel */
	boot_console_puts("Loading kernel: ");
	boot_console_puts(boot_args.boot_file);
	boot_console_puts("\n");

	if (boot_load_kernel(boot_args.boot_file) < 0) {
		boot_console_puts("ERROR: Failed to load kernel!\n");
		boot_console_puts("System halted.\n");
		while (1) { /* Halt */ }
	}

	boot_console_puts("Kernel loaded successfully\n");
	boot_console_puts("Transferring control to kernel...\n");
	boot_console_puts("\n");

	/* Start kernel */
	boot_start_kernel(&boot_args);

	/* Should never return */
	boot_console_puts("ERROR: Kernel returned!\n");
	while (1) { /* Halt */ }
}

/*
 * Load Darwin kernel from disk
 */
int boot_load_kernel(const char *path)
{
	unsigned char *load_addr = (unsigned char *)KERNEL_LOAD_ADDR;
	unsigned long long sector;
	int total_sectors = 2048; /* Load first 16 MB as kernel */

	boot_console_puts("Reading kernel sectors...\n");

	for (sector = 0; sector < total_sectors; sector++) {
		if (boot_disk_read(sector, load_addr + (sector * PAGE_SIZE), 1) < 0) {
			boot_console_puts("Disk read error at sector ");
			/* TODO: print sector number */
			boot_console_puts("\n");
			return -1;
		}

		/* Show progress every 256 sectors */
		if ((sector & 0xFF) == 0) {
			boot_console_putc('.');
		}
	}

	boot_console_puts("\n");
	return 0;
}

/*
 * Transfer control to kernel
 */
void boot_start_kernel(struct mmix_boot_args *args)
{
	/* Kernel entry point */
	void (*kernel_entry)(struct mmix_boot_args *) =
		(void (*)(struct mmix_boot_args *))KERNEL_VIRT_BASE;

	/* Flush caches */
	boot_flush_cache();

	/* Jump to kernel */
	kernel_entry(args);
}

/*
 * Utility: memset
 */
void *boot_memset(void *s, int c, unsigned long long n)
{
	unsigned char *p = (unsigned char *)s;
	unsigned long long i;

	for (i = 0; i < n; i++) {
		p[i] = (unsigned char)c;
	}

	return s;
}

/*
 * Utility: memcpy
 */
void *boot_memcpy(void *dest, const void *src, unsigned long long n)
{
	unsigned char *d = (unsigned char *)dest;
	const unsigned char *s = (const unsigned char *)src;
	unsigned long long i;

	for (i = 0; i < n; i++) {
		d[i] = s[i];
	}

	return dest;
}

/*
 * Utility: strcmp
 */
int boot_strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}

	return *s1 - *s2;
}

/*
 * Utility: strlen
 */
unsigned long long boot_strlen(const char *s)
{
	unsigned long long len = 0;

	while (*s++) {
		len++;
	}

	return len;
}
