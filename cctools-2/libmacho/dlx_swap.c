/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * DLX byte swapping functions for Mach-O file format
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#import <mach-o/dlx/swap.h>
#import <string.h>

/*
 * swap_dlx_thread_state() - swap a DLX thread state structure
 */
void
swap_dlx_thread_state(
    struct dlx_thread_state *ts,
    enum NXByteOrder target_byte_order)
{
	/* Swap all 32 general-purpose registers */
	ts->r1 = NXSwapLong(ts->r1);
	ts->r2 = NXSwapLong(ts->r2);
	ts->r3 = NXSwapLong(ts->r3);
	ts->r4 = NXSwapLong(ts->r4);
	ts->r5 = NXSwapLong(ts->r5);
	ts->r6 = NXSwapLong(ts->r6);
	ts->r7 = NXSwapLong(ts->r7);
	ts->r8 = NXSwapLong(ts->r8);
	ts->r9 = NXSwapLong(ts->r9);
	ts->r10 = NXSwapLong(ts->r10);
	ts->r11 = NXSwapLong(ts->r11);
	ts->r12 = NXSwapLong(ts->r12);
	ts->r13 = NXSwapLong(ts->r13);
	ts->r14 = NXSwapLong(ts->r14);
	ts->r15 = NXSwapLong(ts->r15);
	ts->r16 = NXSwapLong(ts->r16);
	ts->r17 = NXSwapLong(ts->r17);
	ts->r18 = NXSwapLong(ts->r18);
	ts->r19 = NXSwapLong(ts->r19);
	ts->r20 = NXSwapLong(ts->r20);
	ts->r21 = NXSwapLong(ts->r21);
	ts->r22 = NXSwapLong(ts->r22);
	ts->r23 = NXSwapLong(ts->r23);
	ts->r24 = NXSwapLong(ts->r24);
	ts->r25 = NXSwapLong(ts->r25);
	ts->r26 = NXSwapLong(ts->r26);
	ts->r27 = NXSwapLong(ts->r27);
	ts->r28 = NXSwapLong(ts->r28);
	ts->r29 = NXSwapLong(ts->r29);	/* Stack pointer */
	ts->r30 = NXSwapLong(ts->r30);	/* Frame pointer */
	ts->r31 = NXSwapLong(ts->r31);	/* Return address */

	/* Swap control registers */
	ts->pc = NXSwapLong(ts->pc);
	ts->status = NXSwapLong(ts->status);
	ts->padding = NXSwapLong(ts->padding);
}

/*
 * swap_dlx_float_state() - swap a DLX floating point state structure
 */
void
swap_dlx_float_state(
    struct dlx_float_state *fs,
    enum NXByteOrder target_byte_order)
{
	int i;

	/* Swap all 32 floating-point registers */
	for (i = 0; i < 32; i++) {
		fs->f[i] = NXSwapLong(fs->f[i]);
	}

	/* Swap FP status register */
	fs->fpstatus = NXSwapLong(fs->fpstatus);
}

/*
 * swap_dlx_exception_state() - swap a DLX exception state structure
 */
void
swap_dlx_exception_state(
    struct dlx_exception_state *es,
    enum NXByteOrder target_byte_order)
{
	es->trapno = NXSwapLong(es->trapno);
	es->err = NXSwapLong(es->err);
	es->faultaddr = NXSwapLong(es->faultaddr);
	es->status = NXSwapLong(es->status);
	es->cause = NXSwapLong(es->cause);
	es->epc = NXSwapLong(es->epc);
	es->badvaddr = NXSwapLong(es->badvaddr);
	es->padding = NXSwapLong(es->padding);
}
