/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Floating Point Unit Support
 */

#include <mach/mach_types.h>
#include <architecture/alpha/reg.h>

/*
 * Initialize FPU for thread
 */
void
fpu_init(void)
{
	/* Enable floating point */
	extern void pal_unix_wrfen(unsigned long);
	pal_unix_wrfen(1);
}

/*
 * Save FPU state
 */
void
fpu_save_context(alpha_float_state_t *state)
{
	/* Save FP registers to state */
}

/*
 * Restore FPU state
 */
void
fpu_restore_context(alpha_float_state_t *state)
{
	/* Restore FP registers from state */
}
