/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Firmware Abstraction Layer
 *
 * Detects and handles differences between SRM and ARC firmware.
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/reg.h>
#include <architecture/alpha/pal.h>
#include "firmware.h"
#include "hwrpb.h"

/* Global boot information */
static struct alpha_boot_info boot_info;

/* PAL trampoline page */
static struct pal_trampoline pal_trampolines[256];

/*
 * HWRPB (Hardware Restart Parameter Block) structure
 * Used by SRM firmware to pass boot information
 */
struct hwrpb_header {
	unsigned long phys_addr;	/* Physical address of HWRPB */
	unsigned long signature;	/* "HWRPB" */
	unsigned long revision;		/* HWRPB version */
	unsigned long size;		/* Size of HWRPB */
	unsigned long cpu_id;		/* Primary CPU ID */
	unsigned long pagesize;		/* System page size */
	unsigned long pa_size;		/* Physical address size */
	unsigned long max_asn;		/* Max ASN */
	unsigned char serial[16];	/* System serial number */
	unsigned long system_type;	/* System type */
	unsigned long system_variation;	/* System variation */
	unsigned long system_revision;	/* System revision */
};

#define HWRPB_SIGNATURE		0x4250525748UL	/* "HWRPB" */

/*
 * Detect firmware type from boot parameters
 */
static enum alpha_firmware_type
detect_firmware_type(unsigned long hwrpb_addr)
{
	struct hwrpb_header *hwrpb;

	if (hwrpb_addr == 0)
		return FIRMWARE_UNKNOWN;

	/* Try to read HWRPB signature */
	hwrpb = (struct hwrpb_header *)hwrpb_addr;

	/* Check for SRM HWRPB signature */
	if (hwrpb->signature == HWRPB_SIGNATURE) {
		return FIRMWARE_SRM;
	}

	/* Check for ARC signature (different structure) */
	/* ARC firmware passes different data structure */
	/* For now, if not SRM, assume ARC */
	return FIRMWARE_ARC;
}

/*
 * Detect PALcode variant
 */
static enum alpha_pal_variant
detect_pal_variant(void)
{
	unsigned long pal_rev;
	int can_call_unix_pal = 1;

	/*
	 * Try UNIX PALcode function (whami)
	 * If it works, we have UNIX PALcode
	 * If it faults, we have NT PALcode
	 */
	__asm__ volatile (
		"call_pal %1\n\t"
		"bis $31, $31, %0\n\t"	/* Success, set to 1 */
		".section .fixup,\"ax\"\n"
		"2: bis $31, $31, %0\n\t"	/* Failure, set to 0 */
		".previous\n"
		".section __ex_table,\"a\"\n"
		".long 1b,2b\n"
		".previous"
		: "=r" (can_call_unix_pal)
		: "i" (PAL_UNIX_whami)
		: "$0"
	);

	if (can_call_unix_pal) {
		return PAL_VARIANT_UNIX;
	}

	/*
	 * Try NT PALcode function
	 */
	return PAL_VARIANT_NT;
}

/*
 * Initialize firmware abstraction layer
 */
void
alpha_firmware_init(unsigned long hwrpb, unsigned long argc,
                    unsigned long argv, unsigned long envp)
{
	struct hwrpb_header *hwrpb_ptr;

	/* Clear boot info */
	bzero((char *)&boot_info, sizeof(boot_info));

	/* Store boot parameters */
	boot_info.hwrpb = hwrpb;
	boot_info.argc = argc;
	boot_info.argv = argv;
	boot_info.envp = envp;

	/* Detect firmware type */
	boot_info.firmware_type = detect_firmware_type(hwrpb);

	/* Detect PALcode variant */
	boot_info.pal_variant = detect_pal_variant();

	/* Get firmware-specific information */
	if (boot_info.firmware_type == FIRMWARE_SRM) {
		hwrpb_ptr = (struct hwrpb_header *)hwrpb;
		boot_info.mem_size = 0;	/* Will be filled from HWRPB */

		printf("SRM Firmware detected\n");
		printf("  HWRPB at 0x%lx\n", hwrpb);
		printf("  PALcode: %s\n",
		       boot_info.pal_variant == PAL_VARIANT_UNIX ? "UNIX" : "NT");
		printf("  System Type: 0x%lx\n", hwrpb_ptr->system_type);
		printf("  Page Size: %ld bytes\n", hwrpb_ptr->pagesize);
	} else if (boot_info.firmware_type == FIRMWARE_ARC) {
		printf("ARC Firmware detected\n");
		printf("  Using NT PALcode\n");

		/* ARC firmware uses different boot protocol */
		boot_info.pal_variant = PAL_VARIANT_NT;
	} else {
		printf("WARNING: Unknown firmware type!\n");
	}

	/* Parse memory descriptors */
	alpha_parse_memory_descriptors();

	/* Set up PAL trampolines for userspace */
	alpha_setup_pal_trampolines();
}

/*
 * Get current firmware type
 */
