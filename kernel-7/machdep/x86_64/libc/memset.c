/*
 * memset - fill memory with a constant byte
 * x86-64 version
 */

#include <sys/types.h>

void *
memset(void *dst, int c, size_t n)
{
	char *d = dst;
	unsigned char uc = (unsigned char)c;

	/* Fill 8 bytes at a time when possible */
	if (n >= 8) {
		unsigned long pattern = uc;
		pattern |= pattern << 8;
		pattern |= pattern << 16;
		pattern |= pattern << 32;

		while (n >= 8) {
			*(unsigned long *)d = pattern;
			d += 8;
			n -= 8;
		}
	}

	/* Fill remaining bytes */
	while (n--) {
		*d++ = uc;
	}

	return dst;
}
