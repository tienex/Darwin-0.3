/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Memory Device Driver
 */

#include <sys/param.h>
#include <sys/conf.h>

/*
 * Memory device read
 */
int
mmread(dev_t dev, struct uio *uio)
{
	/* Read from physical memory */
	return 0;
}

/*
 * Memory device write
 */
int
mmwrite(dev_t dev, struct uio *uio)
{
	/* Write to physical memory */
	return 0;
}
