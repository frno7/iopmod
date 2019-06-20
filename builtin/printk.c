// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include "iopmod/printk.h"
#include "iopmod/sif.h"
#include "iopmod/sifcmd.h"
#include "iopmod/string.h"

int putk(const char *s)
{
	size_t length = strlen(s);

	int err = sif_cmd(SIF_CMD_PRINTK, s, length + 1);

	return err < 0 ? err : length;
}
