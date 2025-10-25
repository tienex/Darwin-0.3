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
 * Endian Swapping Support for IA64 (Itanium)
 *
 * IA64 processors support runtime endian switching via the PSR.be bit.
 * This header provides support for running big-endian binaries on
 * little-endian systems and vice versa.
 */

#ifndef _IA64_ENDIAN_SWAP_H_
#define _IA64_ENDIAN_SWAP_H_

/* Check if processor supports endian switching */
#define IA64_SUPPORTS_ENDIAN_SWITCH 1

/* PSR (Processor Status Register) bit for big-endian mode */
#define PSR_BE_BIT 1

/*
 * Set endianness mode for user space
 * Returns 0 on success, -1 on failure
 */
static __inline__ int
ia64_set_endian_mode(int big_endian)
{
    /* This would use the cover/rfi instruction sequence to modify PSR.be
     * In user space, this requires kernel support via a system call
     */
    #if defined(__KERNEL__)
        /* Kernel can directly modify PSR */
        if (big_endian) {
            __asm__ volatile("sum psr.be");
        } else {
            __asm__ volatile("rum psr.be");
        }
        return 0;
    #else
        /* User space needs syscall - return error for now */
        return -1;
    #endif
}

/*
 * Get current endianness mode
 * Returns 1 for big-endian, 0 for little-endian
 */
static __inline__ int
ia64_get_endian_mode(void)
{
    unsigned long psr;

    __asm__ volatile("mov %0 = psr" : "=r" (psr));

    return (psr & (1UL << PSR_BE_BIT)) ? 1 : 0;
}

/*
 * Binary format indicators for multi-endian support
 */
#define IA64_BINARY_LE  0  /* Little-endian binary */
#define IA64_BINARY_BE  1  /* Big-endian binary */

/*
 * Library path suffixes for different endianness
 */
#define IA64_LIB_LE_SUFFIX "/ia64"      /* Little-endian libraries */
#define IA64_LIB_BE_SUFFIX "/ia64be"    /* Big-endian libraries */

#endif /* _IA64_ENDIAN_SWAP_H_ */
