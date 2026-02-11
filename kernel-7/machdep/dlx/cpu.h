/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX CPU Definitions
 */

#ifndef _DLX_CPU_H_
#define _DLX_CPU_H_

/* CPU type and subtype */
#define CPU_TYPE_DLX		18
#define CPU_SUBTYPE_DLX_ALL	0

/* CPU feature flags */
#define DLX_FEATURE_FPU		0x001		/* Has FPU */
#define DLX_FEATURE_MMU		0x002		/* Has MMU */
#define DLX_FEATURE_TLB		0x004		/* Has TLB */

/* Cache sizes (typical for DLX) */
#define DLX_ICACHE_SIZE		8192		/* 8KB I-cache */
#define DLX_DCACHE_SIZE		8192		/* 8KB D-cache */
#define DLX_CACHE_LINE_SIZE	32		/* 32-byte cache lines */

/* Register definitions */
#define DLX_SP_REG		29		/* Stack pointer */
#define DLX_FP_REG		30		/* Frame pointer */
#define DLX_RA_REG		31		/* Return address */

/* CPU-specific functions */
extern void cpu_init(void);
extern void cpu_sleep(void);
extern unsigned int cpu_number(void);

/* SPL levels (interrupt priority levels) */
#define SPLOFF			0x0f		/* All interrupts off */
#define SPLHIGH			0x0e
#define SPLSCHED		0x08
#define SPLCLOCK		0x06
#define SPLVM			0x04
#define SPLTTY			0x02
#define SPLBIO			0x02
#define SPLNET			0x01
#define SPL0			0x00		/* All interrupts on */

#ifndef __ASSEMBLER__

/* SPL functions */
static inline int splhigh(void)
{
	extern unsigned int dlx_status_register;
	int old = dlx_status_register & 0x0f;
	dlx_status_register = (dlx_status_register & ~0x0f) | SPLHIGH;
	return old;
}

static inline int spl0(void)
{
	extern unsigned int dlx_status_register;
	int old = dlx_status_register & 0x0f;
	dlx_status_register = (dlx_status_register & ~0x0f) | SPL0;
	return old;
}

static inline void splx(int level)
{
	extern unsigned int dlx_status_register;
	dlx_status_register = (dlx_status_register & ~0x0f) | (level & 0x0f);
}

#endif /* !__ASSEMBLER__ */

#endif /* _DLX_CPU_H_ */
