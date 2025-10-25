/*
 * MMIX Bootloader Disk Driver
 * Copyright (c) 1999-2025 Apple Computer, Inc.
 *
 * Virtual disk I/O via emulator memory-mapped device
 */

#include "boot.h"

/* Disk device registers */
static volatile unsigned long long *disk_sector =
	(volatile unsigned long long *)MMIX_DISK_SECTOR;
static volatile unsigned char *disk_buffer =
	(volatile unsigned char *)MMIX_DISK_BUFFER;
static volatile unsigned long long *disk_command =
	(volatile unsigned long long *)MMIX_DISK_COMMAND;
static volatile unsigned long long *disk_status =
	(volatile unsigned long long *)MMIX_DISK_STATUS;

/*
 * Initialize disk subsystem
 */
void boot_disk_init(void)
{
	/* Wait for disk to be ready */
	while (*disk_status == DISK_STATUS_BUSY) {
		/* Spin */
	}

	if (*disk_status == DISK_STATUS_ERROR) {
		boot_console_puts("WARNING: Disk reports error status\n");
	}
}

/*
 * Read sectors from disk
 *
 * sector: Starting sector number
 * buffer: Destination buffer
 * count: Number of sectors to read
 *
 * Returns: 0 on success, -1 on error
 */
int boot_disk_read(unsigned long long sector, void *buffer, int count)
{
	unsigned char *dest = (unsigned char *)buffer;
	int i, j;

	for (i = 0; i < count; i++) {
		/* Set sector number */
		*disk_sector = sector + i;

		/* Issue read command */
		*disk_command = DISK_CMD_READ;

		/* Wait for completion */
		while (*disk_status == DISK_STATUS_BUSY) {
			/* Spin */
		}

		/* Check for error */
		if (*disk_status == DISK_STATUS_ERROR) {
			return -1;
		}

		/* Copy data from disk buffer to destination */
		for (j = 0; j < PAGE_SIZE; j++) {
			dest[i * PAGE_SIZE + j] = disk_buffer[j];
		}
	}

	return 0;
}
