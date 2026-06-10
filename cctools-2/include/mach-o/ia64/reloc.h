/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * IA64 (Intel Itanium) Relocation Types
 */

#ifndef _MACH_O_IA64_RELOC_H_
#define _MACH_O_IA64_RELOC_H_

/*
 * IA64 relocation types.
 *
 * IA-64 instructions are packed three to a 128-bit bundle, so
 * instruction-field relocations identify the slot within the bundle
 * via the r_address of the relocation entry (the address of the
 * containing bundle) plus the slot number encoded in the relocation
 * type itself where needed.
 */
#define IA64_RELOC_VANILLA	0	/* Generic data relocation */
#define IA64_RELOC_PAIR		1	/* Second entry of a pair */
#define IA64_RELOC_IMM14	2	/* 14-bit immediate (add/adds) */
#define IA64_RELOC_IMM22	3	/* 22-bit immediate (addl) */
#define IA64_RELOC_IMM64	4	/* 64-bit immediate (movl) */
#define IA64_RELOC_PCREL21B	5	/* 21-bit PC-relative branch (br) */
#define IA64_RELOC_PCREL60B	6	/* 60-bit PC-relative branch (brl) */
#define IA64_RELOC_GPREL22	7	/* 22-bit GP-relative offset */
#define IA64_RELOC_LTOFF22	8	/* 22-bit linkage-table offset */
#define IA64_RELOC_SECTDIFF	9	/* Section difference */
#define IA64_RELOC_LOCAL_SECTDIFF 10	/* Local section difference */

#endif /* _MACH_O_IA64_RELOC_H_ */
