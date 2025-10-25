/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha OpenVMS PALcode Support
 *
 * This file implements support for OpenVMS PALcode.
 * VMS PALcode is the most complex variant, providing extensive
 * queue management and system service instructions.
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

/*
 * OpenVMS PALcode system calls
 */

/*
 * I-stream memory barrier
 */
void
pal_vms_imb(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_VMS_imb)
		: "memory"
	);
}

/*
 * Drain aborts
 */
void
pal_vms_draina(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_VMS_draina)
		: "memory"
	);
}

/*
 * Read processor status
 */
unsigned long
pal_vms_rd_ps(void)
{
	unsigned long ps;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (ps)
		: "i" (PAL_VMS_rd_ps)
		: "$0"
	);

	return ps;
}

/*
 * Write processor status software field
 */
void
pal_vms_wr_ps_sw(unsigned long ps_sw)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (ps_sw), "i" (PAL_VMS_wr_ps_sw)
		: "$16", "memory"
	);
}

/*
 * Swap privileged context
 */
unsigned long
pal_vms_swpctx(unsigned long new_pcb)
{
	unsigned long old_pcb;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (old_pcb)
		: "i" (PAL_VMS_swpctx), "r" (new_pcb)
		: "$0", "$16", "memory"
	);

	return old_pcb;
}

/*
 * Load quadword physical
 */
unsigned long
pal_vms_ldqp(unsigned long pa)
{
	unsigned long val;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (val)
		: "i" (PAL_VMS_ldqp), "r" (pa)
		: "$0", "$16", "memory"
	);

	return val;
}

/*
 * Store quadword physical
 */
void
pal_vms_stqp(unsigned long pa, unsigned long val)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"bis %1, %1, $17\n\t"
		"call_pal %2"
		: : "r" (pa), "r" (val), "i" (PAL_VMS_stqp)
		: "$16", "$17", "memory"
	);
}

/*
 * Move from processor register - ASN
 */
unsigned long
pal_vms_mfpr_asn(void)
{
	unsigned long asn;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (asn)
		: "i" (PAL_VMS_mfpr_asn)
		: "$0"
	);

	return asn;
}

/*
 * Move to processor register - ASTEN
 */
void
pal_vms_mtpr_asten(unsigned long asten)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (asten), "i" (PAL_VMS_mtpr_asten)
		: "$16", "memory"
	);
}

/*
 * Move to processor register - ASTSR
 */
void
pal_vms_mtpr_astsr(unsigned long astsr)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (astsr), "i" (PAL_VMS_mtpr_astsr)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - IPL
 */
unsigned long
pal_vms_mfpr_ipl(void)
{
	unsigned long ipl;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (ipl)
		: "i" (PAL_VMS_mfpr_ipl)
		: "$0"
	);

	return ipl;
}

/*
 * Move to processor register - IPL
 */
void
pal_vms_mtpr_ipl(unsigned long ipl)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (ipl), "i" (PAL_VMS_mtpr_ipl)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - MCES
 */
unsigned long
pal_vms_mfpr_mces(void)
{
	unsigned long mces;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (mces)
		: "i" (PAL_VMS_mfpr_mces)
		: "$0"
	);

	return mces;
}

/*
 * Move to processor register - MCES
 */
void
pal_vms_mtpr_mces(unsigned long mces)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (mces), "i" (PAL_VMS_mtpr_mces)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - PCBB
 */
unsigned long
pal_vms_mfpr_pcbb(void)
{
	unsigned long pcbb;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (pcbb)
		: "i" (PAL_VMS_mfpr_pcbb)
		: "$0"
	);

	return pcbb;
}

/*
 * Move from processor register - PRBR
 */
unsigned long
pal_vms_mfpr_prbr(void)
{
	unsigned long prbr;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (prbr)
		: "i" (PAL_VMS_mfpr_prbr)
		: "$0"
	);

	return prbr;
}

/*
 * Move to processor register - PRBR
 */
void
pal_vms_mtpr_prbr(unsigned long prbr)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (prbr), "i" (PAL_VMS_mtpr_prbr)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - PTBR
 */
unsigned long
pal_vms_mfpr_ptbr(void)
{
	unsigned long ptbr;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (ptbr)
		: "i" (PAL_VMS_mfpr_ptbr)
		: "$0"
	);

	return ptbr;
}

/*
 * Move from processor register - SCBB
 */
unsigned long
pal_vms_mfpr_scbb(void)
{
	unsigned long scbb;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (scbb)
		: "i" (PAL_VMS_mfpr_scbb)
		: "$0"
	);

	return scbb;
}

/*
 * Move to processor register - SCBB
 */
void
pal_vms_mtpr_scbb(unsigned long scbb)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (scbb), "i" (PAL_VMS_mtpr_scbb)
		: "$16", "memory"
	);
}

/*
 * Move to processor register - SIRR
 */
void
pal_vms_mtpr_sirr(unsigned long sirr)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (sirr), "i" (PAL_VMS_mtpr_sirr)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - SISR
 */
unsigned long
pal_vms_mfpr_sisr(void)
{
	unsigned long sisr;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (sisr)
		: "i" (PAL_VMS_mfpr_sisr)
		: "$0"
	);

	return sisr;
}

/*
 * Move to processor register - TBIA (TLB invalidate all)
 */
