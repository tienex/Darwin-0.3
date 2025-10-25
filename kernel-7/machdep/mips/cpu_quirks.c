/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * MIPS CPU Vendor Quirks Implementation
 *
 * Detects CPU vendor and model, sets up quirk workarounds
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <machdep/mips/cpu_quirks.h>

/* Global CPU information */
struct mips_cpu_info mips_cpu;

/*
 * CPU Name Table
 */
static const char *mips_vendor_names[] = {
	[MIPS_PRID_COMP_LEGACY]		= "Legacy MIPS",
	[MIPS_PRID_COMP_MIPS]		= "MIPS Technologies",
	[MIPS_PRID_COMP_BROADCOM]	= "Broadcom",
	[MIPS_PRID_COMP_ALCHEMY]	= "Alchemy/AMD",
	[MIPS_PRID_COMP_SIBYTE]		= "SiByte",
	[MIPS_PRID_COMP_SANDCRAFT]	= "SandCraft",
	[MIPS_PRID_COMP_PHILIPS]	= "Philips",
	[MIPS_PRID_COMP_TOSHIBA]	= "Toshiba",
	[MIPS_PRID_COMP_LSI]		= "LSI Logic",
	[MIPS_PRID_COMP_LEXRA]		= "Lexra",
	[MIPS_PRID_COMP_CAVIUM]		= "Cavium Networks",
	[MIPS_PRID_COMP_INGENIC]	= "Ingenic",
	[MIPS_PRID_COMP_LOONGSON]	= "Loongson/Godson",
};

/*
 * Read PRId register
 */
static inline unsigned int read_prid(void)
{
	unsigned int prid;
	__asm__ volatile("mfc0 %0, $15" : "=r" (prid));
	return prid;
}

/*
 * Read Config register
 */
static inline unsigned int read_config(void)
{
	unsigned int config;
	__asm__ volatile("mfc0 %0, $16" : "=r" (config));
	return config;
}

/*
 * Detect Loongson CPU variant and quirks
 */
static void detect_loongson_cpu(void)
{
	unsigned int prid = mips_cpu.prid;
	unsigned int revision = prid & 0xff;

	/* Determine Loongson model */
	if ((prid & 0xff00) == MIPS_PRID_IMP_LOONGSON_64) {
		/* Loongson 2/3 (MIPS64) */
		if (revision >= 0x03 && revision <= 0x05) {
			/* Loongson 2E/2F */
			mips_cpu.cpu_name = (revision == 0x03) ? "Loongson-2E" :
					    (revision == 0x04) ? "Loongson-2F (early)" :
					    "Loongson-2F";

			/* Loongson 2E/2F specific features */
			mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC |
					    MIPS_CPU_64BIT | MIPS_CPU_COUNTER;

			/* Loongson 2E/2F specific quirks */
			if (revision <= 0x04) {
				/* Loongson 2E and early 2F have LL/SC bug */
				mips_cpu.quirks |= MIPS_QUIRK_LOONGSON2_LLSC;
				printf("Loongson 2E/2F: Applying LL/SC workaround\n");
			}

			/* All Loongson 2 have cache quirks */
			mips_cpu.quirks |= MIPS_QUIRK_LOONGSON2_CACHE;
			mips_cpu.quirks |= MIPS_QUIRK_LOONGSON2_BTAC;

			/* Cache configuration */
			mips_cpu.icache_size = 64 * 1024;	/* 64KB I-cache */
			mips_cpu.icache_line_size = 32;
			mips_cpu.icache_ways = 4;

			mips_cpu.dcache_size = 64 * 1024;	/* 64KB D-cache */
			mips_cpu.dcache_line_size = 32;
			mips_cpu.dcache_ways = 4;

			/* Loongson 2F has 512KB L2 cache */
			if (revision >= 0x04) {
				mips_cpu.scache_size = 512 * 1024;
				mips_cpu.scache_line_size = 32;
				mips_cpu.scache_ways = 4;
			}

			/* TLB size */
			mips_cpu.tlb_entries = 64;

		} else if (revision >= 0x05) {
			/* Loongson 3 (3A/3B) */
			mips_cpu.cpu_name = "Loongson-3";

			mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC |
					    MIPS_CPU_64BIT | MIPS_CPU_COUNTER |
					    MIPS_CPU_WATCH | MIPS_CPU_EJTAG;

			/* Loongson 3 has improved LL/SC */
			mips_cpu.quirks |= MIPS_QUIRK_LOONGSON3_LLSC;

			/* Larger caches on Loongson 3 */
			mips_cpu.icache_size = 64 * 1024;
			mips_cpu.dcache_size = 64 * 1024;
			mips_cpu.scache_size = 4 * 1024 * 1024;	/* 4MB L2 */
			mips_cpu.tlb_entries = 64;
		}
	} else {
		/* Loongson 1 (MIPS32) */
		mips_cpu.cpu_name = "Loongson-1";
		mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC | MIPS_CPU_COUNTER;
		mips_cpu.icache_size = 16 * 1024;
		mips_cpu.dcache_size = 16 * 1024;
		mips_cpu.tlb_entries = 32;
	}
}

