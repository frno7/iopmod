// SPDX-License-Identifier: GPL-2.0
/*
 * Dual Shock gamepad
 *
 * Copyright (C) 2020 Fredrik Noring
 */

#include "iopmod/errno.h"
#include "iopmod/interrupt.h"
#include "iopmod/io.h"
#include "iopmod/iop-error.h"
#include "iopmod/irq.h"
#include "iopmod/module.h"
#include "iopmod/printk.h"
#include "iopmod/sif.h"
#include "iopmod/sifcmd.h"
#include "iopmod/sifman.h"
#include "iopmod/sio2.h"
#include "iopmod/thread.h"

#include "iopmod/asm/macro.h"

#define SIO2_CTRL_SETTINGS						\
	.timeout_enable = true,						\
	.error_proceed = true,						\
	.unknown = true,						\
	.error_irq_enable = true,					\
	.tx_irq_enable = true

#define SIO2_REG(reg) { #reg, SIO2_REG_##reg }

struct digital_pad {
	struct {
		u8 select : 1;
		u8 : 2;
		u8 start : 1;
		u8 up : 1;
		u8 right : 1;
		u8 down : 1;
		u8 left : 1;
	};
	struct {
		u8 l2 : 1;
		u8 r2 : 1;
		u8 l1 : 1;
		u8 r1 : 1;
		u8 triangle : 1;
		u8 circle : 1;
		u8 cross : 1;
		u8 square : 1;
	};
};

struct analogue_pad {
	u8 rx;
	u8 ry;
	u8 lx;
	u8 ly;

	u8 right;
	u8 left;
	u8 up;
	u8 down;

	u8 triangle;
	u8 circle;
	u8 cross;
	u8 square;

	u8 l1;
	u8 r1;
	u8 l2;
	u8 r2;
};

union controller {
	struct digital_pad digital_pad;
	u8 byte[2];
};

static struct {
	enum {
		CONTROLLER_DISCOVER,
		CONTROLLER_ATTACHED,
	} state;
	struct {
		u32 us;
	} event;
	union controller controller;
} controller_state[2];

static int event_stid;
static int event_sema_id;

static enum irq_status sio2_irq(void *arg)
{
	thsemap_isignal_sema(event_sema_id);

	sio2_cl_irq_stat();

	return IRQ_HANDLED;
}

static void controller_tx(void)
{
	SIO2_WS_CMD(0, .port = 1, .cfg = 4, .tx_size = 5, .rx_size = 5);
	SIO2_WS_CMD(1, .port = 0, .cfg = 4, .tx_size = 5, .rx_size = 5);
	SIO2_WS_CMD(2,);

	sio2_wr_tx8(1);
	sio2_wr_tx8('B');
	sio2_wr_tx8(0);
	sio2_wr_tx8(0);
	sio2_wr_tx8(0);

	sio2_wr_tx8(1);
	sio2_wr_tx8('B');
	sio2_wr_tx8(0);
	sio2_wr_tx8(0);
	sio2_wr_tx8(0);

	SIO2_WS_CTRL(.start = true, SIO2_CTRL_SETTINGS);
}

static bool controller_rx_digital_pad(union controller *controller)
{
	sio2_rd_rx8();
	sio2_rd_rx8();
	sio2_rd_rx8();

	const u8 d0 = sio2_rd_rx8();
	const u8 d1 = sio2_rd_rx8();
	const bool change =
		d0 != controller->byte[0] ||
		d1 != controller->byte[1];

	controller->byte[0] = d0;
	controller->byte[1] = d1;

	return change;
}

static void controller_rx(void)
{
	for (int i = 0; i < ARRAY_SIZE(controller_state); i++)
		if (controller_rx_digital_pad(&controller_state[i].controller)) {
			const struct {
				u8 port, d0, d1;
			} packet = {
				i,
				controller_state[i].controller.byte[0],
				controller_state[i].controller.byte[1],
			};
			int err;

			err = sif_cmd(SIF_CMD_GAMEPAD, &packet, sizeof(packet));
			if (err < 0)
				pr_err("%s: sif_cmd failed with %d\n", __func__, err);
		}
}

static unsigned int event_alarm(void *arg)
{
	thsemap_isignal_sema(event_sema_id);

	return 0;
}

static void sio2_event(void *arg)
{
	bool schedule = true;

	for (;;) {
		thsemap_wait_sema(event_sema_id);

		if (schedule) {
			controller_tx();

			schedule = false;
		} else {
			struct iop_sys_clock next_event;

			controller_rx();

			thbase_us_to_sys_clock(5000, &next_event);
			thbase_set_alarm(&next_event, event_alarm, NULL);

			schedule = true;
		}
	}
}

static enum module_init_status gamepad_init(int argc, char *argv[])
{
	pr_info("gamepad: initialised\n");

	for (size_t i = 0; i < 256; i++) {
		iowr8(0, SIO2_MEM_FIFO_TX + i);
		iowr8(0, SIO2_MEM_FIFO_RX + i);
	}
	for (size_t i = 0; i < 16; i++)
		sio2_wr_cmd(i, 0);

	int err = request_irq(IRQ_IOP_SIO2, sio2_irq, NULL);
	if (err < 0) {
		pr_err("%s: request_irq for IRQ_IOP_SIO2 failed with %d\n", __func__, err);
		return MODULE_EXIT;
	}

	pr_info("gamepad: register\n");

	SIO2_WS_CTRL(.reset = true, .reset_fifo = true, SIO2_CTRL_SETTINGS);

	int ioperr;

	static const struct iop_thread event_th = {
		.attr = THREAD_ATTR_C,
		.thread = sio2_event,
		.stacksize = 1024,
		.priority = 0x26,	/* FIXME: What are reasonable priorities? */
	};

	event_stid = thbase_create(&event_th);
	if (event_stid < 0) {
		pr_err("%s: thbase_create failed with %d: %s\n",
			__func__, event_stid, iop_error_message(event_stid));
		goto err_event_create;
	}

	const struct iop_sema event_sema = { .initial = 1, .max = 1 };
	event_sema_id = thsemap_create_sema(&event_sema);
	if (event_sema_id < 0) {
		pr_err("%s: thsemap_create_sema failed with %d: %s\n",
			__func__, event_sema_id, iop_error_message(event_sema_id));
		goto err_sema_create;
	}

	ioperr = thbase_start(event_stid, NULL);
	if (ioperr < 0) {
		pr_err("%s: thbase_start failed with %d: %s\n",
			__func__, ioperr, iop_error_message(ioperr));
		goto err_event_start;
	}

	pr_info("sio2: ready\n");

	return MODULE_RESIDENT;

err_event_start:
	thsemap_delete_sema(event_sema_id);

err_sema_create:
	thbase_delete(event_stid);

err_event_create:
	return MODULE_EXIT;
}
module_init(gamepad_init);
