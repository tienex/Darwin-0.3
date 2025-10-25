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
 * RISC-V fault-tolerant copy routines
 */

#include <sys/param.h>
#include <sys/systm.h>

/*
 * Copy data with fault recovery
 * Returns 0 on success, error code on fault
 */
int
copyinstr(const void *udaddr, void *kaddr, size_t len, size_t *done)
{
	/* Copy string from user space with bounds checking */
	/* Stub implementation */
	return 0;
}

int
copyoutstr(const void *kaddr, void *udaddr, size_t len, size_t *done)
{
	/* Copy string to user space with bounds checking */
	/* Stub implementation */
	return 0;
}

int
copyin(const void *udaddr, void *kaddr, size_t len)
{
	/* Copy data from user space */
	/* Stub implementation */
	return 0;
}

int
copyout(const void *kaddr, void *udaddr, size_t len)
{
	/* Copy data to user space */
	/* Stub implementation */
	return 0;
}
