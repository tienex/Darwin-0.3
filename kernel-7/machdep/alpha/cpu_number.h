/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha CPU Number Implementation
 */

#ifndef _MACHDEP_ALPHA_CPU_NUMBER_H_
#define _MACHDEP_ALPHA_CPU_NUMBER_H_

#include <architecture/alpha/pal.h>

#ifndef __ASSEMBLER__

/*
 * Get CPU number using PALcode WHAMI function
 */
static inline int
cpu_number(void)
{
	unsigned long cpu_id;

#if NCPUS > 1
	/* Use PALcode to get CPU ID */
	extern unsigned long pal_unix_whami(void);
	cpu_id = pal_unix_whami();
#else
	/* Single CPU system */
	cpu_id = 0;
#endif

	return (int)cpu_id;
}

#endif /* !__ASSEMBLER__ */

#endif /* _MACHDEP_ALPHA_CPU_NUMBER_H_ */
