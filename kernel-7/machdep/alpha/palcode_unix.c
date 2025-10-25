/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha UNIX (Digital UNIX/Tru64) PALcode Support
 *
 * This file implements support for UNIX PALcode, which is the most
 * appropriate variant for Darwin on Alpha.
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

/*
 * UNIX PALcode system calls
 */

/*
 * Swap interrupt priority level
 * Returns the old IPL
 */
unsigned long
pal_unix_swpipl(unsigned long new_ipl)
{
	unsigned long old_ipl;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (old_ipl)
		: "i" (PAL_UNIX_swpipl), "r" (new_ipl)
		: "$0", "$16", "memory"
	);

	return old_ipl;
}

/*
 * Read processor status
 */
unsigned long
pal_unix_rdps(void)
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
 * Write user stack pointer
 */
void
pal_unix_wrusp(unsigned long usp)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (usp), "i" (PAL_UNIX_wrusp)
		: "$16", "memory"
	);
}

/*
 * Read user stack pointer
 */
unsigned long
pal_unix_rdusp(void)
{
	unsigned long usp;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (usp)
		: "i" (PAL_UNIX_rdusp)
		: "$0"
	);

	return usp;
}

/*
 * Who am I (return CPU number)
 */
unsigned long
pal_unix_whami(void)
{
	unsigned long cpu_id;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (cpu_id)
		: "i" (PAL_UNIX_whami)
		: "$0"
	);

	return cpu_id;
}

/*
 * I-stream memory barrier
 * Ensures instruction cache coherency after code modification
 */
void
pal_unix_imb(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_UNIX_imb)
		: "memory"
	);
}

/*
 * Drain aborts
 * Ensures all pending memory accesses complete
 */
void
pal_unix_draina(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_UNIX_draina)
		: "memory"
	);
}

/*
 * TLB invalidate
 */
void
pal_unix_tbi(unsigned long type, unsigned long arg)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"bis %1, %1, $17\n\t"
		"call_pal %2"
		: : "r" (type), "r" (arg), "i" (PAL_UNIX_tbi)
		: "$16", "$17", "memory"
	);
}

/*
 * Swap privileged context
 * Used for process/thread switching
 */
unsigned long
pal_unix_swpctx(unsigned long new_pcb)
{
	unsigned long old_pcb;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (old_pcb)
		: "i" (PAL_UNIX_swpctx), "r" (new_pcb)
		: "$0", "$16", "memory"
	);

	return old_pcb;
}

/*
 * Write system entry address
 * Sets up exception/interrupt vectors
 */
void
pal_unix_wrent(unsigned long entry, unsigned long type)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"bis %1, %1, $17\n\t"
		"call_pal %2"
		: : "r" (entry), "r" (type), "i" (PAL_UNIX_wrent)
		: "$16", "$17", "memory"
	);
}

/*
 * Write kernel global pointer
 */
void
pal_unix_wrkgp(unsigned long kgp)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (kgp), "i" (PAL_UNIX_wrkgp)
		: "$16", "memory"
	);
}

/*
 * Write virtual page table pointer
 */
void
pal_unix_wrvptptr(unsigned long vptptr)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (vptptr), "i" (PAL_UNIX_wrvptptr)
		: "$16", "memory"
	);
}

/*
 * Read machine check error summary
 */
unsigned long
pal_unix_rdmces(void)
{
	unsigned long mces;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (mces)
		: "i" (PAL_UNIX_rdmces)
		: "$0"
	);

	return mces;
}

/*
 * Write machine check error summary
 */
void
pal_unix_wrmces(unsigned long mces)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (mces), "i" (PAL_UNIX_wrmces)
		: "$16", "memory"
	);
}

/*
 * Write floating-point enable
 */
void
pal_unix_wrfen(unsigned long fen)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (fen), "i" (PAL_UNIX_wrfen)
		: "$16", "memory"
	);
}

/*
 * Read system value
 */
unsigned long
pal_unix_rdval(void)
{
	unsigned long val;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (val)
		: "i" (PAL_UNIX_rdval)
		: "$0"
	);

	return val;
}

/*
 * Write system value
 */
void
pal_unix_wrval(unsigned long val)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (val), "i" (PAL_UNIX_wrval)
		: "$16", "memory"
	);
}

/*
 * Console service
 */
unsigned long
pal_unix_cserve(unsigned long cmd, unsigned long arg1, unsigned long arg2)
{
	unsigned long result;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"bis %3, %3, $17\n\t"
		"bis %4, %4, $18\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "i" (PAL_UNIX_cserve), "r" (cmd), "r" (arg1), "r" (arg2)
		: "$0", "$16", "$17", "$18", "memory"
	);

	return result;
}

/*
 * Return from trap/interrupt
 * This is called by the kernel to return from an exception or interrupt
 */
void
pal_unix_rti(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_UNIX_rti)
		: "memory"
	);
}

/*
 * Return from system call
 */
void
pal_unix_retsys(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_UNIX_retsys)
		: "memory"
	);
}

/*
 * Halt processor
 */
void
pal_unix_halt(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_UNIX_halt)
		: "memory"
	);
}

/*
 * Cache flush
 */
void
pal_unix_cflush(unsigned long pfn)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (pfn), "i" (PAL_UNIX_cflush)
		: "$16", "memory"
	);
}

/*
 * Swap PALcode
 * Load a new PALcode image
 */
unsigned long
pal_unix_swppal(unsigned long new_pal, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
	unsigned long result;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"bis %3, %3, $17\n\t"
		"bis %4, %4, $18\n\t"
		"bis %5, %5, $19\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "i" (PAL_UNIX_swppal), "r" (new_pal), "r" (arg1), "r" (arg2), "r" (arg3)
		: "$0", "$16", "$17", "$18", "$19", "memory"
	);

	return result;
}

/*
 * Read unique value (thread-specific value)
 */
unsigned long
pal_unix_rdunique(void)
{
	unsigned long val;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (val)
		: "i" (PAL_UNIX_rdunique)
		: "$0"
	);

	return val;
}

/*
 * Write unique value
 */
void
pal_unix_wrunique(unsigned long val)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (val), "i" (PAL_UNIX_wrunique)
		: "$16", "memory"
	);
}
