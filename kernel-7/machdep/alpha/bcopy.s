/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha bcopy/memcpy Implementation
 */

#include <architecture/alpha/asm_help.h>

	.text
	.set noreorder

/*
 * bcopy(src, dst, len)
 */
LEAF(bcopy)
	/* Simple byte-by-byte copy */
	beq	a2, 2f
1:	ldq_u	t0, 0(a0)
	stq_u	t0, 0(a1)
	addq	a0, 1, a0
	addq	a1, 1, a1
	subq	a2, 1, a2
	bne	a2, 1b
2:	ret	zero, (ra), 1
END(bcopy)

LEAF(memcpy)
	bis	a1, a1, v0
	br	bcopy
END(memcpy)
