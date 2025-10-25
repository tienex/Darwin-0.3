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
 * RISC-V interrupt management
 */

#ifndef _MACHDEP_RISCV_INTERRUPTS_H_
#define _MACHDEP_RISCV_INTERRUPTS_H_

/*
 * RISC-V interrupt numbers (from PLIC or CLINT)
 */
#define MAX_INTERRUPTS  64

/*
 * Interrupt control
 */
#ifndef __ASSEMBLER__

#include <mach/boolean.h>

/*
 * Interrupt handler structure
 */
typedef void (*intr_handler_t)(void *arg);

typedef struct {
	intr_handler_t  handler;
	void            *arg;
	int             irq;
	boolean_t       enabled;
} intr_entry_t;

/*
 * Interrupt management functions
 */
void intr_init(void);
int  intr_establish(int irq, intr_handler_t handler, void *arg);
void intr_disestablish(int irq);
void intr_enable(int irq);
void intr_disable(int irq);

/*
 * Low-level interrupt control
 */
static inline void cpu_intr_enable(void)
{
	__asm__ __volatile__("csrsi sstatus, 0x2" ::: "memory");
}

static inline void cpu_intr_disable(void)
{
	__asm__ __volatile__("csrci sstatus, 0x2" ::: "memory");
}

static inline unsigned long cpu_intr_save(void)
{
	unsigned long status;
	__asm__ __volatile__(
		"csrrc %0, sstatus, %1"
		: "=r" (status)
		: "r" (0x2)
		: "memory");
	return status;
}

static inline void cpu_intr_restore(unsigned long status)
{
	__asm__ __volatile__(
		"csrw sstatus, %0"
		:: "r" (status)
		: "memory");
}

#endif /* !__ASSEMBLER__ */

#endif /* _MACHDEP_RISCV_INTERRUPTS_H_ */
