/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha CPU Definitions
 */

#ifndef _ARCH_ALPHA_CPU_H_
#define _ARCH_ALPHA_CPU_H_

#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

/*
 * Alpha CPU Types and Subtypes
 */

/* CPU Type (from mach/machine.h) */
#define CPU_TYPE_ALPHA		((cpu_type_t) 10)

/* CPU Subtypes */
#define CPU_SUBTYPE_ALPHA_ALL		((cpu_subtype_t) 0)
#define CPU_SUBTYPE_ALPHA_EV3		((cpu_subtype_t) 1)	/* 21064 */
#define CPU_SUBTYPE_ALPHA_EV4		((cpu_subtype_t) 2)	/* 21064A */
#define CPU_SUBTYPE_ALPHA_EV5		((cpu_subtype_t) 3)	/* 21164 */
#define CPU_SUBTYPE_ALPHA_EV45		((cpu_subtype_t) 4)	/* 21064A */
#define CPU_SUBTYPE_ALPHA_LCA		((cpu_subtype_t) 5)	/* 21066 */
#define CPU_SUBTYPE_ALPHA_EV56		((cpu_subtype_t) 6)	/* 21164A */
#define CPU_SUBTYPE_ALPHA_EV6		((cpu_subtype_t) 7)	/* 21264 */
#define CPU_SUBTYPE_ALPHA_PCA56		((cpu_subtype_t) 8)	/* 21164PC */
#define CPU_SUBTYPE_ALPHA_EV67		((cpu_subtype_t) 9)	/* 21264A */

/*
 * Alpha implementation variants (chip generations)
 */
#define ALPHA_IMPL_EV3		0	/* 21064 - original Alpha */
#define ALPHA_IMPL_EV4		1	/* 21064A - improved EV3 */
#define ALPHA_IMPL_LCA		2	/* 21066 - low cost Alpha */
#define ALPHA_IMPL_EV45		3	/* 21064A with BWX */
#define ALPHA_IMPL_EV5		4	/* 21164 - second generation */
#define ALPHA_IMPL_EV56		5	/* 21164A with BWX, MVI */
#define ALPHA_IMPL_PCA56	6	/* 21164PC - embedded */
#define ALPHA_IMPL_EV6		7	/* 21264 - third generation */
#define ALPHA_IMPL_EV67		8	/* 21264A with CIX */

/*
 * Alpha instruction set extensions
 */
#define ALPHA_EXT_BWX		0x01	/* Byte/word extension */
#define ALPHA_EXT_FIX		0x02	/* Floating point convert extension */
#define ALPHA_EXT_CIX		0x04	/* Count extension */
#define ALPHA_EXT_MVI		0x08	/* Motion video instructions */
#define ALPHA_EXT_PAT		0x10	/* Precision architecture */

/*
 * Cache properties (implementation-specific)
 */

/* EV4 (21064A) caches */
#define ALPHA_EV4_ICACHE_SIZE	8192
#define ALPHA_EV4_ICACHE_LINE	32
#define ALPHA_EV4_DCACHE_SIZE	8192
#define ALPHA_EV4_DCACHE_LINE	32

/* EV5 (21164) caches */
#define ALPHA_EV5_ICACHE_SIZE	8192
#define ALPHA_EV5_ICACHE_LINE	32
#define ALPHA_EV5_DCACHE_SIZE	8192
#define ALPHA_EV5_DCACHE_LINE	32
#define ALPHA_EV5_SCACHE_SIZE	(96*1024)
#define ALPHA_EV5_SCACHE_LINE	64

/* EV6 (21264) caches */
#define ALPHA_EV6_ICACHE_SIZE	65536
#define ALPHA_EV6_ICACHE_LINE	64
#define ALPHA_EV6_DCACHE_SIZE	65536
#define ALPHA_EV6_DCACHE_LINE	64

/*
 * Page size
 */
#define ALPHA_PGSHIFT		13		/* 8K pages */
#define ALPHA_PGBYTES		(1 << ALPHA_PGSHIFT)
#define ALPHA_PGMASK		(ALPHA_PGBYTES - 1)

/*
 * Segment granularity
 */
