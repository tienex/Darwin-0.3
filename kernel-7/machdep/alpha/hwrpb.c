/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Hardware Restart Parameter Block (HWRPB) Support
 *
 * The HWRPB is a data structure provided by the console firmware
 * that describes the system hardware configuration. It's essential
 * for system initialization.
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>

/*
 * HWRPB structure (simplified)
 *
 * This is passed to the kernel by the bootloader in register a0
 */
struct hwrpb {
	unsigned long	phys_addr;		/* Physical address of HWRPB */
	unsigned long	signature;		/* "HWRPB" signature */
	unsigned long	revision;		/* HWRPB revision */
	unsigned long	size;			/* Size of HWRPB */

	unsigned long	primary_cpu;		/* Primary CPU ID */
	unsigned long	page_size;		/* System page size */
	unsigned long	pa_size;		/* Physical address size */
	unsigned long	max_asn;		/* Maximum ASN value */

	unsigned char	system_serial[10];	/* System serial number */
	unsigned char	system_revision[8];	/* System revision */

	unsigned long	intr_freq;		/* Interrupt clock frequency */
	unsigned long	cycle_freq;		/* CPU cycle frequency */
	unsigned long	vptb;			/* Virtual page table base */
	unsigned long	reserved1;

	unsigned long	tbhint_offset;		/* TB hint block offset */
	unsigned long	num_cpus;		/* Number of CPUs */
	unsigned long	cpu_offset;		/* Offset to CPU info */
	unsigned long	cpu_size;		/* Size of CPU info */

	unsigned long	ctb_offset;		/* Console terminal block */
	unsigned long	crb_offset;		/* Console routine block */
	unsigned long	mddt_offset;		/* Memory data descriptor table */
	unsigned long	config_offset;		/* Configuration tree */
	unsigned long	fru_offset;		/* FRU table */

	unsigned long	save_terminal;		/* Save terminal routine */
	unsigned long	restore_terminal;	/* Restore terminal routine */
	unsigned long	restart;		/* Restart routine */
	unsigned long	reserve_os;		/* Reserve OS callback */

	unsigned long	checksum;		/* HWRPB checksum */
	unsigned long	rxrdy;			/* Receive ready */
	unsigned long	txrdy;			/* Transmit ready */
};

/*
 * Per-CPU information structure
 */
struct percpu_info {
	unsigned long	cpu_type;		/* CPU type */
	unsigned long	cpu_variation;		/* CPU variation */
	unsigned long	cpu_revision;		/* CPU revision */
	unsigned long	cpu_serial[2];		/* CPU serial number */

	unsigned long	pal_logout;		/* PAL logout area */
	unsigned long	pal_logout_len;		/* PAL logout length */
	unsigned long	halt_pcb;		/* Halt PCB address */
	unsigned long	halt_pc;		/* Halt PC */
	unsigned long	halt_ps;		/* Halt PS */
	unsigned long	halt_arg;		/* Halt argument */
	unsigned long	halt_ra;		/* Halt return address */
	unsigned long	halt_pv;		/* Halt procedure value */
	unsigned long	halt_reason;		/* Halt reason */
	unsigned long	res1;
	unsigned long	res2;
	unsigned long	res3;

	unsigned long	flags;			/* CPU flags */
};

extern unsigned long boot_hwrpb;
static struct hwrpb *hwrpb_ptr = NULL;

/*
 * Initialize HWRPB support
 */
void
alpha_hwrpb_init(void)
{
	if (boot_hwrpb == 0) {
		printf("Warning: No HWRPB provided by bootloader\n");
		return;
	}

	/*
	 * Map HWRPB into kernel address space
	 */
	hwrpb_ptr = (struct hwrpb *)boot_hwrpb;

	/*
	 * Verify HWRPB signature
	 */
	if (hwrpb_ptr->signature != 0x425052574820UL) {  /* "HWRPB " */
		printf("Warning: Invalid HWRPB signature: 0x%lx\n",
		       hwrpb_ptr->signature);
		hwrpb_ptr = NULL;
		return;
	}

	printf("HWRPB found at 0x%lx (revision %ld)\n",
	       hwrpb_ptr->phys_addr, hwrpb_ptr->revision);
}

