/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Console Driver
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <architecture/alpha/pal.h>

/*
 * Console output via PALcode
 */
void
cnputc(int c)
{
	/* Use PALcode console service to output character */
	extern unsigned long pal_unix_cserve(unsigned long, unsigned long, unsigned long);
	pal_unix_cserve(0, c, 0);  /* Console output command */
}

/*
 * Console input
 */
int
cngetc(void)
{
	/* Use PALcode console service to read character */
	return 0;
}

/*
 * Console probe
 */
void
cninit(void)
{
	/* Initialize console */
}
