// SPDX-License-Identifier: GPL-2.0

#include "iopmod/string.h"

void *memcpy(void *dst, const void *src, size_t nbytes)
{
	char *d = dst;
	char *e = &d[nbytes];
	const char *s = src;

	while (d != e)
		*d++ = *s++;

	return dst;
}
