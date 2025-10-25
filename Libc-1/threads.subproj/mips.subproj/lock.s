/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * MIPS thread locking primitives using LL/SC instructions
 */

#include <architecture/mips/asm_help.h>

	.text
	.set noreorder

/*
 * void spin_lock(int *lock)
 *
 * Acquire a spin lock using LL/SC atomic instructions
 */
LEAF(spin_lock)
1:
	ll	t0, 0(a0)	/* Load-linked: t0 = *lock */
	bnez	t0, 1b		/* if (*lock != 0) retry */
	nop
	li	t0, 1		/* t0 = 1 */
	sc	t0, 0(a0)	/* Store-conditional: *lock = 1 */
	beqz	t0, 1b		/* if (sc failed) retry */
	nop
	sync			/* Memory barrier */
	jr	ra
	nop
END(spin_lock)

/*
 * void spin_unlock(int *lock)
 *
 * Release a spin lock
 */
LEAF(spin_unlock)
	sync			/* Memory barrier */
	sw	zero, 0(a0)	/* *lock = 0 */
	jr	ra
	nop
END(spin_unlock)

/*
 * int spin_try_lock(int *lock)
 *
 * Try to acquire a spin lock
 * Returns 1 if lock acquired, 0 if lock already held
 */
LEAF(spin_try_lock)
	ll	t0, 0(a0)	/* Load-linked: t0 = *lock */
	bnez	t0, 2f		/* if (*lock != 0) return 0 */
	li	v0, 0		/* (delay slot) v0 = 0 */
	li	t0, 1		/* t0 = 1 */
	sc	t0, 0(a0)	/* Store-conditional: *lock = 1 */
	beqz	t0, 2f		/* if (sc failed) return 0 */
	nop
	sync			/* Memory barrier */
	jr	ra
	li	v0, 1		/* (delay slot) return 1 */
2:
	jr	ra
	nop
END(spin_try_lock)

/*
 * void mutex_lock(int *mutex)
 *
 * Acquire a mutex
 */
LEAF(mutex_lock)
1:
	ll	t0, 0(a0)	/* Load-linked: t0 = *mutex */
	bnez	t0, 1b		/* if (*mutex != 0) retry */
	nop
	li	t0, 1		/* t0 = 1 */
	sc	t0, 0(a0)	/* Store-conditional: *mutex = 1 */
	beqz	t0, 1b		/* if (sc failed) retry */
	nop
	sync			/* Memory barrier */
	jr	ra
	nop
END(mutex_lock)

/*
 * void mutex_unlock(int *mutex)
 *
 * Release a mutex
 */
LEAF(mutex_unlock)
	sync			/* Memory barrier */
	sw	zero, 0(a0)	/* *mutex = 0 */
	jr	ra
	nop
END(mutex_unlock)

/*
 * int mutex_try_lock(int *mutex)
 *
 * Try to acquire a mutex
 * Returns 1 if mutex acquired, 0 if mutex already held
 */
LEAF(mutex_try_lock)
	ll	t0, 0(a0)	/* Load-linked: t0 = *mutex */
	bnez	t0, 2f		/* if (*mutex != 0) return 0 */
	li	v0, 0		/* (delay slot) v0 = 0 */
	li	t0, 1		/* t0 = 1 */
	sc	t0, 0(a0)	/* Store-conditional: *mutex = 1 */
	beqz	t0, 2f		/* if (sc failed) return 0 */
	nop
	sync			/* Memory barrier */
	jr	ra
	li	v0, 1		/* (delay slot) return 1 */
2:
	jr	ra
	nop
END(mutex_try_lock)

/*
 * int atomic_add(int *value, int increment)
 *
 * Atomically add increment to *value and return old value
 */
LEAF(atomic_add)
1:
	ll	v0, 0(a0)	/* Load-linked: v0 = *value */
	addu	t0, v0, a1	/* t0 = *value + increment */
	sc	t0, 0(a0)	/* Store-conditional: *value = t0 */
	beqz	t0, 1b		/* if (sc failed) retry */
	nop
	sync			/* Memory barrier */
	jr	ra
	nop			/* return old value in v0 */
END(atomic_add)

/*
 * int atomic_sub(int *value, int decrement)
 *
 * Atomically subtract decrement from *value and return old value
 */
LEAF(atomic_sub)
1:
	ll	v0, 0(a0)	/* Load-linked: v0 = *value */
	subu	t0, v0, a1	/* t0 = *value - decrement */
	sc	t0, 0(a0)	/* Store-conditional: *value = t0 */
	beqz	t0, 1b		/* if (sc failed) retry */
	nop
	sync			/* Memory barrier */
	jr	ra
	nop			/* return old value in v0 */
END(atomic_sub)

/*
 * int compare_and_swap(int *ptr, int old_val, int new_val)
 *
 * Atomically compare *ptr with old_val and if equal, set to new_val
 * Returns 1 if swap occurred, 0 otherwise
 */
LEAF(compare_and_swap)
1:
	ll	t0, 0(a0)	/* Load-linked: t0 = *ptr */
	bne	t0, a1, 2f	/* if (*ptr != old_val) return 0 */
	li	v0, 0		/* (delay slot) v0 = 0 */
	move	t0, a2		/* t0 = new_val */
	sc	t0, 0(a0)	/* Store-conditional: *ptr = new_val */
	beqz	t0, 1b		/* if (sc failed) retry */
	nop
	sync			/* Memory barrier */
	jr	ra
	li	v0, 1		/* (delay slot) return 1 */
2:
	jr	ra
	nop
END(compare_and_swap)
