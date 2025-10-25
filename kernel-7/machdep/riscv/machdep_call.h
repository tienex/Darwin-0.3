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
 * RISC-V machine-dependent system call definitions
 */

#ifndef _MACHDEP_RISCV_MACHDEP_CALL_H_
#define _MACHDEP_RISCV_MACHDEP_CALL_H_

#include <mach/mach_types.h>

/*
 * Machine-dependent call table entry
 */
typedef struct {
	kern_return_t   (*func)(void);  /* Function pointer */
	int             nargs;          /* Number of arguments */
} machdep_call_t;

/*
 * Machine-dependent call numbers
 */
#define MACHDEP_CALL_THREAD_GET_CTHREAD 0
#define MACHDEP_CALL_THREAD_SET_CTHREAD 1

#define MACHDEP_CALL_COUNT              2

/*
 * Exported table
 */
extern machdep_call_t machdep_call_table[];
extern int machdep_call_count;

#endif /* _MACHDEP_RISCV_MACHDEP_CALL_H_ */
