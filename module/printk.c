// SPDX-License-Identifier: GPL-2.0
/*
 * Kernel print module.
 *
 * Copyright (C) 2019 Fredrik Noring
 */

#include "iopmod/module.h"
#include "iopmod/printk.h"
#include "iopmod/sifcmd.h"
#include "iopmod/stdarg.h"
#include "iopmod/stdio.h"

int printk(const char *fmt, ...)
{
	char buf[CMD_PACKET_PAYLOAD_MAX];
	int length;
	va_list ap;

	va_start(ap, fmt);
	length = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (length < 0)
		return length;

	return putk(buf);
}
