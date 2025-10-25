/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha PALcode Definitions
 *
 * PALcode (Privileged Architecture Library code) provides the interface
 * between the Alpha hardware and the operating system. Different operating
 * systems use different PALcode variants:
 * - Windows NT PALcode
 * - UNIX (Digital UNIX/Tru64) PALcode
 * - OpenVMS PALcode
 */

#ifndef _ARCH_ALPHA_PAL_H_
#define _ARCH_ALPHA_PAL_H_

/*
 * Common PALcode function codes
 */

/* PALcode instruction format: CALL_PAL function */
#define PAL_CALL(func)		(0x00000000 | ((func) & 0x3f))

/*
 * Windows NT PALcode Functions
 */
#define PAL_NT_bpt		0x80	/* Breakpoint */
#define PAL_NT_bugchk		0x81	/* Bugcheck */
#define PAL_NT_callsys		0x83	/* System call */
#define PAL_NT_imb		0x86	/* I-stream memory barrier */
#define PAL_NT_gentrap		0xAA	/* Generate trap */
#define PAL_NT_rdteb		0xAB	/* Read thread environment block */
#define PAL_NT_kbpt		0xAC	/* Kernel breakpoint */
#define PAL_NT_callkd		0xAD	/* Call kernel debugger */
#define PAL_NT_halt		0x00	/* Halt processor (privileged) */
#define PAL_NT_restart		0x01	/* Restart processor (privileged) */
#define PAL_NT_draina		0x02	/* Drain aborts (privileged) */
#define PAL_NT_reboot		0x03	/* Reboot (privileged) */
#define PAL_NT_initpal		0x04	/* Initialize PALcode (privileged) */
#define PAL_NT_wrentry		0x05	/* Write system entry address (privileged) */
#define PAL_NT_swpirql		0x06	/* Swap IRQL (privileged) */
#define PAL_NT_rdirql		0x07	/* Read IRQL (privileged) */
#define PAL_NT_di		0x08	/* Disable interrupts (privileged) */
#define PAL_NT_ei		0x09	/* Enable interrupts (privileged) */
#define PAL_NT_swppal		0x0A	/* Swap PALcode (privileged) */
#define PAL_NT_ssir		0x0C	/* Set software interrupt request (privileged) */
#define PAL_NT_csir		0x0D	/* Clear software interrupt request (privileged) */
#define PAL_NT_rfe		0x0E	/* Return from exception (privileged) */
#define PAL_NT_retsys		0x0F	/* Return from system call (privileged) */
#define PAL_NT_swpctx		0x10	/* Swap privileged context (privileged) */
#define PAL_NT_swpprocess	0x11	/* Swap process (privileged) */
#define PAL_NT_rdmces		0x12	/* Read machine check error summary (privileged) */
#define PAL_NT_wrmces		0x13	/* Write machine check error summary (privileged) */
#define PAL_NT_tbia		0x14	/* TB invalidate all (privileged) */
#define PAL_NT_tbis		0x15	/* TB invalidate single (privileged) */
#define PAL_NT_dtbis		0x16	/* D-TB invalidate single (privileged) */
#define PAL_NT_rdksp		0x18	/* Read kernel stack pointer (privileged) */
#define PAL_NT_swpksp		0x19	/* Swap kernel stack pointer (privileged) */
#define PAL_NT_rdpsr		0x1A	/* Read processor status (privileged) */
#define PAL_NT_rdpcr		0x1C	/* Read processor control region (privileged) */
#define PAL_NT_rdthread		0x1E	/* Read thread pointer (privileged) */
#define PAL_NT_tbim		0x20	/* TB invalidate multiple (privileged) */
#define PAL_NT_tbimasn		0x21	/* TB invalidate multiple ASN (privileged) */
#define PAL_NT_rdcounters	0x30	/* Read performance counters */
#define PAL_NT_rdstate		0x31	/* Read system state */

/*
 * UNIX (Digital UNIX/Tru64) PALcode Functions
 */