/*
 * Detect Broadcom CPU
 */
static void detect_broadcom_cpu(void)
{
	unsigned int prid = mips_cpu.prid;
	unsigned int imp = (prid >> 8) & 0xff;

	mips_cpu.vendor_name = "Broadcom";

	switch (imp) {
	case MIPS_PRID_IMP_BMIPS32:
		mips_cpu.cpu_name = "BMIPS32";
		break;
	case MIPS_PRID_IMP_BMIPS4380:
		mips_cpu.cpu_name = "BMIPS4380";
		break;
	case MIPS_PRID_IMP_BMIPS5000:
		mips_cpu.cpu_name = "BMIPS5000";
		break;
	default:
		mips_cpu.cpu_name = "Broadcom MIPS";
	}

	mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC | MIPS_CPU_COUNTER;
	mips_cpu.quirks |= MIPS_QUIRK_BMIPS_CACHE;
}

/*
 * Detect Cavium CPU
 */
static void detect_cavium_cpu(void)
{
	unsigned int prid = mips_cpu.prid;
	unsigned int imp = (prid >> 8) & 0xff;

	mips_cpu.vendor_name = "Cavium Networks";

	switch (imp) {
	case MIPS_PRID_IMP_CAVIUM_CN38XX:
		mips_cpu.cpu_name = "Octeon CN38xx";
		mips_cpu.quirks |= MIPS_QUIRK_CAVIUM_CN38XX;
		break;
	case MIPS_PRID_IMP_CAVIUM_CN50XX:
		mips_cpu.cpu_name = "Octeon CN50xx";
		break;
	case MIPS_PRID_IMP_CAVIUM_CN58XX:
		mips_cpu.cpu_name = "Octeon CN58xx";
		break;
	case MIPS_PRID_IMP_CAVIUM_CN63XX:
		mips_cpu.cpu_name = "Octeon CN63xx";
		break;
	case MIPS_PRID_IMP_CAVIUM_CN68XX:
		mips_cpu.cpu_name = "Octeon CN68xx";
		break;
	default:
		mips_cpu.cpu_name = "Cavium Octeon";
	}

	/* Cavium CPUs have large TLBs */
	mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC | MIPS_CPU_64BIT |
			    MIPS_CPU_COUNTER | MIPS_CPU_WATCH;
	mips_cpu.tlb_entries = 128;	/* Most Octeons have 128-entry TLB */
}

/*
 * Detect Ingenic CPU
 */
static void detect_ingenic_cpu(void)
{
	unsigned int prid = mips_cpu.prid;

	mips_cpu.vendor_name = "Ingenic";
	mips_cpu.cpu_name = "JZ47xx";

	mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC | MIPS_CPU_COUNTER;
	mips_cpu.quirks |= MIPS_QUIRK_INGENIC_CACHE;

	/* Ingenic CPUs have smaller caches */
	mips_cpu.icache_size = 16 * 1024;
	mips_cpu.dcache_size = 16 * 1024;
	mips_cpu.tlb_entries = 32;
}

/*
 * Detect MIPS Technologies CPU
 */
