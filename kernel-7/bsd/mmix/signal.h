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
 * Machine specific signal information.
 *
 * MMIX signal handling structures.
 */

#ifndef	_MMIX_SIGNAL_
#define	_MMIX_SIGNAL_ 1

typedef long long sig_atomic_t;

/*
 * Machine-dependant flags used in sigvec call.
 */
#define	SV_SAVE_REGS	0x1000	/* Save all regs in sigcontext */

/*
 * regs_saved_t -- Describes which registers beyond what the kernel cares
 *		   about are saved to and restored from this sigcontext.
 *
 * The default is REGS_SAVED_CALLER, only the caller saved registers
 * are saved.  If the SV_SAVE_REGS flag was set when the signal
 * handler was registered with sigvec() then all the registers will be
 * saved in the sigcontext, and REGS_SAVED_ALL will be set.  The C
 * library uses REGS_SAVED_NONE in order to quickly restore kernel
 * state during a longjmp().
 */
typedef enum {
	REGS_SAVED_NONE,		/* Only kernel managed regs restored */
	REGS_SAVED_CALLER,		/* "Caller saved" regs: pc, $0-$15, $24-$31 */
	REGS_SAVED_ALL			/* All registers including $16-$23 */
} regs_saved_t;

/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.  It is also made available
 * to the handler to allow it to properly restore state if
 * a non-standard exit is performed.
 *
 * For MMIX, all values are 64-bit.
 */
struct sigcontext {
    long long	sc_onstack;     /* sigstack state to restore */
    long long	sc_mask;        /* signal mask to restore */
	long long	sc_pc;		/* program counter (rW) */
    long long	sc_ps;          /* processor status */
    long long	sc_sp;      	/* stack pointer ($254) if sc_regs == NULL */
	void	*sc_regs;	/* (kernel private) saved state */
};

/*
 * MMIX-specific signal codes
 */
#define	ILL_RESAD_FAULT		0x0	/* reserved addressing fault */
#define	ILL_PRIVIN_FAULT	0x1	/* privileged instruction fault */
#define	ILL_RESOP_FAULT		0x2	/* reserved operand fault */

/* codes for SIGFPE */
#define	FPE_INTOVF_TRAP		0x1	/* integer overflow */
#define	FPE_INTDIV_TRAP		0x2	/* integer divide by zero */
#define	FPE_FLTOVF_TRAP		0x3	/* floating overflow */
#define	FPE_FLTDIV_TRAP		0x4	/* floating/decimal divide by zero */
#define	FPE_FLTUND_TRAP		0x5	/* floating underflow */
#define	FPE_DECOVF_TRAP		0x6	/* decimal overflow */
#define	FPE_SUBRNG_TRAP		0x7	/* subscript out of range */
#define	FPE_FLTOVF_FAULT	0x8	/* floating overflow fault */
#define	FPE_FLTDIV_FAULT	0x9	/* divide by zero floating fault */
#define	FPE_FLTUND_FAULT	0xa	/* floating underflow fault */

#endif /* _MMIX_SIGNAL_ */