#define PAL_UNIX_bpt		0x80	/* Breakpoint */
#define PAL_UNIX_bugchk		0x81	/* Bugcheck */
#define PAL_UNIX_callsys	0x83	/* System call */
#define PAL_UNIX_imb		0x86	/* I-stream memory barrier */
#define PAL_UNIX_urti		0x92	/* Return from user mode trap/interrupt */
#define PAL_UNIX_rdunique	0x9E	/* Read unique value */
#define PAL_UNIX_wrunique	0x9F	/* Write unique value */
#define PAL_UNIX_gentrap	0xAA	/* Generate trap */
#define PAL_UNIX_halt		0x00	/* Halt processor (privileged) */
#define PAL_UNIX_cflush		0x01	/* Cache flush (privileged) */
#define PAL_UNIX_draina		0x02	/* Drain aborts (privileged) */
#define PAL_UNIX_cserve		0x09	/* Console service (privileged) */
#define PAL_UNIX_swppal		0x0A	/* Swap PALcode (privileged) */
#define PAL_UNIX_wripir		0x0D	/* Write interprocessor interrupt request (privileged) */
#define PAL_UNIX_rdmces		0x10	/* Read machine check error summary (privileged) */
#define PAL_UNIX_wrmces		0x11	/* Write machine check error summary (privileged) */
#define PAL_UNIX_wrfen		0x2B	/* Write floating-point enable (privileged) */
#define PAL_UNIX_wrvptptr	0x2D	/* Write virtual page table pointer (privileged) */
#define PAL_UNIX_swpctx		0x30	/* Swap privileged context (privileged) */
#define PAL_UNIX_wrval		0x31	/* Write system value (privileged) */
#define PAL_UNIX_rdval		0x32	/* Read system value (privileged) */
#define PAL_UNIX_tbi		0x33	/* TB invalidate (privileged) */
#define PAL_UNIX_wrent		0x34	/* Write system entry address (privileged) */
#define PAL_UNIX_swpipl		0x35	/* Swap interrupt priority level (privileged) */
#define PAL_UNIX_rdps		0x36	/* Read processor status (privileged) */
#define PAL_UNIX_wrkgp		0x37	/* Write kernel global pointer (privileged) */
#define PAL_UNIX_wrusp		0x38	/* Write user stack pointer (privileged) */
#define PAL_UNIX_wrperfmon	0x39	/* Write performance monitor (privileged) */
#define PAL_UNIX_rdusp		0x3A	/* Read user stack pointer (privileged) */
#define PAL_UNIX_whami		0x3C	/* Who am I (privileged) */
#define PAL_UNIX_retsys		0x3D	/* Return from system call (privileged) */
#define PAL_UNIX_wtint		0x3E	/* Wait for interrupt (privileged) */
#define PAL_UNIX_rti		0x3F	/* Return from trap/interrupt (privileged) */

/*
 * OpenVMS PALcode Functions
 */
