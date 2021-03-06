// SPDX-License-Identifier: GPL-2.0

#include "iopmod/string.h"

size_t strlen(const char *s)
{
	const char *b = s;

	while (*s)
		s++;

	return s - b;
}
