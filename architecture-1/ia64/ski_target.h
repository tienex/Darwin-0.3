/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * SKI Simulator Target Configuration for IA64
 *
 * Ski is an IA-64 instruction set simulator from HP Labs.
 * This header provides configuration and support for running Darwin
 * on the Ski simulator.
 */

#ifndef _IA64_SKI_TARGET_H_
#define _IA64_SKI_TARGET_H_

/* SKI simulator identification */
#define IA64_TARGET_SKI 1

/*
 * SKI-specific features and limitations
 */
#define SKI_MAX_CPUS            4      /* Ski supports up to 4 CPUs */
#define SKI_MEMORY_SIZE         (256 * 1024 * 1024)  /* Default 256MB */
#define SKI_CONSOLE_DEVICE      "/dev/ttyS0"

/*
 * SKI system call interface
 * Ski provides special break instructions for system calls
 */
#define SKI_SYSCALL_BREAK       0x80000  /* Break code for Ski syscalls */

/*
 * SKI-specific system calls
 */
#define SKI_SYSCALL_PUTCHAR     0  /* Write character to console */
#define SKI_SYSCALL_GETCHAR     1  /* Read character from console */
#define SKI_SYSCALL_EXIT        2  /* Exit simulator */
#define SKI_SYSCALL_PUTSTRING   3  /* Write string to console */

/*
 * SKI console I/O functions
 */
static __inline__ void
ski_putchar(int c)
{
    __asm__ volatile(
        "mov r15 = %0\n"
        "mov r8 = %1\n"
        "break.i 0x80000"
        :
        : "i" (SKI_SYSCALL_PUTCHAR), "r" ((long)c)
        : "r8", "r15", "memory"
    );
}

static __inline__ int
ski_getchar(void)
{
    long c;

    __asm__ volatile(
        "mov r15 = %1\n"
        "break.i 0x80000\n"
        "mov %0 = r8"
        : "=r" (c)
        : "i" (SKI_SYSCALL_GETCHAR)
        : "r8", "r15", "memory"
    );

    return (int)c;
}

static __inline__ void
ski_putstring(const char *str)
{
    __asm__ volatile(
        "mov r15 = %0\n"
        "mov r8 = %1\n"
        "break.i 0x80000"
        :
        : "i" (SKI_SYSCALL_PUTSTRING), "r" (str)
        : "r8", "r15", "memory"
    );
}

static __inline__ void
ski_exit(int status)
{
    __asm__ volatile(
        "mov r15 = %0\n"
        "mov r8 = %1\n"
        "break.i 0x80000"
        :
        : "i" (SKI_SYSCALL_EXIT), "r" ((long)status)
        : "r8", "r15", "memory"
    );
}

/*
 * SKI device emulation
 */
#define SKI_DISK_DEVICE         "/dev/sd0"  /* Simulated disk */
#define SKI_NET_DEVICE          "ski0"      /* Simulated network */

/*
 * SKI debugging support
 */
#define SKI_BREAKPOINT() \
    __asm__ volatile("break.i 0x0")

#define SKI_DEBUG_PUTCHAR(c)    ski_putchar(c)
#define SKI_DEBUG_PUTSTRING(s)  ski_putstring(s)

/*
 * Detect if running on Ski simulator
 * Returns 1 if running on Ski, 0 otherwise
 */
static __inline__ int
ia64_is_ski(void)
{
    /* Ski has specific CPUID characteristics
     * Check for Ski-specific vendor string
     */
    unsigned long cpuid_regs[4];

    __asm__ volatile(
        "mov r8 = cpuid[r0]\n"
        "mov %0 = r8"
        : "=r" (cpuid_regs[0])
        :
        : "r8"
    );

    /* Ski returns specific values in CPUID */
    /* This is a simplified check - actual implementation would be more complex */
    return 0;  /* Placeholder */
}

#endif /* _IA64_SKI_TARGET_H_ */
