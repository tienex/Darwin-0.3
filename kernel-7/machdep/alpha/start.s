/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Kernel Bootstrap and Entry Point
 */

#include <architecture/alpha/asm_help.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

	.text
	.set	noreorder
	.set	noat

/*
 * Kernel entry point
 *
 * This is where the bootloader/firmware transfers control to the kernel.
 * We must handle both SRM and ARC boot protocols:
 *
 * SRM (UNIX PALcode):
 *   - a0 = HWRPB (Hardware Restart Parameter Block) address
 *   - a1 = page table base (may be 0)
 *   - a2 = boot flags
 *   - Uses PAL_UNIX_* functions
 *
 * ARC (NT PALcode):
 *   - a0 = argc
 *   - a1 = argv
 *   - a2 = envp
 *   - a3 = firmware callback
 *   - Uses PAL_NT_* functions
 */
NESTED(start, 16, ra)
	.prologue 0

	/*
	 * Save all boot parameters
	 * We don't know yet if this is SRM or ARC, so save everything
	 */
	lda	t0, boot_hwrpb
	stq	a0, 0(t0)		/* Could be HWRPB or argc */
	lda	t0, boot_pgtbl
	stq	a1, 0(t0)		/* Could be page table or argv */
	lda	t0, boot_flags
	stq	a2, 0(t0)		/* Could be boot flags or envp */
	lda	t0, boot_argc
	stq	a0, 0(t0)		/* Save as argc too */
	lda	t0, boot_argv
	stq	a1, 0(t0)		/* Save as argv too */
	lda	t0, boot_envp
	stq	a2, 0(t0)		/* Save as envp too */

	/*
	 * Detect PALcode variant
	 * Try UNIX swpipl - if it works, we have UNIX PALcode (SRM)
	 * If it fails, we have NT PALcode (ARC)
	 */
	lda	t0, 1f
	lda	a0, ALPHA_IPL_HIGH
	call_pal PAL_UNIX_swpipl	/* Try UNIX PALcode */
	br	2f			/* Success - UNIX PALcode */

1:	/* UNIX PALcode failed, try NT PALcode */
	lda	a0, ALPHA_IPL_HIGH
	call_pal PAL_NT_swpirql		/* Try NT PALcode */
	lda	t0, is_arc_firmware
	lda	t1, 1
	stq	t1, 0(t0)		/* Mark as ARC firmware */

2:	/* PALcode detected, continue */

	/*
	 * Initialize GP (global pointer) for kernel
	 */
	br	gp, 3f
3:	ldgp	gp, 0(gp)

	/*
	 * Initialize kernel stack
	 * We use a temporary boot stack until we set up proper per-CPU stacks
	 */
	lda	sp, boot_stack_top

	/*
	 * Clear the frame pointer
	 */
	bis	zero, zero, fp

	/*
	 * Call alpha_init() to perform early initialization
	 * This function will:
	 *   - Detect firmware type (SRM/ARC)
	 *   - Detect CPU type and features
	 *   - Initialize PALcode interface
	 *   - Set up virtual memory
	 *   - Initialize console
	 */
	jsr	ra, alpha_init
	ldgp	gp, 0(ra)

	/*
	 * Call setup_main() to continue with generic kernel initialization
	 */
	jsr	ra, setup_main
	ldgp	gp, 0(ra)

	/*
	 * Should not return here
	 * Try both PALcode variants for halt
	 */
9:	lda	t0, is_arc_firmware
	ldq	t0, 0(t0)
	bne	t0, 10f
	call_pal PAL_UNIX_halt		/* SRM halt */
	br	9b

10:	call_pal PAL_NT_halt		/* ARC halt */
	br	9b

END(start)

/*
 * Secondary CPU entry point (for SMP systems)
 */
NESTED(start_secondary, 16, ra)
	.prologue 0

	/*
	 * Initialize GP
	 */
	br	gp, 1f
1:	ldgp	gp, 0(gp)

	/*
	 * Get CPU ID
	 */
	call_pal PAL_UNIX_whami
	bis	v0, v0, s0		/* Save CPU ID in s0 */

	/*
	 * Set up stack for this CPU
	 * Each CPU gets its own stack in the cpu_stacks array
	 */
	lda	t0, cpu_stacks
	s8addq	s0, t0, t0		/* t0 = &cpu_stacks[cpu_id] */
	ldq	sp, 0(t0)

	/*
	 * Clear frame pointer
	 */
	bis	zero, zero, fp

	/*
	 * Call slave_main() to initialize this CPU
	 */
	bis	s0, s0, a0		/* Pass CPU ID as argument */
	jsr	ra, slave_main
	ldgp	gp, 0(ra)

	/*
	 * Should not return
	 */
9:	call_pal PAL_UNIX_halt
	br	9b

END(start_secondary)

/*
 * Boot parameters saved from bootloader
 */
	.data
	.align 3

	/* SRM boot parameters */
	.globl boot_hwrpb
boot_hwrpb:
	.quad 0

	.globl boot_pgtbl
boot_pgtbl:
	.quad 0

	.globl boot_flags
boot_flags:
	.quad 0

	/* ARC boot parameters */
	.globl boot_argc
boot_argc:
	.quad 0

	.globl boot_argv
boot_argv:
	.quad 0

	.globl boot_envp
boot_envp:
	.quad 0

	/* Firmware type flag */
	.globl is_arc_firmware
is_arc_firmware:
	.quad 0			/* 0 = SRM, 1 = ARC */

/*
 * Boot stack
 */
	.comm boot_stack, 16384, 3
	.globl boot_stack_top
boot_stack_top = boot_stack + 16384

/*
 * Per-CPU stacks (for SMP)
 */
	.comm cpu_stacks, 64*8, 3	/* Pointers to 64 CPU stacks */
