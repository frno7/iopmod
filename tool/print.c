// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iopmod/tool/print.h"

static void report(const char *prefix, const char *suffix,
	const char *fmt, va_list ap)
{
	char msg[4096];

	vsnprintf(msg, sizeof(msg), fmt, ap);

	fprintf(stderr, "%s: %s%s%s%s%s\n", progname,
		prefix, prefix[0] ? ": " : "",
		msg,
		suffix[0] ? ": " : "", suffix);
}

void pr_info(const char *fmt, ...)
{
	char msg[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	printf("%s\n", msg);
	va_end(ap);
}

void pr_warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	report("warning", "", fmt, ap);
	va_end(ap);
}

void NORETURN pr_fatal_error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	report("error", "", fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

void NORETURN pr_fatal_errno(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	report("error", strerror(errno), fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}
