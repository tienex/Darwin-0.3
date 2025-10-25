/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Device Configuration
 */

#include <sys/param.h>
#include <sys/conf.h>

/* Block device switch table */
struct bdevsw bdevsw[] = {
	/* Stub - to be filled in */
};

int nblkdev = sizeof(bdevsw) / sizeof(bdevsw[0]);

/* Character device switch table */
struct cdevsw cdevsw[] = {
	/* Stub - to be filled in */
};

int nchrdev = sizeof(cdevsw) / sizeof(cdevsw[0]);
