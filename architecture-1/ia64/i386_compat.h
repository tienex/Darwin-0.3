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
 * i386 Compatibility Layer for IA64 (Itanium)
 *
 * Intel Itanium processors with x86 ISA support can execute i386 binaries
 * natively. This header provides support for running i386 executables on
 * IA64 systems.
 */

#ifndef _IA64_I386_COMPAT_H_
#define _IA64_I386_COMPAT_H_

/* Check if processor has x86 ISA support */
#define IA64_HAS_X86_ISA 1

/*
 * CPU Feature Flags for x86 compatibility
 */
#define IA64_X86_FEATURE_FPU    (1 << 0)  /* x87 FPU present */
#define IA64_X86_FEATURE_MMX    (1 << 1)  /* MMX support */
#define IA64_X86_FEATURE_SSE    (1 << 2)  /* SSE support */
#define IA64_X86_FEATURE_SSE2   (1 << 3)  /* SSE2 support */

/*
 * Binary format identifiers
 */
#define BINARY_FORMAT_IA64      0  /* Native IA64 binary */
#define BINARY_FORMAT_I386      1  /* i386 binary (x86 ISA) */

/*
 * Execution mode switching
 */
#define IA64_EXEC_MODE_IA64     0  /* Native IA64 mode */
#define IA64_EXEC_MODE_I386     1  /* i386 compatibility mode */

/*
 * Check if running in x86 compatibility mode
 * Returns 1 if in x86 mode, 0 if in native IA64 mode
 */
static __inline__ int
ia64_is_x86_mode(void)
{
    unsigned long psr;

    /* Check PSR.is (Instruction Set) bit
     * PSR.is = 1 indicates x86 instruction set
     */
    __asm__ volatile("mov %0 = psr" : "=r" (psr));

    return (psr & (1UL << 14)) ? 1 : 0;  /* Bit 14 is PSR.is */
}

/*
 * System call translation table
 * Maps i386 syscall numbers to IA64 syscall numbers
 */
struct i386_syscall_xlat {
    int i386_nr;    /* i386 syscall number */
    int ia64_nr;    /* Corresponding IA64 syscall number */
};

/*
 * i386 binary execution support
 * Returns 0 on success, -1 on failure
 */
static __inline__ int
ia64_exec_i386_binary(const char *path, char *const argv[], char *const envp[])
{
    /* This would use a special exec syscall that switches to x86 mode
     * Requires kernel support for x86 ISA binaries
     */
    #if defined(__KERNEL__)
        /* Kernel can switch to x86 mode and execute */
        return 0;  /* Placeholder */
    #else
        /* User space calls execve normally - kernel handles mode switch */
        extern int execve(const char *, char *const [], char *const []);
        return execve(path, argv, envp);
    #endif
}

/*
 * Library paths for i386 compatibility
 */
#define I386_LIB_PATH           "/usr/lib/i386"
#define I386_LIB_PATH_COMPAT    "/emul/i386/usr/lib"

/*
 * Dynamic linker paths
 */
#define IA64_DYNAMIC_LINKER     "/lib/ld-linux-ia64.so.2"
#define I386_DYNAMIC_LINKER     "/lib/ld-linux.so.2"

#endif /* _IA64_I386_COMPAT_H_ */
