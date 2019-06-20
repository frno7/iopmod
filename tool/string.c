// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <stdlib.h>
#include <string.h>

#include "iopmod/tool/print.h"
#include "iopmod/tool/string.h"

char *strdupcat(const char *a, const char *b)
{
	const size_t a_length = strlen(a);
	const size_t b_length = strlen(b);
	const size_t length = a_length + b_length;

	char *s = malloc(length + 1);
	if (!s)
		pr_fatal_errno(__func__);

	memcpy(s, a, a_length);
	memcpy(&s[a_length], b, b_length);
	s[length] = '\0';

	return s;
}
