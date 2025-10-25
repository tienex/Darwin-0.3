/*
 * memcpy - copy memory area
 * x86-64 version
 */

#include <sys/types.h>

void *
memcpy(void *dst, const void *src, size_t n)
{
	char *d = dst;
	const char *s = src;

	/* Copy 8 bytes at a time when possible (64-bit aligned) */
	while (n >= 8) {
		*(unsigned long *)d = *(const unsigned long *)s;
		d += 8;
		s += 8;
		n -= 8;
	}

	/* Copy remaining bytes */
	while (n--) {
		*d++ = *s++;
	}

	return dst;
}
