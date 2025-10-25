/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Early Initialization
 */

#include <mach/mach_types.h>
#include <mach/machine.h>
#include <kern/cpu_number.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>

/* External references */
extern unsigned long boot_hwrpb;
extern unsigned long boot_pgtbl;
extern unsigned long boot_flags;

/* PALcode variant detected at boot */
enum alpha_pal_variant current_pal_variant = PAL_VARIANT_UNKNOWN;

/* CPU information */
alpha_cpu_info_t cpu_info;

/*
 * Detect PALcode variant
 *
 * We try to determine which PALcode variant is loaded by attempting
 * to execute PALcode functions that are specific to each variant.
 */
static void
detect_pal_variant(void)
{
	unsigned long test_val;

	/*
	 * Try UNIX PALcode-specific function first (most common for Darwin)
	 */
	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $0, $0, %0"
		: "=r" (test_val)
		: "i" (PAL_UNIX_whami)
		: "$0"
	);

	/* If we got here without a fault, we're likely running UNIX PALcode */
	current_pal_variant = PAL_VARIANT_UNIX;

	/*
	 * TODO: Add more sophisticated detection if needed
	 * For now, we assume UNIX PALcode as it's most appropriate for Darwin
	 */
}

/*
 * Detect CPU implementation and features
 */
static void
detect_cpu_features(void)
{
	unsigned long impl_ver;

	/*
	 * Read implementation version from AMASK/IMPLVER
	 * IMPLVER returns implementation type (EV4, EV5, EV6, etc.)
	 */
	__asm__ volatile (
		"implver %0"
		: "=r" (impl_ver)
	);

	cpu_info.implementation = impl_ver;

	/*
	 * Check for instruction set extensions using AMASK
	 * AMASK returns a bit mask of unsupported extensions
	 */
	unsigned long amask_bwx, amask_fix, amask_cix, amask_mvi;

	__asm__ volatile ("amask %1, %0" : "=r" (amask_bwx) : "i" (ALPHA_EXT_BWX));
	__asm__ volatile ("amask %1, %0" : "=r" (amask_fix) : "i" (ALPHA_EXT_FIX));
	__asm__ volatile ("amask %1, %0" : "=r" (amask_cix) : "i" (ALPHA_EXT_CIX));
	__asm__ volatile ("amask %1, %0" : "=r" (amask_mvi) : "i" (ALPHA_EXT_MVI));

	cpu_info.extensions = 0;
	if ((amask_bwx & ALPHA_EXT_BWX) == 0)
		cpu_info.extensions |= ALPHA_EXT_BWX;
	if ((amask_fix & ALPHA_EXT_FIX) == 0)
		cpu_info.extensions |= ALPHA_EXT_FIX;
	if ((amask_cix & ALPHA_EXT_CIX) == 0)
		cpu_info.extensions |= ALPHA_EXT_CIX;
	if ((amask_mvi & ALPHA_EXT_MVI) == 0)
		cpu_info.extensions |= ALPHA_EXT_MVI;

	/*
	 * Set cache sizes based on implementation
	 */
	switch (impl_ver) {
	case ALPHA_IMPL_EV4:
	case ALPHA_IMPL_EV45:
		cpu_info.icache_size = ALPHA_EV4_ICACHE_SIZE;
		cpu_info.dcache_size = ALPHA_EV4_DCACHE_SIZE;
		cpu_info.scache_size = 0;
		break;

	case ALPHA_IMPL_EV5:
	case ALPHA_IMPL_EV56:
	case ALPHA_IMPL_PCA56:
		cpu_info.icache_size = ALPHA_EV5_ICACHE_SIZE;
		cpu_info.dcache_size = ALPHA_EV5_DCACHE_SIZE;
		cpu_info.scache_size = ALPHA_EV5_SCACHE_SIZE;
		break;

	case ALPHA_IMPL_EV6:
	case ALPHA_IMPL_EV67:
		cpu_info.icache_size = ALPHA_EV6_ICACHE_SIZE;
		cpu_info.dcache_size = ALPHA_EV6_DCACHE_SIZE;
		cpu_info.scache_size = 0;	/* EV6 has no external cache */
		break;

	default:
		/* Unknown implementation, use conservative values */
		cpu_info.icache_size = 8192;
		cpu_info.dcache_size = 8192;
		cpu_info.scache_size = 0;
		break;
	}

	cpu_info.pal_variant = current_pal_variant;
}

/*
 * Initialize PALcode interface
 *
 * Set up PALcode entry points for exceptions, interrupts, and system calls.
 */
static void
init_palcode(void)
{
	extern void alpha_exception_entry(void);
	extern void alpha_interrupt_entry(void);
	extern void alpha_syscall_entry(void);

	/*
	 * Set up entry points based on PALcode variant
	 */
	switch (current_pal_variant) {
	case PAL_VARIANT_UNIX:
		/*
		 * For UNIX PALcode, use WRENT to set exception entry points
		 */
		__asm__ volatile (
			"lda $16, alpha_exception_entry\n\t"
			"lda $17, 0\n\t"	/* Entry type 0: arithmetic exception */
			"call_pal %0"
			: : "i" (PAL_UNIX_wrent) : "$16", "$17", "memory"
		);

		__asm__ volatile (
			"lda $16, alpha_interrupt_entry\n\t"
			"lda $17, 1\n\t"	/* Entry type 1: interrupt */
			"call_pal %0"
			: : "i" (PAL_UNIX_wrent) : "$16", "$17", "memory"
		);

		__asm__ volatile (
			"lda $16, alpha_syscall_entry\n\t"
			"lda $17, 2\n\t"	/* Entry type 2: system call */
			"call_pal %0"
			: : "i" (PAL_UNIX_wrent) : "$16", "$17", "memory"
		);
		break;

	case PAL_VARIANT_NT:
		/*
		 * For Windows NT PALcode, use WRENTRY to set entry points
		 */
		__asm__ volatile (
			"lda $16, alpha_exception_entry\n\t"
			"call_pal %0"
			: : "i" (PAL_NT_wrentry) : "$16", "memory"
		);
		break;

	case PAL_VARIANT_VMS:
		/*
		 * For VMS PALcode, entry points are in the SCB (System Control Block)
		 * which is set up by the firmware
		 */
		break;

	default:
		/* Unknown PALcode, can't proceed */
		__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_halt));
		break;
	}
}

/*
 * Initialize virtual memory
 */
static void
init_vm(void)
{
	extern void alpha_pmap_bootstrap(unsigned long);

	/*
	 * Bootstrap the physical map module
	 * This sets up kernel page tables and enables virtual memory
	 */
	alpha_pmap_bootstrap(boot_pgtbl);
}

/*
 * Main Alpha initialization routine
 *
 * Called from start.s before entering generic kernel initialization.
 */
void
alpha_init(void)
{
	/*
	 * Detect PALcode variant
	 */
	detect_pal_variant();

	/*
	 * Detect CPU features
	 */
	detect_cpu_features();

	/*
	 * Initialize PALcode interface
	 */
	init_palcode();

	/*
	 * Initialize virtual memory
	 */
	init_vm();

	/*
	 * Early console initialization would go here
	 */

	/*
	 * At this point, we're ready for generic kernel initialization
	 */
}

/*
 * Get CPU information
 */
void
alpha_get_cpu_info(alpha_cpu_info_t *info)
{
	*info = cpu_info;
}

/*
 * Get current PALcode variant
 */
enum alpha_pal_variant
alpha_get_pal_variant(void)
{
	return current_pal_variant;
}
