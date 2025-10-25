/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * Alpha Internet Checksum
 */

#include <sys/types.h>

/*
 * Compute Internet checksum
 */
unsigned short
in_cksum(void *addr, int len)
{
	unsigned short *w = addr;
	int sum = 0;

	while (len > 1) {
		sum += *w++;
		len -= 2;
	}

	if (len > 0)
		sum += *(unsigned char *)w;

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	return ~sum;
}
