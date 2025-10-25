/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * MMIX Relocation Header
 */

#ifndef _MMIX_RELOC_H_
#define _MMIX_RELOC_H_

#include <mach-o/reloc.h>
#include "objects.h"

__private_extern__ void mmix_reloc(
    void *data,
    struct relocation_info *relocs,
    unsigned long nreloc,
    struct section_map *section_map,
    struct nlist *symbols,
    unsigned long nsymbols);

__private_extern__ unsigned long mmix_get_reloc_r_address(
    struct relocation_info *reloc);

__private_extern__ void mmix_free_reloc(
    void *data,
    struct relocation_info *relocs,
    unsigned long nreloc);

#endif /* _MMIX_RELOC_H_ */
