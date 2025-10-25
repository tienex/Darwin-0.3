/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * DLX Simple Lock Implementation
 */

#ifndef _MACH_DLX_SIMPLE_LOCK_H_
#define _MACH_DLX_SIMPLE_LOCK_H_

/*
 * Simple spin lock data structure
 */
typedef struct slock {
	volatile unsigned int lock_data;
} simple_lock_data_t, *simple_lock_t;

#define SIMPLE_LOCK_NULL	((simple_lock_t) 0)

/*
 * Lock initialization
 */
#define simple_lock_init(l) \
	((l)->lock_data = 0)

/*
 * Lock operations (defined in locore.s)
 */
extern void simple_lock(simple_lock_t);
extern void simple_unlock(simple_lock_t);
extern boolean_t simple_lock_try(simple_lock_t);

#define decl_simple_lock_data(class, name) \
	class simple_lock_data_t name

#endif /* _MACH_DLX_SIMPLE_LOCK_H_ */
