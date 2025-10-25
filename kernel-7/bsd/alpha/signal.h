/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Signal Handling Definitions
 */

#ifndef _BSD_ALPHA_SIGNAL_H_
#define _BSD_ALPHA_SIGNAL_H_

#include <architecture/alpha/reg.h>

/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.
 */
struct sigcontext {
	long	sc_onstack;		/* sigstack state to restore */
	long	sc_mask;		/* signal mask to restore */
	long	sc_pc;			/* pc at time of signal */
	long	sc_ps;			/* processor status at time of signal */
	long	sc_regs[32];		/* integer register set (r0-r31) */
	long	sc_ownedfp;		/* fp has been used */
	long	sc_fpregs[32];		/* FP register set (f0-f31) */
	unsigned long sc_fpcr;		/* FP control register */
	unsigned long sc_fp_control;	/* FP software control word */
	long	sc_reserved1;		/* reserved for kernel */
	long	sc_reserved2;		/* reserved for kernel */
	long	sc_ssize;		/* stack size */
	caddr_t	sc_sbase;		/* stack base */
	long	sc_traparg_a0;		/* trap argument a0 */
	long	sc_traparg_a1;		/* trap argument a1 */
	long	sc_traparg_a2;		/* trap argument a2 */
	long	sc_fp_trap_pc;		/* FP trap PC */
	long	sc_fp_trigger_sum;	/* FP trigger summary */
	long	sc_fp_trigger_inst;	/* FP trigger instruction */
};

#endif /* _BSD_ALPHA_SIGNAL_H_ */
