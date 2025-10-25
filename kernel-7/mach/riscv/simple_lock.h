/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * Simple spin locks for RISC-V.
 */

#import <mach/boolean.h>

#ifndef	_MACH_RISCV_SIMPLE_LOCK_H_
#define _MACH_RISCV_SIMPLE_LOCK_H_

#define _MACHINE_SIMPLE_LOCK_DATA_

struct slock {
    volatile boolean_t		locked;
};

typedef struct slock		simple_lock_data_t;
typedef simple_lock_data_t	*simple_lock_t;

static __inline__
void
simple_lock_init(
    simple_lock_t	slock
)
{
    slock->locked = FALSE;
}

static __inline__
boolean_t
simple_lock_try(
    simple_lock_t	slock
)
{
    boolean_t		result = TRUE;

#if defined(__riscv_atomic)
    /* Use RISC-V atomic instruction (amoswap.w.aq) */
    __asm__ __volatile__(
        "amoswap.w.aq %0, %2, %1"
        : "=r" (result), "+A" (slock->locked)
        : "r" (TRUE)
        : "memory");

    /* Return TRUE if we got the lock (was previously FALSE) */
    return (result == FALSE);
#else
    /* Fallback for systems without atomics extension */
    if (slock->locked) {
        return FALSE;
    }
    slock->locked = TRUE;
    return TRUE;
#endif
}

static __inline__
void
simple_lock(
    simple_lock_t	slock
)
{
    do {
        while (slock->locked)
            continue;
    } while (!simple_lock_try(slock));
}

static __inline__
void
simple_unlock(
    simple_lock_t	slock
)
{
#if defined(__riscv_atomic)
    /* Use RISC-V atomic store with release semantics */
    int tmp;
    __asm__ __volatile__(
        "amoswap.w.rl %0, zero, %1"
        : "=r" (tmp), "+A" (slock->locked)
        :
        : "memory");
#else
    slock->locked = FALSE;
#endif
}

#endif	/* _MACH_RISCV_SIMPLE_LOCK_H_ */
