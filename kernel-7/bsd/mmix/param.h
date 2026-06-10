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
 * MMIX machine-dependent parameters
 */

#ifndef	_MMIX_PARAM_H_
#define	_MMIX_PARAM_H_

/*
 * Round p (pointer or byte index) up to a correctly-aligned value for all
 * data types (int, long, ...).   The result is u_long long and must be cast to
 * any desired pointer type.
 *
 * MMIX requires 8-byte alignment for optimal performance.
 */
#define	ALIGNBYTES	7
#define	ALIGN(p)	(((unsigned long long)(p) + ALIGNBYTES) &~ ALIGNBYTES)

/*
 * MMIX uses 8KB pages
 */
#define	NBPG		8192		/* bytes/page */
#define	PGOFSET		(NBPG-1)	/* byte offset into page */
#define	PGSHIFT		13		/* LOG2(NBPG) */

/*
 * Segment size for MMIX (256 MB segments)
 */
#define NBSEG		0x10000000	/* bytes/segment */
#define	SEGOFSET	(NBSEG-1)	/* byte offset into segment */
#define	SEGSHIFT	28		/* LOG2(NBSEG) */

/*
 * Disk block size
 */
#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define BLKDEV_IOSIZE	8192		/* Larger I/O size for 64-bit */
#define	MAXPHYS		(128 * 1024)	/* max raw I/O transfer size (128KB) */

/*
 * Stack growth direction
 */
#define	STACK_GROWTH_UP	0		/* stack grows to lower addresses */

#define	CLSIZE		1
#define	CLSIZELOG2	0

#define STACKSIZE 8			/* pages in kernel stack (64KB) */
#define	UPAGES	(USIZE+STACKSIZE)	/* total pages in u-area */

/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than CLBYTES (the software page size), and,
 * on machines that exchange pages of input or output buffers with mbuf
 * clusters (MAPPED_MBUFS), MCLBYTES must also be an integral multiple
 * of the hardware page size.
 */
#define	MSIZE		256		/* size of an mbuf */
#define	MCLBYTES	8192		/* large enough for jumbo frames */
#define	MCLSHIFT	13
#define	MCLOFSET	(MCLBYTES - 1)
#ifndef NMBCLUSTERS
#if GATEWAY
#define	NMBCLUSTERS	((2 * 1024 * 1024) / MCLBYTES)	/* 2MB for gateway */
#else
#define	NMBCLUSTERS	((2 * 1024 * 1024) / MCLBYTES)	/* 2MB default */
#endif
#endif

/* pages ("clicks") (NBPG bytes) to disk blocks */
#define	ctod(x)	((x)<<(PGSHIFT-DEV_BSHIFT))
#define	dtoc(x)	((x)>>(PGSHIFT-DEV_BSHIFT))
#define	dtob(x)	((x)<<DEV_BSHIFT)

/* pages to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to pages */
#define	btoc(x)	(((unsigned long long)(x)+(PGOFSET))>>PGSHIFT)

#ifdef __APPLE__
#define  btodb(bytes, devBlockSize)         \
        ((unsigned long long)(bytes) / devBlockSize)
#define  dbtob(db, devBlockSize)            \
             ((unsigned long long)(db) * devBlockSize)
#else
#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned long long)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned long long)(db) << DEV_BSHIFT)
#endif

/*
 * Map a ``block device block'' to a file system block.
 * This should be device dependent, and should use the bsize
 * field from the disk label.
 * For now though just use DEV_BSIZE.
 */
#define	bdbtofsb(bn)	((bn) / (BLKDEV_IOSIZE/DEV_BSIZE))

/*
 * MMIX endianness mask (big-endian)
 */
#if __BIG_ENDIAN__
#define ENDIAN_MASK(val,size) (1ULL << (size-1 - val))
#else
#error MMIX is big-endian only
#endif /* __BIG_ENDIAN__ */

#ifndef MASK
#define MASK(PART)	ENDIAN_MASK(PART ## _BIT, 64)
#endif

/*
 * MMIX processor state bits (from rK - interrupt mask register)
 */
#define	MMIX_K_BIT	0	/* Kernel mode bit */
#define	MMIX_IE_BIT	1	/* Interrupt enable bit */
#define USERMODE(k) ((k & MASK(MMIX_K)) ? FALSE : TRUE)
#define BASEPRI(k) ((k & MASK(MMIX_IE)) ? TRUE : FALSE)

#ifndef __ASSEMBLER__
/*
 * Pull in our definition of simple_lock_data_t.
 */
#import <mach/machine/simple_lock.h>

#endif	/* __ASSEMBLER__ */

#if	defined(_KERNEL) || defined(STANDALONE)
#define	DELAY(n) us_spin(n)
#else
#define	DELAY(n)	{ register long long N = (n); while (--N > 0); }
#endif	/* defined(_KERNEL) || defined(STANDALONE) */

/*
 * MMIX address space identifiers
 */
#define	NPIDS		256	/* maximum number of address space IDs */
#define	NIOPIDS		16	/* maximum number of IO space IDs */

/*
 * MMIX-specific processor features
 */
#define MMIX_HAS_FPU		1	/* Integrated floating point */
#define MMIX_HAS_MMU		1	/* Memory management unit */
#define MMIX_HAS_CACHE		1	/* Instruction and data caches */
#define MMIX_HAS_PIPELINE	1	/* Instruction pipeline */

#endif	/* _MMIX_PARAM_H_ */
