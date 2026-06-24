// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_STRING_H
#define IOPMOD_STRING_H

#include "iopmod/types.h"

int memcmp(const void *s1, const void *s2, size_t nbytes);
void *memcpy(void *dst, const void *src, size_t nbytes);
void *memset(void *buf, int byte, size_t nbytes);
size_t strlen(const char *s);

#endif /* IOPMOD_STRING_H */
