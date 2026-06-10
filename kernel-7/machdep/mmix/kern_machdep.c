/*
 * MMIX kernel machine-dependent functions
 */

#include <mach/machine.h>
#include <sys/systm.h>

void
machine_conf(void)
{
	/* Configure machine-specific features */
}

void
machine_info(void)
{
	printf("MMIX Architecture\n");
	printf("  Page size: 8KB\n");
	printf("  Registers: 256 general + 32 special\n");
}
