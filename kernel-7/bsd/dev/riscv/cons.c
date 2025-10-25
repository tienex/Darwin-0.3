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
 * RISC-V console driver
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/kernel.h>

/* Console device */
struct tty cons;

/*
 * Console open
 */
int
cnopen(dev_t dev, int flag, int mode, struct proc *p)
{
	/* Initialize console */
	return 0;
}

/*
 * Console close
 */
int
cnclose(dev_t dev, int flag, int mode, struct proc *p)
{
	return 0;
}

/*
 * Console read
 */
int
cnread(dev_t dev, struct uio *uio, int ioflag)
{
	/* Read from console */
	return 0;
}

/*
 * Console write
 */
int
cnwrite(dev_t dev, struct uio *uio, int ioflag)
{
	/* Write to console */
	char c;
	int error;

	while (uio->uio_resid > 0) {
		error = ureadc(&c, uio);
		if (error)
			return error;
		cnputc(c);
	}
	return 0;
}

/*
 * Console ioctl
 */
int
cnioctl(dev_t dev, u_long cmd, caddr_t data, int flag, struct proc *p)
{
	/* Console ioctl */
	return ENOTTY;
}

/*
 * Console select
 */
int
cnselect(dev_t dev, int which, struct proc *p)
{
	/* Console select */
	return 0;
}

/*
 * Console putc - output a character
 */
void
cnputc(char c)
{
	/* Output character to console hardware */
	/* This is a stub - would normally write to UART */
	/* For SiFive UART: *(volatile char *)0x10000000 = c; */
}

/*
 * Console getc - input a character
 */
int
cngetc(void)
{
	/* Get character from console hardware */
	/* This is a stub - would normally read from UART */
	return -1;
}

/*
 * Initialize console
 */
void
cninit(void)
{
	/* Initialize console hardware */
}
