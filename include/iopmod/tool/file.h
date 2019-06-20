// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#ifndef IOPMOD_TOOL_FILE_H
#define IOPMOD_TOOL_FILE_H

#include "iopmod/types.h"

/**
 * struct file - file container
 * @path: path of file
 * @size: size in bytes of file
 * @data: contents of file, always NUL terminated
 */
struct file {
	char * path;
	size_t size;
	void * data;
};

struct file file_read(const char *path);

bool file_write(struct file f, int mode);

void file_free(struct file f);

bool file_valid(struct file f);

bool file_rename(const char *path, struct file *f);

struct file file_copy(const char *path, struct file f);

int xopen(const char *path, int oflag, ...);

int xclose(int fd);

ssize_t xread(int fd, void *buf, size_t nbyte);

ssize_t xwrite(int fd, const void *buf, size_t nbyte);

size_t basename_index(const char *filepath);

size_t baseextension_index(const char *filepath);

#endif /* IOPMOD_TOOL_FILE_H */
