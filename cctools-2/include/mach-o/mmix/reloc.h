/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * MMIX Relocation Types
 */

#ifndef _MACH_O_MMIX_RELOC_H_
#define _MACH_O_MMIX_RELOC_H_

/*
 * MMIX relocation types
 */
#define MMIX_RELOC_VANILLA	0	/* Generic relocation */
#define MMIX_RELOC_PAIR		1	/* Pair relocation (for HIGH16/LOW16) */
#define MMIX_RELOC_HIGH16	2	/* High 16 bits of 32-bit value */
#define MMIX_RELOC_LOW16	3	/* Low 16 bits of 32-bit value */
#define MMIX_RELOC_BR24		4	/* 24-bit branch (PC-relative) */
#define MMIX_RELOC_JMP		5	/* Jump/call relocation */
#define MMIX_RELOC_SECTDIFF	6	/* Section difference */
#define MMIX_RELOC_LOCAL_SECTDIFF 7	/* Local section difference */

#endif /* _MACH_O_MMIX_RELOC_H_ */
