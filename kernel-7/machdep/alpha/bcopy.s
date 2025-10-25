/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha bcopy/memcpy Implementation
 *
 * Optimized memory copy routines for Alpha architecture.
 */

#include <architecture/alpha/asm_help.h>

	.text
	.set noreorder

/*
 * bcopy(src, dst, len)
 * memcpy(dst, src, len)
 *
 * Copy len bytes from src to dst. Handles overlapping regions.
 */
LEAF(bcopy)
	/* Check for zero length */
	beq	a2, bcopy_done

	/* Check for overlap: if dst < src or dst >= src+len, no overlap */
	cmpult	a1, a0, t0		/* t0 = (dst < src) */
	bne	t0, bcopy_forward	/* If dst < src, copy forward */

	addq	a0, a2, t1		/* t1 = src + len */
	cmpult	a1, t1, t0		/* t0 = (dst < src+len) */
	beq	t0, bcopy_forward	/* If dst >= src+len, copy forward */

	/* Overlap detected, copy backward */
	addq	a0, a2, a0		/* src = src + len */
	addq	a1, a2, a1		/* dst = dst + len */

bcopy_backward:
	/* Copy backward one byte at a time */
	subq	a0, 1, a0
	subq	a1, 1, a1
	ldbu	t0, 0(a0)
	stb	t0, 0(a1)
	subq	a2, 1, a2
	bne	a2, bcopy_backward
	br	bcopy_done

bcopy_forward:
	/* Check alignment */
	or	a0, a1, t0
	and	t0, 7, t0
	bne	t0, bcopy_byte		/* Not aligned, use byte copy */

	/* Both src and dst are 8-byte aligned */
	/* Copy 8 bytes at a time */
	cmpult	a2, 8, t0
	bne	t0, bcopy_byte		/* Less than 8 bytes, use byte copy */

bcopy_quad:
	ldq	t0, 0(a0)
	stq	t0, 0(a1)
	addq	a0, 8, a0
	addq	a1, 8, a1
	subq	a2, 8, a2
	cmpult	a2, 8, t1
	beq	t1, bcopy_quad

	/* Copy remaining bytes */
	beq	a2, bcopy_done

bcopy_byte:
	/* Copy one byte at a time */
	ldbu	t0, 0(a0)
	stb	t0, 0(a1)
	addq	a0, 1, a0
	addq	a1, 1, a1
	subq	a2, 1, a2
	bne	a2, bcopy_byte

bcopy_done:
	ret	zero, (ra), 1

END(bcopy)

/*
 * memcpy(dst, src, len)
 *
 * Standard C memcpy with different argument order than bcopy
 */
LEAF(memcpy)
	bis	a0, a0, v0		/* Return original dst */
	bis	a1, a1, t0		/* Save src */
	bis	a0, a0, a1		/* dst -> a1 */
	bis	t0, t0, a0		/* src -> a0 */
	br	bcopy			/* Call bcopy */
END(memcpy)

/*
 * memmove(dst, src, len)
 *
 * Same as memcpy but handles overlapping regions
 */
LEAF(memmove)
	bis	a0, a0, v0		/* Return original dst */
	bis	a1, a1, t0		/* Save src */
	bis	a0, a0, a1		/* dst -> a1 */
	bis	t0, t0, a0		/* src -> a0 */
	br	bcopy			/* Call bcopy */
END(memmove)