#define PAL_VMS_bpt		0x80	/* Breakpoint */
#define PAL_VMS_bugchk		0x81	/* Bugcheck */
#define PAL_VMS_chme		0x82	/* Change mode to executive */
#define PAL_VMS_chmk		0x83	/* Change mode to kernel */
#define PAL_VMS_chms		0x84	/* Change mode to supervisor */
#define PAL_VMS_chmu		0x85	/* Change mode to user */
#define PAL_VMS_imb		0x86	/* I-stream memory barrier */
#define PAL_VMS_insqhil		0x87	/* Insert into longword queue at head, interlocked */
#define PAL_VMS_insqtil		0x88	/* Insert into longword queue at tail, interlocked */
#define PAL_VMS_insqhiq		0x89	/* Insert into quadword queue at head, interlocked */
#define PAL_VMS_insqtiq		0x8A	/* Insert into quadword queue at tail, interlocked */
#define PAL_VMS_insquel		0x8B	/* Insert entry into longword queue */
#define PAL_VMS_insqueq		0x8C	/* Insert entry into quadword queue */
#define PAL_VMS_insquel_d	0x8D	/* Insert entry into longword queue, deferred */
#define PAL_VMS_insqueq_d	0x8E	/* Insert entry into quadword queue, deferred */
#define PAL_VMS_prober		0x8F	/* Probe for read access */
#define PAL_VMS_probew		0x90	/* Probe for write access */
#define PAL_VMS_rd_ps		0x91	/* Read processor status */
#define PAL_VMS_rei		0x92	/* Return from exception or interrupt */
#define PAL_VMS_remqhil		0x93	/* Remove from longword queue at head, interlocked */
#define PAL_VMS_remqtil		0x94	/* Remove from longword queue at tail, interlocked */
#define PAL_VMS_remqhiq		0x95	/* Remove from quadword queue at head, interlocked */
#define PAL_VMS_remqtiq		0x96	/* Remove from quadword queue at tail, interlocked */
#define PAL_VMS_remquel		0x97	/* Remove entry from longword queue */
#define PAL_VMS_remqueq		0x98	/* Remove entry from quadword queue */
#define PAL_VMS_remquel_d	0x99	/* Remove entry from longword queue, deferred */
#define PAL_VMS_remqueq_d	0x9A	/* Remove entry from quadword queue, deferred */
#define PAL_VMS_swasten		0x9B	/* Swap AST enable */
#define PAL_VMS_wr_ps_sw	0x9C	/* Write processor status software field */
#define PAL_VMS_rscc		0x9D	/* Read system cycle counter */
#define PAL_VMS_read_unq	0x9E	/* Read unique context */
#define PAL_VMS_write_unq	0x9F	/* Write unique context */
#define PAL_VMS_amovrr		0xA0	/* Atomic move from register to register */
#define PAL_VMS_amovrm		0xA1	/* Atomic move from register to memory */
#define PAL_VMS_insqhilr	0xA2	/* Insert into longword queue at head, interlocked, resident */
#define PAL_VMS_insqtilr	0xA3	/* Insert into longword queue at tail, interlocked, resident */
#define PAL_VMS_insqhiqr	0xA4	/* Insert into quadword queue at head, interlocked, resident */
#define PAL_VMS_insqtiqr	0xA5	/* Insert into quadword queue at tail, interlocked, resident */
#define PAL_VMS_remqhilr	0xA6	/* Remove from longword queue at head, interlocked, resident */
#define PAL_VMS_remqtilr	0xA7	/* Remove from longword queue at tail, interlocked, resident */
#define PAL_VMS_remqhiqr	0xA8	/* Remove from quadword queue at head, interlocked, resident */
#define PAL_VMS_remqtiqr	0xA9	/* Remove from quadword queue at tail, interlocked, resident */
#define PAL_VMS_gentrap		0xAA	/* Generate trap */
#define PAL_VMS_halt		0x00	/* Halt processor (privileged) */
#define PAL_VMS_cflush		0x01	/* Cache flush (privileged) */
#define PAL_VMS_draina		0x02	/* Drain aborts (privileged) */
#define PAL_VMS_ldqp		0x03	/* Load quadword physical (privileged) */
#define PAL_VMS_stqp		0x04	/* Store quadword physical (privileged) */
#define PAL_VMS_swpctx		0x05	/* Swap privileged context (privileged) */
#define PAL_VMS_mfpr_asn	0x06	/* Move from processor register ASN (privileged) */
#define PAL_VMS_mtpr_asten	0x07	/* Move to processor register ASTEN (privileged) */
#define PAL_VMS_mtpr_astsr	0x08	/* Move to processor register ASTSR (privileged) */
#define PAL_VMS_cserve		0x09	/* Console service (privileged) */
#define PAL_VMS_swppal		0x0A	/* Swap PALcode (privileged) */
#define PAL_VMS_mfpr_fen	0x0B	/* Move from processor register FEN (privileged) */
#define PAL_VMS_mtpr_fen	0x0C	/* Move to processor register FEN (privileged) */
#define PAL_VMS_mtpr_ipir	0x0D	/* Move to processor register IPIR (privileged) */
#define PAL_VMS_mfpr_ipl	0x0E	/* Move from processor register IPL (privileged) */
#define PAL_VMS_mtpr_ipl	0x0F	/* Move to processor register IPL (privileged) */
#define PAL_VMS_mfpr_mces	0x10	/* Move from processor register MCES (privileged) */
#define PAL_VMS_mtpr_mces	0x11	/* Move to processor register MCES (privileged) */
#define PAL_VMS_mfpr_pcbb	0x12	/* Move from processor register PCBB (privileged) */
#define PAL_VMS_mfpr_prbr	0x13	/* Move from processor register PRBR (privileged) */
#define PAL_VMS_mtpr_prbr	0x14	/* Move to processor register PRBR (privileged) */
#define PAL_VMS_mfpr_ptbr	0x15	/* Move from processor register PTBR (privileged) */
#define PAL_VMS_mfpr_scbb	0x16	/* Move from processor register SCBB (privileged) */
#define PAL_VMS_mtpr_scbb	0x17	/* Move to processor register SCBB (privileged) */
#define PAL_VMS_mtpr_sirr	0x18	/* Move to processor register SIRR (privileged) */
#define PAL_VMS_mfpr_sisr	0x19	/* Move from processor register SISR (privileged) */
#define PAL_VMS_mfpr_tbchk	0x1A	/* Move from processor register TBCHK (privileged) */
#define PAL_VMS_mtpr_tbia	0x1B	/* Move to processor register TBIA (privileged) */
#define PAL_VMS_mtpr_tbiap	0x1C	/* Move to processor register TBIAP (privileged) */
#define PAL_VMS_mtpr_tbis	0x1D	/* Move to processor register TBIS (privileged) */
#define PAL_VMS_mfpr_esp	0x1E	/* Move from processor register ESP (privileged) */
#define PAL_VMS_mtpr_esp	0x1F	/* Move to processor register ESP (privileged) */
#define PAL_VMS_mfpr_ssp	0x20	/* Move from processor register SSP (privileged) */
#define PAL_VMS_mtpr_ssp	0x21	/* Move to processor register SSP (privileged) */
#define PAL_VMS_mfpr_usp	0x22	/* Move from processor register USP (privileged) */
#define PAL_VMS_mtpr_usp	0x23	/* Move to processor register USP (privileged) */
#define PAL_VMS_mtpr_tbisd	0x24	/* Move to processor register TBISD (privileged) */
#define PAL_VMS_mtpr_tbisi	0x25	/* Move to processor register TBISI (privileged) */
#define PAL_VMS_mfpr_asten	0x26	/* Move from processor register ASTEN (privileged) */
#define PAL_VMS_mfpr_astsr	0x27	/* Move from processor register ASTSR (privileged) */
#define PAL_VMS_mfpr_vptb	0x29	/* Move from processor register VPTB (privileged) */
#define PAL_VMS_mtpr_vptb	0x2A	/* Move to processor register VPTB (privileged) */
#define PAL_VMS_mtpr_perfmon	0x2B	/* Move to processor register PERFMON (privileged) */
#define PAL_VMS_mtpr_datfx	0x2E	/* Move to processor register DATFX (privileged) */
#define PAL_VMS_mfpr_whami	0x3F	/* Move from processor register WHAMI (privileged) */

