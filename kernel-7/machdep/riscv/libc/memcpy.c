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
memcpy(void *dst, const void *src, size_t n)
{
	const char *s = src;
	char *d = dst;
	size_t i;

	/* Word-aligned copy for better performance */
	if (((unsigned long)s | (unsigned long)d | n) & (sizeof(long) - 1)) {
		/* Byte copy if not aligned */
		for (i = 0; i < n; i++)
			d[i] = s[i];
	} else {
		/* Word copy if aligned */
		unsigned long *ld = (unsigned long *)d;
		const unsigned long *ls = (const unsigned long *)s;
		size_t nwords = n / sizeof(long);

		for (i = 0; i < nwords; i++)
			ld[i] = ls[i];
	}

	return dst;
}
