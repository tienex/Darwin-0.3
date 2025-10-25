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
 * RISC-V Internet Checksum Routine
 *
 * Portable implementation for TCP/IP checksumming
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>

/*
 * Checksum routine for Internet Protocol family headers (Portable Version).
 *
 * This routine is very heavily used in the network code and should be
 * modified for each CPU to be as fast as possible.
 */
int
in_cksum(struct mbuf *m, int len)
{
	unsigned short *w;
	int sum = 0;
	int mlen = 0;
	int byte_swapped = 0;
	union {
		unsigned char c[2];
		unsigned short s;
	} s_util;
	union {
		unsigned short s[2];
		long l;
	} l_util;

	for (; m && len; m = m->m_next) {
		if (m->m_len == 0)
			continue;
		w = mtod(m, unsigned short *);
		if (mlen == -1) {
			/*
			 * The first byte of this mbuf is the continuation
			 * of a word spanning between this mbuf and the
			 * last mbuf.
			 */
			s_util.c[1] = *(unsigned char *)w;
			sum += s_util.s;
			w = (unsigned short *)((char *)w + 1);
			mlen = m->m_len - 1;
			len--;
		} else
			mlen = m->m_len;
		if (len < mlen)
			mlen = len;
		len -= mlen;

		/*
		 * Force to even boundary.
		 */
		if ((3 & (long)w) && (mlen > 0)) {
			sum <<= 8;
			s_util.c[0] = *(unsigned char *)w;
			w = (unsigned short *)((char *)w + 1);
			mlen--;
			byte_swapped = 1;
		}

		/*
		 * Unroll the loop to make overhead from branches &
		 * misc small.
		 */
		while ((mlen -= 32) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
			sum += w[4]; sum += w[5]; sum += w[6]; sum += w[7];
			sum += w[8]; sum += w[9]; sum += w[10]; sum += w[11];
			sum += w[12]; sum += w[13]; sum += w[14]; sum += w[15];
			w += 16;
		}
		mlen += 32;
		while ((mlen -= 8) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
			w += 4;
		}
		mlen += 8;
		if (mlen == 0 && byte_swapped == 0)
			continue;
		while ((mlen -= 2) >= 0) {
			sum += *w++;
		}
		if (byte_swapped) {
			sum <<= 8;
			byte_swapped = 0;
			if (mlen == -1) {
				s_util.c[1] = *(unsigned char *)w;
				sum += s_util.s;
				mlen = 0;
			} else
				mlen = -1;
		} else if (mlen == -1)
			s_util.c[0] = *(unsigned char *)w;
	}
	if (len)
		printf("cksum: out of data\n");
	if (mlen == -1) {
		/* The last mbuf has odd # of bytes. Follow the
		   standard (the odd byte may be shifted left by 8 bits
		   or not as determined by endian-ness of the machine) */
		s_util.c[1] = 0;
		sum += s_util.s;
	}

	/*
	 * Add together 16-bit pieces of checksum.
	 */
	l_util.l = sum;
	sum = l_util.s[0] + l_util.s[1];
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return (~sum & 0xffff);
}
