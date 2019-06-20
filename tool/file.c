// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "iopmod/asm/macro.h"

#include "iopmod/compare.h"
#include "iopmod/types.h"

#include "iopmod/tool/file.h"
#include "iopmod/tool/print.h"

struct file file_read(const char *path)
{
	char * const p = strdup(path);
	size_t size = 0;
	u8 *data = NULL;
	int fd = -1;

	if (!p)
		goto err;

	fd = xopen(path, O_RDONLY);
	if (fd < 0)
		goto err;

	for (;;) {
		const size_t capacity =
			size + clamp_val(size, 0x1000, 0x100000);

		void * const d = realloc(data, capacity);
		if (!d)
			goto err;
		data = d;

		const ssize_t r = xread(fd, &data[size], capacity - size);
		if (!r) {
			data[size] = '\0';	/* Always NUL terminate */
			break;
		}
		if (r == -1)
			goto err;

		size += r;
	}

	if (xclose(fd) == -1) {
		fd = -1;
		goto err;
	} else
		fd = -1;

	void * const d = realloc(data, size + 1);	/* +1 for NUL */
	if (!d)
		goto err;

	return (struct file) {
		.path = p,
		.size = size,
		.data = d
	};

err:
	preserve (errno) {
		if (fd >= 0)
			xclose(fd);

		free(data);
		free(p);
	}

	return (struct file) { };
}

bool file_write(struct file f, int mode)
{
	int fd = xopen(f.path, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (fd < 0)
		goto err;

	const ssize_t w = xwrite(fd, f.data, f.size);
	if (w == -1)
		goto err;

	if (xclose(fd) == -1) {
		fd = -1;
		goto err;
	} else
		fd = -1;

	if (f.size != w) {
		errno = ENOSPC;
		goto err;
	}

	return true;

err:
	preserve (errno) {
		if (fd >= 0)
			xclose(fd);
	}

	return false;
}

void file_free(struct file f)
{
	free(f.path);
	free(f.data);
}

bool file_valid(struct file f)
{
	return f.path != NULL;
}

struct file file_copy(const char *path, struct file f)
{
	char * const p = strdup(path);
	void *d = NULL;

	if (!p)
		goto err;

	d = malloc(f.size);
	if (!d && f.size > 0)
		goto err;

	memcpy(d, f.data, f.size);

	return (struct file) {
		.path = p,
		.size = f.size,
		.data = d
	};

err:
	preserve (errno) {
		free(d);
		free(p);
	}

	return (struct file) { };
}

bool file_rename(const char *path, struct file *f)
{
	char * const p = strdup(path);

	if (!p)
		return false;

	free(f->path);
	f->path = p;

	return true;
}

int xopen(const char *path, int oflag, ...)
{
        mode_t mode = 0;
        va_list ap;

        va_start(ap, oflag);
        if (oflag & O_CREAT)
                mode = va_arg(ap, int);
        va_end(ap);

	do {
                const int fd = open(path, oflag, mode);

                if (fd >= 0)
                        return fd;
	} while (errno == EINTR);

	return -1;
}

int xclose(int fd)
{
	int err;

	do {
		err = close(fd);
	} while (err == EINTR);

	return err < 0 ? -1 : 0;
}

ssize_t xread(int fd, void *buf, size_t nbyte)
{
	u8 *data = buf;
	size_t size = 0;

	while (size < nbyte) {
		const ssize_t r = read(fd, &data[size], nbyte - size);

		if (r < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (!r)
			return size;

		size += r;
	}

	return size;
}

ssize_t xwrite(int fd, const void *buf, size_t nbyte)
{
	const u8 *data = buf;
	size_t size = 0;

	while (size < nbyte) {
		const ssize_t w = write(fd, &data[size], nbyte - size);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (!w)
			return size;

		size += w;
	}

	return size;
}

size_t basename_index(const char *filepath)
{
	size_t i = 0;

	for (size_t k = 0; filepath[k]; k++)
		if (filepath[k] == '/')
			i = k + 1;

	return i;
}

size_t baseextension_index(const char *filepath)
{
	size_t i;

	for (i = basename_index(filepath); filepath[i]; i++)
		if (filepath[i] == '.')
			break;

	return i;
}
