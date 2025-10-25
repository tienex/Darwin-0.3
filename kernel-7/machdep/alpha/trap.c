/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Trap and Exception Handling
 */

#include <mach/mach_types.h>
#include <mach/exception_types.h>
#include <kern/thread.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

/*
 * Exception handler
 *
 * Called from exception.s when a hardware exception occurs
 */
void
alpha_exception_handler(alpha_saved_state_t *state)
{
	unsigned long exc_type;
	unsigned long exc_addr;

	/*
	 * Determine exception type
	 * This would normally come from PALcode-specific registers
	 */

	/*
	 * Handle common exception types
	 */

	/* For now, panic on any exception during development */
	panic("alpha_exception_handler: exception at PC=0x%lx", state->pc);
}

/*
 * Interrupt handler
 *
 * Called from exception.s when a hardware interrupt occurs
 */
void
alpha_interrupt_handler(alpha_saved_state_t *state)
{
	unsigned long ipl;

	/*
	 * Read current interrupt priority level
	 */
	ipl = alpha_rdps() & ALPHA_PS_IPL;

	/*
	 * Dispatch to appropriate interrupt handler based on IPL
	 */
	switch (ipl) {
	case ALPHA_IPL_CLOCK:
		/* Handle clock interrupt */
		/* rtclock_intr(); */
		break;

	case ALPHA_IPL_IO:
		/* Handle I/O device interrupt */
		break;

	case ALPHA_IPL_SOFT:
		/* Handle software interrupt */
		break;

	default:
		/* Unknown interrupt */
		break;
	}
}

/*
 * System call handler
 *
 * Called from exception.s when a system call is made
 */
void
alpha_syscall_handler(alpha_saved_state_t *state)
{
	unsigned long syscall_num;
	unsigned long ret_val;

	/*
	 * System call number is in v0
	 * Arguments are in a0-a5
	 */
	syscall_num = state->v0;

	/*
	 * Dispatch to BSD system call handler
	 */
	/* ret_val = unix_syscall(syscall_num, state->a0, state->a1,
				  state->a2, state->a3, state->a4, state->a5); */

	/*
	 * Return value goes in v0
	 */
	/* state->v0 = ret_val; */

	/* For now, return error */
	state->v0 = -1;
}

/*
 * Machine check handler
 */
void
alpha_machine_check(alpha_saved_state_t *state)
{
	unsigned long mces;

	/*
	 * Read machine check error summary
	 */
	mces = alpha_rdps();  /* Simplified - actual MCES read would use PALcode */

	/*
	 * Log the machine check
	 */
	printf("Machine check at PC=0x%lx, MCES=0x%lx\n", state->pc, mces);

	/*
	 * Panic if uncorrectable
	 */
	panic("Uncorrectable machine check");
}

/*
 * Alignment fault handler
 */
void
alpha_alignment_fault(alpha_saved_state_t *state, unsigned long va)
{
	/*
	 * Alpha requires aligned accesses for most data types
	 * We can either:
	 * 1. Fix up the access in software (slow)
	 * 2. Send a signal to the process
	 */

	printf("Alignment fault at PC=0x%lx, VA=0x%lx\n", state->pc, va);

	/* For now, panic during development */
	panic("Alignment fault");
}

/*
 * Page fault handler
 */
void
alpha_page_fault(alpha_saved_state_t *state, unsigned long va, int fault_type)
{
	vm_map_t map;
	kern_return_t result;

	/*
	 * Determine which map to use (kernel or user)
	 */
	if (va >= ALPHA_KSEG_START) {
		map = kernel_map;
	} else {
		/* Get current thread's map */
		/* map = current_thread()->task->map; */
		map = kernel_map;  /* Simplified for now */
	}

	/*
	 * Try to handle the fault
	 */
	/* result = vm_fault(map, va, fault_type, FALSE, FALSE, NULL, 0); */

	/*
	 * If fault handling failed, panic or send signal
	 */
	/* if (result != KERN_SUCCESS) {
		panic("Page fault at PC=0x%lx, VA=0x%lx", state->pc, va);
	} */

	/* Simplified for now */
	panic("Page fault at PC=0x%lx, VA=0x%lx", state->pc, va);
}
