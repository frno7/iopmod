// SPDX-License-Identifier: GPL-2.0
/*
 * DEV9
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

/* FIXME: mutex */
static int dev9_use_count;

static void dev9_power_on()
{
	iowr32(0x51011,    SSBUS_REG_1420);
	iowr32(0xe01a3043, SSBUS_REG_1418);
	iowr32(0xef1a3043, SSBUS_REG_141c);

	if (!(DEV9_REG(DEV9_REG_POWER) & 0x04)) {
		pr_info("dev9: power is off\n");

		iowr16(1, DEV9_REG(DEV9_REG_1466));
		iowr16(0, DEV9_REG(DEV9_REG_1464));
		iowr16(iord16(DEV9_REG(DEV9_REG_1464)), DEV9_REG(DEV9_REG_1460));

		/* FIXME: Init Speed device */
	} else {
		pr_info("dev9: power is already on\n");
	}

	iowr16(0, DEV9_REG(DEV9_REG_1466));

	const u16 spdrev = iord16(SPD_REG(SPD_REG_REV_1));
	pr_info("dev9: spdrev %x\n", spdrev);

	pr_info("dev9: rev %x\n", iord8(DEV9_REG(DEV9_REG_REV)));
}

static void dev9_power_off()
{
	/* FIXME */
}

int dev9_request()
{
	int err = 0;

	if (!dev9_use_count)
		dev9_power_on();

	dev9_use_count++;

	return err;
}

int dev9_release()
{
	int err = 0;

	dev9_use_count--;

	if (!dev9_use_count)
		dev9_power_off();

	return err;
}

static enum module_init_status dev9_init(int argc, char *argv[])
{
	dev9_power_off();

	return MODULE_RESIDENT;
}
module_init(dev9_init);
