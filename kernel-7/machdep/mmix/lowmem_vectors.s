/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Exception and Trap Vectors
 *
 * This file implements the low-level exception handling for MMIX.
 * When an exception occurs, MMIX:
 *   1. Saves return address in rW ($27)
 *   2. Puts exception info in rX ($28), rY ($29), rZ ($30)
 *   3. Saves rJ in rB ($25)
 *   4. Jumps to address in rT ($26)
 *
 * Exception types (in rX):
 *   0x00: Dynamic trap (TRIP/TRAP instructions)
 *   0x01: Forced trap
 *   0x02: Arithmetic overflow
 *   0x03: Floating point overflow
 *   0x04: Floating point underflow
 *   0x05: Floating point invalid operation
 *   0x06: Floating point divide by zero
 *   0x07: Floating point inexact
 *   0x10: Memory protection violation
 *   0x11: Nonexistent memory
 *   0x12: Privileged instruction
 *   0x20: Timer interrupt (rI reached zero)
 *   0x21: External interrupt
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <machdep/mmix/asm.h>
#include <machdep/mmix/thread.h>
#include <mach/mmix/vm_param.h>

	.file	"lowmem_vectors.s"

/*
 * Saved state structure on stack
 * This must match mmix_saved_state in thread.h
 */
#define SS_R0		0
#define SS_R1		8
#define SS_R2		16
#define SS_R3		24
#define SS_R4		32
#define SS_R5		40
#define SS_R6		48
#define SS_R7		56
#define SS_R8		64
#define SS_R9		72
#define SS_R10		80
#define SS_R11		88
#define SS_R12		96
#define SS_R13		104
#define SS_R14		112
#define SS_R15		120
#define SS_R16		128
#define SS_R17		136
#define SS_R18		144
#define SS_R19		152
#define SS_R20		160
#define SS_R21		168
#define SS_R22		176
#define SS_R23		184
#define SS_SP		192	/* $254 */
#define SS_FP		200	/* $253 */
#define SS_RW		208	/* Return address */
#define SS_RX		216	/* Exception type */
#define SS_RY		224	/* Exception info 1 */
#define SS_RZ		232	/* Exception info 2 */
#define SS_RJ		240	/* Saved rJ */
#define SS_RL		248	/* Local threshold */
#define SS_RG		256	/* Global threshold */
#define SS_RA		264	/* Arithmetic status */
#define SS_SIZE		272	/* Total size */

	.data
	.align	3

	/* Saved state for nested exceptions */
	.globl	exception_save_area
exception_save_area:
	.space	SS_SIZE

	.text
	.align	4

/*
 * Main exception handler entry point
 *
 * This address is loaded into rT at boot time.
 * All exceptions/traps jump here.
 *
 * On entry:
 *   rW ($27) = return address (where exception occurred)
 *   rX ($28) = exception number
 *   rY ($29) = exception info 1
 *   rZ ($30) = exception info 2
 *   rB ($25) = saved rJ
 *   rK has high bit set (in privileged mode)
 */

	.globl	exception_handler
	.globl	_exception_handler
exception_handler:
_exception_handler:

	/*
	 * Save minimal state to determine if we need to switch stacks
	 * We use $31 (rBB) as a temporary - it's saved in rS
	 */

	/* Get current SP into $31 */
	SET	$31,$254

	/* Check if SP is already on interrupt stack */
	GETA	$1,intstack
	CMP	$2,$31,$1
	BN	$2,switch_to_intstack	/* SP < intstack base */

	GETA	$1,intstack_top
	CMP	$2,$31,$1
	BP	$2,switch_to_intstack	/* SP > intstack top */

	/* Already on interrupt stack, continue */
	JMP	save_state

switch_to_intstack:
	/* Switch to interrupt stack */
	GETA	$254,intstack_top
	SUBU	$254,$254,SS_SIZE	/* Reserve space for saved state #/

save_state:
	/*
	 * Save complete processor state
	 * $254 now points to save area
	 */

	/* Save general registers $0-$23 */
	GET	$1,rS		/* Get $0-$2 from rS (saved by MMIX) */
	SRU	$2,$1,32
	SRU	$3,$1,16
	STOU	$2,$254,SS_R0
	STOU	$3,$254,SS_R1
	STOU	$1,$254,SS_R2

	STOU	$3,$254,SS_R3
	STOU	$4,$254,SS_R4
	STOU	$5,$254,SS_R5
	STOU	$6,$254,SS_R6
	STOU	$7,$254,SS_R7
	STOU	$8,$254,SS_R8
	STOU	$9,$254,SS_R9
	STOU	$10,$254,SS_R10
	STOU	$11,$254,SS_R11
	STOU	$12,$254,SS_R12
	STOU	$13,$254,SS_R13
	STOU	$14,$254,SS_R14
	STOU	$15,$254,SS_R15
	STOU	$16,$254,SS_R16
	STOU	$17,$254,SS_R17
	STOU	$18,$254,SS_R18
	STOU	$19,$254,SS_R19
	STOU	$20,$254,SS_R20
	STOU	$21,$254,SS_R21
	STOU	$22,$254,SS_R22
	STOU	$23,$254,SS_R23

	/* Save SP and FP */
	STOU	$31,$254,SS_SP		/* Original SP (saved earlier) */
	STOU	$253,$254,SS_FP

	/* Save special registers from exception */
	GET	$1,rW
	STOU	$1,$254,SS_RW
	GET	$1,rX
	STOU	$1,$254,SS_RX
	GET	$1,rY
	STOU	$1,$254,SS_RY
	GET	$1,rZ
	STOU	$1,$254,SS_RZ
	GET	$1,rB			/* rB contains saved rJ */
	STOU	$1,$254,SS_RJ

	/* Save other special registers */
	GET	$1,rL
	STOU	$1,$254,SS_RL
	GET	$1,rG
	STOU	$1,$254,SS_RG
	GET	$1,rA
	STOU	$1,$254,SS_RA

	/*
	 * Determine exception type and call appropriate handler
	 *
	 * Call C function: trap(trapno, saved_state, rW, rX)
	 *   $0 = trapno (from rX)
	 *   $1 = pointer to saved state
	 *   $2 = rW (return address)
	 *   $3 = rX (exception type)
	 */

	LDOU	$0,$254,SS_RX		/* trapno = rX */
	SET	$1,$254			/* saved_state pointer */
	LDOU	$2,$254,SS_RW		/* rW */
	LDOU	$3,$254,SS_RX		/* rX */

	/* Call trap() in trap.c */
	PUSHJ	$255,trap

	/*
	 * trap() returns pointer to saved state (possibly different thread)
	 * Restore state and return from exception
	 */
	SET	$254,$0		/* Get returned saved state pointer */

	/* Fall through to exception_return */

