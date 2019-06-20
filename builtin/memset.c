// SPDX-License-Identifier: GPL-2.0

#include "iopmod/string.h"

void *memset(void *buf, int byte, size_t nbytes)
{
	char *b = buf;
	char *e = &b[nbytes];

	while (b != e)
		*b++ = byte;

	return buf;
}
