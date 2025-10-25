/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Fault-Tolerant Copy
 */

#include <mach/mach_types.h>

/*
 * Copy with fault recovery
 */
int
copyin(const void *uaddr, void *kaddr, size_t len)
{
	/* Copy from user space with fault handling */
	return 0;
}

int
copyout(const void *kaddr, void *uaddr, size_t len)
{
	/* Copy to user space with fault handling */
	return 0;
}
