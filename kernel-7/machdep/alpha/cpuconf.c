/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha CPU Detection and Configuration
 */

#include <mach/mach_types.h>
#include <architecture/alpha/cpu.h>
#include <architecture/alpha/pal.h>

/*
 * CPU implementation database
 */
struct alpha_cpu_impl {
	unsigned long	impl_ver;		/* IMPLVER value */
	unsigned long	amask;			/* Architecture mask */
	const char	*name;			/* CPU name */
	const char	*codename;		/* Codename */
	unsigned long	icache_size;		/* I-cache size */
	unsigned long	dcache_size;		/* D-cache size */
	unsigned long	scache_size;		/* S-cache size */
	unsigned long	features;		/* Feature flags */
};

static const struct alpha_cpu_impl cpu_impls[] = {
	/* EV3 - 21064 */
	{
		.impl_ver = 0,
		.amask = 0,
		.name = "21064 (EV3)",
		.codename = "EV3",
		.icache_size = 8192,
		.dcache_size = 8192,
		.scache_size = 0,
		.features = 0
	},

	/* EV4 - 21064A */
	{
		.impl_ver = 1,
		.amask = 0,
		.name = "21064A (EV4)",
		.codename = "EV4",
		.icache_size = 8192,
		.dcache_size = 8192,
		.scache_size = 0,
		.features = 0
	},

	/* LCA - 21066 */
	{
		.impl_ver = 1,
		.amask = 0,
		.name = "21066 (LCA)",
		.codename = "LCA",
		.icache_size = 8192,
		.dcache_size = 8192,
		.scache_size = 0,
		.features = 0
	},

	/* EV5 - 21164 */
	{
		.impl_ver = 2,
		.amask = 0,
		.name = "21164 (EV5)",
		.codename = "EV5",
		.icache_size = 8192,
		.dcache_size = 8192,
		.scache_size = 96 * 1024,
		.features = 0
	},

	/* EV56 - 21164A with BWX */
	{
		.impl_ver = 2,
		.amask = ALPHA_EXT_BWX,
		.name = "21164A (EV56)",
		.codename = "EV56",
		.icache_size = 8192,
		.dcache_size = 8192,
		.scache_size = 96 * 1024,
		.features = ALPHA_EXT_BWX | ALPHA_EXT_MVI
	},

	/* PCA56 - 21164PC */
	{
		.impl_ver = 2,
		.amask = ALPHA_EXT_BWX | ALPHA_EXT_MVI,
		.name = "21164PC (PCA56)",
		.codename = "PCA56",
		.icache_size = 16384,
		.dcache_size = 16384,
		.scache_size = 0,
		.features = ALPHA_EXT_BWX | ALPHA_EXT_MVI
	},

	/* EV6 - 21264 */
	{
		.impl_ver = 3,
		.amask = 0,
		.name = "21264 (EV6)",
		.codename = "EV6",
		.icache_size = 65536,
		.dcache_size = 65536,
		.scache_size = 0,
		.features = ALPHA_EXT_BWX | ALPHA_EXT_FIX | ALPHA_EXT_MVI
	},

	/* EV67 - 21264A with CIX */
	{
		.impl_ver = 3,
		.amask = ALPHA_EXT_CIX,
		.name = "21264A (EV67)",
		.codename = "EV67",
		.icache_size = 65536,
		.dcache_size = 65536,
		.scache_size = 0,
		.features = ALPHA_EXT_BWX | ALPHA_EXT_FIX | ALPHA_EXT_CIX | ALPHA_EXT_MVI
	},

	/* Sentinel */
	{ .impl_ver = -1 }
};

/*
 * Detect CPU implementation and features
 */
