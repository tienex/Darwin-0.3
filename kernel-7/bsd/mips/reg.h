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
 * MIPS register definitions for debugging
 */

#ifndef _BSD_MIPS_REG_H_
#define _BSD_MIPS_REG_H_

#ifdef	KERNEL_PRIVATE

#include <machdep/mips/trap.h>

/* Index into the thread_state / saved_state */
#define SP	29	/* Stack pointer ($29) */
#define PC	-1	/* PC stored separately in saved_state */
#define RA	31	/* Return address ($31) */
#define FP	30	/* Frame pointer ($30) */
#define GP	28	/* Global pointer ($28) */

#endif /* KERNEL_PRIVATE */
#endif /* _BSD_MIPS_REG_H_ */