void
pal_vms_mtpr_tbia(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_VMS_mtpr_tbia)
		: "memory"
	);
}

/*
 * Move to processor register - TBIS (TLB invalidate single)
 */
void
pal_vms_mtpr_tbis(unsigned long va)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (va), "i" (PAL_VMS_mtpr_tbis)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - ESP (executive stack pointer)
 */
unsigned long
pal_vms_mfpr_esp(void)
{
	unsigned long esp;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (esp)
		: "i" (PAL_VMS_mfpr_esp)
		: "$0"
	);

	return esp;
}

/*
 * Move to processor register - ESP
 */
void
pal_vms_mtpr_esp(unsigned long esp)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (esp), "i" (PAL_VMS_mtpr_esp)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - SSP (supervisor stack pointer)
 */
unsigned long
pal_vms_mfpr_ssp(void)
{
	unsigned long ssp;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (ssp)
		: "i" (PAL_VMS_mfpr_ssp)
		: "$0"
	);

	return ssp;
}

/*
 * Move to processor register - SSP
 */
void
pal_vms_mtpr_ssp(unsigned long ssp)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (ssp), "i" (PAL_VMS_mtpr_ssp)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - USP (user stack pointer)
 */
unsigned long
pal_vms_mfpr_usp(void)
{
	unsigned long usp;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (usp)
		: "i" (PAL_VMS_mfpr_usp)
		: "$0"
	);

	return usp;
}

/*
 * Move to processor register - USP
 */
void
pal_vms_mtpr_usp(unsigned long usp)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (usp), "i" (PAL_VMS_mtpr_usp)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - VPTB (virtual page table base)
 */
unsigned long
pal_vms_mfpr_vptb(void)
{
	unsigned long vptb;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (vptb)
		: "i" (PAL_VMS_mfpr_vptb)
		: "$0"
	);

	return vptb;
}

/*
 * Move to processor register - VPTB
 */
void
pal_vms_mtpr_vptb(unsigned long vptb)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (vptb), "i" (PAL_VMS_mtpr_vptb)
		: "$16", "memory"
	);
}

/*
 * Move from processor register - WHAMI (CPU ID)
 */
unsigned long
pal_vms_mfpr_whami(void)
{
	unsigned long whami;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (whami)
		: "i" (PAL_VMS_mfpr_whami)
		: "$0"
	);

	return whami;
}

/*
 * Return from exception or interrupt
 */
void
pal_vms_rei(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_VMS_rei)
		: "memory"
	);
}

/*
 * Probe for read access
 */
unsigned long
pal_vms_prober(unsigned long mode, unsigned long len, unsigned long addr)
{
	unsigned long result;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"bis %3, %3, $17\n\t"
		"bis %4, %4, $18\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "i" (PAL_VMS_prober), "r" (mode), "r" (len), "r" (addr)
		: "$0", "$16", "$17", "$18", "memory"
	);

	return result;
}

/*
 * Probe for write access
 */
unsigned long
pal_vms_probew(unsigned long mode, unsigned long len, unsigned long addr)
{
	unsigned long result;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"bis %3, %3, $17\n\t"
		"bis %4, %4, $18\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "i" (PAL_VMS_probew), "r" (mode), "r" (len), "r" (addr)
		: "$0", "$16", "$17", "$18", "memory"
	);

	return result;
}

/*
 * Read system cycle counter
 */
unsigned long
pal_vms_rscc(void)
{
	unsigned long scc;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (scc)
		: "i" (PAL_VMS_rscc)
		: "$0"
	);

	return scc;
}

/*
 * Read unique context
 */
unsigned long
pal_vms_read_unq(void)
{
	unsigned long unq;

	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (unq)
		: "i" (PAL_VMS_read_unq)
		: "$0"
	);

	return unq;
}

/*
 * Write unique context
 */
void
pal_vms_write_unq(unsigned long unq)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (unq), "i" (PAL_VMS_write_unq)
		: "$16", "memory"
	);
}

/*
 * Console service
 */
unsigned long
pal_vms_cserve(unsigned long cmd, unsigned long arg1, unsigned long arg2)
{
	unsigned long result;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"bis %3, %3, $17\n\t"
		"bis %4, %4, $18\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "i" (PAL_VMS_cserve), "r" (cmd), "r" (arg1), "r" (arg2)
		: "$0", "$16", "$17", "$18", "memory"
	);

	return result;
}

/*
 * Swap PALcode
 */
unsigned long
pal_vms_swppal(unsigned long new_pal, unsigned long arg1, unsigned long arg2)
{
	unsigned long result;

	__asm__ volatile (
		"bis %2, %2, $16\n\t"
		"bis %3, %3, $17\n\t"
		"bis %4, %4, $18\n\t"
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "i" (PAL_VMS_swppal), "r" (new_pal), "r" (arg1), "r" (arg2)
		: "$0", "$16", "$17", "$18", "memory"
	);

	return result;
}

/*
 * Cache flush
 */
void
pal_vms_cflush(unsigned long pfn)
{
	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"call_pal %1"
		: : "r" (pfn), "i" (PAL_VMS_cflush)
		: "$16", "memory"
	);
}

/*
 * Halt processor
 */
void
pal_vms_halt(void)
{
	__asm__ volatile (
		"call_pal %0"
		: : "i" (PAL_VMS_halt)
		: "memory"
	);
}
