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

#include <sys/types.h>

void *
memset(void *b, int c, size_t len)
{
	char *p = b;
	size_t i;

	/* Optimized version for zero fill */
	if (c == 0 && !(((unsigned long)p | len) & (sizeof(long) - 1))) {
		unsigned long *lp = (unsigned long *)p;
		size_t nwords = len / sizeof(long);

		for (i = 0; i < nwords; i++)
			lp[i] = 0;
	} else {
		/* Byte fill */
		for (i = 0; i < len; i++)
			p[i] = (unsigned char)c;
	}

	return b;
}
