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

#import <mach-o/mmix/swap.h>
#import <string.h>

void
swap_mmix_thread_state_t(
mmix_thread_state_t *cpu,
enum NXByteOrder target_byte_sex)
{
    unsigned long i;

    cpu->pc = NXSwapLongLong(cpu->pc);

    /* Swap saved general-purpose registers (first 32) */
    for (i = 0; i < MMIX_SAVED_GREGS; i++)
        cpu->r[i] = NXSwapLongLong(cpu->r[i]);

    /* Swap special registers */
    cpu->rA = NXSwapLongLong(cpu->rA);
    cpu->rB = NXSwapLongLong(cpu->rB);
    cpu->rC = NXSwapLongLong(cpu->rC);
    cpu->rD = NXSwapLongLong(cpu->rD);
    cpu->rE = NXSwapLongLong(cpu->rE);
    cpu->rF = NXSwapLongLong(cpu->rF);
    cpu->rG = NXSwapLongLong(cpu->rG);
    cpu->rH = NXSwapLongLong(cpu->rH);
    cpu->rI = NXSwapLongLong(cpu->rI);
    cpu->rJ = NXSwapLongLong(cpu->rJ);
    cpu->rK = NXSwapLongLong(cpu->rK);
    cpu->rL = NXSwapLongLong(cpu->rL);
    cpu->rM = NXSwapLongLong(cpu->rM);
    cpu->rN = NXSwapLongLong(cpu->rN);
    cpu->rO = NXSwapLongLong(cpu->rO);
    cpu->rP = NXSwapLongLong(cpu->rP);
    cpu->rQ = NXSwapLongLong(cpu->rQ);
    cpu->rR = NXSwapLongLong(cpu->rR);
    cpu->rS = NXSwapLongLong(cpu->rS);
    cpu->rT = NXSwapLongLong(cpu->rT);
    cpu->rU = NXSwapLongLong(cpu->rU);
    cpu->rV = NXSwapLongLong(cpu->rV);
    cpu->rW = NXSwapLongLong(cpu->rW);
    cpu->rX = NXSwapLongLong(cpu->rX);
    cpu->rY = NXSwapLongLong(cpu->rY);
    cpu->rZ = NXSwapLongLong(cpu->rZ);

    /* Swap trap registers */
    cpu->rBB = NXSwapLongLong(cpu->rBB);
    cpu->rTT = NXSwapLongLong(cpu->rTT);
    cpu->rWW = NXSwapLongLong(cpu->rWW);
    cpu->rXX = NXSwapLongLong(cpu->rXX);
    cpu->rYY = NXSwapLongLong(cpu->rYY);
    cpu->rZZ = NXSwapLongLong(cpu->rZZ);
}

void
swap_mmix_exception_state_t(
mmix_exception_state_t *state,
enum NXByteOrder target_byte_sex)
{
    state->dar = NXSwapLongLong(state->dar);
    state->dsisr = NXSwapLongLong(state->dsisr);
    state->exception = NXSwapLongLong(state->exception);
}
