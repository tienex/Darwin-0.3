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
 * Generate assembly constants from C structures
 * This file is compiled to produce assym.s
 */

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/map.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <sys/msgbuf.h>

#include <mach/mips/thread_status.h>
#include <machdep/mips/pcb.h>
#include <machdep/mips/trap.h>
#include <machdep/mips/pmap.h>

/* Macro to define assembly constants */
#define DEFINE(name, value) \
	__asm__ __volatile__("\n#define " #name " %0" : : "i" (value))

#define DEFINE_STR(name, value) \
	__asm__ __volatile__("\n#define " #name " " value)

int
main(void)
{
	/* PCB offsets */
	DEFINE(PCB_S0, offsetof(struct mips_pcb, s0));
	DEFINE(PCB_S1, offsetof(struct mips_pcb, s1));
	DEFINE(PCB_S2, offsetof(struct mips_pcb, s2));
	DEFINE(PCB_S3, offsetof(struct mips_pcb, s3));
	DEFINE(PCB_S4, offsetof(struct mips_pcb, s4));
	DEFINE(PCB_S5, offsetof(struct mips_pcb, s5));
	DEFINE(PCB_S6, offsetof(struct mips_pcb, s6));
	DEFINE(PCB_S7, offsetof(struct mips_pcb, s7));
	DEFINE(PCB_S8, offsetof(struct mips_pcb, s8));
	DEFINE(PCB_GP, offsetof(struct mips_pcb, gp));
	DEFINE(PCB_SP, offsetof(struct mips_pcb, sp));
	DEFINE(PCB_RA, offsetof(struct mips_pcb, ra));
	DEFINE(PCB_PC, offsetof(struct mips_pcb, pc));
	DEFINE(PCB_SIZE, sizeof(struct mips_pcb));

	/* Saved state offsets */
	DEFINE(SS_ZERO, offsetof(struct mips_saved_state, zero));
	DEFINE(SS_AT, offsetof(struct mips_saved_state, at));
	DEFINE(SS_V0, offsetof(struct mips_saved_state, v0));
	DEFINE(SS_V1, offsetof(struct mips_saved_state, v1));
	DEFINE(SS_A0, offsetof(struct mips_saved_state, a0));
	DEFINE(SS_A1, offsetof(struct mips_saved_state, a1));
	DEFINE(SS_A2, offsetof(struct mips_saved_state, a2));
	DEFINE(SS_A3, offsetof(struct mips_saved_state, a3));
	DEFINE(SS_T0, offsetof(struct mips_saved_state, t0));
	DEFINE(SS_T1, offsetof(struct mips_saved_state, t1));
	DEFINE(SS_T2, offsetof(struct mips_saved_state, t2));
	DEFINE(SS_T3, offsetof(struct mips_saved_state, t3));
	DEFINE(SS_T4, offsetof(struct mips_saved_state, t4));
	DEFINE(SS_T5, offsetof(struct mips_saved_state, t5));
	DEFINE(SS_T6, offsetof(struct mips_saved_state, t6));
	DEFINE(SS_T7, offsetof(struct mips_saved_state, t7));
	DEFINE(SS_S0, offsetof(struct mips_saved_state, s0));
	DEFINE(SS_S1, offsetof(struct mips_saved_state, s1));
	DEFINE(SS_S2, offsetof(struct mips_saved_state, s2));
	DEFINE(SS_S3, offsetof(struct mips_saved_state, s3));
	DEFINE(SS_S4, offsetof(struct mips_saved_state, s4));
	DEFINE(SS_S5, offsetof(struct mips_saved_state, s5));
	DEFINE(SS_S6, offsetof(struct mips_saved_state, s6));
	DEFINE(SS_S7, offsetof(struct mips_saved_state, s7));
	DEFINE(SS_T8, offsetof(struct mips_saved_state, t8));
	DEFINE(SS_T9, offsetof(struct mips_saved_state, t9));
	DEFINE(SS_K0, offsetof(struct mips_saved_state, k0));
	DEFINE(SS_K1, offsetof(struct mips_saved_state, k1));
	DEFINE(SS_GP, offsetof(struct mips_saved_state, gp));
	DEFINE(SS_SP, offsetof(struct mips_saved_state, sp));
	DEFINE(SS_FP, offsetof(struct mips_saved_state, fp));
	DEFINE(SS_RA, offsetof(struct mips_saved_state, ra));
	DEFINE(SS_LO, offsetof(struct mips_saved_state, lo));
	DEFINE(SS_HI, offsetof(struct mips_saved_state, hi));
	DEFINE(SS_STATUS, offsetof(struct mips_saved_state, status));
	DEFINE(SS_CAUSE, offsetof(struct mips_saved_state, cause));
	DEFINE(SS_EPC, offsetof(struct mips_saved_state, epc));
	DEFINE(SS_BADVADDR, offsetof(struct mips_saved_state, badvaddr));
	DEFINE(SS_SIZE, sizeof(struct mips_saved_state));

	/* Page size */
	DEFINE(NBPG, MIPS_PGSIZE);
	DEFINE(PGSHIFT, MIPS_PGSHIFT);

	/* Miscellaneous */
	DEFINE(UPAGES, UPAGES);

	return 0;
}
