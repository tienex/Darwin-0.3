/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Trap and Exception Handling
 *
 * This file handles all exceptions, traps, and interrupts for MMIX.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <debug.h>
#include <cpus.h>
#include <kern/thread.h>
#include <mach/exception.h>
#include <kern/syscall_sw.h>
#include <mach/thread_status.h>
#include <vm/vm_fault.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <machdep/mmix/trap.h>
#include <machdep/mmix/pmap.h>
#include <machdep/mmix/thread.h>

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/resourcevar.h>
#include <sys/kdebug.h>

#define NULL 0

/*
 * MMIX Exception Types (in rX)
 */
#define MMIX_EXC_TRAP			0x00	/* TRIP/TRAP instruction */
#define MMIX_EXC_FORCED_TRAP		0x01	/* Forced trap */
#define MMIX_EXC_OVERFLOW		0x02	/* Arithmetic overflow */
#define MMIX_EXC_FP_OVERFLOW		0x03	/* Floating point overflow */
#define MMIX_EXC_FP_UNDERFLOW		0x04	/* Floating point underflow */
#define MMIX_EXC_FP_INVALID		0x05	/* Floating point invalid op */
#define MMIX_EXC_FP_DIV_ZERO		0x06	/* Floating point divide by zero */
#define MMIX_EXC_FP_INEXACT		0x07	/* Floating point inexact */
#define MMIX_EXC_MEM_PROTECTION		0x10	/* Memory protection violation */
#define MMIX_EXC_MEM_NONEXIST		0x11	/* Nonexistent memory */
#define MMIX_EXC_PRIVILEGED		0x12	/* Privileged instruction */
#define MMIX_EXC_TIMER			0x20	/* Timer interrupt */
#define MMIX_EXC_EXTERNAL		0x21	/* External interrupt */

/*
 * Protection modes
 */
#define PROT_EXEC	(VM_PROT_READ)
#define PROT_RO		(VM_PROT_READ)
#define PROT_RW		(VM_PROT_READ|VM_PROT_WRITE)

/*
 * Check if in user mode
 */
#define USER_MODE(rW)	(((rW) & 0x8000000000000000ULL) == 0)

#if DEBUG
static int trapdebug = 0;
#define TRAP_DEBUG(A) { if (trapdebug) { printf A; } }
#else
#define TRAP_DEBUG(A)
#endif

/*
 * Forward declarations
 */
static void unresolved_kernel_trap(int trapno,
				   struct mmix_saved_state *ssp,
				   unsigned long long fault_addr,
				   char *message);

extern void doexception(int exc, int code, int sub, thread_t th);

/*
 * Main trap handler
 *
 * Called from lowmem_vectors.s with:
 *   trapno = exception number (from rX)
 *   ssp = pointer to saved state
 *   rW = return address (where exception occurred)
 *   rX = exception type
 *
 * Returns pointer to saved state (possibly different thread after context switch)
 */
