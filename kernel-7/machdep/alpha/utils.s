/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Utility Functions (Assembly)
 */

#include <architecture/alpha/asm_help.h>
#include <architecture/alpha/pal.h>

	.text
	.set noreorder

/*
 * strlen - Calculate string length
 */
LEAF(strlen)
	bis	zero, zero, v0		/* Initialize count to 0 */
1:	ldq_u	t0, 0(a0)		/* Load unaligned quad */
	cmpbge	zero, t0, t1		/* Compare bytes for zero */
	bne	t1, 2f			/* Found zero byte */
	addq	v0, 8, v0		/* Add 8 to count */
	addq	a0, 8, a0		/* Advance pointer */
	br	1b			/* Continue loop */
2:	/* Found zero, count trailing bytes */
	blbs	t1, 3f			/* Byte 0 is zero */
	addq	v0, 1, v0
	srl	t1, 8, t1
	blbs	t1, 3f			/* Byte 1 is zero */
	addq	v0, 1, v0
	srl	t1, 8, t1
	blbs	t1, 3f			/* Byte 2 is zero */
	addq	v0, 1, v0
	srl	t1, 8, t1
	blbs	t1, 3f			/* Byte 3 is zero */
	addq	v0, 1, v0
	srl	t1, 8, t1
	blbs	t1, 3f			/* Byte 4 is zero */
	addq	v0, 1, v0
	srl	t1, 8, t1
	blbs	t1, 3f			/* Byte 5 is zero */
	addq	v0, 1, v0
	srl	t1, 8, t1
	blbs	t1, 3f			/* Byte 6 is zero */
	addq	v0, 1, v0
3:	ret	zero, (ra), 1
END(strlen)

/*
 * strcmp - Compare two strings
 */
LEAF(strcmp)
1:	ldq_u	t0, 0(a0)		/* Load from s1 */
	ldq_u	t1, 0(a1)		/* Load from s2 */
	cmpbge	zero, t0, t2		/* Check for null in s1 */
	bne	t2, 2f			/* Found null */
	cmpeq	t0, t1, t3		/* Compare quads */
	beq	t3, 2f			/* Not equal */
	addq	a0, 8, a0		/* Advance s1 */
	addq	a1, 8, a1		/* Advance s2 */
	br	1b			/* Continue */
2:	/* Do byte-by-byte comparison */
	ldbu	t0, 0(a0)
	ldbu	t1, 0(a1)
	subl	t0, t1, v0
	bne	v0, 3f			/* Different */
	beq	t0, 3f			/* Both null */
	addq	a0, 1, a0
	addq	a1, 1, a1
	br	2b
3:	ret	zero, (ra), 1
END(strcmp)

/*
 * ffs - Find first bit set
 * Returns position of first set bit (1-based), or 0 if no bits set
 */
LEAF(ffs)
	beq	a0, 2f			/* Return 0 if input is 0 */

	/* Use cmpbge trick to find first set bit */
	bis	zero, zero, v0		/* Initialize result */
	negq	a0, t0			/* Two's complement */
	and	a0, t0, t0		/* Isolate rightmost set bit */

	/* Count trailing zeros */
	bis	zero, 1, v0
1:	srl	t0, 1, t1
	beq	t1, 2f
	addq	v0, 1, v0
	bis	t1, t1, t0
	br	1b

2:	ret	zero, (ra), 1
END(ffs)

/*
 * setjmp - Save execution context
 */
LEAF(setjmp)
	stq	s0, 0(a0)
	stq	s1, 8(a0)
	stq	s2, 16(a0)
	stq	s3, 24(a0)
	stq	s4, 32(a0)
	stq	s5, 40(a0)
	stq	fp, 48(a0)
	stq	ra, 56(a0)
	stq	sp, 64(a0)
	bis	zero, zero, v0		/* Return 0 */
	ret	zero, (ra), 1
END(setjmp)

/*
 * longjmp - Restore execution context
 */
LEAF(longjmp)
	ldq	s0, 0(a0)
	ldq	s1, 8(a0)
	ldq	s2, 16(a0)
	ldq	s3, 24(a0)
	ldq	s4, 32(a0)
	ldq	s5, 40(a0)
	ldq	fp, 48(a0)
	ldq	ra, 56(a0)
	ldq	sp, 64(a0)
	bis	a1, a1, v0		/* Return val */
	bne	v0, 1f
	bis	zero, 1, v0		/* Never return 0 */
1:	ret	zero, (ra), 1
END(longjmp)

/*
 * memchr - Find byte in memory
 */
LEAF(memchr)
	beq	a2, 2f			/* Length is 0 */
	and	a1, 0xFF, a1		/* Mask to byte */
1:	ldbu	t0, 0(a0)		/* Load byte */
	cmpeq	t0, a1, t1		/* Compare */
	bne	t1, 3f			/* Found */
	addq	a0, 1, a0		/* Advance */
	subq	a2, 1, a2		/* Decrement count */
	bne	a2, 1b			/* Continue */
2:	bis	zero, zero, v0		/* Not found */
	ret	zero, (ra), 1
3:	bis	a0, a0, v0		/* Return pointer */
	ret	zero, (ra), 1
END(memchr)

/*
 * delay - Delay for specified number of microseconds
 */
LEAF(delay)
	beq	a0, 2f			/* Zero delay */

	/* Calculate cycles from microseconds */
	/* Assume 500 MHz = 500 cycles per microsecond */
	s8addq	a0, a0, t0		/* t0 = us * 9 */
	s8addq	t0, t0, t0		/* t0 = us * 81 */
	s8addq	t0, a0, t0		/* t0 = us * 730 (approx) */

	rpcc	t1			/* Read start cycle count */
	addq	t1, t0, t0		/* Calculate end cycle */

1:	rpcc	t1			/* Read current cycle count */
	cmpult	t1, t0, t2		/* Compare with end */
	bne	t2, 1b			/* Continue if not done */

2:	ret	zero, (ra), 1
END(delay)
