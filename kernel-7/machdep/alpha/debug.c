/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Debugging Support
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/reg.h>

/*
 * Print saved state for debugging
 */
void
alpha_print_saved_state(alpha_saved_state_t *state)
{
	printf("\nAlpha Saved State at 0x%lx:\n", (unsigned long)state);
	printf("  PC:  0x%016lx   PS:  0x%016lx\n", state->pc, state->ps);
	printf("  SP:  0x%016lx   GP:  0x%016lx\n", state->sp, state->gp);
	printf("  RA:  0x%016lx   FP:  0x%016lx\n", state->ra, state->fp);
	printf("\nInteger Registers:\n");
	printf("  v0:  0x%016lx   t0:  0x%016lx\n", state->v0, state->t0);
	printf("  t1:  0x%016lx   t2:  0x%016lx\n", state->t1, state->t2);
	printf("  t3:  0x%016lx   t4:  0x%016lx\n", state->t3, state->t4);
	printf("  t5:  0x%016lx   t6:  0x%016lx\n", state->t5, state->t6);
	printf("  t7:  0x%016lx   s0:  0x%016lx\n", state->t7, state->s0);
	printf("  s1:  0x%016lx   s2:  0x%016lx\n", state->s1, state->s2);
	printf("  s3:  0x%016lx   s4:  0x%016lx\n", state->s3, state->s4);
	printf("  s5:  0x%016lx   a0:  0x%016lx\n", state->s5, state->a0);
	printf("  a1:  0x%016lx   a2:  0x%016lx\n", state->a1, state->a2);
	printf("  a3:  0x%016lx   a4:  0x%016lx\n", state->a3, state->a4);
	printf("  a5:  0x%016lx   t8:  0x%016lx\n", state->a5, state->t8);
	printf("  t9:  0x%016lx   t10: 0x%016lx\n", state->t9, state->t10);
	printf("  t11: 0x%016lx   t12: 0x%016lx\n", state->t11, state->t12);
	printf("  at:  0x%016lx\n", state->at);
}

/*
 * Print CPU information
 */
void
alpha_print_cpu_info(void)
{
	extern alpha_cpu_info_t cpu_info;

	printf("\nAlpha CPU Information:\n");
	printf("  Implementation: ");
	switch (cpu_info.implementation) {
	case ALPHA_IMPL_EV3:
		printf("EV3 (21064)\n");
		break;
	case ALPHA_IMPL_EV4:
		printf("EV4 (21064A)\n");
		break;
	case ALPHA_IMPL_LCA:
		printf("LCA (21066)\n");
		break;
	case ALPHA_IMPL_EV45:
		printf("EV45 (21064A)\n");
		break;
	case ALPHA_IMPL_EV5:
		printf("EV5 (21164)\n");
		break;
	case ALPHA_IMPL_EV56:
		printf("EV56 (21164A)\n");
		break;
	case ALPHA_IMPL_PCA56:
		printf("PCA56 (21164PC)\n");
		break;
	case ALPHA_IMPL_EV6:
		printf("EV6 (21264)\n");
		break;
	case ALPHA_IMPL_EV67:
		printf("EV67 (21264A)\n");
		break;
	default:
		printf("Unknown (%ld)\n", cpu_info.implementation);
		break;
	}

	printf("  Extensions: ");
	if (cpu_info.extensions & ALPHA_EXT_BWX)
		printf("BWX ");
	if (cpu_info.extensions & ALPHA_EXT_FIX)
		printf("FIX ");
	if (cpu_info.extensions & ALPHA_EXT_CIX)
		printf("CIX ");
	if (cpu_info.extensions & ALPHA_EXT_MVI)
		printf("MVI ");
	if (cpu_info.extensions & ALPHA_EXT_PAT)
		printf("PAT ");
	if (cpu_info.extensions == 0)
		printf("None");
	printf("\n");

	printf("  PALcode: ");
	switch (cpu_info.pal_variant) {
	case PAL_VARIANT_UNIX:
		printf("UNIX (Digital UNIX/Tru64)\n");
		break;
	case PAL_VARIANT_NT:
		printf("Windows NT\n");
		break;
	case PAL_VARIANT_VMS:
		printf("OpenVMS\n");
		break;
	default:
		printf("Unknown\n");
		break;
	}

	printf("  I-cache: %ld KB\n", cpu_info.icache_size / 1024);
	printf("  D-cache: %ld KB\n", cpu_info.dcache_size / 1024);
	if (cpu_info.scache_size > 0)
		printf("  S-cache: %ld KB\n", cpu_info.scache_size / 1024);
}

/*
 * Backtrace for debugging
 */
void
alpha_backtrace(unsigned long pc, unsigned long sp, unsigned long fp)
{
	int frame = 0;
	unsigned long *frame_ptr;

	printf("\nBacktrace:\n");
	printf("  #%d: PC=0x%016lx\n", frame++, pc);

	/*
	 * Walk the stack using frame pointers
	 * Each frame has: [saved fp][saved ra]
	 */
	frame_ptr = (unsigned long *)fp;
	while (frame < 20 && frame_ptr != NULL &&
	       (unsigned long)frame_ptr >= sp &&
	       (unsigned long)frame_ptr < (sp + 0x10000)) {
		unsigned long saved_fp = frame_ptr[0];
		unsigned long saved_ra = frame_ptr[1];

		if (saved_ra == 0)
			break;

		printf("  #%d: PC=0x%016lx FP=0x%016lx\n",
		       frame++, saved_ra, (unsigned long)frame_ptr);

		frame_ptr = (unsigned long *)saved_fp;
	}
}

/*
 * Memory dump for debugging
 */
void
alpha_dump_memory(unsigned long addr, int count)
{
	unsigned long *p = (unsigned long *)addr;
	int i;

	printf("\nMemory dump at 0x%016lx:\n", addr);
	for (i = 0; i < count; i++) {
		if ((i % 2) == 0)
			printf("  0x%016lx: ", (unsigned long)&p[i]);
		printf("0x%016lx ", p[i]);
		if ((i % 2) == 1)
			printf("\n");
	}
	if ((count % 2) != 0)
		printf("\n");
}

/*
 * Breakpoint support
 */
void
alpha_breakpoint(void)
{
	__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_bpt));
}

/*
 * Single step support (not available on all Alpha implementations)
 */
void
alpha_single_step_enable(void)
{
	/* Would set trace bit in PS if available */
}

void
alpha_single_step_disable(void)
{
	/* Would clear trace bit in PS */
}

/*
 * Read/write physical memory (for debugging)
 */
unsigned long
alpha_read_physical(unsigned long paddr)
{
	/*
	 * Use VMS PALcode LDQP if available
	 * Otherwise use direct mapping in KSEG
	 */
	return *(unsigned long *)(ALPHA_KSEG_START | paddr);
}

void
alpha_write_physical(unsigned long paddr, unsigned long value)
{
	/*
	 * Use VMS PALcode STQP if available
	 * Otherwise use direct mapping in KSEG
	 */
	*(unsigned long *)(ALPHA_KSEG_START | paddr) = value;
}
