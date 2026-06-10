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

#ifndef _BSD_MMIX_PROFILE_H_
#define _BSD_MMIX_PROFILE_H_

#ifdef KERNEL
/*
 * Block interrupts during mcount so that those interrupts can also be
 * counted (as soon as we get done with the current counting).  On the
 * MMIX platform, we disable interrupts by manipulating the rK register.
 */
extern unsigned long long disable_interrupts();
extern void restore_interrupts(unsigned long long old_rK);

#define MCOUNT_INIT		register unsigned long long saved_rK;

#define	MCOUNT_ENTER	saved_rK = disable_interrupts();

#define	MCOUNT_EXIT		restore_interrupts(saved_rK);

#endif /* KERNEL */

#endif /* _BSD_MMIX_PROFILE_H_ */