const struct alpha_cpu_impl *
alpha_detect_cpu(void)
{
	unsigned long impl_ver;
	unsigned long amask_bwx, amask_fix, amask_cix, amask_mvi;
	unsigned long features = 0;
	int i;

	/*
	 * Read implementation version
	 */
	__asm__ volatile ("implver %0" : "=r" (impl_ver));

	/*
	 * Check for instruction set extensions using AMASK
	 * AMASK returns bits that are NOT supported
	 */
	__asm__ volatile ("amask %1, %0" : "=r" (amask_bwx) : "i" (ALPHA_EXT_BWX));
	__asm__ volatile ("amask %1, %0" : "=r" (amask_fix) : "i" (ALPHA_EXT_FIX));
	__asm__ volatile ("amask %1, %0" : "=r" (amask_cix) : "i" (ALPHA_EXT_CIX));
	__asm__ volatile ("amask %1, %0" : "=r" (amask_mvi) : "i" (ALPHA_EXT_MVI));

	if ((amask_bwx & ALPHA_EXT_BWX) == 0)
		features |= ALPHA_EXT_BWX;
	if ((amask_fix & ALPHA_EXT_FIX) == 0)
		features |= ALPHA_EXT_FIX;
	if ((amask_cix & ALPHA_EXT_CIX) == 0)
		features |= ALPHA_EXT_CIX;
	if ((amask_mvi & ALPHA_EXT_MVI) == 0)
		features |= ALPHA_EXT_MVI;

	/*
	 * Match against known implementations
	 */
	for (i = 0; cpu_impls[i].impl_ver != -1; i++) {
		if (cpu_impls[i].impl_ver == impl_ver &&
		    (cpu_impls[i].features & features) == cpu_impls[i].features) {
			return &cpu_impls[i];
		}
	}

	/* Return generic implementation */
	return &cpu_impls[0];
}

/*
 * Get CPU name
 */
const char *
alpha_cpu_name(void)
{
	const struct alpha_cpu_impl *impl = alpha_detect_cpu();
	return impl->name;
}

/*
 * Get CPU codename
 */
const char *
alpha_cpu_codename(void)
{
	const struct alpha_cpu_impl *impl = alpha_detect_cpu();
	return impl->codename;
}

/*
 * Check if CPU has specific feature
 */
boolean_t
alpha_cpu_has_feature(unsigned long feature)
{
	const struct alpha_cpu_impl *impl = alpha_detect_cpu();
	return (impl->features & feature) != 0;
}

/*
 * Get all CPU features
 */
unsigned long
alpha_cpu_features(void)
{
	const struct alpha_cpu_impl *impl = alpha_detect_cpu();
	return impl->features;
}

/*
 * Print CPU information
 */
void
alpha_print_cpu_config(void)
{
	const struct alpha_cpu_impl *impl;
	extern alpha_cpu_info_t cpu_info;

	impl = alpha_detect_cpu();

	printf("\nAlpha CPU Configuration:\n");
	printf("  Processor:    %s\n", impl->name);
	printf("  Codename:     %s\n", impl->codename);
	printf("  IMPLVER:      %ld\n", impl->impl_ver);

	printf("  Features:     ");
	if (impl->features == 0) {
		printf("None (base instruction set)\n");
	} else {
		if (impl->features & ALPHA_EXT_BWX)
			printf("BWX ");
		if (impl->features & ALPHA_EXT_FIX)
			printf("FIX ");
		if (impl->features & ALPHA_EXT_CIX)
			printf("CIX ");
		if (impl->features & ALPHA_EXT_MVI)
			printf("MVI ");
		if (impl->features & ALPHA_EXT_PAT)
			printf("PAT ");
		printf("\n");
	}

	printf("  I-cache:      %ld KB (%ld byte lines)\n",
	       impl->icache_size / 1024,
	       impl->impl_ver == 3 ? 64 : 32);
	printf("  D-cache:      %ld KB (%ld byte lines)\n",
	       impl->dcache_size / 1024,
	       impl->impl_ver == 3 ? 64 : 32);

	if (impl->scache_size > 0)
		printf("  S-cache:      %ld KB (64 byte lines)\n",
		       impl->scache_size / 1024);
	else
		printf("  S-cache:      None (on-chip caches only)\n");

	printf("  PALcode:      ");
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

	/*
	 * Print frequency information if available
	 */
	extern unsigned long alpha_hwrpb_cycle_freq(void);
	unsigned long freq = alpha_hwrpb_cycle_freq();
	if (freq > 0) {
		printf("  Frequency:    %ld MHz\n", freq / 1000000);
	}
}

/*
 * Determine optimal memory copy strategy based on CPU
 */
int
alpha_optimal_copy_size(void)
{
	const struct alpha_cpu_impl *impl = alpha_detect_cpu();

	/*
	 * EV6 has larger caches, use bigger blocks
	 */
	if (impl->impl_ver == 3)
		return 256;

	/*
	 * EV4/EV5 use smaller blocks
	 */
	return 64;
}

/*
 * Check if CPU supports unaligned access
 *
 * All Alpha CPUs require aligned access, but some can
 * handle unaligned in firmware/PALcode
 */
boolean_t
alpha_supports_unaligned(void)
{
	/*
	 * EV56 and later have better unaligned access handling
	 */
	const struct alpha_cpu_impl *impl = alpha_detect_cpu();
	return (impl->features & ALPHA_EXT_BWX) != 0;
}
