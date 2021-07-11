// SPDX-License-Identifier: GPL-2.0
/*
 * ATA
 *
 * Copyright (C) 2021 Fredrik Noring
 */

#include "iopmod/bits.h"
#include "iopmod/dev9.h"
#include "iopmod/errno.h"
#include "iopmod/io.h"
#include "iopmod/iop-error.h"
#include "iopmod/irq.h"
#include "iopmod/module.h"
#include "iopmod/printk.h"
#include "iopmod/spd.h"

static enum module_init_status ata_init(int argc, char *argv[])
{
	int err;

	err = dev9_request();
	if (err < 0) {
		pr_err("%s: dev9_request failed with %d\n", __func__, err);
		return MODULE_EXIT;
	}

	return MODULE_RESIDENT;
}
module_init(ata_init);
