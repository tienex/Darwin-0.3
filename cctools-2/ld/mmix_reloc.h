/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * MMIX Relocation Header
 */

#ifndef _MMIX_RELOC_H_
#define _MMIX_RELOC_H_

#if defined(__MWERKS__) && !defined(__private_extern__)
#define __private_extern__ __declspec(private_extern)
#endif

/*
 * Global types, variables and routines declared in the file mmix_reloc.c.
 *
 * The following include file need to be included before this file:
 * #include <reloc.h>
 * #include "sections.h"
 */
__private_extern__ void mmix_reloc(
    char *contents,
    struct relocation_info *relocs,
    struct section_map *map);

#endif /* _MMIX_RELOC_H_ */
