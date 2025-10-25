/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Firmware Abstraction Layer
 *
 * Handles differences between SRM and ARC firmware, providing a
 * unified interface to the kernel and userspace.
 */

#ifndef _MACHDEP_ALPHA_FIRMWARE_H_
#define _MACHDEP_ALPHA_FIRMWARE_H_

/*
 * Firmware types
 */
enum alpha_firmware_type {
	FIRMWARE_UNKNOWN = 0,
	FIRMWARE_SRM,		/* DEC SRM Console */
	FIRMWARE_ARC,		/* ARC/AlphaBIOS */
	FIRMWARE_MILO		/* Linux Mini-Loader (not typically supported) */
};

/*
 * PALcode variants associated with firmware
 */
enum alpha_pal_variant {
	PAL_VARIANT_UNKNOWN = 0,
	PAL_VARIANT_UNIX,	/* Digital UNIX PALcode (SRM) */
	PAL_VARIANT_NT,		/* Windows NT PALcode (ARC) */
	PAL_VARIANT_VMS		/* OpenVMS PALcode */
};

/*
 * Boot protocol information
 */
struct alpha_boot_info {
	enum alpha_firmware_type firmware_type;
	enum alpha_pal_variant pal_variant;

	/* Boot parameters from firmware */
	unsigned long hwrpb;		/* Hardware Restart Parameter Block */
	unsigned long argc;		/* Argument count */
	unsigned long argv;		/* Argument vector */
	unsigned long envp;		/* Environment pointer */
	unsigned long boot_flags;	/* Boot flags */

	/* Memory information */
	unsigned long mem_size;		/* Total memory size */
	unsigned long mem_clusters;	/* Memory cluster count */

	/* Firmware callback functions */
	unsigned long fw_callback;	/* Firmware callback entry point */

	/* PALcode information */
	unsigned long pal_base;		/* PALcode base address */
	unsigned long pal_size;		/* PALcode size */
	unsigned long pal_rev;		/* PALcode revision */
};

/*
 * Fixed virtual address for userspace PAL call page
 * This is a read-only page mapped at a fixed address in every
 * process that contains trampolines to the correct PALcode.
 *
 * Placed at the very end of user address space to avoid conflicts
 * with user programs and dynamic libraries.
 *
 * Alpha user address space: 0x0000000000000000 - 0x000003ffffffffff (43 bits)
 * We place the PAL page at the last page of the 4TB region.
 */
#define ALPHA_PAL_CALL_PAGE	0x000003ffffffe000UL	/* ~4TB - 8KB */

/*
 * PAL call trampoline structure
 * Each userspace PAL call goes through a trampoline that:
 * 1. Validates the call
 * 2. Switches to the appropriate PALcode variant
 * 3. Performs the call
 * 4. Returns to userspace
 */
struct pal_trampoline {
	unsigned int opcode[4];		/* Trampoline code */
};

/*
 * Function prototypes
 */

/* Firmware detection and initialization */
void alpha_firmware_init(unsigned long hwrpb, unsigned long argc,
                         unsigned long argv, unsigned long envp);
enum alpha_firmware_type alpha_get_firmware_type(void);
enum alpha_pal_variant alpha_get_pal_variant(void);
struct alpha_boot_info *alpha_get_boot_info(void);

/* PALcode abstraction */
void alpha_setup_pal_trampolines(void);
unsigned long alpha_map_pal_page(void);
int alpha_pal_call_allowed(unsigned long pal_func);

/* Firmware-specific operations */
void alpha_firmware_halt(int reboot);
void alpha_firmware_poweroff(void);
void alpha_firmware_restart(void);
void *alpha_firmware_callback(void);

/* SRM-specific functions */
void alpha_srm_fixup(void);
unsigned long alpha_srm_get_hwrpb(void);

/* ARC-specific functions */
void alpha_arc_fixup(void);
void *alpha_arc_get_system_parameter_block(void);

/* Memory descriptor parsing */
void alpha_parse_memory_descriptors(void);

#endif /* _MACHDEP_ALPHA_FIRMWARE_H_ */