struct mmix_saved_state *
trap(int trapno,
     struct mmix_saved_state *ssp,
     unsigned long long rW,
     unsigned long long rX)
{
	int exception = 0;
	int code = 0;
	int subcode = 0;
	vm_map_t map;
	unsigned long long fault_addr;
	thread_t th = current_thread();
	int error;
	vm_prot_t prot;
	boolean_t user_mode;

	TRAP_DEBUG(("MMIX TRAP %d rW=0x%llx, rX=0x%llx\n", trapno, rW, rX));

	user_mode = USER_MODE(rW);

	/* Handle kernel traps first */
	if (!user_mode) {
		/*
		 * Trap came from kernel mode
		 */
		switch (trapno) {

		case MMIX_EXC_TIMER:
			/* Timer interrupt - handle in interrupt context */
			/* Call machine_clock interrupt handler */
			extern void mmix_timer_interrupt(void);
			mmix_timer_interrupt();
			break;

		case MMIX_EXC_EXTERNAL:
			/* External interrupt from device */
			/* Determine which device and call handler */
			extern void mmix_external_interrupt(void);
			mmix_external_interrupt();
			break;

		case MMIX_EXC_MEM_PROTECTION:
		case MMIX_EXC_MEM_NONEXIST:
			/* Page fault in kernel */
			fault_addr = ssp->rY;	/* rY contains fault address */

			TRAP_DEBUG(("Kernel page fault at 0x%llx\n", fault_addr));

			/* Determine if read or write */
			/* rZ bit 0 indicates write */
			prot = (ssp->rZ & 1) ? PROT_RW : PROT_RO;

			/* Try to fault in the page */
			map = kernel_map;
			error = vm_fault(map, trunc_page(fault_addr),
					prot, FALSE, FALSE, NULL, 0);

			if (error != KERN_SUCCESS) {
				/* Unrecoverable kernel page fault */
				unresolved_kernel_trap(trapno, ssp,
						      fault_addr,
						      "Kernel page fault");
			}
			break;

		case MMIX_EXC_OVERFLOW:
		case MMIX_EXC_FP_OVERFLOW:
		case MMIX_EXC_FP_UNDERFLOW:
		case MMIX_EXC_FP_INVALID:
		case MMIX_EXC_FP_DIV_ZERO:
		case MMIX_EXC_FP_INEXACT:
			/* Arithmetic exceptions in kernel - usually fatal */
			unresolved_kernel_trap(trapno, ssp, 0,
					      "Arithmetic exception");
			break;

		case MMIX_EXC_PRIVILEGED:
			/* Privileged instruction - shouldn't happen in kernel */
			unresolved_kernel_trap(trapno, ssp, 0,
					      "Privileged instruction");
			break;

		case MMIX_EXC_TRAP:
			/* Kernel debugger trap */
#if DEBUG
			/* Enter kernel debugger */
			extern void kdp_trap(int, struct mmix_saved_state *);
			kdp_trap(trapno, ssp);
#else
			unresolved_kernel_trap(trapno, ssp, 0, "Debug trap");
#endif
			break;

		default:
			unresolved_kernel_trap(trapno, ssp, 0, "Unknown trap");
			break;
		}

		return ssp;
	}

	/*
	 * Trap came from user mode
	 */

	switch (trapno) {

	case MMIX_EXC_TIMER:
		/* Timer interrupt */
		extern void mmix_timer_interrupt(void);
		mmix_timer_interrupt();
		break;

	case MMIX_EXC_EXTERNAL:
		/* External interrupt */
		extern void mmix_external_interrupt(void);
		mmix_external_interrupt();
		break;

	case MMIX_EXC_MEM_PROTECTION:
	case MMIX_EXC_MEM_NONEXIST:
		/* Page fault in user space */
		fault_addr = ssp->rY;

		TRAP_DEBUG(("User page fault at 0x%llx\n", fault_addr));

		/* Determine protection needed */
		prot = (ssp->rZ & 1) ? PROT_RW : PROT_RO;

		/* Fault in the page */
		map = th->task->map;
		error = vm_fault(map, trunc_page(fault_addr),
				prot, FALSE, FALSE, NULL, 0);

		if (error != KERN_SUCCESS) {
			/* Send SIGSEGV to process */
			exception = EXC_BAD_ACCESS;
			code = fault_addr;
			subcode = error;
			goto signal_exception;
		}
		break;

	case MMIX_EXC_OVERFLOW:
		/* Integer overflow */
		exception = EXC_ARITHMETIC;
		code = EXC_MMIX_OVERFLOW;
		goto signal_exception;

	case MMIX_EXC_FP_OVERFLOW:
		exception = EXC_ARITHMETIC;
		code = EXC_MMIX_FP_OVERFLOW;
		goto signal_exception;

	case MMIX_EXC_FP_UNDERFLOW:
		exception = EXC_ARITHMETIC;
		code = EXC_MMIX_FP_UNDERFLOW;
		goto signal_exception;

	case MMIX_EXC_FP_INVALID:
		exception = EXC_ARITHMETIC;
		code = EXC_MMIX_FP_INVALID;
		goto signal_exception;

	case MMIX_EXC_FP_DIV_ZERO:
		exception = EXC_ARITHMETIC;
		code = EXC_MMIX_FP_DIV_ZERO;
		goto signal_exception;

	case MMIX_EXC_FP_INEXACT:
		exception = EXC_ARITHMETIC;
		code = EXC_MMIX_FP_INEXACT;
		goto signal_exception;

	case MMIX_EXC_PRIVILEGED:
		/* User tried to execute privileged instruction */
		exception = EXC_BAD_INSTRUCTION;
		code = EXC_MMIX_PRIVILEGED;
		goto signal_exception;

	case MMIX_EXC_TRAP:
		/* User mode trap - could be system call or breakpoint */
		/* Check if system call (TRAP 0,syscall_num,0) */
		if (ssp->r0 != 0) {
			/* System call */
			extern void unix_syscall(struct mmix_saved_state *);
			unix_syscall(ssp);
		} else {
			/* Breakpoint/debug trap */
			exception = EXC_BREAKPOINT;
			code = EXC_MMIX_TRAP;
			goto signal_exception;
		}
		break;

	default:
		/* Unknown trap */
		exception = EXC_BAD_INSTRUCTION;
		code = trapno;
		goto signal_exception;
	}

	/* Check for AST (asynchronous system trap) */
	extern void ast_check(thread_t);
	ast_check(th);

	return ssp;

signal_exception:
	/* Deliver Mach exception to thread */
	doexception(exception, code, subcode, th);

	return ssp;
}

/*
 * Unresolved kernel trap
 *
 * Print diagnostic information and panic
 */
static void
unresolved_kernel_trap(int trapno,
		       struct mmix_saved_state *ssp,
		       unsigned long long fault_addr,
		       char *message)
{
	printf("\n\nUnresolved kernel trap:\n");
	if (message)
		printf("  %s\n", message);
	printf("  Trap number: %d (0x%x)\n", trapno, trapno);
	printf("  rW (return address): 0x%llx\n", ssp->rW);
	printf("  rX (exception type): 0x%llx\n", ssp->rX);
	printf("  rY (fault address):  0x%llx\n", ssp->rY);
	printf("  rZ (fault info):     0x%llx\n", ssp->rZ);
	printf("  SP ($254):           0x%llx\n", ssp->sp);
	printf("  FP ($253):           0x%llx\n", ssp->fp);
	if (fault_addr)
		printf("  Fault address:       0x%llx\n", fault_addr);

	printf("\nRegisters:\n");
	printf("  $0  = 0x%llx  $1  = 0x%llx\n", ssp->r0, ssp->r1);
	printf("  $2  = 0x%llx  $3  = 0x%llx\n", ssp->r2, ssp->r3);
	printf("  $16 = 0x%llx  $17 = 0x%llx\n", ssp->r16, ssp->r17);
	printf("  $18 = 0x%llx  $19 = 0x%llx\n", ssp->r18, ssp->r19);

	panic("Unresolved kernel trap");
}

/*
 * Interrupt handler entry points
 */

void
mmix_timer_interrupt(void)
{
	/* Reload rI with new interval */
	/* Update system time */
	/* Check for thread quantum expiration */
	/* TODO: Implement timer handling */
}

void
mmix_external_interrupt(void)
{
	/* Determine interrupt source */
	/* Call appropriate device interrupt handler */
	/* TODO: Implement interrupt routing */
}
