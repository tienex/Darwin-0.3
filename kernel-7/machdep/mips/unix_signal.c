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
 * MIPS:	Unix signal delivery
 *
 * Implements signal delivery mechanism for MIPS architecture.
 * Builds signal frame on user stack and sets up handler execution.
 */

#include <mach/mach_types.h>
#include <mach/exception.h>

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/signal.h>

#include <machdep/mips/thread.h>
#include <machdep/mips/pcb.h>

/*
 * Signal context structure
 * Saved on user stack when signal is delivered
 */
struct sigcontext {
	int		sc_onstack;	/* Saved onstack flag */
	int		sc_mask;	/* Saved signal mask */
	unsigned int	sc_pc;		/* Saved program counter */
	unsigned int	sc_regs[32];	/* Saved general registers */
	unsigned int	sc_mdlo;	/* Saved multiply lo */
	unsigned int	sc_mdhi;	/* Saved multiply hi */
	unsigned int	sc_status;	/* Saved SR register */
	unsigned int	sc_cause;	/* Saved Cause register */
};

/*
 * Signal frame structure
 * Pushed on user stack for signal handler
 */
struct sigframe {
	int			sig;		/* Signal number */
	int			code;		/* Signal code (for SIGILL, SIGFPE) */
	struct sigcontext	*scp;		/* Pointer to sigcontext */
	struct sigcontext	sc;		/* Actual sigcontext */
	unsigned int		retcode[2];	/* Signal trampoline code */
};

/*
 * Send signal to process
 *
 * Build signal frame on user stack and set up signal handler call.
 * Stack layout after this function:
 *   +------------------+ <- old sp
 *   | signal frame     |
 *   |  - sig number    |
 *   |  - code          |
 *   |  - scp pointer   |
 *   |  - sigcontext    |
 *   |  - trampoline    |
 *   +------------------+ <- new sp
 *
 * On return to user mode:
 *   - PC points to signal handler
 *   - a0 = signal number
 *   - a1 = signal code
 *   - a2 = pointer to sigcontext
 *   - ra = signal trampoline (calls sigreturn)
 */
void
sendsig(struct proc *p, sig_t catcher, int sig, int mask, u_long code)
{
	struct sigframe frame;
	struct sigframe *fp;
	struct sigcontext *scp;
	struct sigacts *ps = p->p_sigacts;
	int oonstack;
	thread_t thread = current_thread();
	thread_saved_state_t *saved_state = USER_REGS(thread);
	unsigned int sp;

	/* Determine if currently on signal stack */
	oonstack = ps->ps_sigstk.ss_flags & SA_ONSTACK;

	/* Decide where to put signal frame */
	if ((ps->ps_flags & SAS_ALTSTACK) && !oonstack &&
	    (ps->ps_sigonstack & sigmask(sig))) {
		/* Use alternate signal stack */
		fp = (struct sigframe *)(ps->ps_sigstk.ss_sp +
					 ps->ps_sigstk.ss_size) - 1;
		ps->ps_sigstk.ss_flags |= SA_ONSTACK;
	} else {
		/* Use current user stack */
		sp = saved_state->regs.regs[29];  /* $sp */
		fp = (struct sigframe *)sp - 1;
	}

	/* Build signal frame */
	frame.sig = sig;
	frame.code = (sig == SIGILL || sig == SIGFPE) ? code : 0;
	frame.scp = &fp->sc;

	/* Save current context */
	scp = &frame.sc;
	scp->sc_onstack = oonstack;
	scp->sc_mask = mask;
	scp->sc_pc = saved_state->regs.pc;

	/* Save all general registers */
	bcopy((caddr_t)saved_state->regs.regs, (caddr_t)scp->sc_regs,
	      sizeof(scp->sc_regs));

	scp->sc_mdlo = saved_state->regs.lo;
	scp->sc_mdhi = saved_state->regs.hi;
	scp->sc_status = saved_state->regs.status;
	scp->sc_cause = saved_state->regs.cause;

	/* Build signal trampoline
	 * This code executes after signal handler returns:
	 *   li v0, SYS_sigreturn
	 *   syscall
	 */
	frame.retcode[0] = 0x24020067;	/* li v0, 103 (SYS_sigreturn) */
	frame.retcode[1] = 0x0000000c;	/* syscall */

	/* Copy frame to user stack */
	if (copyout((caddr_t)&frame, (caddr_t)fp, sizeof(frame)))
		goto bad;

	/* Set up registers for signal handler */
	saved_state->regs.regs[4] = sig;		/* a0 = signal number */
	saved_state->regs.regs[5] = frame.code;		/* a1 = code */
	saved_state->regs.regs[6] = (unsigned int)frame.scp;  /* a2 = scp */
	saved_state->regs.regs[29] = (unsigned int)fp;	/* sp = frame pointer */
	saved_state->regs.regs[31] = (unsigned int)&fp->retcode;  /* ra = trampoline */
	saved_state->regs.pc = (unsigned int)catcher;	/* pc = handler */

	return;

bad:
	/* Failed to build signal frame - kill process */
	printf("sendsig: bad stack pid=%d sig=%d\n", p->p_pid, sig);
	sigexit(p, SIGILL);
}

/*
 * System call to restore context after signal handler returns
 * Called by signal trampoline
 */
struct sigreturn_args {
	struct sigcontext *scp;
};

int
sigreturn(struct proc *p, struct sigreturn_args *uap, int *retval)
{
	struct sigcontext context;
	struct sigcontext *scp = uap->scp;
	thread_t thread = current_thread();
	thread_saved_state_t *saved_state = USER_REGS(thread);
	struct sigacts *ps = p->p_sigacts;

	/* Copy sigcontext from user space */
	if (copyin((caddr_t)scp, (caddr_t)&context, sizeof(context)))
		return EFAULT;

	/* Restore signal stack state */
	if (context.sc_onstack & 01)
		ps->ps_sigstk.ss_flags |= SA_ONSTACK;
	else
		ps->ps_sigstk.ss_flags &= ~SA_ONSTACK;

	/* Restore signal mask */
	p->p_sigmask = context.sc_mask & ~sigcantmask;

	/* Restore registers */
	saved_state->regs.pc = context.sc_pc;
	bcopy((caddr_t)context.sc_regs, (caddr_t)saved_state->regs.regs,
	      sizeof(context.sc_regs));

	saved_state->regs.lo = context.sc_mdlo;
	saved_state->regs.hi = context.sc_mdhi;

	/* Don't restore Status register completely - preserve kernel bits */
	saved_state->regs.status = (saved_state->regs.status & 0xFFFF0000) |
				   (context.sc_status & 0x0000FFFF);

	return EJUSTRETURN;
}

/*
 * Machine-dependent signal initialization
 */
void
siginit(struct proc *p)
{
	/* Nothing special for MIPS */
}

/*
 * Signal frame validation
 * Verify that signal frame is valid before attempting sigreturn
 */
boolean_t
valid_signal_frame(struct sigcontext *scp)
{
	/* Check if address is in user space */
	if ((unsigned int)scp >= 0x80000000)
		return FALSE;

	/* Check alignment */
	if ((unsigned int)scp & 0x3)
		return FALSE;

	return TRUE;
}
