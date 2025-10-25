/*
 * x86-64 machine-dependent code
 *
 * This file contains x86-64 specific machine-dependent initialization
 * and support functions.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <mach/machine.h>
#include <mach/machine/vm_types.h>
#include <mach/machine/vm_param.h>

/*
 * Machine-dependent startup code
 */
void
machine_startup(void)
{
	/* Stub implementation */
	printf("x86-64 machine startup\n");
}

/*
 * Machine-dependent initialization
 */
void
machine_init(void)
{
	/* Stub implementation */
	printf("x86-64 machine init\n");
}

/*
 * Machine-dependent configuration
 */
void
machine_conf(void)
{
	/* Stub implementation */
}

/*
 * Return CPU type
 */
cpu_type_t
machine_slot_type(void)
{
	return CPU_TYPE_X86_64;
}

/*
 * Return CPU subtype
 */
cpu_subtype_t
machine_slot_subtype(void)
{
	return CPU_SUBTYPE_X86_64_ALL;
}
