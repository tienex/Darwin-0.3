/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Console Driver Implementation
 *
 * Provides console I/O via emulator memory-mapped device
 */

#import "MMIXConsole.h"
#import <driverkit/KernDeviceDescription.h>

/* Console device base addresses */
#define MMIX_CONSOLE_BASE	0xFFFFFFFF00000000ULL
#define MMIX_CONSOLE_OUT_OFFSET	0x00
#define MMIX_CONSOLE_IN_OFFSET	0x08

@implementation MMIXConsole

+ (BOOL)probe:deviceDescription
{
    MMIXConsole *console;

    console = [self alloc];
    if ([console initFromDeviceDescription:deviceDescription] == nil) {
        return NO;
    }

    [console registerDevice];
    return YES;
}

- initFromDeviceDescription:deviceDescription
{
    if ([super initFromDeviceDescription:deviceDescription] == nil) {
        return nil;
    }

    /* Map console device registers */
    consoleOut = (volatile unsigned long long *)(MMIX_CONSOLE_BASE +
                                                  MMIX_CONSOLE_OUT_OFFSET);
    consoleIn = (volatile unsigned long long *)(MMIX_CONSOLE_BASE +
                                                 MMIX_CONSOLE_IN_OFFSET);

    initialized = YES;

    [self setName:"MMIXConsole"];
    [self setDeviceKind:"Console"];
    [self setLocation:"Emulator"];

    return self;
}

- (void)putc:(char)c
{
    if (!initialized) {
        return;
    }

    /* Handle newline */
    if (c == '\n') {
        *consoleOut = '\r';
    }

    *consoleOut = (unsigned long long)c;
}

- (int)getc
{
    unsigned long long c;

    if (!initialized) {
        return -1;
    }

    /* Non-blocking read */
    c = *consoleIn;
    if (c == 0) {
        return -1;
    }

    return (int)(c & 0xFF);
}

- (void)puts:(const char *)s
{
    if (!initialized || !s) {
        return;
    }

    while (*s) {
        [self putc:*s++];
    }
}

@end