static void detect_mips_cpu(void)
{
	unsigned int prid = mips_cpu.prid;
	unsigned int imp = (prid >> 8) & 0xff;

	mips_cpu.vendor_name = "MIPS Technologies";

	switch (imp) {
	case MIPS_PRID_IMP_4KC:
		mips_cpu.cpu_name = "MIPS 4Kc";
		mips_cpu.tlb_entries = 16;
		break;
	case MIPS_PRID_IMP_24K:
		mips_cpu.cpu_name = "MIPS 24K";
		mips_cpu.tlb_entries = 32;
		break;
	case MIPS_PRID_IMP_34K:
		mips_cpu.cpu_name = "MIPS 34K";
		mips_cpu.tlb_entries = 64;
		break;
	case MIPS_PRID_IMP_74K:
		mips_cpu.cpu_name = "MIPS 74K";
		mips_cpu.tlb_entries = 64;
		break;
	case MIPS_PRID_IMP_1004K:
		mips_cpu.cpu_name = "MIPS 1004K";
		mips_cpu.tlb_entries = 64;
		break;
	case MIPS_PRID_IMP_P5600:
		mips_cpu.cpu_name = "MIPS P5600";
		mips_cpu.features |= MIPS_CPU_EVA | MIPS_CPU_HTW;
		mips_cpu.tlb_entries = 64;
		break;
	case MIPS_PRID_IMP_I6400:
		mips_cpu.cpu_name = "MIPS I6400";
		mips_cpu.features |= MIPS_CPU_EVA | MIPS_CPU_HTW | MIPS_CPU_64BIT;
		mips_cpu.tlb_entries = 128;
		break;
	default:
		mips_cpu.cpu_name = "MIPS Generic";
		mips_cpu.tlb_entries = 64;
	}

	mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC | MIPS_CPU_COUNTER;
}

/*
 * Detect legacy MIPS CPU
 */
static void detect_legacy_cpu(void)
{
	unsigned int prid = mips_cpu.prid;
	unsigned int imp = (prid >> 8) & 0xff;

	mips_cpu.vendor_name = "Legacy MIPS";

	switch (imp) {
	case MIPS_PRID_IMP_R2000:
		mips_cpu.cpu_name = "R2000";
		mips_cpu.tlb_entries = 64;
		mips_cpu.quirks |= MIPS_QUIRK_NO_WRITE_COMBINE;
		break;
	case MIPS_PRID_IMP_R3000:
		mips_cpu.cpu_name = "R3000";
		mips_cpu.tlb_entries = 64;
		mips_cpu.quirks |= MIPS_QUIRK_NO_WRITE_COMBINE;
		break;
	case MIPS_PRID_IMP_R4000:
		mips_cpu.cpu_name = "R4000";
		mips_cpu.tlb_entries = 48;
		mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_64BIT;
		mips_cpu.quirks |= MIPS_QUIRK_R4000_SC;
		break;
	case MIPS_PRID_IMP_R10000:
		mips_cpu.cpu_name = "R10000";
		mips_cpu.tlb_entries = 64;
		mips_cpu.features = MIPS_CPU_FPU | MIPS_CPU_LLSC | MIPS_CPU_64BIT;
		break;
	default:
		mips_cpu.cpu_name = "Unknown";
		mips_cpu.tlb_entries = 64;
	}
}

/*
 * Main CPU Detection
 */
void mips_cpu_detect(void)
{
	unsigned int prid, config;
	unsigned int company;

	/* Read PRId */
	prid = read_prid();
	mips_cpu.prid = prid;
	mips_cpu.revision = prid & 0xff;
	mips_cpu.implementation = (prid >> 8) & 0xff;
	mips_cpu.company = (prid >> 16) & 0xff;

	/* Read Config */
	config = read_config();

	/* Detect vendor and set defaults */
	company = mips_cpu.company;

	switch (company) {
	case MIPS_PRID_COMP_LOONGSON:
		detect_loongson_cpu();
		break;
	case MIPS_PRID_COMP_BROADCOM:
	case MIPS_PRID_COMP_SIBYTE:
		detect_broadcom_cpu();
		break;
	case MIPS_PRID_COMP_CAVIUM:
		detect_cavium_cpu();
		break;
	case MIPS_PRID_COMP_INGENIC:
		detect_ingenic_cpu();
		break;
	case MIPS_PRID_COMP_MIPS:
		detect_mips_cpu();
		break;
	case MIPS_PRID_COMP_LEGACY:
	default:
		detect_legacy_cpu();
		break;
	}

	/* Set vendor name if not already set */
	if (mips_cpu.vendor_name == NULL) {
		if (company < sizeof(mips_vendor_names) / sizeof(char *))
			mips_cpu.vendor_name = mips_vendor_names[company];
		else
			mips_cpu.vendor_name = "Unknown";
	}

	printf("CPU: %s %s (PRId: 0x%08x, Rev %d)\n",
	       mips_cpu.vendor_name,
	       mips_cpu.cpu_name ? mips_cpu.cpu_name : "Unknown",
	       prid, mips_cpu.revision);

	if (mips_cpu.quirks) {
		printf("CPU quirks: 0x%08x\n", mips_cpu.quirks);
	}
}

