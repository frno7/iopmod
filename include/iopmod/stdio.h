// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_STDIO_H
#define IOPMOD_STDIO_H

#include <stdarg.h>

#include "iopmod/types.h"

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

int printf(const char *format, ...);	/* Alias for stdio_printf */

#endif /* IOPMOD_STDIO_H */
