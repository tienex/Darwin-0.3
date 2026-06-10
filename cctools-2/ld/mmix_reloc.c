/*
 * Copyright (c) 1999-2025 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * MMIX Relocation Support
 *
 * This file implements relocation handling for MMIX architecture in the
 * Darwin static linker (ld).
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <strings.h>
#include <mach-o/loader.h>
#include <mach-o/reloc.h>
#include <mach-o/mmix/reloc.h>
#include "stuff/bool.h"
#include "stuff/errors.h"
#include "ld.h"
#include "objects.h"
#include "sections.h"
#include "pass1.h"
#include "symbols.h"
#include "layout.h"
#include "mmix_reloc.h"

/*
 * mmix_reloc() is called from pass1() in pass1.c for MMIX relocation entries
 * to perform the necessary relocation processing.
 */
__private_extern__
void
mmix_reloc(
char *contents,
struct relocation_info *relocs,
struct section_map *section_map)
{
	unsigned long i, j, value, offset, r_symbolnum, r_address, r_type;
	unsigned long r_extern, r_pcrel, r_length;
	unsigned long nreloc;
	unsigned long content;
	unsigned long long content_long;
	struct scattered_relocation_info *sreloc;
	struct nlist *symbols;

	/* Get relocation count from section map */
	nreloc = section_map->s->nreloc;

	/* Get symbols from current object */
	symbols = cur_obj->symbols;

	for(i = 0; i < nreloc; i++){
	    /*
	     * Break out the fields of the relocation entry and set the other
	     * local variables.
	     */
	    if((relocs[i].r_address & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)(relocs + i);
		r_address = sreloc->r_address;
		r_pcrel = sreloc->r_pcrel;
		r_type = sreloc->r_type;
		r_extern = 0;
		/* Calculate r_symbolnum (the section ordinal) */
		r_symbolnum = 0;
		for(j = 0; j < cur_obj->nsection_maps; j++){
		    if(sreloc->r_value >= cur_obj->section_maps[j].s->addr &&
		       sreloc->r_value < cur_obj->section_maps[j].s->addr +
		                         cur_obj->section_maps[j].s->size){
			r_symbolnum = j + 1;
			break;
		    }
		}
		if(r_symbolnum == 0){
		    error_with_cur_obj("bad scattered relocation entry %lu (address or "
			"offset out of range) in section (%.16s,%.16s)", i,
			section_map->s->segname, section_map->s->sectname);
		    return;
		}
		r_length = sreloc->r_length;
		offset = sreloc->r_value;
	    }
	    else{
		r_address = relocs[i].r_address;
		r_symbolnum = relocs[i].r_symbolnum;
		r_pcrel = relocs[i].r_pcrel;
		r_length = relocs[i].r_length;
		r_extern = relocs[i].r_extern;
		r_type = relocs[i].r_type;
		offset = 0;
	    }

	    /*
	     * Check the r_address field
	     */
	    if(r_address >= section_map->s->size){
		error_with_cur_obj("bad r_address (0x%x) for relocation entry %lu in "
		    "section (%.16s,%.16s)", (unsigned int)r_address, i,
		    section_map->s->segname, section_map->s->sectname);
		return;
	    }

	    /*
	     * Get the content at the address to be relocated
	     */
	    if(r_length == 2){
		content = *((unsigned long *)(contents + r_address));
		if(section_map->input_reloc_flags & RELOC_IN_NEED_SWAP)
		    content = SWAP_LONG(content);
	    }
	    else if(r_length == 3){
		content_long = *((unsigned long long *)(contents + r_address));
		if(section_map->input_reloc_flags & RELOC_IN_NEED_SWAP)
		    content_long = SWAP_LONG_LONG(content_long);
	    }
	    else{
		error_with_cur_obj("bad r_length (%lu) for MMIX relocation entry %lu in "
		    "section (%.16s,%.16s)", r_length, i,
		    section_map->s->segname, section_map->s->sectname);
		return;
	    }

	    /*
	     * Process relocation based on type
	     */
	    switch(r_type){
	    case MMIX_RELOC_VANILLA:
		/*
		 * Vanilla relocation - simple address relocation
		 */
		if(r_extern){
		    value = symbols[r_symbolnum].n_value + offset;
		}
		else{
		    if(r_symbolnum == R_ABS)
			value = offset;
		    else if(r_symbolnum > cur_obj->nsection_maps){
			error_with_cur_obj("bad r_symbolnum (%lu) for relocation entry "
			    "%lu", r_symbolnum, i);
			return;
		    }
		    else
			value = section_map->output_section->s.addr + offset;
		}

		if(r_length == 3){
		    content_long += value;
		    if(section_map->output_reloc_flags & RELOC_OUT_NEED_SWAP)
			content_long = SWAP_LONG_LONG(content_long);
		    *((unsigned long long *)(contents + r_address)) = content_long;
		}
		else{
		    content += value;
		    if(section_map->output_reloc_flags & RELOC_OUT_NEED_SWAP)
			content = SWAP_LONG(content);
		    *((unsigned long *)(contents + r_address)) = content;
		}
		break;

	    case MMIX_RELOC_PAIR:
		/* Pair relocation - handled with other relocation types */
		break;

	    case MMIX_RELOC_HIGH16:
		/* High 16 bits of address */
		if(r_extern)
		    value = symbols[r_symbolnum].n_value + offset;
		else
		    value = section_map->output_section->s.addr + offset;

		content = (content & 0xFFFF0000) | ((value >> 16) & 0xFFFF);
		if(section_map->output_reloc_flags & RELOC_OUT_NEED_SWAP)
		    content = SWAP_LONG(content);
		*((unsigned long *)(contents + r_address)) = content;
		break;

	    case MMIX_RELOC_LOW16:
		/* Low 16 bits of address */
		if(r_extern)
		    value = symbols[r_symbolnum].n_value + offset;
		else
		    value = section_map->output_section->s.addr + offset;

		content = (content & 0xFFFF0000) | (value & 0xFFFF);
		if(section_map->output_reloc_flags & RELOC_OUT_NEED_SWAP)
		    content = SWAP_LONG(content);
		*((unsigned long *)(contents + r_address)) = content;
		break;

	    case MMIX_RELOC_BR24:
		/* 24-bit branch relocation */
		if(r_pcrel == 0){
		    error_with_cur_obj("MMIX_RELOC_BR24 must be PC-relative");
		    return;
		}

		if(r_extern)
		    value = symbols[r_symbolnum].n_value + offset;
		else
		    value = section_map->output_section->s.addr + offset;

		/* Calculate PC-relative offset */
		value = value - (section_map->output_section->s.addr + r_address);
		value = value >> 2;  /* Instruction addresses are word-aligned */

		if((value & 0xFF000000) != 0 && (value & 0xFF000000) != 0xFF000000){
		    error_with_cur_obj("branch out of range for relocation entry %lu", i);
		    return;
		}

		content = (content & 0xFF000000) | (value & 0x00FFFFFF);
		if(section_map->output_reloc_flags & RELOC_OUT_NEED_SWAP)
		    content = SWAP_LONG(content);
		*((unsigned long *)(contents + r_address)) = content;
		break;

	    default:
		error_with_cur_obj("unknown relocation type %lu for MMIX relocation entry "
		    "%lu", r_type, i);
		return;
	    }
	}
}