/*
 * Get HWRPB pointer
 */
struct hwrpb *
alpha_get_hwrpb(void)
{
	return hwrpb_ptr;
}

/*
 * Get number of CPUs from HWRPB
 */
unsigned long
alpha_hwrpb_num_cpus(void)
{
	if (hwrpb_ptr == NULL)
		return 1;

	return hwrpb_ptr->num_cpus;
}

/*
 * Get CPU information for specific CPU
 */
struct percpu_info *
alpha_hwrpb_cpu_info(int cpu_id)
{
	unsigned long offset;

	if (hwrpb_ptr == NULL)
		return NULL;

	if (cpu_id < 0 || cpu_id >= hwrpb_ptr->num_cpus)
		return NULL;

	offset = hwrpb_ptr->cpu_offset + (cpu_id * hwrpb_ptr->cpu_size);
	return (struct percpu_info *)((char *)hwrpb_ptr + offset);
}

/*
 * Get system serial number
 */
void
alpha_hwrpb_serial(char *buf, int buflen)
{
	if (hwrpb_ptr == NULL || buflen < 11) {
		if (buflen > 0)
			buf[0] = '\0';
		return;
	}

	memcpy(buf, hwrpb_ptr->system_serial, 10);
	buf[10] = '\0';
}

/*
 * Get CPU cycle frequency
 */
unsigned long
alpha_hwrpb_cycle_freq(void)
{
	if (hwrpb_ptr == NULL)
		return 0;

	return hwrpb_ptr->cycle_freq;
}

/*
 * Get interrupt frequency
 */
unsigned long
alpha_hwrpb_intr_freq(void)
{
	if (hwrpb_ptr == NULL)
		return 1024;  /* Default 1024 Hz */

	return hwrpb_ptr->intr_freq;
}

/*
 * Get page size from HWRPB
 */
unsigned long
alpha_hwrpb_page_size(void)
{
	if (hwrpb_ptr == NULL)
		return 8192;  /* Default 8KB */

	return hwrpb_ptr->page_size;
}

/*
 * Print HWRPB information
 */
void
alpha_hwrpb_print(void)
{
	int i;
	struct percpu_info *cpu;

	if (hwrpb_ptr == NULL) {
		printf("No HWRPB available\n");
		return;
	}

	printf("\nHardware Restart Parameter Block:\n");
	printf("  Physical address: 0x%016lx\n", hwrpb_ptr->phys_addr);
	printf("  Revision:         %ld\n", hwrpb_ptr->revision);
	printf("  Size:             %ld bytes\n", hwrpb_ptr->size);
	printf("  Primary CPU:      %ld\n", hwrpb_ptr->primary_cpu);
	printf("  Number of CPUs:   %ld\n", hwrpb_ptr->num_cpus);
	printf("  Page size:        %ld bytes\n", hwrpb_ptr->page_size);
	printf("  Cycle frequency:  %ld Hz\n", hwrpb_ptr->cycle_freq);
	printf("  Intr frequency:   %ld Hz\n", hwrpb_ptr->intr_freq);

	printf("\nCPU Information:\n");
	for (i = 0; i < hwrpb_ptr->num_cpus; i++) {
		cpu = alpha_hwrpb_cpu_info(i);
		if (cpu) {
			printf("  CPU %d:\n", i);
			printf("    Type:      0x%lx\n", cpu->cpu_type);
			printf("    Variation: 0x%lx\n", cpu->cpu_variation);
			printf("    Revision:  0x%lx\n", cpu->cpu_revision);
			printf("    Serial:    %016lx%016lx\n",
			       cpu->cpu_serial[1], cpu->cpu_serial[0]);
		}
	}
}

/* Stub for memcpy if not available */
void *memcpy(void *dst, const void *src, size_t n)
{
	char *d = dst;
	const char *s = src;
	while (n--)
		*d++ = *s++;
	return dst;
}
