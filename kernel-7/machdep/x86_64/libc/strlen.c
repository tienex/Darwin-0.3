/*
 * strlen - calculate the length of a string
 * x86-64 optimized version
 */

#include <sys/types.h>

size_t
strlen(const char *str)
{
	const char *s;

	for (s = str; *s; ++s)
		;
	return (s - str);
}
