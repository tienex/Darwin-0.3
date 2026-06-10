/*
 * MMIX Bootloader Assembly Entry Point
 * Copyright (c) 1999-2025 Apple Computer, Inc.
 *
 * This file provides the assembly entry point for the MMIX bootloader
 * and low-level register manipulation routines.
 */

	.text
	.align 4

/*
 * Bootloader entry point
 * Called by emulator after loading bootloader into memory at 0x1000
 *
 * On entry:
 *   $0 = memory size in bytes
 *   $1 = bootloader size
 *   $255 = emulator signature (0x4D4D4958 = 'MMIX')
 */
	.globl	_boot_entry
	.globl	boot_entry
_boot_entry:
boot_entry:
	/* Save boot parameters */
	SETL	$2,#1000		/* Boot stack top (temporary) */
	STOU	$0,$2,0			/* Save memory size */
	STOU	$1,$2,8			/* Save bootloader size */
	STOU	$255,$2,16		/* Save emulator signature */

	/* Set up initial stack */
	SETL	$254,#0000		/* Stack pointer low */
	ORMH	$254,#0000		/* Stack pointer mid-high */
	ORML	$254,#0010		/* Stack pointer mid-low = 0x100000 */

	/* Clear BSS section */
	GETA	$0,__bss_start
	GETA	$1,__bss_end
	SUBU	$2,$1,$0		/* BSS size */
	SET	$3,0			/* Zero value */
1:	BZ	$2,2f			/* Done if size == 0 */
	STOU	$3,$0,0			/* Store zero */
	ADDU	$0,$0,8			/* Next word */
	SUBU	$2,$2,8			/* Decrement size */
	JMP	1b
2:

	/* Initialize special registers */
	SET	$0,0
	PUT	rG,$0			/* Global threshold = 0 */
	SETL	$0,32
	PUT	rL,$0			/* Local threshold = 32 */

	/* Set interrupt mask (disable all interrupts initially) */
	SETL	$0,0
	PUT	rK,$0

	/* Jump to C main function */
	PUSHJ	$0,boot_main		/* Call boot_main() */

	/* Should never return, but halt if it does */
	TRAP	0,0,0

/*
 * Get special register value
 * unsigned long long boot_get_special(int reg)
 *
 * $0 = register number
 * Returns value in $0
 */
	.globl	_boot_get_special
	.globl	boot_get_special
_boot_get_special:
boot_get_special:
	/* This is a simplified version - real implementation would
	   use a jump table to handle all special registers */
	CMP	$1,$0,21		/* Check if rA */
	BZ	$1,get_rA
	CMP	$1,$0,15		/* Check if rK */
	BZ	$1,get_rK
	/* Add more special registers as needed */
	SET	$0,0			/* Default: return 0 */
	POP	1,0

get_rA:
	GET	$0,rA
	POP	1,0

get_rK:
	GET	$0,rK
	POP	1,0

/*
 * Put special register value
 * void boot_put_special(int reg, unsigned long long val)
 *
 * $0 = register number
 * $1 = value
 */
	.globl	_boot_put_special
	.globl	boot_put_special
_boot_put_special:
boot_put_special:
	CMP	$2,$0,21		/* Check if rA */
	BZ	$2,put_rA
	CMP	$2,$0,15		/* Check if rK */
	BZ	$2,put_rK
	POP	0,0			/* Return */

put_rA:
	PUT	rA,$1
	POP	0,0

put_rK:
	PUT	rK,$1
	POP	0,0

/*
 * Flush instruction and data caches
 * void boot_flush_cache(void)
 */
	.globl	_boot_flush_cache
	.globl	boot_flush_cache
_boot_flush_cache:
boot_flush_cache:
	SYNC	0			/* MMIX sync instruction */
	POP	0,0

/*
 * Data section
 */
	.data
	.align 3

boot_signature:
	.ascii	"MMIX Boot 1.0\0"

	.globl	__bss_start
	.globl	__bss_end

/*
 * BSS section
 */
	.bss
	.align 3

__bss_start:
	.space	65536			/* 64KB BSS for bootloader */
__bss_end:
