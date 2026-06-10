/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * MMIX Disassembler Header
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#import <stuff/bytesex.h>
#import <mach-o/reloc.h>
#import <mach-o/nlist.h>
#import <stuff/bool.h>

extern unsigned long mmix_disassemble(
    char *sect,
    unsigned long left,
    unsigned long addr,
    unsigned long sect_addr,
    enum byte_sex object_byte_sex,
    struct relocation_info *sorted_relocs,
    unsigned long nsorted_relocs,
    struct nlist *symbols,
    unsigned long nsymbols,
    struct nlist *sorted_symbols,
    unsigned long nsorted_symbols,
    char *strings,
    unsigned long strings_size,
    unsigned long *indirect_symbols,
    unsigned long nindirect_symbols,
    struct mach_header *mh,
    struct load_command *load_commands,
    enum bool verbose);
