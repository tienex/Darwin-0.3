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
 * MIPS profiling support
 */

#ifndef _BSD_MIPS_PROFILE_H_
#define _BSD_MIPS_PROFILE_H_

#ifdef KERNEL
/*
 * Block interrupts during mcount so that those interrupts can also be
 * counted (as soon as we get done with the current counting).
 * Use MIPS interrupt enable/disable for profiling.
 */

#ifndef __ASSEMBLER__

extern unsigned int mips_get_sr(void);
extern void mips_set_sr(unsigned int);

static __inline__ unsigned int _profile_spl(void) {
	unsigned int sr = mips_get_sr();
	mips_set_sr(sr & ~0x00000001);	/* Clear IE bit */
	return sr;
}

static __inline__ void _profile_splx(unsigned int sr) {
	mips_set_sr(sr);
}

#define MCOUNT_INIT
#define	MCOUNT_ENTER	unsigned int _mcount_sr = _profile_spl();
#define	MCOUNT_EXIT	_profile_splx(_mcount_sr);

#endif /* !__ASSEMBLER__ */

#endif /* KERNEL */

#endif /* _BSD_MIPS_PROFILE_H_ */
