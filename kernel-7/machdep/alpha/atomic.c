/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Atomic Operations
 *
 * Alpha provides atomic operations through load-locked/store-conditional
 * instructions (LDx_L / STx_C). These form the basis for lock-free
 * data structures and synchronization primitives.
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>

/*
 * Atomic compare-and-swap (32-bit)
 *
 * Atomically compares *ptr with old_val, and if equal, sets *ptr to new_val.
 * Returns the original value of *ptr.
 */
unsigned int
alpha_atomic_cas_32(volatile unsigned int *ptr, unsigned int old_val, unsigned int new_val)
{
	unsigned int prev, tmp;

	__asm__ volatile (
		"1:	ldl_l	%0, 0(%3)\n"		/* Load-locked */
		"	cmpeq	%0, %4, %1\n"		/* Compare with old_val */
		"	beq	%1, 2f\n"		/* If not equal, exit */
		"	bis	%5, %5, %1\n"		/* Move new_val to tmp */
		"	stl_c	%1, 0(%3)\n"		/* Store-conditional */
		"	beq	%1, 1b\n"		/* Retry if failed */
		"2:	mb\n"				/* Memory barrier */
		: "=&r" (prev), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (old_val), "r" (new_val)
		: "memory"
	);

	return prev;
}

/*
 * Atomic compare-and-swap (64-bit)
 */
unsigned long
alpha_atomic_cas_64(volatile unsigned long *ptr, unsigned long old_val, unsigned long new_val)
{
	unsigned long prev, tmp;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%3)\n"
		"	cmpeq	%0, %4, %1\n"
		"	beq	%1, 2f\n"
		"	bis	%5, %5, %1\n"
		"	stq_c	%1, 0(%3)\n"
		"	beq	%1, 1b\n"
		"2:	mb\n"
		: "=&r" (prev), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (old_val), "r" (new_val)
		: "memory"
	);

	return prev;
}

/*
 * Atomic increment (32-bit)
 * Returns the new value.
 */
unsigned int
alpha_atomic_inc_32(volatile unsigned int *ptr)
{
	unsigned int val, tmp;

	__asm__ volatile (
		"1:	ldl_l	%0, 0(%2)\n"
		"	addl	%0, 1, %1\n"
		"	stl_c	%1, 0(%2)\n"
		"	beq	%1, 1b\n"
		"	addl	%0, 1, %0\n"
		"	mb\n"
		: "=&r" (val), "=&r" (tmp)
		: "r" (ptr), "m" (*ptr)
		: "memory"
	);

	return val;
}

/*
 * Atomic increment (64-bit)
 */
unsigned long
alpha_atomic_inc_64(volatile unsigned long *ptr)
{
	unsigned long val, tmp;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%2)\n"
		"	addq	%0, 1, %1\n"
		"	stq_c	%1, 0(%2)\n"
		"	beq	%1, 1b\n"
		"	addq	%0, 1, %0\n"
		"	mb\n"
		: "=&r" (val), "=&r" (tmp)
		: "r" (ptr), "m" (*ptr)
		: "memory"
	);

	return val;
}

/*
 * Atomic decrement (32-bit)
 * Returns the new value.
 */
unsigned int
alpha_atomic_dec_32(volatile unsigned int *ptr)
{
	unsigned int val, tmp;

	__asm__ volatile (
		"1:	ldl_l	%0, 0(%2)\n"
		"	subl	%0, 1, %1\n"
		"	stl_c	%1, 0(%2)\n"
		"	beq	%1, 1b\n"
		"	subl	%0, 1, %0\n"
		"	mb\n"
		: "=&r" (val), "=&r" (tmp)
		: "r" (ptr), "m" (*ptr)
		: "memory"
	);

	return val;
}

/*
 * Atomic decrement (64-bit)
 */
unsigned long
alpha_atomic_dec_64(volatile unsigned long *ptr)
{
	unsigned long val, tmp;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%2)\n"
		"	subq	%0, 1, %1\n"
		"	stq_c	%1, 0(%2)\n"
		"	beq	%1, 1b\n"
		"	subq	%0, 1, %0\n"
		"	mb\n"
		: "=&r" (val), "=&r" (tmp)
		: "r" (ptr), "m" (*ptr)
		: "memory"
	);

	return val;
}

/*
 * Atomic add (32-bit)
 * Returns the old value.
 */
unsigned int
alpha_atomic_add_32(volatile unsigned int *ptr, unsigned int val)
{
	unsigned int old, tmp;

	__asm__ volatile (
		"1:	ldl_l	%0, 0(%3)\n"
		"	addl	%0, %4, %1\n"
		"	stl_c	%1, 0(%3)\n"
		"	beq	%1, 1b\n"
		"	mb\n"
		: "=&r" (old), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (val)
		: "memory"
	);

	return old;
}

/*
 * Atomic add (64-bit)
 */