#define ALPHA_SEG_SHIFT		43		/* 8TB segments (for 3-level page tables) */

/*
 * Address space layout
 */
#define ALPHA_KSEG_START	0xfffffc0000000000UL	/* Kernel segment (cached) */
#define ALPHA_KSEG_END		0xfffffe0000000000UL
#define ALPHA_K0SEG_START	ALPHA_KSEG_START	/* Alias for kernel segment */
#define ALPHA_K1SEG_START	0xfffffe0000000000UL	/* I/O segment (uncached) */
#define ALPHA_K1SEG_END		0xffffffff00000000UL
#define ALPHA_USEG_START	0x0000000000000000UL	/* User segment */
#define ALPHA_USEG_END		0x0000400000000000UL

/*
 * Interrupt Priority Levels
 */
#define ALPHA_IPL_0		0	/* IPL 0: normal execution */
#define ALPHA_IPL_SOFT		1	/* Software interrupts */
#define ALPHA_IPL_IO		4	/* I/O device interrupts */
#define ALPHA_IPL_CLOCK		6	/* Clock interrupt */
#define ALPHA_IPL_HIGH		7	/* High priority, all interrupts disabled */

/*
 * Processor internal registers (PALtemp)
 */
#define ALPHA_PALTEMP_SIZE	24	/* 24 PALtemp registers */

#ifndef __ASSEMBLER__

#include <stdint.h>

/*
 * Alpha CPU information structure
 */
struct alpha_cpu_info {
	uint64_t	implementation;		/* Implementation type (EV4, EV5, EV6, etc.) */
	uint64_t	extensions;		/* Instruction set extensions */
	uint64_t	pal_variant;		/* PALcode variant (NT, UNIX, VMS) */
	uint64_t	pal_revision;		/* PALcode revision */
	uint64_t	cpu_serial[2];		/* CPU serial number */
	uint64_t	icache_size;		/* I-cache size */
	uint64_t	dcache_size;		/* D-cache size */
	uint64_t	scache_size;		/* S-cache size (if present) */
};

typedef struct alpha_cpu_info alpha_cpu_info_t;

/*
 * Memory barrier and synchronization primitives
 */

/* Memory barrier */
static inline void alpha_mb(void)
{
	__asm__ volatile ("mb" : : : "memory");
}

/* Write memory barrier */
static inline void alpha_wmb(void)
{
	__asm__ volatile ("wmb" : : : "memory");
}

/* I-stream memory barrier */
static inline void alpha_imb(void)
{
	__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_imb) : "memory");
}

/* Drain aborts */
static inline void alpha_draina(void)
{
	__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_draina) : "memory");
}

/*
 * Interrupt control
 */

/* Swap IPL (interrupt priority level) */
static inline unsigned long alpha_swpipl(unsigned long ipl)
{
	unsigned long old_ipl;
	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (old_ipl)
		: "i" (PAL_UNIX_swpipl), "0" (ipl)
		: "$0", "memory"
	);
	return old_ipl;
}

/* Read processor status */
static inline unsigned long alpha_rdps(void)
{
	unsigned long ps;
	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (ps)
		: "i" (PAL_UNIX_rdps)
		: "$0"
	);
	return ps;
}

/*
 * Atomic operations
 */

/* Load locked quadword */
static inline unsigned long alpha_ldq_l(unsigned long *addr)
{
	unsigned long val;
	__asm__ volatile ("ldq_l %0, 0(%1)" : "=r" (val) : "r" (addr) : "memory");
	return val;
}

/* Store conditional quadword */
static inline unsigned long alpha_stq_c(unsigned long val, unsigned long *addr)
{
	unsigned long result;
	__asm__ volatile ("stq_c %1, 0(%2)\n\tbis %1, %1, %0"
			  : "=r" (result)
			  : "r" (val), "r" (addr)
			  : "memory");
	return result;
}

/*
 * Processor cycle counter
 */
static inline unsigned long alpha_rpcc(void)
{
	unsigned long result;
	__asm__ volatile ("rpcc %0" : "=r" (result));
	return result;
}

#endif /* !__ASSEMBLER__ */

#endif /* _ARCH_ALPHA_CPU_H_ */
