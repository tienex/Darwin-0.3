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
 * MIPS:	Machine-dependent SPL definitions.
 *
 * SPLs (Software Priority Levels) are implemented by manipulating
 * the MIPS CP0 Status register interrupt mask.
 */

#ifndef	_MACHINE_MACHSPL_H_
#define	_MACHINE_MACHSPL_H_ 1

/*
 * SPL type is an integer representing the SR register value
 */
typedef unsigned int	spl_t;

/*
 * SPL function declarations - implemented in assembly or C
 */
extern spl_t	splhigh(void);
extern spl_t	splsched(void);
extern spl_t	splclock(void);
extern spl_t	splvm(void);
extern spl_t	splbio(void);
extern spl_t	splimp(void);
extern spl_t	spltty(void);
extern spl_t	splnet(void);
extern spl_t	splsoftclock(void);
extern spl_t	spl0(void);
extern void	splx(spl_t);

#endif	/* _MACHINE_MACHSPL_H_ */
