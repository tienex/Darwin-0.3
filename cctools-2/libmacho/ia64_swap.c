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

#import <mach-o/ia64/swap.h>

void
swap_ia64_thread_state_t(
ia64_thread_state_t *cpu,
enum NXByteOrder target_byte_sex)
{
    unsigned long i;

    cpu->ip = NXSwapLongLong(cpu->ip);
    cpu->psr = NXSwapLongLong(cpu->psr);
    cpu->cfm = NXSwapLongLong(cpu->cfm);

    /* Swap static general registers r0-r31 */
    for (i = 0; i < IA64_STATIC_GREGS; i++)
        cpu->r[i] = NXSwapLongLong(cpu->r[i]);

    cpu->nat = NXSwapLongLong(cpu->nat);

    /* Swap branch registers b0-b7 */
    for (i = 0; i < IA64_BRANCH_REGS; i++)
        cpu->b[i] = NXSwapLongLong(cpu->b[i]);

    cpu->pr = NXSwapLongLong(cpu->pr);

    /* Swap application registers */
    cpu->ar_rsc = NXSwapLongLong(cpu->ar_rsc);
    cpu->ar_bsp = NXSwapLongLong(cpu->ar_bsp);
    cpu->ar_bspstore = NXSwapLongLong(cpu->ar_bspstore);
    cpu->ar_rnat = NXSwapLongLong(cpu->ar_rnat);
    cpu->ar_ccv = NXSwapLongLong(cpu->ar_ccv);
    cpu->ar_unat = NXSwapLongLong(cpu->ar_unat);
    cpu->ar_fpsr = NXSwapLongLong(cpu->ar_fpsr);
    cpu->ar_pfs = NXSwapLongLong(cpu->ar_pfs);
    cpu->ar_lc = NXSwapLongLong(cpu->ar_lc);
    cpu->ar_ec = NXSwapLongLong(cpu->ar_ec);
}

void
swap_ia64_exception_state_t(
ia64_exception_state_t *state,
enum NXByteOrder target_byte_sex)
{
    state->ifa = NXSwapLongLong(state->ifa);
    state->isr = NXSwapLongLong(state->isr);
    state->iip = NXSwapLongLong(state->iip);
    state->exception = NXSwapLongLong(state->exception);
}

void
swap_ia64_float_state_t(
ia64_float_state_t *fpu,
enum NXByteOrder target_byte_sex)
{
    unsigned long i;

    for (i = 0; i < IA64_STATIC_FPREGS; i++){
        fpu->f[i].lo = NXSwapLongLong(fpu->f[i].lo);
        fpu->f[i].hi = NXSwapLongLong(fpu->f[i].hi);
    }
}
