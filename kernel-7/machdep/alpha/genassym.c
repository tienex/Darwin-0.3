/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Generate assembly language constants
 *
 * This file is compiled to generate assym.h, which contains
 * constant definitions for use in assembly language files.
 */

#include <mach/mach_types.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/cpu.h>

#define DECLARE(SYM, VAL) \
	__asm__ volatile("\n .ascii \"@" SYM "@%0@\"\n" : : "n" (VAL))

int
main(void)
{
	/* Saved state offsets */
	DECLARE("SS_V0", offsetof(struct alpha_saved_state, v0));
	DECLARE("SS_T0", offsetof(struct alpha_saved_state, t0));
	DECLARE("SS_T1", offsetof(struct alpha_saved_state, t1));
	DECLARE("SS_T2", offsetof(struct alpha_saved_state, t2));
	DECLARE("SS_T3", offsetof(struct alpha_saved_state, t3));
	DECLARE("SS_T4", offsetof(struct alpha_saved_state, t4));
	DECLARE("SS_T5", offsetof(struct alpha_saved_state, t5));
	DECLARE("SS_T6", offsetof(struct alpha_saved_state, t6));
	DECLARE("SS_T7", offsetof(struct alpha_saved_state, t7));
	DECLARE("SS_S0", offsetof(struct alpha_saved_state, s0));
	DECLARE("SS_S1", offsetof(struct alpha_saved_state, s1));
	DECLARE("SS_S2", offsetof(struct alpha_saved_state, s2));
	DECLARE("SS_S3", offsetof(struct alpha_saved_state, s3));
	DECLARE("SS_S4", offsetof(struct alpha_saved_state, s4));
	DECLARE("SS_S5", offsetof(struct alpha_saved_state, s5));
	DECLARE("SS_FP", offsetof(struct alpha_saved_state, fp));
	DECLARE("SS_A0", offsetof(struct alpha_saved_state, a0));
	DECLARE("SS_A1", offsetof(struct alpha_saved_state, a1));
	DECLARE("SS_A2", offsetof(struct alpha_saved_state, a2));
	DECLARE("SS_A3", offsetof(struct alpha_saved_state, a3));
	DECLARE("SS_A4", offsetof(struct alpha_saved_state, a4));
	DECLARE("SS_A5", offsetof(struct alpha_saved_state, a5));
	DECLARE("SS_T8", offsetof(struct alpha_saved_state, t8));
	DECLARE("SS_T9", offsetof(struct alpha_saved_state, t9));
	DECLARE("SS_T10", offsetof(struct alpha_saved_state, t10));
	DECLARE("SS_T11", offsetof(struct alpha_saved_state, t11));
	DECLARE("SS_RA", offsetof(struct alpha_saved_state, ra));
	DECLARE("SS_T12", offsetof(struct alpha_saved_state, t12));
	DECLARE("SS_AT", offsetof(struct alpha_saved_state, at));
	DECLARE("SS_GP", offsetof(struct alpha_saved_state, gp));
	DECLARE("SS_SP", offsetof(struct alpha_saved_state, sp));
	DECLARE("SS_PC", offsetof(struct alpha_saved_state, pc));
	DECLARE("SS_PS", offsetof(struct alpha_saved_state, ps));

	DECLARE("SS_SIZE", sizeof(struct alpha_saved_state));

	/* Page size */
	DECLARE("PAGE_SHIFT", ALPHA_PGSHIFT);
	DECLARE("PAGE_SIZE", ALPHA_PGBYTES);
	DECLARE("PAGE_MASK", ALPHA_PGMASK);

	/* Interrupt priority levels */
	DECLARE("IPL_HIGH", ALPHA_IPL_HIGH);
	DECLARE("IPL_CLOCK", ALPHA_IPL_CLOCK);
	DECLARE("IPL_IO", ALPHA_IPL_IO);
	DECLARE("IPL_SOFT", ALPHA_IPL_SOFT);
	DECLARE("IPL_0", ALPHA_IPL_0);

	return 0;
}
