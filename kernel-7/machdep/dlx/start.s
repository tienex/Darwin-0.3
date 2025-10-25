/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX Bootstrap Code
 * Entry point for Darwin kernel on DLX architecture
 */

	.text
	.globl	_start
	.globl	_boot_args

/*
 * Kernel entry point
 * Called by boot loader with:
 *   r1 = boot arguments pointer
 *   r2 = available memory size
 */
_start:
	/* Set up initial stack */
	lhi	r29, bootstack_top
	sw	r1, _boot_args		/* Save boot arguments */

	/* Clear BSS */
	lhi	r4, _edata		/* Start of BSS */
	lhi	r5, _end		/* End of BSS */
	sub	r6, r5, r4		/* Length */
	beqz	r6, bss_done
bss_loop:
	sw	r0, 0(r4)		/* Clear word */
	addi	r4, r4, #4
	subi	r6, r6, #4
	bnez	r6, bss_loop
bss_done:

	/* Enable page tables (but not TLB yet) */
	lhi	r4, 0x140		/* SYSMODE | PAGE_TABLE */
	movs2i	r4, r0			/* Set status register */

	/* Initialize page table base */
	lhi	r4, _kernel_page_directory
	setreg	r4, 1			/* DLX_SREG_PGTBL_BASE = 1 */

	/* Set page table bits: L1=19, L2=13 */
	lhi	r4, 0x130013		/* (19 << 16) | 13 */
	setreg	r4, 2			/* DLX_SREG_PGTBL_BITS = 2 */

	/* Set page table size */
	lhi	r4, 32			/* 32 L1 entries */
	setreg	r4, 3			/* DLX_SREG_PGTBL_SIZE = 3 */

	/* Call C initialization */
	lhi	r1, _edata		/* Load start */
	lhi	r2, _avail_end		/* Memory end */
	jal	_pmap_bootstrap		/* Bootstrap pmap */

	/* Call machine initialization */
	jal	_machine_startup

	/* Call kernel main */
	jal	_main

	/* Should never return */
halt:
	trap	#0x300			/* EXIT trap */
	j	halt

/*
 * Boot stack (4KB)
 */
	.data
	.align	13			/* Align to 8KB */
bootstack:
	.space	4096
bootstack_top:

_boot_args:
	.word	0

	.globl	_kernel_page_directory
	.align	13
_kernel_page_directory:
	.space	8192			/* L1 page table */

_avail_end:
	.word	0x1000000		/* 16MB default */

	.globl	_edata
	.globl	_end
