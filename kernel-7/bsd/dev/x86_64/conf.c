/*
 * x86-64 device configuration table
 *
 * This file contains the device switch tables for x86-64.
 *
 * STUB IMPLEMENTATION
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>

/*
 * Character device switch table
 */
struct cdevsw cdevsw[] = {
	/* Stub - would contain actual device entries */
	{ 0 }
};

int nchrdev = sizeof(cdevsw) / sizeof(cdevsw[0]);

/*
 * Block device switch table
 */
struct bdevsw bdevsw[] = {
	/* Stub - would contain actual device entries */
	{ 0 }
};

int nblkdev = sizeof(bdevsw) / sizeof(bdevsw[0]);
