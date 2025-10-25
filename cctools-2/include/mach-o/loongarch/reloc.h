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
 * Relocation types used in the LoongArch implementation.  Relocation entries
 * for things other than instructions use the same generic relocation as
 * described above and their r_type is RELOC_VANILLA.  The rest of the
 * relocation types are for instructions.
 */
enum reloc_type_loongarch
{
    LOONGARCH_RELOC_VANILLA,	/* generic relocation as described above */
    LOONGARCH_RELOC_PAIR,	/* the second relocation entry of a pair */
    LOONGARCH_RELOC_PCREL26,	/* 26 bit PC-relative branch */
    LOONGARCH_RELOC_PCREL21,	/* 21 bit PC-relative branch */
    LOONGARCH_RELOC_PCREL16,	/* 16 bit PC-relative branch */
    LOONGARCH_RELOC_ABS_HI20,	/* high 20 bits of absolute address */
    LOONGARCH_RELOC_ABS_LO12,	/* low 12 bits of absolute address */
    LOONGARCH_RELOC_ABS64_LO20,	/* low 20 bits of 64-bit absolute address */
    LOONGARCH_RELOC_ABS64_HI12	/* high 12 bits of 64-bit absolute address */
};
