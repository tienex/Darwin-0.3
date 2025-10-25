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
 * RISC-V Software Priority Level (SPL) definitions
 */

#ifndef _MACHDEP_RISCV_MACHSPL_H_
#define _MACHDEP_RISCV_MACHSPL_H_

/*
 * Software interrupt priority levels
 */
typedef unsigned spl_t;

#define SPLOFF          0       /* All interrupts disabled */
#define SPLPOWER        1       /* Power management */
#define SPLVM           2       /* Virtual memory */
#define SPLBIO          3       /* Block I/O */
#define SPLTTY          4       /* TTY */
#define SPLNET          5       /* Network */
#define SPLIMP          6       /* Important */
#define SPLCLOCK        7       /* Clock */
#define SPLSCHED        7       /* Scheduler */
#define SPLHIGH         7       /* Highest priority */

#ifndef __ASSEMBLER__

/*
 * RISC-V interrupt control via sstatus.SIE bit
 */

static inline spl_t splhigh(void)
{
	unsigned long status, old_status;
	__asm__ __volatile__(
		"csrrc %0, sstatus, %1"
		: "=r" (old_status)
		: "r" (0x2)  /* SIE bit */
		: "memory");
	return (spl_t)(old_status & 0x2);
}

static inline spl_t spl0(void)
{
	unsigned long status, old_status;
	__asm__ __volatile__(
		"csrrsi %0, sstatus, 2"
		: "=r" (old_status)
		:
		: "memory");
	return (spl_t)(old_status & 0x2);
}

static inline spl_t splx(spl_t ipl)
{
	unsigned long status, old_status;
	if (ipl) {
		__asm__ __volatile__(
			"csrrsi %0, sstatus, 2"
			: "=r" (old_status)
			:
			: "memory");
	} else {
		__asm__ __volatile__(
			"csrrc %0, sstatus, %1"
			: "=r" (old_status)
			: "r" (0x2)
			: "memory");
	}
	return (spl_t)(old_status & 0x2);
}

#define splon()         spl0()
#define sploff()        splhigh()

/* Specific SPL functions */
#define splnet()        splhigh()
#define splbio()        splhigh()
#define spltty()        splhigh()
#define splimp()        splhigh()
#define splclock()      splhigh()
#define splvm()         splhigh()
#define splsched()      splhigh()
#define splpower()      splhigh()

#endif /* !__ASSEMBLER__ */

#endif /* _MACHDEP_RISCV_MACHSPL_H_ */
