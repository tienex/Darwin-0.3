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
 * MIPS signal handling definitions
 */

#ifndef _MIPS_SIGNAL_H_
#define	_MIPS_SIGNAL_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_OBSOLETE

/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.  It is also made available
 * to the handler to allow it to restore state properly if
 * a non-standard exit is performed.
 */
#if defined(_MIPS64) || defined(__mips64)
struct	sigcontext {
	int	sc_onstack;		/* sigstack state to restore */
	int	sc_mask;		/* signal mask to restore */
	unsigned long long sc_pc;	/* pc at time of signal */
	unsigned long long sc_regs[32];	/* processor registers */
	int	sc_fpc_csr;		/* FPU control/status register */
	int	sc_fpc_eir;		/* FPU exception instruction register */
	int	sc_xxx[2];		/* reserved */
	unsigned long long sc_mdhi;	/* hi register */
	unsigned long long sc_mdlo;	/* lo register */
	int	sc_cause;		/* CP0 cause register */
	int	sc_badvaddr;		/* CP0 bad virtual address */
	int	sc_badpaddr;		/* CPU physical address */
};
#else
struct	sigcontext {
	int	sc_onstack;		/* sigstack state to restore */
	int	sc_mask;		/* signal mask to restore */
	int	sc_pc;			/* pc at time of signal */
	int	sc_regs[32];		/* processor registers */
	int	sc_fpc_csr;		/* FPU control/status register */
	int	sc_fpc_eir;		/* FPU exception instruction register */
	int	sc_xxx[2];		/* reserved */
	int	sc_mdhi;		/* hi register */
	int	sc_mdlo;		/* lo register */
	int	sc_cause;		/* CP0 cause register */
	int	sc_badvaddr;		/* CP0 bad virtual address */
	int	sc_badpaddr;		/* CPU physical address */
};
#endif

#endif /* __APPLE_API_OBSOLETE */

#endif	/* _MIPS_SIGNAL_H_ */
