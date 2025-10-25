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
 * MIPS endian definitions
 * MIPS supports both big-endian and little-endian byte ordering
 */

#ifndef _MIPS_ENDIAN_H_
#define	_MIPS_ENDIAN_H_

/*
 * Define the order of 32-bit words in 64-bit words.
 */
#if defined(__MIPSEB__) || defined(_MIPSEB) || defined(__BIG_ENDIAN__)
#define _QUAD_HIGHWORD 0
#define _QUAD_LOWWORD 1
#else
#define _QUAD_HIGHWORD 1
#define _QUAD_LOWWORD 0
#endif

#if	defined(_KERNEL) || !defined(_POSIX_SOURCE)
/*
 * Definitions for byte order, according to byte significance from low
 * address to high.
 */
#define	LITTLE_ENDIAN	1234	/* LSB first: i386, vax, mipsel */
#define	BIG_ENDIAN	4321	/* MSB first: 68000, ibm, net, ppc, mipseb */
#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long */

/*
 * MIPS byte order - determined at compile time
 */
#if defined(__MIPSEB__) || defined(_MIPSEB) || defined(__BIG_ENDIAN__)
#define	BYTE_ORDER	BIG_ENDIAN
#elif defined(__MIPSEL__) || defined(_MIPSEL) || defined(__LITTLE_ENDIAN__)
#define	BYTE_ORDER	LITTLE_ENDIAN
#else
/* Default to big-endian if not specified */
#define	BYTE_ORDER	BIG_ENDIAN
#endif

#include <sys/cdefs.h>

#ifndef __ASSEMBLER__
__BEGIN_DECLS
unsigned long	htonl __P((unsigned long));
unsigned short	htons __P((unsigned short));
unsigned long	ntohl __P((unsigned long));
unsigned short	ntohs __P((unsigned short));
__END_DECLS
#endif /* __ASSEMBLER__ */

/*
 * Macros for network/external number representation conversion.
 */
#if BYTE_ORDER == BIG_ENDIAN && !defined(lint)
/* Big-endian: network byte order == native byte order */
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)

#define	NTOHL(x)	(x)
#define	NTOHS(x)	(x)
#define	HTONL(x)	(x)
#define	HTONS(x)	(x)

#else
/* Little-endian: need byte swapping */
#include <architecture/mips/byte_order.h>

#define ntohl(x)	NXSwapBigLongToHost(x)
#define ntohs(x)	NXSwapBigShortToHost(x)
#define htonl(x)	NXSwapHostLongToBig(x)
#define htons(x)	NXSwapHostShortToBig(x)

#define	NTOHL(x)	(x) = ntohl((u_long)x)
#define	NTOHS(x)	(x) = ntohs((u_short)x)
#define	HTONL(x)	(x) = htonl((u_long)x)
#define	HTONS(x)	(x) = htons((u_short)x)
#endif

#endif /* defined(_KERNEL) || !defined(_POSIX_SOURCE) */
#endif /* !_MIPS_ENDIAN_H_ */
