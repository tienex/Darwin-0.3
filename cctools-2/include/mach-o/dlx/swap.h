/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX Byte Swapping Functions
 * For Mach-O file format support on different endian hosts
 */

#ifndef _MACH_O_DLX_SWAP_H_
#define _MACH_O_DLX_SWAP_H_

#include <mach/dlx/thread_status.h>

/*
 * swap_dlx_thread_state() - swap a DLX thread state structure
 */
extern void swap_dlx_thread_state(
    struct dlx_thread_state *ts,
    enum NXByteOrder target_byte_order);

/*
 * swap_dlx_float_state() - swap a DLX floating point state structure
 */
extern void swap_dlx_float_state(
    struct dlx_float_state *fs,
    enum NXByteOrder target_byte_order);

/*
 * swap_dlx_exception_state() - swap a DLX exception state structure
 */
extern void swap_dlx_exception_state(
    struct dlx_exception_state *es,
    enum NXByteOrder target_byte_order);

#endif /* _MACH_O_DLX_SWAP_H_ */