/*
 * PALcode exception and interrupt vectors
 */

/* Common exception vectors */
#define PAL_EXC_MCHK		0	/* Machine check */
#define PAL_EXC_ARITH		1	/* Arithmetic exception */
#define PAL_EXC_INTERRUPT	2	/* External interrupt */
#define PAL_EXC_DFAULT		3	/* D-stream memory management fault */
#define PAL_EXC_IFAULT		4	/* I-stream memory management fault */
#define PAL_EXC_UNALIGNED	5	/* Unaligned access */
#define PAL_EXC_OPCDEC		6	/* Opcode reserved for DEC */
#define PAL_EXC_FEN		7	/* Floating-point disabled */

/* PALcode exception frame offsets */
#define PAL_EXC_FRAME_SIZE	56	/* Size of exception frame in bytes */

#ifndef __ASSEMBLER__

/*
 * PALcode data structures
 */

/* PALcode exception frame */
struct alpha_pal_frame {
	unsigned long	exc_addr;	/* Exception PC */
	unsigned long	exc_sum;	/* Exception summary */
	unsigned long	exc_mask;	/* Exception mask */
	unsigned long	pal_base;	/* PALcode base address */
	unsigned long	iccsr;		/* Instruction cache control/status */
	unsigned long	pal_ps;		/* PALcode processor status */
	unsigned long	exc_cause;	/* Exception cause */
};

typedef struct alpha_pal_frame alpha_pal_frame_t;

/*
 * PALcode variant identification
 */
enum alpha_pal_variant {
	PAL_VARIANT_UNKNOWN = 0,
	PAL_VARIANT_NT = 1,		/* Windows NT PALcode */
	PAL_VARIANT_UNIX = 2,		/* UNIX (Digital UNIX/Tru64) PALcode */
	PAL_VARIANT_VMS = 3		/* OpenVMS PALcode */
};

#endif /* !__ASSEMBLER__ */

#endif /* _ARCH_ALPHA_PAL_H_ */
