/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Timer Driver Header
 */

#ifndef _MMIX_TIMER_H_
#define _MMIX_TIMER_H_

#import <driverkit/KernDevice.h>

@interface MMIXTimer : KernDevice
{
    volatile unsigned long long *timerCounter;
    volatile unsigned long long *timerInterval;
    volatile unsigned long long *timerControl;
    BOOL initialized;
    unsigned long long frequency;
}

+ (BOOL)probe: deviceDescription;
- initFromDeviceDescription:deviceDescription;
- (unsigned long long)readCounter;
- (void)setInterval:(unsigned long long)ticks;
- (void)enableInterrupts;
- (void)disableInterrupts;

@end

#endif /* _MMIX_TIMER_H_ */
