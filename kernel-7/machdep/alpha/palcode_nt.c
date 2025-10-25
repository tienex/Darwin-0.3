/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Windows NT PALcode Support
 *
 * This file implements support for Windows NT PALcode.
 * NT PALcode is used by Windows NT on Alpha systems.
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

/*
 * Windows NT PALcode system calls
 */

/*
 * Swap IRQL (Interrupt Request Level)
 * Returns the old IRQL
 */
unsigned long
pal_nt_swpirql(unsigned long new_irql)
{
	unsigned long old_irql;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (old_irql)
		: "i" (PAL_NT_swpirql), "r" (new_irql)
		: "$0", "$16", "memory"
	);

	return old_irql;
}

/*
 * Read IRQL
 */
unsigned long
pal_nt_rdirql(void)
{
	unsigned long irql;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (irql)
		: "i" (PAL_NT_rdirql)
		: "$0"
	);

	return irql;
}

/*
 * Disable interrupts
 */
void
pal_nt_di(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_di)
		: "memory"
	);
}

/*
 * Enable interrupts
 */
void
pal_nt_ei(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_ei)
		: "memory"
	);
}

/*
 * I-stream memory barrier
 */
void
pal_nt_imb(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_imb)
		: "memory"
	);
}

/*
 * Drain aborts
 */
void
pal_nt_draina(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_draina)
		: "memory"
	);
}

/*
 * Initialize PALcode
 */
unsigned long
pal_nt_initpal(unsigned long pal_base, unsigned long entry1, unsigned long entry2, unsigned long entry3)
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
		: "i" (PAL_NT_initpal), "r" (pal_base), "r" (entry1), "r" (entry2), "r" (entry3)
		: "$0", "$16", "$17", "$18", "$19", "memory"
	);

	return result;
}

/*
 * Write system entry address
 */
void
pal_nt_wrentry(unsigned long entry)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (entry), "i" (PAL_NT_wrentry)
		: "$16", "memory"
	);
}

/*
 * Swap privileged context
 */
unsigned long
pal_nt_swpctx(unsigned long new_pcb)
{
	unsigned long old_pcb;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (old_pcb)
		: "i" (PAL_NT_swpctx), "r" (new_pcb)
		: "$0", "$16", "memory"
	);

	return old_pcb;
}

/*
 * Swap process
 */
void
pal_nt_swpprocess(unsigned long new_process)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (new_process), "i" (PAL_NT_swpprocess)
		: "$16", "memory"
	);
}

/*
 * Read machine check error summary
 */
unsigned long
pal_nt_rdmces(void)
{
	unsigned long mces;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (mces)
		: "i" (PAL_NT_rdmces)
		: "$0"
	);

	return mces;
}

/*
 * Write machine check error summary
 */
void
pal_nt_wrmces(unsigned long mces)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (mces), "i" (PAL_NT_wrmces)
		: "$16", "memory"
	);
}

/*
 * TLB invalidate all
 */
void
pal_nt_tbia(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_tbia)
		: "memory"
	);
}

/*
 * TLB invalidate single
 */
void
pal_nt_tbis(unsigned long va)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (va), "i" (PAL_NT_tbis)
		: "$16", "memory"
	);
}

/*
 * D-TLB invalidate single
 */
void
pal_nt_dtbis(unsigned long va)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (va), "i" (PAL_NT_dtbis)
		: "$16", "memory"
	);
}

/*
 * TLB invalidate multiple
 */
void
pal_nt_tbim(unsigned long count, unsigned long va_list)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"bis %1, %1, $17\n\t"
		"call_pal %2"
		: : "r" (count), "r" (va_list), "i" (PAL_NT_tbim)
		: "$16", "$17", "memory"
	);
}

/*
 * Read kernel stack pointer
 */
unsigned long
pal_nt_rdksp(void)
{
	unsigned long ksp;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (ksp)
		: "i" (PAL_NT_rdksp)
		: "$0"
	);

	return ksp;
}

/*
 * Swap kernel stack pointer
 */
unsigned long
pal_nt_swpksp(unsigned long new_ksp)
{
	unsigned long old_ksp;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (old_ksp)
		: "i" (PAL_NT_swpksp), "r" (new_ksp)
		: "$0", "$16", "memory"
	);

	return old_ksp;
}

/*
 * Read processor status
 */
unsigned long
pal_nt_rdpsr(void)
{
	unsigned long psr;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (psr)
		: "i" (PAL_NT_rdpsr)
		: "$0"
	);

	return psr;
}

/*
 * Read processor control region
 */
unsigned long
pal_nt_rdpcr(void)
{
	unsigned long pcr;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (pcr)
		: "i" (PAL_NT_rdpcr)
		: "$0"
	);

	return pcr;
}

/*
 * Read thread pointer
 */
unsigned long
pal_nt_rdthread(void)
{
	unsigned long thread;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (thread)
		: "i" (PAL_NT_rdthread)
		: "$0"
	);

	return thread;
}

/*
 * Read thread environment block
 */
unsigned long
pal_nt_rdteb(void)
{
	unsigned long teb;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (teb)
		: "i" (PAL_NT_rdteb)
		: "$0"
	);

	return teb;
}

/*
 * Return from exception
 */
void
pal_nt_rfe(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_rfe)
		: "memory"
	);
}

/*
 * Return from system call
 */
void
pal_nt_retsys(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_retsys)
		: "memory"
	);
}

/*
 * Halt processor
 */
void
pal_nt_halt(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_halt)
		: "memory"
	);
}

/*
 * Restart processor
 */
void
pal_nt_restart(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_restart)
		: "memory"
	);
}

/*
 * Reboot system
 */
void
pal_nt_reboot(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_NT_reboot)
		: "memory"
	);
}

/*
 * Set software interrupt request
 */
void
pal_nt_ssir(unsigned long mask)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (mask), "i" (PAL_NT_ssir)
		: "$16", "memory"
	);
}

/*
 * Clear software interrupt request
 */
void
pal_nt_csir(unsigned long mask)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (mask), "i" (PAL_NT_csir)
		: "$16", "memory"
	);
}

/*
 * Swap PALcode
 */
unsigned long
pal_nt_swppal(unsigned long new_pal, unsigned long arg1, unsigned long arg2)
{
	unsigned long result;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"bis %3, %3, $17\n\t"
		"bis %4, %4, $18\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "i" (PAL_NT_swppal), "r" (new_pal), "r" (arg1), "r" (arg2)
		: "$0", "$16", "$17", "$18", "memory"
	);

	return result;
}