unsigned long
alpha_atomic_add_64(volatile unsigned long *ptr, unsigned long val)
{
	unsigned long old, tmp;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%3)\n"
		"	addq	%0, %4, %1\n"
		"	stq_c	%1, 0(%3)\n"
		"	beq	%1, 1b\n"
		"	mb\n"
		: "=&r" (old), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (val)
		: "memory"
	);

	return old;
}

/*
 * Atomic OR (64-bit)
 */
unsigned long
alpha_atomic_or_64(volatile unsigned long *ptr, unsigned long mask)
{
	unsigned long old, tmp;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%3)\n"
		"	bis	%0, %4, %1\n"
		"	stq_c	%1, 0(%3)\n"
		"	beq	%1, 1b\n"
		"	mb\n"
		: "=&r" (old), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (mask)
		: "memory"
	);

	return old;
}

/*
 * Atomic AND (64-bit)
 */
unsigned long
alpha_atomic_and_64(volatile unsigned long *ptr, unsigned long mask)
{
	unsigned long old, tmp;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%3)\n"
		"	and	%0, %4, %1\n"
		"	stq_c	%1, 0(%3)\n"
		"	beq	%1, 1b\n"
		"	mb\n"
		: "=&r" (old), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (mask)
		: "memory"
	);

	return old;
}

/*
 * Atomic exchange (swap)
 */
unsigned long
alpha_atomic_xchg_64(volatile unsigned long *ptr, unsigned long new_val)
{
	unsigned long old, tmp;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%3)\n"
		"	bis	%4, %4, %1\n"
		"	stq_c	%1, 0(%3)\n"
		"	beq	%1, 1b\n"
		"	mb\n"
		: "=&r" (old), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (new_val)
		: "memory"
	);

	return old;
}

/*
 * Atomic test-and-set
 * Returns 1 if bit was already set, 0 otherwise.
 */
int
alpha_atomic_test_and_set(volatile unsigned long *ptr, int bit)
{
	unsigned long old, tmp, mask;

	mask = 1UL << bit;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%4)\n"
		"	bis	%0, %5, %1\n"
		"	stq_c	%1, 0(%4)\n"
		"	beq	%1, 1b\n"
		"	and	%0, %5, %2\n"
		"	mb\n"
		: "=&r" (old), "=&r" (tmp), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (mask)
		: "memory"
	);

	return (old & mask) != 0;
}

/*
 * Atomic test-and-clear
 * Returns 1 if bit was set, 0 otherwise.
 */
int
alpha_atomic_test_and_clear(volatile unsigned long *ptr, int bit)
{
	unsigned long old, tmp, mask;

	mask = 1UL << bit;

	__asm__ volatile (
		"1:	ldq_l	%0, 0(%4)\n"
		"	bic	%0, %5, %1\n"
		"	stq_c	%1, 0(%4)\n"
		"	beq	%1, 1b\n"
		"	and	%0, %5, %2\n"
		"	mb\n"
		: "=&r" (old), "=&r" (tmp), "=&r" (tmp)
		: "m" (*ptr), "r" (ptr), "r" (mask)
		: "memory"
	);

	return (old & mask) != 0;
}

/*
 * Spinlock implementation using atomic operations
 */
typedef struct {
	volatile unsigned long lock;
} alpha_spinlock_t;

void
alpha_spin_lock(alpha_spinlock_t *lock)
{
	while (alpha_atomic_xchg_64(&lock->lock, 1) != 0) {
		/* Spin while lock is held */
		while (lock->lock != 0)
			/* Busy wait */ ;
	}
}

void
alpha_spin_unlock(alpha_spinlock_t *lock)
{
	alpha_wmb();
	lock->lock = 0;
}

int
alpha_spin_trylock(alpha_spinlock_t *lock)
{
	return alpha_atomic_xchg_64(&lock->lock, 1) == 0;
}

/*
 * Read-write lock implementation
 */
typedef struct {
	volatile long readers;	/* Number of readers (negative means writer) */
} alpha_rwlock_t;

void
alpha_read_lock(alpha_rwlock_t *lock)
{
	long old;

	do {
		old = lock->readers;
		while (old < 0) {
			/* Writer holds lock, wait */
			old = lock->readers;
		}
	} while (alpha_atomic_cas_64((unsigned long *)&lock->readers,
				     old, old + 1) != old);

	alpha_mb();
}

void
alpha_read_unlock(alpha_rwlock_t *lock)
{
	alpha_mb();
	alpha_atomic_dec_64((unsigned long *)&lock->readers);
}

void
alpha_write_lock(alpha_rwlock_t *lock)
{
	while (alpha_atomic_cas_64((unsigned long *)&lock->readers, 0, -1) != 0) {
		/* Wait for all readers to finish */
		while (lock->readers != 0)
			/* Busy wait */ ;
	}

	alpha_mb();
}

void
alpha_write_unlock(alpha_rwlock_t *lock)
{
	alpha_mb();
	lock->readers = 0;
}