/*
 * Return from exception
 *
 * Restore saved state and resume execution
 * $254 points to saved state structure
 */

	.globl	exception_return
exception_return:

	/* Restore general registers */
	LDOU	$3,$254,SS_R3
	LDOU	$4,$254,SS_R4
	LDOU	$5,$254,SS_R5
	LDOU	$6,$254,SS_R6
	LDOU	$7,$254,SS_R7
	LDOU	$8,$254,SS_R8
	LDOU	$9,$254,SS_R9
	LDOU	$10,$254,SS_R10
	LDOU	$11,$254,SS_R11
	LDOU	$12,$254,SS_R12
	LDOU	$13,$254,SS_R13
	LDOU	$14,$254,SS_R14
	LDOU	$15,$254,SS_R15
	LDOU	$16,$254,SS_R16
	LDOU	$17,$254,SS_R17
	LDOU	$18,$254,SS_R18
	LDOU	$19,$254,SS_R19
	LDOU	$20,$254,SS_R20
	LDOU	$21,$254,SS_R21
	LDOU	$22,$254,SS_R22
	LDOU	$23,$254,SS_R23

	/* Restore special registers */
	LDOU	$1,$254,SS_RJ
	PUT	rJ,$1
	LDOU	$1,$254,SS_RL
	PUT	rL,$1
	LDOU	$1,$254,SS_RG
	PUT	rG,$1
	LDOU	$1,$254,SS_RA
	PUT	rA,$1

	/* Restore rW (return address) for RESUME */
	LDOU	$1,$254,SS_RW
	PUT	rW,$1

	/* Restore SP and FP last */
	LDOU	$253,$254,SS_FP
	LDOU	$31,$254,SS_SP		/* Temp in $31 */

	/* Restore $0-$2 */
	LDOU	$0,$254,SS_R0
	LDOU	$1,$254,SS_R1
	LDOU	$2,$254,SS_R2

	/* Restore SP */
	SET	$254,$31

	/* Return from exception */
	RESUME	1		/* Resume and clear rX */

/*
 * Interrupt stack boundaries
 */
	.data
	.align	3

	.globl	intstack
intstack:
	.space	16384		/* 16KB interrupt stack */

	.globl	intstack_top
intstack_top:

	.text

/*
 * System call entry point
 *
 * User programs use TRAP to make system calls
 * TRAP sets rX to trap number and jumps to exception handler
 *
 * Syscall number is passed in $0
 * Arguments in $1-$6
 * Return value in $0
 */

	.globl	syscall_entry
syscall_entry:

	/* Save state (same as exception) */
	/* Handled by main exception_handler */

	/* Syscall-specific handling done in trap.c */

	JMP	exception_handler

/*
 * Initialize exception handler
 *
 * Called from mmix_init() to set up rT
 */

	.globl	exception_init
exception_init:

	/* Set rT to point to exception handler */
	GETA	$0,exception_handler
	PUT	rT,$0

	/* Enable interrupts we want to handle */
	/* Set bits in rK for timer (bit 12) and external interrupts */
	SET	$0,0		/* Start with all disabled */
	PUT	rK,$0

	POP	1,0

/*
 * External interrupt handler entry
 *
 * Hardware devices signal interrupts via emulator
 * Emulator sets rX = 0x21 and triggers exception
 */

	.globl	interrupt_entry
interrupt_entry:

	/* Handled by main exception handler */
	/* Specific interrupt source determined in trap.c */

	JMP	exception_handler

/*
 * Timer interrupt handler
 *
 * When rI (interval timer) counts down to zero,
 * MMIX triggers exception with rX = 0x20
 */

	.globl	timer_interrupt
timer_interrupt:

	/* Handled by main exception handler */

	JMP	exception_handler
