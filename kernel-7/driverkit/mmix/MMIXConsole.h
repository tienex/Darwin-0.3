/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Console Driver Header
 */

#ifndef _MMIX_CONSOLE_H_
#define _MMIX_CONSOLE_H_

#import <driverkit/KernDevice.h>

@interface MMIXConsole : KernDevice
{
    volatile unsigned long long *consoleOut;
    volatile unsigned long long *consoleIn;
    BOOL initialized;
}

+ (BOOL)probe: deviceDescription;
- initFromDeviceDescription:deviceDescription;
- (void)putc:(char)c;
- (int)getc;
- (void)puts:(const char *)s;

@end

#endif /* _MMIX_CONSOLE_H_ */
