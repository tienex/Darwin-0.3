/*
 * MMIX Bootloader Header
 * Copyright (c) 1999-2025 Apple Computer, Inc.
 */

#ifndef _MMIX_BOOT_H_
#define _MMIX_BOOT_H_

/* Bootloader version */
#define MMIX_BOOT_VERSION_MAJOR  1
#define MMIX_BOOT_VERSION_MINOR  0
#define MMIX_BOOT_VERSION_PATCH  0

/* Memory layout constants */
#define BOOT_LOAD_ADDR		0x0000000000001000ULL	/* Bootloader load address */
#define BOOT_STACK_TOP		0x0000000000100000ULL	/* Top of boot stack */
#define KERNEL_LOAD_ADDR	0x0000000000100000ULL	/* Kernel load address */
#define KERNEL_VIRT_BASE	0x8000000000100000ULL	/* Kernel virtual base */

/* Page size */
#define PAGE_SIZE		8192
#define PAGE_SHIFT		13
#define PAGE_MASK		(PAGE_SIZE - 1)

/* Emulator device addresses */
#define MMIX_CONSOLE_BASE	0xFFFFFFFF00000000ULL
#define MMIX_CONSOLE_OUT	(MMIX_CONSOLE_BASE + 0x00)
#define MMIX_CONSOLE_IN		(MMIX_CONSOLE_BASE + 0x08)

#define MMIX_DISK_BASE		0xFFFFFFFF00001000ULL
#define MMIX_DISK_SECTOR	(MMIX_DISK_BASE + 0x00)
#define MMIX_DISK_BUFFER	(MMIX_DISK_BASE + 0x08)
#define MMIX_DISK_COMMAND	(MMIX_DISK_BASE + 0x2008)
#define MMIX_DISK_STATUS	(MMIX_DISK_BASE + 0x2010)

/* Disk commands */
#define DISK_CMD_READ		1
#define DISK_CMD_WRITE		2

/* Disk status */
#define DISK_STATUS_READY	0
#define DISK_STATUS_BUSY	1
#define DISK_STATUS_ERROR	2

/* Boot arguments passed to kernel */
struct mmix_boot_args {
	unsigned long long	version;		/* Boot args version */
	unsigned long long	phys_mem_size;		/* Physical memory size */
	unsigned long long	virt_base;		/* Kernel virtual base */
	unsigned long long	phys_base;		/* Kernel physical base */
	unsigned long long	console_base;		/* Console device base */
	unsigned long long	disk_base;		/* Disk device base */
	unsigned long long	page_table_base;	/* Initial page table */
	unsigned long long	initrd_base;		/* Initial ramdisk (optional) */
	unsigned long long	initrd_size;		/* Initial ramdisk size */
	char			boot_file[256];		/* Kernel file path */
};

/* Special register definitions */
#define rA	21	/* Arithmetic status register */
#define rB	0	/* Bootstrap register (trap) */
#define rC	8	/* Continuation register */
#define rD	1	/* Dividend register */
#define rE	2	/* Epsilon register */
#define rG	19	/* Global threshold register */
#define rH	3	/* Himult register */
#define rI	12	/* Interval counter */
#define rJ	4	/* Return-jump register */
#define rK	15	/* Interrupt mask register */
#define rL	20	/* Local threshold register */
#define rM	5	/* Multiplex mask register */
#define rN	9	/* Serial number */
#define rO	10	/* Register stack offset */
#define rP	23	/* Prediction register */
#define rQ	16	/* Interrupt request register */
#define rR	6	/* Remainder register */
#define rS	11	/* Register stack pointer */
#define rT	13	/* Trap address register */
#define rU	17	/* Usage counter */
#define rV	18	/* Virtual translation register */
#define rW	24	/* Where-interrupted register (trap 1) */
#define rX	25	/* Execution register (trap 2) */
#define rY	26	/* Y operand (trap 3) */
#define rZ	27	/* Z operand */

/* Function prototypes */
void boot_console_init(void);
void boot_console_putc(char c);
void boot_console_puts(const char *s);
int boot_console_getc(void);

void boot_disk_init(void);
int boot_disk_read(unsigned long long sector, void *buffer, int count);

void boot_mmu_init(void);
void boot_mmu_map(unsigned long long virt, unsigned long long phys,
                  unsigned long long size, unsigned int flags);

int boot_load_kernel(const char *path);
void boot_start_kernel(struct mmix_boot_args *args);

/* Utility functions */
void *boot_memset(void *s, int c, unsigned long long n);
void *boot_memcpy(void *dest, const void *src, unsigned long long n);
int boot_strcmp(const char *s1, const char *s2);
unsigned long long boot_strlen(const char *s);

/* Assembly functions */
extern void boot_entry(void);
extern unsigned long long boot_get_special(int reg);
extern void boot_put_special(int reg, unsigned long long val);
extern void boot_flush_cache(void);

#endif /* _MMIX_BOOT_H_ */