enum alpha_firmware_type
alpha_get_firmware_type(void)
{
	return boot_info.firmware_type;
}

/*
 * Get current PALcode variant
 */
enum alpha_pal_variant
alpha_get_pal_variant(void)
{
	return boot_info.pal_variant;
}

/*
 * Get boot information structure
 */
struct alpha_boot_info *
alpha_get_boot_info(void)
{
	return &boot_info;
}

/*
 * Set up PAL call trampolines for userspace
 *
 * Creates a page of trampoline code that userspace can call to
 * invoke PALcode functions. The trampolines handle the differences
 * between UNIX and NT PALcode.
 */
void
alpha_setup_pal_trampolines(void)
{
	int i;

	/* Clear trampoline page */
	bzero((char *)pal_trampolines, sizeof(pal_trampolines));

	/*
	 * For each PALcode function, create a trampoline
	 * The trampoline just does:
	 *   call_pal <function>
	 *   ret
	 */
	for (i = 0; i < 256; i++) {
		/* call_pal instruction: opcode 0x00, function in lower bits */
		pal_trampolines[i].opcode[0] = 0x00000000 | i;

		/* ret instruction: opcode 0x6BFA8001 (ret zero, (ra), 1) */
		pal_trampolines[i].opcode[1] = 0x6BFA8001;

		/* Padding */
		pal_trampolines[i].opcode[2] = 0x47FF041F;  /* nop */
		pal_trampolines[i].opcode[3] = 0x47FF041F;  /* nop */
	}

	printf("PAL trampolines initialized (%s PALcode)\n",
	       boot_info.pal_variant == PAL_VARIANT_UNIX ? "UNIX" : "NT");
}

/*
 * Map PAL trampoline page into userspace
 *
 * Returns physical address of trampoline page
 */
unsigned long
alpha_map_pal_page(void)
{
	extern unsigned long kvtophys(unsigned long);

	return kvtophys((unsigned long)pal_trampolines);
}

/*
 * Check if PAL call is allowed from userspace
 */
int
alpha_pal_call_allowed(unsigned long pal_func)
{
	/*
	 * Only allow specific PAL functions from userspace
	 * Dangerous functions (like halt, swpctx) are kernel-only
	 */
	switch (pal_func) {
	/* UNIX PALcode - allowed */
	case PAL_UNIX_bpt:
	case PAL_UNIX_bugchk:
	case PAL_UNIX_gentrap:
	case PAL_UNIX_rdunique:
	case PAL_UNIX_wrunique:
		return 1;

	/* NT PALcode - allowed */
	case PAL_NT_bpt:
	case PAL_NT_bugchk:
	case PAL_NT_gentrap:
		return 1;

	/* Everything else denied */
	default:
		return 0;
	}
}

/*
 * Halt/reboot through firmware
 */
void
alpha_firmware_halt(int reboot)
{
	if (boot_info.firmware_type == FIRMWARE_SRM) {
		/* SRM halt */
		__asm__ volatile ("call_pal %0" : : "i" (PAL_UNIX_halt));
	} else {
		/* ARC/NT halt */
		__asm__ volatile ("call_pal %0" : : "i" (PAL_NT_halt));
	}

	/* Should not return */
	for (;;)
		;
}

/*
 * Power off through firmware
 */
void
alpha_firmware_poweroff(void)
{
	/* Most Alpha systems don't support power off */
	alpha_firmware_halt(0);
}

/*
 * Restart through firmware
 */
void
alpha_firmware_restart(void)
{
	alpha_firmware_halt(1);
}

/*
 * Parse memory descriptors from firmware
 */
void
alpha_parse_memory_descriptors(void)
{
	struct hwrpb_header *hwrpb;
	unsigned long total_mem = 0;

	if (boot_info.firmware_type == FIRMWARE_SRM) {
		hwrpb = (struct hwrpb_header *)boot_info.hwrpb;

		/*
		 * Memory descriptors are in HWRPB
		 * For now, just estimate based on physical address size
		 */
		total_mem = 64 * 1024 * 1024;  /* Default 64MB */

		boot_info.mem_size = total_mem;
		boot_info.mem_clusters = 1;

		printf("Memory: %ld MB\n", total_mem / (1024 * 1024));
	} else if (boot_info.firmware_type == FIRMWARE_ARC) {
		/*
		 * ARC firmware has different memory descriptor format
		 */
		total_mem = 64 * 1024 * 1024;  /* Default 64MB */
		boot_info.mem_size = total_mem;
	}
}

/*
 * SRM-specific fixups
 */
void
alpha_srm_fixup(void)
{
	/*
	 * SRM-specific initialization
	 * - Set up callback vectors
	 * - Configure console
	 * - etc.
	 */
}

/*
 * ARC-specific fixups
 */
void
alpha_arc_fixup(void)
{
	/*
	 * ARC-specific initialization
	 * - Different boot protocol
	 * - Different console
	 * - etc.
	 */
}
