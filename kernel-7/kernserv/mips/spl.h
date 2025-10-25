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
 * MIPS Software Priority Level (SPL) definitions
 */

#ifndef	_KERNSERV_MIPS_SPL_H_
#define	_KERNSERV_MIPS_SPL_H_

#ifndef __ASSEMBLER__

/*
 * Machine-dependent SPL definitions for MIPS.
 *
 * MIPS uses the Status register IM (Interrupt Mask) field
 * to control interrupt priority levels.
 */

/* SPL priority levels */
#define SPL0		0	/* All interrupts enabled */
#define SPLSOFTCLOCK	1	/* Software clock */
#define SPLNET		2	/* Network */
#define SPLTTY		3	/* TTY */
#define SPLBIO		4	/* Block I/O */
#define SPLIMP		5	/* Network (import) */
#define SPLVM		6	/* VM */
#define SPLCLOCK	7	/* Hardware clock */
#define SPLSCHED	7	/* Scheduler */
#define SPLHIGH		8	/* All interrupts disabled */
#define SPLOFF		8	/* All interrupts off */
#define SPLPOWER	SPLHIGH	/* Power management */

/*
 * SPL function prototypes
 */
extern unsigned int set_priority_level(unsigned int level);

extern unsigned int sploff(void);
extern unsigned int splhigh(void);
extern unsigned int splsched(void);
extern unsigned int splclock(void);
extern unsigned int splpower(void);
extern unsigned int splvm(void);
extern unsigned int splbio(void);
extern unsigned int splimp(void);
extern unsigned int spltty(void);
extern unsigned int splnet(void);
extern unsigned int splsclk(void);

extern void spllo(void);
extern void splon(unsigned int level);
extern void splx(unsigned int level);
extern void spln(unsigned int level);

/*
 * Inline SPL implementations using MIPS CP0 Status register
 */

static __inline__ unsigned int
mips_get_sr(void)
{
	unsigned int sr;
	__asm__ __volatile__(
		"mfc0 %0, $12\n\t"
		: "=r" (sr)
	);
	return sr;
}

static __inline__ void
mips_set_sr(unsigned int sr)
{
	__asm__ __volatile__(
		"mtc0 %0, $12\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		:: "r" (sr)
	);
}

static __inline__ unsigned int
mips_disable_interrupts(void)
{
	unsigned int sr = mips_get_sr();
	mips_set_sr(sr & ~0x00000001);  /* Clear IE bit */
	return sr;
}

static __inline__ void
mips_enable_interrupts(void)
{
	unsigned int sr = mips_get_sr();
	mips_set_sr(sr | 0x00000001);  /* Set IE bit */
}

static __inline__ void
mips_restore_interrupts(unsigned int sr)
{
	mips_set_sr(sr);
}

#endif /* __ASSEMBLER__ */

#endif	/* _KERNSERV_MIPS_SPL_H_ */
