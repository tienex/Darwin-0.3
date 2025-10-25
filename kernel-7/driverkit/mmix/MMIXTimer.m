/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Timer Driver Implementation
 *
 * Provides timer services using MMIX interval counter (rI)
 */

#import "MMIXTimer.h"
#import <driverkit/KernDeviceDescription.h>

/* Timer frequency: 1 GHz (1 tick per nanosecond) */
#define MMIX_TIMER_FREQUENCY	1000000000ULL

/* Special register numbers */
#define MMIX_rI		12	/* Interval counter */

@implementation MMIXTimer

+ (BOOL)probe:deviceDescription
{
    MMIXTimer *timer;

    timer = [self alloc];
    if ([timer initFromDeviceDescription:deviceDescription] == nil) {
        return NO;
    }

    [timer registerDevice];
    return YES;
}

- initFromDeviceDescription:deviceDescription
{
    if ([super initFromDeviceDescription:deviceDescription] == nil) {
        return nil;
    }

    frequency = MMIX_TIMER_FREQUENCY;
    initialized = YES;

    [self setName:"MMIXTimer"];
    [self setDeviceKind:"Timer"];
    [self setLocation:"CPU"];

    return self;
}

- (unsigned long long)readCounter
{
    unsigned long long count;

    if (!initialized) {
        return 0;
    }

    /* Read rI register using inline assembly */
    asm volatile("GET %0,$12" : "=r" (count));

    return count;
}

- (void)setInterval:(unsigned long long)ticks
{
    if (!initialized) {
        return;
    }

    /* Set rI register */
    asm volatile("PUT $12,%0" : : "r" (ticks));
}

- (void)enableInterrupts
{
    unsigned long long rK;

    if (!initialized) {
        return;
    }

    /* Enable timer interrupt bit in rK */
    asm volatile("GET %0,$15" : "=r" (rK));
    rK |= (1ULL << 12);  /* Bit 12 for rI interrupt */
    asm volatile("PUT $15,%0" : : "r" (rK));
}

- (void)disableInterrupts
{
    unsigned long long rK;

    if (!initialized) {
        return;
    }

    /* Disable timer interrupt bit in rK */
    asm volatile("GET %0,$15" : "=r" (rK));
    rK &= ~(1ULL << 12);  /* Clear bit 12 */
    asm volatile("PUT $15,%0" : : "r" (rK));
}

@end
