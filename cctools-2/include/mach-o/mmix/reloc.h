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
 * Relocation types used in the MMIX implementation.  Relocation entries for
 * things other than instructions use the same generic relocation as described
 * above and their r_type is RELOC_VANILLA.  The rest of the relocation types
 * are for instructions.  Since they are for instructions the r_address field
 * indicates the 32 bit instruction that the relocation is to be performed on.
 */
enum reloc_type_mmix
{
    MMIX_RELOC_VANILLA,     /* generic relocation as described above */
    MMIX_RELOC_PAIR,        /* the second relocation entry of a pair */
    MMIX_RELOC_WYDE,        /* 16 bit immediate value */
    MMIX_RELOC_TETRA,       /* 32 bit immediate value */
    MMIX_RELOC_OCTA,        /* 64 bit immediate value */
    MMIX_RELOC_JMP,         /* 24 bit branch displacement (to a tetra address) */
    MMIX_RELOC_GETA,        /* GETA instruction (load address) */
    MMIX_RELOC_PUSHJ,       /* PUSHJ instruction (procedure call) */
    MMIX_RELOC_PUSHGO,      /* PUSHGO instruction (procedure call with offset) */
    MMIX_RELOC_HI,          /* high 16 bits of address */
    MMIX_RELOC_MH,          /* medium-high 16 bits of address */
    MMIX_RELOC_ML,          /* medium-low 16 bits of address */
    MMIX_RELOC_LO,          /* low 16 bits of address */
    MMIX_RELOC_SECTDIFF,    /* a PAIR follows with subtract symbol value */
    MMIX_RELOC_HI_SECTDIFF, /* section difference forms of above */
    MMIX_RELOC_LO_SECTDIFF
};
