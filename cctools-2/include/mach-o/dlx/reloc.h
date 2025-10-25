/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * DLX Relocation Types
 * Defines relocation types for DLX architecture object files
 */

#ifndef _MACH_O_DLX_RELOC_H_
#define _MACH_O_DLX_RELOC_H_

/*
 * DLX relocation types
 */
enum reloc_type_dlx
{
	DLX_RELOC_VANILLA,	/* Generic relocation - absolute 32-bit address */
	DLX_RELOC_PAIR,		/* Second relocation entry in pair */
	DLX_RELOC_HI16,		/* High 16 bits of 32-bit address */
	DLX_RELOC_LO16,		/* Low 16 bits of 32-bit address */
	DLX_RELOC_J26,		/* 26-bit jump/branch target */
	DLX_RELOC_BR16,		/* 16-bit PC-relative branch */
	DLX_RELOC_SECTDIFF,	/* Section difference */
	DLX_RELOC_LOCAL_SECTDIFF, /* Section difference for local symbols */
	DLX_RELOC_PB_LA_PTR	/* Lazy pointer relocation */
};

#endif /* _MACH_O_DLX_RELOC_H_ */
