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
 *	File:	mmix/reboot.h
 *
 *	MMIX specific reboot flags.
 */

#ifndef	_BSD_MMIX_REBOOT_H_
#define _BSD_MMIX_REBOOT_H_

/*
 * Empty file (publicly)
 */
#ifdef KERNEL
/*
 *	Use most significant 16 bits to avoid collisions with
 *	machine independent flags.
 */
#define RB_POWERDOWN	0x00010000	/* power down on halt */
#define	RB_NOBOOTRC	0x00020000	/* don't run '/etc/rc.boot' */
#define	RB_DEBUG	0x00040000	/* drop into mini monitor on panic */
#define	RB_EJECT	0x00080000	/* eject disks on halt */
#define	RB_COMMAND	0x00100000	/* new boot command specified */

#endif /* KERNEL */
#endif /* _BSD_MMIX_REBOOT_H_ */
