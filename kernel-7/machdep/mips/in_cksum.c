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
 * MIPS:	Internet checksum routines
 *
 * Compute TCP/IP checksums for network packets.
 * Optimized for MIPS architecture.
 */

#include <sys/param.h>
#include <sys/mbuf.h>

/*
 * Compute Internet checksum
 *
 * This is the standard Internet checksum algorithm:
 * 1. Sum all 16-bit words
 * 2. Add carry bits back into sum
 * 3. Take one's complement
 */
unsigned short
in_cksum(struct mbuf *m, int len)
{
	unsigned char *addr;
	unsigned int sum = 0;
	int count = len;
	unsigned short *w;

	while (m && count > 0) {
		addr = mtod(m, unsigned char *);
		w = (unsigned short *)addr;

		/* Handle partial mbuf */
		int mlen = m->m_len;
		if (mlen > count)
			mlen = count;

		count -= mlen;

		/* Sum 16-bit words */
		while (mlen > 1) {
			sum += *w++;
			mlen -= 2;
		}

		/* Handle odd byte */
		if (mlen == 1) {
#if BYTE_ORDER == BIG_ENDIAN
			sum += *(unsigned char *)w << 8;
#else
			sum += *(unsigned char *)w;
#endif
		}

		m = m->m_next;
	}

	/* Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	/* Return one's complement */
	return ~sum;
}

/*
 * Compute checksum for contiguous buffer
 * Optimized version for aligned data
 */
unsigned short
in_cksum_buf(void *buf, int len)
{
	unsigned int sum = 0;
	unsigned short *w = (unsigned short *)buf;
	int count = len;

	/* Sum 32-bit words for speed */
	while (count >= 32) {
		sum += w[0];
		sum += w[1];
		sum += w[2];
		sum += w[3];
		sum += w[4];
		sum += w[5];
		sum += w[6];
		sum += w[7];
		sum += w[8];
		sum += w[9];
		sum += w[10];
		sum += w[11];
		sum += w[12];
		sum += w[13];
		sum += w[14];
		sum += w[15];
		w += 16;
		count -= 32;
	}

	/* Sum remaining 16-bit words */
	while (count > 1) {
		sum += *w++;
		count -= 2;
	}

	/* Add odd byte if present */
	if (count == 1) {
#if BYTE_ORDER == BIG_ENDIAN
		sum += *(unsigned char *)w << 8;
#else
		sum += *(unsigned char *)w;
#endif
	}

	/* Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

/*
 * Incremental checksum update
 * Used when modifying packet headers
 */
unsigned short
in_cksum_update(unsigned short old_cksum, unsigned short old_val,
                unsigned short new_val)
{
	unsigned int sum;

	/* Convert checksum to one's complement form */
	sum = ~old_cksum & 0xffff;

	/* Subtract old value */
	sum += ~old_val & 0xffff;

	/* Add new value */
	sum += new_val;

	/* Fold carries */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

/*
 * Partial checksum computation
 * For fragmented packets
 */
unsigned int
in_cksum_partial(void *buf, int len, unsigned int sum)
{
	unsigned short *w = (unsigned short *)buf;
	int count = len;

	/* Sum 16-bit words */
	while (count > 1) {
		sum += *w++;
		count -= 2;
	}

	/* Add odd byte */
	if (count == 1) {
#if BYTE_ORDER == BIG_ENDIAN
		sum += *(unsigned char *)w << 8;
#else
		sum += *(unsigned char *)w;
#endif
	}

	return sum;
}

/*
 * Finalize partial checksum
 */
unsigned short
in_cksum_finish(unsigned int sum)
{
	/* Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}
