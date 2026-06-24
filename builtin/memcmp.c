// SPDX-License-Identifier: GPL-2.0

#include "iopmod/string.h"

int memcmp(const void *s1, const void *s2, size_t nbytes)
{
	const unsigned char *c1 = s1;
	const unsigned char *c2 = s2;

	while (nbytes--) {
		const int d = (int)*c1++ - (int)*c2++;

		if (d)
			return d;
	}

	return 0;
}
