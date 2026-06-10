/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Disk Driver Implementation
 *
 * Provides block device I/O via emulator virtual disk
 */

#import "MMIXDisk.h"
#import <driverkit/KernDeviceDescription.h>

/* Disk device base address */
#define MMIX_DISK_BASE		0xFFFFFFFF00001000ULL
#define MMIX_DISK_SECTOR_OFFSET	0x00
#define MMIX_DISK_BUFFER_OFFSET	0x08
#define MMIX_DISK_COMMAND_OFFSET	0x2008
#define MMIX_DISK_STATUS_OFFSET		0x2010

/* Disk commands */
#define DISK_CMD_READ		1
#define DISK_CMD_WRITE		2

/* Disk status values */
#define DISK_STATUS_READY	0
#define DISK_STATUS_BUSY	1
#define DISK_STATUS_ERROR	2

@implementation MMIXDisk

+ (BOOL)probe:deviceDescription
{
    MMIXDisk *disk;

    disk = [self alloc];
    if ([disk initFromDeviceDescription:deviceDescription] == nil) {
        return NO;
    }

    [disk registerDevice];
    return YES;
}

- initFromDeviceDescription:deviceDescription
{
    if ([super initFromDeviceDescription:deviceDescription] == nil) {
        return nil;
    }

    /* Map disk device registers */
    sectorReg = (volatile unsigned long long *)(MMIX_DISK_BASE +
                                                 MMIX_DISK_SECTOR_OFFSET);
    bufferReg = (volatile unsigned char *)(MMIX_DISK_BASE +
                                            MMIX_DISK_BUFFER_OFFSET);
    commandReg = (volatile unsigned long long *)(MMIX_DISK_BASE +
                                                  MMIX_DISK_COMMAND_OFFSET);
    statusReg = (volatile unsigned long long *)(MMIX_DISK_BASE +
                                                 MMIX_DISK_STATUS_OFFSET);

    /* Wait for disk ready */
    while (*statusReg == DISK_STATUS_BUSY) {
        /* Spin */
    }

    if (*statusReg == DISK_STATUS_ERROR) {
        return nil;
    }

    /* Default disk size: 1 GB (131072 sectors of 8KB) */
    totalSectors = 131072;

    initialized = YES;

    [self setName:"MMIXDisk"];
    [self setDeviceKind:"Disk"];
    [self setLocation:"Emulator"];

    return self;
}

- (int)readSector:(unsigned long long)sector buffer:(void *)buffer count:(int)count
{
    unsigned char *dest = (unsigned char *)buffer;
    int i, j;

    if (!initialized || !buffer || sector >= totalSectors) {
        return -1;
    }

    for (i = 0; i < count; i++) {
        if (sector + i >= totalSectors) {
            return i;
        }

        /* Set sector number */
        *sectorReg = sector + i;

        /* Issue read command */
        *commandReg = DISK_CMD_READ;

        /* Wait for completion */
        while (*statusReg == DISK_STATUS_BUSY) {
            /* Spin */
        }

        /* Check for error */
        if (*statusReg == DISK_STATUS_ERROR) {
            return -1;
        }

        /* Copy data from disk buffer */
        for (j = 0; j < MMIX_SECTOR_SIZE; j++) {
            dest[i * MMIX_SECTOR_SIZE + j] = bufferReg[j];
        }
    }

    return count;
}

- (int)writeSector:(unsigned long long)sector buffer:(const void *)buffer count:(int)count
{
    const unsigned char *src = (const unsigned char *)buffer;
    int i, j;

    if (!initialized || !buffer || sector >= totalSectors) {
        return -1;
    }

    for (i = 0; i < count; i++) {
        if (sector + i >= totalSectors) {
            return i;
        }

        /* Copy data to disk buffer */
        for (j = 0; j < MMIX_SECTOR_SIZE; j++) {
            bufferReg[j] = src[i * MMIX_SECTOR_SIZE + j];
        }

        /* Set sector number */
        *sectorReg = sector + i;

        /* Issue write command */
        *commandReg = DISK_CMD_WRITE;

        /* Wait for completion */
        while (*statusReg == DISK_STATUS_BUSY) {
            /* Spin */
        }

        /* Check for error */
        if (*statusReg == DISK_STATUS_ERROR) {
            return -1;
        }
    }

    return count;
}

- (unsigned long long)capacity
{
    return totalSectors * MMIX_SECTOR_SIZE;
}

@end
