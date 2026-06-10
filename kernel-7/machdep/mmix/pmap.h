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
 *	Pmap header for MMIX architecture
 *	Physical memory mapping and MMU management
 */

#ifndef	_MMIX_PMAP_H_
#define	_MMIX_PMAP_H_

#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>
#include <mach/machine/kern_return.h>
#include <kern/lock.h>

/* MMIX uses 64-bit addressing with 8KB pages.
 * Maximum supported physical memory: 256GB
 */
#define MAX_SUPPORTED_PHYSICAL_MEMORY	(256ULL*1024*1024*1024)

struct pmap {
	decl_simple_lock_data(,	lock)	     /* Lock on map */
	int			ref_count;   /* reference count */
	unsigned long long	space;	     /* address space ID for this pmap */
	struct pmap		*next;	     /* linked list of free pmaps */
	struct pmap_statistics	stats;	     /* statistics */
};

typedef struct pmap *pmap_t;

#define PMAP_NULL  ((pmap_t) 0)

extern pmap_t	kernel_pmap;			/* The kernel's map */

#define	PMAP_SWITCH_USER(th, map, my_cpu) th->map = map;

#define PMAP_ACTIVATE(pmap, th, cpu)
#define PMAP_DEACTIVATE(pmap, th, cpu)
#define PMAP_CONTEXT(pmap,th)

#define pmap_kernel_va(VA)	\
	(((VA) >= VM_MIN_KERNEL_ADDRESS) && ((VA) <= VM_MAX_KERNEL_ADDRESS))

/* MMIX address space management - 64-bit virtual addressing */
#define MMIX_ASID_KERNEL  0
#define ASID_MAX	((1<<16) - 1)	/* 16-bit ASID */

#define pmap_kernel()			(kernel_pmap)
#define	pmap_resident_count(pmap)	((pmap)->stats.resident_count)
#define pmap_remove_attributes(pmap,start,end)
#define pmap_copy(dpmap,spmap,da,len,sa)

#define pmap_phys_address(x)	((x) << MMIX_PGSHIFT)
#define pmap_phys_to_frame(x)	((x) >> MMIX_PGSHIFT)

#define PMAP_PTOB(x)		((x) << MMIX_PGSHIFT)

/*
 * prototypes.
 */
extern void 		mmix_protection_init(void);
extern vm_offset_t	kvtophys(vm_offset_t addr);
extern vm_offset_t	phystokv(vm_offset_t addr);
extern vm_offset_t	pmap_map(vm_offset_t va,
				 vm_offset_t spa,
				 vm_offset_t epa,
				 vm_prot_t prot);
extern kern_return_t    pmap_add_physical_memory(vm_offset_t spa,
						 vm_offset_t epa,
						 boolean_t available,
						 unsigned int attr);
extern vm_offset_t	pmap_map_bd(vm_offset_t va,
				    vm_offset_t spa,
				    vm_offset_t epa,
				    vm_prot_t prot);
extern void		pmap_bootstrap(unsigned long long mem_size,
				       vm_offset_t *first_avail);
extern void		pmap_block_map(vm_offset_t pa,
				       vm_size_t size,
				       vm_prot_t prot,
				       int entry,
				       int dtlb);
extern void		pmap_switch(pmap_t);

extern vm_offset_t pmap_extract(pmap_t pmap,
				vm_offset_t va);

extern void pmap_remove_all(vm_offset_t pa);

extern boolean_t pmap_verify_free(vm_offset_t pa);

extern void flush_cache(vm_offset_t pa, unsigned length);
extern void flush_cache_v(vm_offset_t pa, unsigned length);
extern void invalidate_cache_v(vm_offset_t pa, unsigned length);
typedef enum {
    cache_default,
    cache_writethrough,
    cache_disable
} cache_spec_t;

void pmap_enter_cache_spec(
		pmap_t		pmap,
		vm_offset_t	va,
		vm_offset_t	pa,
		vm_prot_t	prot,
		boolean_t	wired,
		cache_spec_t	caching);

#endif /* _MMIX_PMAP_H_ */
