/*
 * x86-64 Unix Signal Support
 *
 * This file handles Unix signal delivery and return for x86-64
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <mach/machine.h>
#include <mach/machine/thread_status.h>

/*
 * Signal frame structure
 * This is pushed onto the user stack when delivering a signal
 */
struct sigframe {
	int			sig;		/* Signal number */
	int			code;		/* Signal code */
	struct sigcontext	*scp;		/* Signal context pointer */
	char			*addr;		/* Faulting address (if any) */
	void			(*handler)(int);/* Signal handler */
	struct sigcontext	sc;		/* Saved context */
};

/*
 * Signal context structure
 * Saved register state for signal return
 */
struct sigcontext {
	unsigned long	sc_rax;
	unsigned long	sc_rbx;
	unsigned long	sc_rcx;
	unsigned long	sc_rdx;
	unsigned long	sc_rsi;
	unsigned long	sc_rdi;
	unsigned long	sc_rbp;
	unsigned long	sc_rsp;
	unsigned long	sc_r8;
	unsigned long	sc_r9;
	unsigned long	sc_r10;
	unsigned long	sc_r11;
	unsigned long	sc_r12;
	unsigned long	sc_r13;
	unsigned long	sc_r14;
	unsigned long	sc_r15;
	unsigned long	sc_rip;
	unsigned long	sc_rflags;
	unsigned long	sc_cs;
	unsigned long	sc_ss;
	unsigned long	sc_mask;	/* Signal mask */
};

/*
 * Send a signal to a process
 */
void
sendsig(void (*handler)(int), int sig, unsigned long mask, unsigned long code, void *addr)
{
	struct sigframe sf;
	x86_64_thread_state_t *regs;
	unsigned long sp;

	/* Stub implementation */
	printf("sendsig: sig=%d, handler=%p\n", sig, handler);

	/* Would build signal frame on user stack */
	/* Would modify thread state to call signal handler */
}

/*
 * Build signal frame on user stack
 */
static int
build_sigframe(struct sigframe *sf, x86_64_thread_state_t *regs,
                int sig, unsigned long code, void *addr, void (*handler)(int))
{
	struct sigcontext *sc = &sf->sc;

	/* Fill in signal frame */
	sf->sig = sig;
	sf->code = code;
	sf->addr = addr;
	sf->handler = handler;
	sf->scp = sc;

	/* Save register state */
	sc->sc_rax = regs->rax;
	sc->sc_rbx = regs->rbx;
	sc->sc_rcx = regs->rcx;
	sc->sc_rdx = regs->rdx;
	sc->sc_rsi = regs->rsi;
	sc->sc_rdi = regs->rdi;
	sc->sc_rbp = regs->rbp;
	sc->sc_rsp = regs->rsp;
	sc->sc_r8 = regs->r8;
	sc->sc_r9 = regs->r9;
	sc->sc_r10 = regs->r10;
	sc->sc_r11 = regs->r11;
	sc->sc_r12 = regs->r12;
	sc->sc_r13 = regs->r13;
	sc->sc_r14 = regs->r14;
	sc->sc_r15 = regs->r15;
	sc->sc_rip = regs->rip;
	sc->sc_rflags = regs->rflags;
	sc->sc_cs = regs->cs;
	/* sc->sc_mask = current signal mask */

	return 0;
}

/*
 * Return from signal handler
 * Restores saved context from signal frame
 */
void
sigreturn(struct sigcontext *scp)
{
	x86_64_thread_state_t regs;

	/* Stub implementation */
	printf("sigreturn: scp=%p\n", scp);

	/* Restore register state from signal context */
	if (copyin(scp, &regs, sizeof(struct sigcontext)) != 0)
		return;

	/* Validate and restore state */
	/* Would restore thread state */
	/* Would restore signal mask */
}

/*
 * Restore context from signal frame
 */
static void
restore_sigcontext(x86_64_thread_state_t *regs, struct sigcontext *sc)
{
	/* Restore registers */
	regs->rax = sc->sc_rax;
	regs->rbx = sc->sc_rbx;
	regs->rcx = sc->sc_rcx;
	regs->rdx = sc->sc_rdx;
	regs->rsi = sc->sc_rsi;
	regs->rdi = sc->sc_rdi;
	regs->rbp = sc->sc_rbp;
	regs->rsp = sc->sc_rsp;
	regs->r8 = sc->sc_r8;
	regs->r9 = sc->sc_r9;
	regs->r10 = sc->sc_r10;
	regs->r11 = sc->sc_r11;
	regs->r12 = sc->sc_r12;
	regs->r13 = sc->sc_r13;
	regs->r14 = sc->sc_r14;
	regs->r15 = sc->sc_r15;
	regs->rip = sc->sc_rip;
	regs->rflags = sc->sc_rflags;

	/* Validate CS (must be user code segment) */
	if ((sc->sc_cs & 3) != 3)
		regs->cs = 0x1B;  /* User code selector */
	else
		regs->cs = sc->sc_cs;
}

/*
 * Post a signal to a process
 */
void
postsig(int sig)
{
	/* Stub implementation */
	printf("postsig: sig=%d\n", sig);

	/* Would mark signal as pending for process */
}

/*
 * Check for pending signals
 */
int
issignal(void *proc)
{
	/* Stub implementation */
	/* Would check if any signals are pending */
	return 0;
}

/*
 * Execute pending signals
 */
void
psig(void)
{
	/* Stub implementation */
	/* Would call sendsig() for each pending signal */
}

/*
 * Core dump support
 */
int
coredump(void *proc)
{
	/* Stub implementation */
	/* Would write core dump file */
	printf("coredump: process dumped core\n");
	return 0;
}

/*
 * Signal trampoline code (in user space)
 * This is copied to user stack/memory to return from signal handler
 */
void
signal_trampoline(void)
{
	/* Assembly code to call sigreturn() */
	__asm__ volatile(
		"movq $139, %rax\n"	/* sigreturn syscall number */
		"syscall\n"
	);
}

/*
 * Install signal trampoline in user space
 */
int
install_signal_trampoline(void *proc)
{
	/* Stub implementation */
	/* Would copy signal_trampoline code to user memory */
	return 0;
}
