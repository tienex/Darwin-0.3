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
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef	_MACHINE_MACHSPL_H_
#define	_MACHINE_MACHSPL_H_ 1

/*
 *	This file defines the interrupt priority levels used by
 *	machine-dependent code.
 *
 *      MMIX uses the rK register for interrupt masking.
 *      Software priority levels are implemented in the kernel.
 */

/* Interrupt priority bit assignments */
#define SPL_CLOCK_BIT		0
#define SPL_POWER_BIT		1
#define SPL_VM_BIT		4
#define SPL_BIO_BIT		9
#define SPL_TTY_BIT		16
#define SPL_NET_BIT		24

/* Software priority levels */
#define SPLHIGH			7	/* Block all interrupts */
#define SPLSCHED		6	/* Block scheduling interrupts */
#define SPLCLOCK		5	/* Block clock interrupts */
#define SPLVM			4	/* Block VM faults */
#define SPLTTY			3	/* Block TTY interrupts */
#define SPLBIO			2	/* Block disk I/O interrupts */
#define SPLNET			1	/* Block network interrupts */
#define SPL0			0	/* Allow all interrupts */

#ifndef __ASSEMBLER__
typedef unsigned int spl_t;
extern spl_t	splhigh(void);
extern spl_t	splsched(void);
extern spl_t	splclock(void);
extern spl_t	splvm(void);
extern spl_t	spltty(void);
extern spl_t	splbio(void);
extern spl_t	splnet(void);
extern spl_t	spl0(void);
extern void	splx(spl_t level);
#endif /* !__ASSEMBLER__ */

#endif	/* _MACHINE_MACHSPL_H_ */
