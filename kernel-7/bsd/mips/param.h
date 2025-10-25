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

#ifndef	_MIPS_PARAM_H_
#define	_MIPS_PARAM_H_

/*
 * Round p (pointer or byte index) up to a correctly-aligned value for all
 * data types (int, long, ...).   The result is u_int and must be cast to
 * any desired pointer type.
 */
#if defined(_MIPS64) || defined(__mips64)
#define	ALIGNBYTES	15		/* 16-byte alignment for MIPS64 */
#else
#define	ALIGNBYTES	7		/* 8-byte alignment for MIPS32 */
#endif
#define	ALIGN(p)	(((u_int)(p) + ALIGNBYTES) &~ ALIGNBYTES)

#define	NBPG		4096		/* bytes/page */
#define	PGOFSET		(NBPG-1)	/* byte offset into page */
#define	PGSHIFT		12		/* LOG2(NBPG) */

#define NBSEG		0x40000000	/* bytes/segment */
#define	SEGOFSET	(NBSEG-1)	/* byte offset into segment */
#define	SEGSHIFT	30		/* LOG2(NBSEG) */

#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define BLKDEV_IOSIZE	2048
#define	MAXPHYS		(64 * 1024)	/* max raw I/O transfer size */

#define	STACK_GROWTH_UP	0		/* stack grows to lower addresses */

#define	CLSIZE		1
#define	CLSIZELOG2	0

#define STACKSIZE 4			/* pages in kernel stack */
#define	UPAGES	(USIZE+STACKSIZE)	/* total pages in u-area */

/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than CLBYTES (the software page size), and,
 * on machines that exchange pages of input or output buffers with mbuf
 * clusters (MAPPED_MBUFS), MCLBYTES must also be an integral multiple
 * of the hardware page size.
 */
#define	MSIZE		256		/* size of an mbuf */
#define	MCLBYTES	2048		/* large enough for ether MTU */
#define	MCLSHIFT	11
#define	MCLOFSET	(MCLBYTES - 1)
#ifndef NMBCLUSTERS
#if GATEWAY
#define	NMBCLUSTERS	((1024 * 1024) / MCLBYTES)	/* cl map size: 1MB */
#else
#define	NMBCLUSTERS	((1024 * 1024) / MCLBYTES)
#endif
#endif

/* pages ("clicks") (NBPG bytes) to disk blocks */
#define	ctod(x)	((x)<<(PGSHIFT-DEV_BSHIFT))
#define	dtoc(x)	((x)>>(PGSHIFT-DEV_BSHIFT))
#define	dtob(x)	((x)<<DEV_BSHIFT)

/* pages to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to pages */
#define	btoc(x)	(((unsigned)(x)+(PGOFSET))>>PGSHIFT)

#ifdef __APPLE__
#define  btodb(bytes, devBlockSize)         \
        ((unsigned)(bytes) / devBlockSize)
#define  dbtob(db, devBlockSize)            \
             ((unsigned)(db) * devBlockSize)
#else
#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned)(db) << DEV_BSHIFT)
#endif

/*
 * Map a ``block device block'' to a file system block.
 * This should be device dependent, and should use the bsize
 * field from the disk label.
 * For now though just use DEV_BSIZE.
 */
#define	bdbtofsb(bn)	((bn) / (BLKDEV_IOSIZE/DEV_BSIZE))

/*
 * MIPS-specific: Cache line size
 */
#define CACHE_LINE_SIZE	32	/* Typical MIPS cache line size */

#endif	/* _MIPS_PARAM_H_ */
