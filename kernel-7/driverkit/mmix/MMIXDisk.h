/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Disk Driver Header
 */

#ifndef _MMIX_DISK_H_
#define _MMIX_DISK_H_

#import <driverkit/KernDevice.h>

#define MMIX_SECTOR_SIZE	8192

@interface MMIXDisk : KernDevice
{
    volatile unsigned long long *sectorReg;
    volatile unsigned char *bufferReg;
    volatile unsigned long long *commandReg;
    volatile unsigned long long *statusReg;
    BOOL initialized;
    unsigned long long totalSectors;
}

+ (BOOL)probe: deviceDescription;
- initFromDeviceDescription:deviceDescription;
- (int)readSector:(unsigned long long)sector buffer:(void *)buffer count:(int)count;
- (int)writeSector:(unsigned long long)sector buffer:(const void *)buffer count:(int)count;
- (unsigned long long)capacity;

@end

#endif /* _MMIX_DISK_H_ */
