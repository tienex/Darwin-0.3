/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha bzero/memset Implementation
 *
 * Optimized memory zeroing routines for Alpha architecture.
 */

#include <architecture/alpha/asm_help.h>

	.text
	.set noreorder

/*
 * bzero(dst, len)
 *
 * Zero len bytes at dst
 */
LEAF(bzero)
	/* Check for zero length */
	beq	a1, bzero_done

	/* Check alignment */
	and	a0, 7, t0
	bne	t0, bzero_byte		/* Not aligned, use byte zero */

	/* dst is 8-byte aligned */
	/* Zero 8 bytes at a time */
	cmpult	a1, 8, t0
	bne	t0, bzero_byte		/* Less than 8 bytes, use byte zero */

bzero_quad:
	stq	zero, 0(a0)
	addq	a0, 8, a0
	subq	a1, 8, a1
	cmpult	a1, 8, t1
	beq	t1, bzero_quad

	/* Zero remaining bytes */
	beq	a1, bzero_done

bzero_byte:
	/* Zero one byte at a time */
	stb	zero, 0(a0)
	addq	a0, 1, a0
	subq	a1, 1, a1
	bne	a1, bzero_byte

bzero_done:
	ret	zero, (ra), 1

END(bzero)

/*
 * memset(dst, c, len)
 *
 * Set len bytes at dst to value c
 */
LEAF(memset)
	bis	a0, a0, v0		/* Return original dst */
	and	a1, 0xFF, a1		/* Mask to byte */
	beq	a2, memset_done		/* Zero length */

	/* Check if filling with zero */
	beq	a1, memset_zero

	/* Replicate byte to quadword */
	sll	a1, 8, t0
	bis	a1, t0, a1
	sll	a1, 16, t0
	bis	a1, t0, a1
	sll	a1, 32, t0
	bis	a1, t0, a1		/* a1 now has byte replicated 8 times */

	/* Check alignment */
	and	a0, 7, t0
	bne	t0, memset_byte		/* Not aligned, use byte fill */

	/* dst is 8-byte aligned */
	/* Fill 8 bytes at a time */
	cmpult	a2, 8, t0
	bne	t0, memset_byte		/* Less than 8 bytes, use byte fill */

memset_quad:
	stq	a1, 0(a0)
	addq	a0, 8, a0
	subq	a2, 8, a2
	cmpult	a2, 8, t1
	beq	t1, memset_quad

	/* Fill remaining bytes */
	beq	a2, memset_done

memset_byte:
	/* Fill one byte at a time */
	stb	a1, 0(a0)
	addq	a0, 1, a0
	subq	a2, 1, a2
	bne	a2, memset_byte
	br	memset_done

memset_zero:
	/* Filling with zero, use bzero path */
	bis	a2, a2, a1		/* len -> a1 */
	br	bzero

memset_done:
	ret	zero, (ra), 1

END(memset)
