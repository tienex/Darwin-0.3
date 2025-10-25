/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Userspace PAL Call Interface for Alpha
 *
 * This header provides the interface for userspace programs to call
 * PALcode functions through the kernel-managed trampoline page.
 */

#ifndef _ALPHA_PAL_USER_H_
#define _ALPHA_PAL_USER_H_

/*
 * PAL call page location
 * This page is mapped read-only + executable at the very end of
 * user address space in every process.
 */
#define PAL_CALL_PAGE_BASE	0x000003ffffffe000UL

/*
 * PAL call trampoline offset
 * Each PAL function has a 16-byte trampoline at:
 *   PAL_CALL_PAGE_BASE + (function_number * 16)
 */
#define PAL_CALL_OFFSET(func)	((func) * 16)
#define PAL_CALL_ADDR(func)	(PAL_CALL_PAGE_BASE + PAL_CALL_OFFSET(func))

/*
 * Userspace-accessible PAL functions
 * These are the only PAL functions that can be safely called from userspace.
 * Dangerous functions (halt, swpctx, etc.) will cause a kernel trap.
 */

/* UNIX PALcode functions (SRM firmware) */
#define PAL_USER_bpt		0x80	/* Breakpoint */
#define PAL_USER_bugchk		0x81	/* Bugcheck */
#define PAL_USER_gentrap	0xAA	/* Generate trap */
#define PAL_USER_rdunique	0x9E	/* Read unique (thread-local storage) */
#define PAL_USER_wrunique	0x9F	/* Write unique (thread-local storage) */

/* NT PALcode functions (ARC firmware) */
#define PAL_USER_NT_bpt		0x80	/* Breakpoint */
#define PAL_USER_NT_bugchk	0x81	/* Bugcheck */
#define PAL_USER_NT_gentrap	0xAA	/* Generate trap */

/*
 * Inline functions for calling PAL from userspace
 * These provide a convenient C interface to the PAL trampolines.
 */

#ifdef __ASSEMBLER__

/*
 * Assembly macro for PAL calls
 * Usage: PAL_CALL function_number
 */
#define PAL_CALL(func) \
	lda	$27, PAL_CALL_ADDR(func); \
	jsr	$26, ($27), 0

#else /* !__ASSEMBLER__ */

/*
 * Read unique value (thread-local storage pointer)
 */
static inline unsigned long
pal_rdunique(void)
{
	unsigned long result;
	void (*pal_func)(void) = (void (*)(void))PAL_CALL_ADDR(PAL_USER_rdunique);

	__asm__ volatile (
		"jsr $26, (%1), 0\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "r" (pal_func)
		: "$0", "$26", "$27", "memory"
	);

	return result;
}

/*
 * Write unique value (thread-local storage pointer)
 */
static inline void
pal_wrunique(unsigned long value)
{
	void (*pal_func)(void) = (void (*)(void))PAL_CALL_ADDR(PAL_USER_wrunique);

	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"jsr $26, (%1), 0"
		:
		: "r" (value), "r" (pal_func)
		: "$0", "$16", "$26", "$27", "memory"
	);
}

/*
 * Breakpoint
 */
static inline void
pal_bpt(void)
{
	void (*pal_func)(void) = (void (*)(void))PAL_CALL_ADDR(PAL_USER_bpt);

	__asm__ volatile (
		"jsr $26, (%0), 0"
		:
		: "r" (pal_func)
		: "$0", "$26", "$27", "memory"
	);
}

/*
 * Generate trap with code
 */
static inline void
pal_gentrap(unsigned long code)
{
	void (*pal_func)(void) = (void (*)(void))PAL_CALL_ADDR(PAL_USER_gentrap);

	__asm__ volatile (
		"bis %0, %0, $16\n\t"
		"jsr $26, (%1), 0"
		:
		: "r" (code), "r" (pal_func)
		: "$0", "$16", "$26", "$27", "memory"
	);
}

/*
 * Generic PAL call (use with caution - not all functions are safe!)
 */
static inline unsigned long
pal_call(unsigned long function, unsigned long a0, unsigned long a1,
         unsigned long a2, unsigned long a3)
{
	unsigned long result;
	void (*pal_func)(void) = (void (*)(void))PAL_CALL_ADDR(function);

	__asm__ volatile (
		"bis %1, %1, $16\n\t"
		"bis %2, %2, $17\n\t"
		"bis %3, %3, $18\n\t"
		"bis %4, %4, $19\n\t"
		"jsr $26, (%5), 0\n\t"
		"bis $0, $0, %0"
		: "=r" (result)
		: "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (pal_func)
		: "$0", "$16", "$17", "$18", "$19", "$26", "$27", "memory"
	);

	return result;
}

#endif /* __ASSEMBLER__ */

/*
 * Thread-local storage convenience macros
 * Alpha uses the PAL unique value for TLS
 */
#define alpha_get_tls()		pal_rdunique()
#define alpha_set_tls(val)	pal_wrunique(val)

#endif /* _ALPHA_PAL_USER_H_ */
