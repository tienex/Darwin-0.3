/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha bzero/memset Implementation
 */

#include <architecture/alpha/asm_help.h>

	.text
	.set noreorder

/*
 * bzero(dst, len)
 */
LEAF(bzero)
	beq	a1, 2f
1:	stq_u	zero, 0(a0)
	addq	a0, 1, a0
	subq	a1, 1, a1
	bne	a1, 1b
2:	ret	zero, (ra), 1
END(bzero)
