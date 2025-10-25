/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha VM Type Definitions
 */

#ifndef _MACH_ALPHA_VM_TYPES_H_
#define _MACH_ALPHA_VM_TYPES_H_

/*
 * Alpha is a 64-bit architecture
 */

typedef unsigned long		natural_t;
typedef signed long		integer_t;

/*
 * Page size
 */
#define PAGE_SIZE		8192		/* 8K pages */
#define PAGE_SHIFT		13
#define PAGE_MASK		(PAGE_SIZE - 1)

/*
 * VM address and size types
 */
typedef unsigned long		vm_offset_t;
typedef unsigned long		vm_size_t;

#endif /* _MACH_ALPHA_VM_TYPES_H_ */