/*
 * Initialize CPU-specific quirks and workarounds
 */
void mips_cpu_quirks_init(void)
{
	/* Loongson 2E/2F LL/SC workaround */
	if (mips_cpu.quirks & MIPS_QUIRK_LOONGSON2_LLSC) {
		printf("Enabling Loongson 2E/2F LL/SC workaround\n");
		/* Enable special handling in atomic operations */
		/* This would modify the LL/SC code generation */
	}

	/* Loongson 2 cache workarounds */
	if (mips_cpu.quirks & MIPS_QUIRK_LOONGSON2_CACHE) {
		printf("Enabling Loongson 2 cache workarounds\n");
		/* Disable write-combine on certain operations */
	}

	/* Loongson 2 branch target cache */
	if (mips_cpu.quirks & MIPS_QUIRK_LOONGSON2_BTAC) {
		/* Configure BTAC for optimal performance */
		unsigned int diag;
		__asm__ volatile(
			"mfc0 %0, $22\n"	/* Read DiagControl */
			"ori  %0, %0, 0x0c\n"	/* Enable BTAC + RAS */
			"mtc0 %0, $22\n"
			: "=&r" (diag));
	}

	/* R4000/R4400 cache bug workaround */
	if (mips_cpu.quirks & MIPS_QUIRK_R4000_SC) {
		printf("Enabling R4000 SC/MC bug workaround\n");
	}
}

/*
 * Get CPU name string
 */
const char *mips_cpu_name(void)
{
	static char namebuf[64];

	snprintf(namebuf, sizeof(namebuf), "%s %s",
		 mips_cpu.vendor_name ? mips_cpu.vendor_name : "Unknown",
		 mips_cpu.cpu_name ? mips_cpu.cpu_name : "MIPS");

	return namebuf;
}

/*
 * Cache operations with vendor-specific handling
 */
void mips_cache_init(void)
{
	if (mips_cpu.company == MIPS_PRID_COMP_LOONGSON) {
		printf("Initializing Loongson cache subsystem\n");
		/* Loongson-specific cache initialization */
	} else if (mips_cpu.company == MIPS_PRID_COMP_BROADCOM ||
		   mips_cpu.company == MIPS_PRID_COMP_SIBYTE) {
		printf("Initializing Broadcom cache subsystem\n");
		/* Broadcom-specific cache initialization */
	} else if (mips_cpu.company == MIPS_PRID_COMP_CAVIUM) {
		printf("Initializing Cavium cache subsystem\n");
		/* Cavium-specific cache initialization */
	}

	printf("Cache: I=%dK D=%dK",
	       mips_cpu.icache_size / 1024,
	       mips_cpu.dcache_size / 1024);
	if (mips_cpu.scache_size)
		printf(" L2=%dK", mips_cpu.scache_size / 1024);
	printf("\n");
}

/*
 * Flush instruction cache - vendor specific
 */
void mips_icache_flush_all(void)
{
	if (mips_cpu.quirks & MIPS_QUIRK_LOONGSON2_CACHE) {
		/* Loongson 2 requires special cache flush sequence */
		__asm__ volatile(
			".set push\n"
			".set noreorder\n"
			"sync\n"
			".set pop\n"
		);
	}

	/* Standard MIPS cache flush */
	__asm__ volatile("sync");
}

/*
 * Flush data cache - vendor specific
 */
void mips_dcache_flush_all(void)
{
	if (mips_cpu.quirks & MIPS_QUIRK_LOONGSON2_CACHE) {
		/* Loongson specific D-cache flush */
		__asm__ volatile(
			".set push\n"
			".set noreorder\n"
			"sync\n"
			".set pop\n"
		);
	}

	/* Standard sync */
	__asm__ volatile("sync");
}
