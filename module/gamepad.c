// SPDX-License-Identifier: GPL-2.0
/*
 * Dual Shock gamepad
 *
 * Copyright (C) 2020 Fredrik Noring
 */

#include "iopmod/compare.h"
#include "iopmod/errno.h"
#include "iopmod/gamepad.h"
#include "iopmod/interrupt.h"
#include "iopmod/io.h"
#include "iopmod/iop-error.h"
#include "iopmod/irq.h"
#include "iopmod/module.h"
#include "iopmod/printk.h"
#include "iopmod/sifcmd.h"
#include "iopmod/sif.h"
#include "iopmod/sifman.h"
#include "iopmod/sio2.h"
#include "iopmod/string.h"
#include "iopmod/struct.h"
#include "iopmod/thread.h"

#include "iopmod/asm/macro.h"

enum {				  /* Times in us */
	CONTROLLER_DISCOVERY_POLL =       500000, /*   2 Hz device discovery */
	CONTROLLER_QUERY_POLL     =       100000, /*  10 Hz device query     */
	CONTROLLER_ACTIVE_POLL    =         5000, /* 200 Hz device active    */
	CONTROLLER_PASSIVE_POLL   =       100000, /*  10 Hz device passive   */
	CONTROLLER_SCHEDULE_TIME  =         1000, /* Schedule at least 1 ms  */
	CONTROLLER_IDLE_TIME      =  180*1000000, /* Idle after 180 seconds   */
};

#define SIO2_CTRL_SETTINGS						\
	.timeout_enable = true,						\
	.error_proceed = true,						\
	.unknown = true,						\
	.error_irq_enable = true,					\
	.tx_irq_enable = true

#define SIO2_REG(reg) { #reg, SIO2_REG_##reg }

struct gamepad_clock { u64 t; };		/* Time in us */

struct gamepad_controller {
	struct gamepad_clock change;
	struct gamepad_cmd_rumble rumble;
	struct gamepad_controller_state state;
};

struct port_transition;

struct port_exchange {
	u16 tx_size;
	u16 rx_size;
	const u8 *tx_data;

	struct port_transition (*rx_fn)(
		struct gamepad_controller *ctrl,
		const struct gamepad_clock now,
		u32 rx_size, const u8 *rx_data);
};

struct port_transition {
	struct gamepad_clock at;

	struct port_exchange (*tx_fn)(
		struct gamepad_controller *ctrl,
		const struct gamepad_clock now);
};

struct gamepad_ports {
	struct gamepad_port {
		struct port_transition transition;
		struct port_exchange exchange;
		struct gamepad_controller ctrl;
		u8 rx_data[32];
		struct gamepad_tx_data {
			union {
				u8 raw[32];
				struct gamepad_tx_read_data {
					u8 byte[5];
				} read_data;
			};
		} tx_data;
	} p[GAMEPAD_PORTS];
};

struct event_state {
	int stid;
	int clk_sema_id;
	int sio2_irq_sema_id;
	struct gamepad_ports ps;
};

static enum irq_status sio2_irq(void *arg)
{
	struct event_state *ev = arg;

	thsemap_isignal_sema(ev->sio2_irq_sema_id);

	sio2_cl_irq_stat();

	return IRQ_HANDLED;
}

static inline int port_id(const struct gamepad_controller *ctrl)
{
	return 1 + ctrl->state.port.index;
}

static void info_activity(const struct gamepad_controller *ctrl, bool active)
{
	static int last_activity[GAMEPAD_PORTS] = { -1, -1 };

	const int i = ctrl->state.port.index;

	if (last_activity[i] == (int)active)
		return;

	last_activity[i] = active;

	pr_debug("gamepad: Controller %d %s\n",
		 port_id(ctrl),
		 active ? "active" : "passive");
}

#define DECLARE_TX_FN(tx_fn_)						\
	static struct port_exchange gamepad_tx_ ## tx_fn_(		\
		struct gamepad_controller *ctrl,			\
		const struct gamepad_clock now)

#define DECLARE_RX_FN(rx_fn_)						\
	static struct port_transition gamepad_rx_ ## rx_fn_(		\
		struct gamepad_controller *ctrl,			\
		const struct gamepad_clock now,				\
		u32 rx_size, const u8 *rx_data)

#define DECLARE_TX_RX_FN(fn_)						\
	DECLARE_TX_FN(fn_);						\
	DECLARE_RX_FN(fn_)

DECLARE_TX_RX_FN(enter_config_mode);
DECLARE_TX_RX_FN(exit_config_mode);
DECLARE_TX_RX_FN(query_model);
DECLARE_TX_RX_FN(actuator_align);
DECLARE_TX_RX_FN(read_data);

#define TRANSITION_AT(dt) ((struct gamepad_clock) { .t = (now).t + (dt) })

#define EXPECT_RX_DATA(...)						\
	({								\
		static const u8 expect[] = { __VA_ARGS__ };		\
									\
		for (size_t i = 0; i < ARRAY_SIZE(expect); i++)		\
			if (rx_data[i] != expect[i])			\
				return gamepad_rx_error(ctrl, now); \
	})

static void sif_cmd_gamepad(const struct gamepad_controller *ctrl)
{
	int err = sif_cmd(SIF_CMD_GAMEPAD,
		&ctrl->state, sizeof(ctrl->state));

	if (err < 0)
		pr_err("%s: sif_cmd failed with %d\n", __func__, err);
}

static struct port_transition gamepad_rx_error(
	struct gamepad_controller *ctrl, const struct gamepad_clock now)
{
	if (ctrl->state.device.mode) {
		pr_debug("gamepad: Controller %d disconnected\n",
			 port_id(ctrl));

		ctrl->state.device.mode = 0;  /* Indicate disconnected */

		sif_cmd_gamepad(ctrl);
	}

	*ctrl = (struct gamepad_controller)
		{ .state = { .port = { .index = ctrl->state.port.index } } };

	return (struct port_transition) {
		.at = TRANSITION_AT(CONTROLLER_DISCOVERY_POLL),
		.tx_fn = gamepad_tx_enter_config_mode,
	};
}

static struct port_exchange gamepad_tx_enter_config_mode(
	struct gamepad_controller *ctrl, const struct gamepad_clock now)
{
	static const u8 data[] = { 1, 'C', 0, 1, 0 };

	return (struct port_exchange) {
		.tx_size = ARRAY_SIZE(data),
		.rx_size = ARRAY_SIZE(data),
		.tx_data = data,
		.rx_fn = gamepad_rx_enter_config_mode,
	};
}

static bool gamepad_device_mode_is_digital(
	const struct gamepad_controller *ctrl)
{
	return ctrl->state.device.mode == 0x41;
}

static bool gamepad_device_mode_is_analog(
	const struct gamepad_controller *ctrl)
{
	return ctrl->state.device.mode == 0x73 ||
	       ctrl->state.device.mode == 0x79;
}

static bool gamepad_device_is_dualshock(
	const struct gamepad_controller *ctrl)
{
	return (gamepad_device_mode_is_digital(ctrl) ||
		gamepad_device_mode_is_analog(ctrl)) &&
	       ctrl->state.model.type == 1 &&
	       ctrl->state.model.modes == 2 &&
	       ctrl->state.model.actuators == 2;
}

static bool gamepad_device_is_dualshock2(
	const struct gamepad_controller *ctrl)
{
	return (gamepad_device_mode_is_digital(ctrl) ||
		gamepad_device_mode_is_analog(ctrl)) &&
	       ctrl->state.model.type == 3 &&
	       ctrl->state.model.modes == 2 &&
	       ctrl->state.model.actuators == 2;
}

static enum gamepad_device_id gamepad_device_id(
	const struct gamepad_controller *ctrl)
{
	return gamepad_device_is_dualshock(ctrl)  ? GAMEPAD_DEVICE_DUALSHOCK  :
	       gamepad_device_is_dualshock2(ctrl) ? GAMEPAD_DEVICE_DUALSHOCK2 :
						    GAMEPAD_DEVICE_UNDEFINED;
}

static const char *gamepad_device_name(const struct gamepad_controller *ctrl)
{
	switch (gamepad_device_id(ctrl)) {
	default:
#define GAMEPAD_DEVICE_CASE(id, name)					\
	case GAMEPAD_DEVICE_ ## id: return name;
GAMEPAD_DEVICES(GAMEPAD_DEVICE_CASE)
	}
}

static const char *gamepad_device_mode_name(int m)
{
	static const struct {
		int mode;
		const char *name;
	} modes[] = {
		{ 0x12, "mouse"     },
		{ 0x41, "digital"   },
		{ 0x73, "analog"    },
		{ 0x79, "analog"    },
		{ 0x23, "negicon"   },
		{ 0x53, "joystick"  },
		{ 0x31, "konamigun" },
		{ 0x63, "namcogun"  },
		{ 0xe3, "jogcon"    },
		{ 0xe5, "jogcon"    },
		{ 0x80, "multitap"  },
		{ 0xf3, "config"    },
	};

	for (size_t i = 0; i < ARRAY_SIZE(modes); i++)
		if (modes[i].mode == m)
			return modes[i].name;

	return "undefined";
}

static struct port_transition gamepad_rx_enter_config_mode(
	struct gamepad_controller *ctrl, const struct gamepad_clock now,
	u32 rx_size, const u8 *rx_data)
{
	if (rx_data[0] == 0xff &&
	    rx_data[1] == 0xff &&
	    rx_data[2] == 0xff)		/* Disconnected */
		return (struct port_transition) {
			.at = TRANSITION_AT(CONTROLLER_DISCOVERY_POLL),
			.tx_fn = gamepad_tx_enter_config_mode,
		};

	if (rx_data[0] != 0xff || (rx_data[2] != 'Z' &&
				   rx_data[2] != 0)) {
		/* Does not respond to config mode */
		pr_warn("gamepad: Controller %d malformed config enter: %02x %02x %02x\n",
			port_id(ctrl), rx_data[0], rx_data[1], rx_data[2]);

		return (struct port_transition) {
			.at = TRANSITION_AT(CONTROLLER_QUERY_POLL),
			.tx_fn = gamepad_tx_read_data,
		};
	}

	ctrl->state.device.mode = rx_data[1];

	pr_debug("gamepad: Controller %d enters config mode with device mode %s (%02x)\n",
		 port_id(ctrl),
		 gamepad_device_mode_name(ctrl->state.device.mode),
		 ctrl->state.device.mode);

	return (struct port_transition) {
		.at = TRANSITION_AT(CONTROLLER_QUERY_POLL),
		.tx_fn = gamepad_tx_query_model,
	};
}

static struct port_exchange gamepad_tx_exit_config_mode(
	struct gamepad_controller *ctrl, const struct gamepad_clock now)
{
	static const u8 data[] = { 1, 'C', 0, 0, 'Z', 'Z', 'Z', 'Z', 'Z' };

	return (struct port_exchange) {
		.tx_size = ARRAY_SIZE(data),
		.rx_size = ARRAY_SIZE(data),
		.tx_data = data,
		.rx_fn = gamepad_rx_exit_config_mode,
	};
}

static struct port_transition gamepad_rx_exit_config_mode(
	struct gamepad_controller *ctrl, const struct gamepad_clock now,
	u32 rx_size, const u8 *rx_data)
{
	if (rx_data[0] != 0xff ||
	    rx_data[1] != 0xf3 ||
	    rx_data[2] != 'Z') {
		pr_warn("gamepad: Controller %d malformed config exit: "
			"%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			port_id(ctrl),
			rx_data[0], rx_data[1], rx_data[2],
			rx_data[3], rx_data[4], rx_data[5],
			rx_data[6], rx_data[7], rx_data[8]);
	}

	pr_debug("gamepad: Controller %d exits config mode\n", port_id(ctrl));

	return (struct port_transition) {
		.at = TRANSITION_AT(CONTROLLER_QUERY_POLL),
		.tx_fn = gamepad_tx_read_data,
	};
}

static struct port_exchange gamepad_tx_query_model(
	struct gamepad_controller *ctrl, const struct gamepad_clock now)
{
	static const u8 data[] = { 1, 'E', 0, 'Z', 'Z', 'Z', 'Z', 'Z', 'Z' };

	return (struct port_exchange) {
		.tx_size = ARRAY_SIZE(data),
		.rx_size = ARRAY_SIZE(data),
		.tx_data = data,
		.rx_fn = gamepad_rx_query_model,
	};
}

static struct port_transition gamepad_rx_query_model(
	struct gamepad_controller *ctrl, const struct gamepad_clock now,
	u32 rx_size, const u8 *rx_data)
{
	if (rx_data[0] != 0xff ||
	    rx_data[1] != 0xf3 ||
	    rx_data[2] != 'Z')
		pr_warn("gamepad: Controller %d unknown model: "
			"%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			port_id(ctrl),
			rx_data[0], rx_data[1], rx_data[2],
			rx_data[3], rx_data[4], rx_data[5],
			rx_data[6], rx_data[7], rx_data[8]);

	ctrl->state.model = (struct gamepad_controller_model) {
		.type      = rx_data[3],
		.modes     = rx_data[4],
		.mode      = rx_data[5],
		.actuators = rx_data[6],
	};

	pr_debug("gamepad: Controller %d as %s "
		 "model type %02x modes %d mode %02x actuators %d %02x %02x\n",
		 port_id(ctrl), gamepad_device_name(ctrl),
		 ctrl->state.model.type,
		 ctrl->state.model.modes,
		 ctrl->state.model.mode,
		 ctrl->state.model.actuators,
		 rx_data[7],
		 rx_data[8]);

	if (ctrl->state.model.actuators >= 2)
		return (struct port_transition) {
			.at = TRANSITION_AT(CONTROLLER_QUERY_POLL),
			.tx_fn = gamepad_tx_actuator_align,
		};

	return (struct port_transition) {
		.at = TRANSITION_AT(CONTROLLER_QUERY_POLL),
		.tx_fn = gamepad_tx_exit_config_mode,
	};
}

static struct port_exchange gamepad_tx_actuator_align(
	struct gamepad_controller *ctrl, const struct gamepad_clock now)
{
	static const u8 data[] = { 1, 'M', 0, 0, 1, 0xff, 0xff, 0xff, 0xff };

	return (struct port_exchange) {
		.tx_size = ARRAY_SIZE(data),
		.rx_size = ARRAY_SIZE(data),
		.tx_data = data,
		.rx_fn = gamepad_rx_actuator_align,
	};
}

static struct port_transition gamepad_rx_actuator_align(
	struct gamepad_controller *ctrl, const struct gamepad_clock now,
	u32 rx_size, const u8 *rx_data)
{
	if (rx_data[0] != 0xff ||
	    rx_data[1] != 0xf3 ||
	    rx_data[2] != 'Z')
		pr_warn("gamepad: Controller %d unexpected actuator align: "
			"%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			port_id(ctrl),
			rx_data[0], rx_data[1], rx_data[2],
			rx_data[3], rx_data[4], rx_data[5],
			rx_data[6], rx_data[7], rx_data[8]);

	return (struct port_transition) {
		.at = TRANSITION_AT(CONTROLLER_QUERY_POLL),
		.tx_fn = gamepad_tx_exit_config_mode,
	};
}

static inline bool device_is_digital(const struct gamepad_controller *ctrl)
{
	return ctrl->state.device.mode == 0x41;
}

static inline bool device_is_analog(const struct gamepad_controller *ctrl)
{
	return !device_is_digital(ctrl);
}

static struct port_exchange gamepad_tx_read_data(
	struct gamepad_controller *ctrl, const struct gamepad_clock now)
{
	struct gamepad_port *port =
		container_of(ctrl, struct gamepad_port, ctrl);

	port->tx_data.read_data = (struct gamepad_tx_read_data) { {
		1, 'B', 0,
		ctrl->rumble.small,
		ctrl->rumble.large,
	} };

	return (struct port_exchange) {
		.tx_size = sizeof(port->tx_data.read_data),
		.rx_size = device_is_digital(ctrl) ? 5 : 21,
		.tx_data = port->tx_data.read_data.byte,
		.rx_fn = gamepad_rx_read_data,
	};
}

static bool controller_change(const struct gamepad_controller *ctrl,
	u32 rx_size, const u8 *rx_data)
{
	if (rx_data[3] != ctrl->state.digital.byte[0] ||
	    rx_data[4] != ctrl->state.digital.byte[1])
		return true;

	if (device_is_analog(ctrl) && rx_size == 21)
		return memcmp(&rx_data[5], ctrl->state.analog.byte,
			sizeof(ctrl->state.analog));

	return false;
}

static struct port_transition gamepad_rx_read_data(
	struct gamepad_controller *ctrl, const struct gamepad_clock now,
	u32 rx_size, const u8 *rx_data)
{
	if (rx_data[0] != 0xff || (rx_data[2] != 'Z' && rx_data[2] != 0))
		return gamepad_rx_error(ctrl, now);

	if (ctrl->state.device.mode != rx_data[1]) {
		ctrl->state.device.mode = rx_data[1];

		pr_debug("gamepad: Controller %d device mode change %s (%02x)\n",
			 port_id(ctrl),
			 gamepad_device_mode_name(ctrl->state.device.mode),
			 ctrl->state.device.mode);

		/*
		 * Enter config mode after device mode change,
		 * to realign (rumble) actuators, and so on.
		 */
		return (struct port_transition) {
			.at = TRANSITION_AT(CONTROLLER_QUERY_POLL),
			.tx_fn = gamepad_tx_enter_config_mode,
		};
	}

	const bool change = controller_change(ctrl, rx_size, rx_data);
	const bool active = change ||
		now.t < ctrl->change.t + CONTROLLER_IDLE_TIME;

	info_activity(ctrl, active);

	if (!change)
		goto out;

	ctrl->state.digital.byte[0] = rx_data[3];
	ctrl->state.digital.byte[1] = rx_data[4];

	if (device_is_analog(ctrl) && rx_size == 21)
		memcpy(ctrl->state.analog.byte, &rx_data[5],
			sizeof(ctrl->state.analog));

	ctrl->change = now;

	sif_cmd_gamepad(ctrl);

out:
	return (struct port_transition) {
		.at = TRANSITION_AT(active ?
			CONTROLLER_ACTIVE_POLL : CONTROLLER_PASSIVE_POLL),
		.tx_fn = gamepad_tx_read_data,
	};
}

#define for_each_port(i, ps, now)					\
	for (int i = 0; i < ARRAY_SIZE((ps)->p); i++)			\
		if ((now).t < (ps)->p[i].transition.at.t) ; else

static void sio2_exchange(struct gamepad_ports *ps,
	const struct gamepad_clock now, struct event_state *ev)
{
	int n = 0;

	for_each_port (i, ps, now)
		if (ps->p[i].exchange.tx_size)
			SIO2_WS_CMD(n++,
				.port = i,
				.cfg = 4,
				.tx_size = ps->p[i].exchange.tx_size,
				.rx_size = ps->p[i].exchange.rx_size);

	if (!n)
		return;

	SIO2_WS_CMD(n,);

	for_each_port (i, ps, now)
		for (int k = 0; k < ps->p[i].exchange.tx_size; k++)
			sio2_wr_tx8(ps->p[i].exchange.tx_data[k]);

	SIO2_WS_CTRL(.start = true, SIO2_CTRL_SETTINGS);

	thsemap_wait_sema(ev->sio2_irq_sema_id);

	for_each_port (i, ps, now)
		for (int k = 0; k < ps->p[i].exchange.rx_size; k++)
			ps->p[i].rx_data[k] = sio2_rd_rx8();
}

static void controller_exchange(struct gamepad_ports *ps,
	const struct gamepad_clock now, struct event_state *ev)
{
	for_each_port (i, ps, now)
		ps->p[i].exchange = ps->p[i].transition.tx_fn(
			&ps->p[i].ctrl, now);

	sio2_exchange(ps, now, ev);

	for_each_port (i, ps, now)
		ps->p[i].transition = ps->p[i].exchange.rx_fn(
			&ps->p[i].ctrl, now,
			ps->p[i].exchange.rx_size, ps->p[i].rx_data);
}

static unsigned int event_alarm(void *arg)
{
	struct event_state *ev = arg;

	thsemap_isignal_sema(ev->clk_sema_id);

	return 0;
}

static struct gamepad_clock read_clock(void)
{
	struct iop_sys_clock sys_clock = { };

	int err = thbase_get_system_time(&sys_clock);
	if (err < 0) {
		pr_err("%s: thbase_get_system_time failed with %d: %s\n",
			__func__, err, iop_error_message(err));

		return (struct gamepad_clock) { };
	}

	u32 s = 0;
	u32 us = 0;

	thbase_sys_clock_to_us(&sys_clock, &s, &us);

	const u64 t = (u64)s * 1000000 + us;

	if (!t)
		pr_err("iop: %s: Zero time\n", __func__);

	return (struct gamepad_clock) { .t = t };
}

static void schedule_event(struct gamepad_ports *ps,
	const struct gamepad_clock now, struct event_state *ev)
{
	u64 t = 0;
	for (int i = 0; i < ARRAY_SIZE(ps->p); i++)
		if (!t || t > ps->p[i].transition.at.t)
			t = ps->p[i].transition.at.t;

	const struct gamepad_clock sch = read_clock();
	if (!sch.t)
		return;

	const u64 d = max(sch.t + CONTROLLER_SCHEDULE_TIME, t) - sch.t;

	struct iop_sys_clock next_event;
	thbase_us_to_sys_clock(d, &next_event);
	thbase_set_alarm(&next_event, event_alarm, ev);
}

static void gamepad_sif_cmd(const struct sif_cmd_header *header, void *arg)
{
	const struct gamepad_sif_opt opt = { .raw = header->opt };
	struct gamepad_ports *ps = arg;

	switch (opt.op)
	{
	case gamepad_rop_rumble: {
		const struct gamepad_cmd_rumble rumble = { .raw = opt.data };

		ps->p[rumble.index].ctrl.rumble = rumble;

		pr_debug("rumble index %d small %d large %d\n",
			 rumble.index,
			 rumble.small,
			 rumble.large);
		break;
	}
	default:
		pr_err("%s: Unknown op %d\n", __func__, opt.op);
	}
}

static void gamepad_event(void *arg)
{
	struct event_state *ev = arg;

	for (int i = 0; i < GAMEPAD_PORTS; i++) {
		ev->ps.p[i].ctrl.state.port.index = i;
		ev->ps.p[i].transition.tx_fn = gamepad_tx_enter_config_mode;
	}

	for (;;) {
		thsemap_wait_sema(ev->clk_sema_id);

		const struct gamepad_clock now = read_clock();
		if (!now.t)
			continue;

		controller_exchange(&ev->ps, now, ev);

		schedule_event(&ev->ps, now, ev);
	}
}

static bool gamepad_create_sema(int *sema_id, u32 initial)
{
	const struct iop_sema sema = { .initial = initial, .max = 1 };

	*sema_id = thsemap_create_sema(&sema);
	if (*sema_id >= 0)
		return true;

	pr_err("%s: thsemap_create_sema failed with %d: %s\n",
		__func__, *sema_id, iop_error_message(*sema_id));

	return false;
}

static bool gamepad_init_sio2(struct event_state *ev)
{
	for (size_t i = 0; i < 256; i++) {
		iowr8(0, SIO2_MEM_FIFO_TX + i);
		iowr8(0, SIO2_MEM_FIFO_RX + i);
	}
	for (size_t i = 0; i < 16; i++)
		sio2_wr_cmd(i, 0);

	if (!gamepad_create_sema(&ev->sio2_irq_sema_id, 0))
		return false;

	int err = request_irq(IRQ_IOP_SIO2, sio2_irq, ev);
	if (err < 0) {
		pr_err("%s: request_irq for IRQ_IOP_SIO2 failed with %d\n",
			__func__, err);

		goto err_request_irq_sio2;
	}

	SIO2_WS_CTRL(.reset = true, .reset_fifo = true, SIO2_CTRL_SETTINGS);

	return true;

err_request_irq_sio2:
	thsemap_delete_sema(ev->sio2_irq_sema_id);

	return false;
}

static bool gamepad_init_event_thread(struct event_state *ev)
{
	static const struct iop_thread et = {
		.attr = THREAD_ATTR_C,
		.thread = gamepad_event,
		.stacksize = 1024,
		.priority = 0x26,  /* FIXME: What are reasonable priorities? */
	};

	BUILD_BUG_ON(sizeof(struct gamepad_digital_pad) != 2);
	BUILD_BUG_ON(sizeof(struct gamepad_analog_pad) != 16);

	BUILD_BUG_ON(sizeof(struct gamepad_sif_opt) != sizeof(u32));
	BUILD_BUG_ON(sizeof(struct gamepad_cmd_rumble) != sizeof(u32));

	ev->stid = thbase_create(&et);
	if (ev->stid < 0) {
		pr_err("%s: thbase_create failed with %d: %s\n",
			__func__, ev->stid, iop_error_message(ev->stid));
		goto err_event_create;
	}

	if (!gamepad_create_sema(&ev->clk_sema_id, 1))
		goto err_event_sema_create;

	int ioperr = thbase_start(ev->stid, ev);
	if (ioperr < 0) {
		pr_err("%s: thbase_start failed with %d: %s\n",
			__func__, ioperr, iop_error_message(ioperr));
		goto err_event_start;
	}

	return true;

err_event_start:
	thsemap_delete_sema(ev->clk_sema_id);

err_event_sema_create:
	thbase_delete(ev->stid);

err_event_create:
	return false;
}

static enum module_init_status gamepad_init(int argc, char *argv[])
{
	static struct event_state ev = { };

	pr_debug("gamepad: Initialise\n");

	if (!gamepad_init_sio2(&ev))
		return MODULE_EXIT;

	if (!gamepad_init_event_thread(&ev))
		goto err_event_thread;

	sif_request_cmd(SIF_CMD_GAMEPAD, gamepad_sif_cmd, &ev.ps);

	pr_debug("gamepad: Ready\n");

	return MODULE_RESIDENT;

err_event_thread:
	release_irq(IRQ_IOP_SIO2);
	thsemap_delete_sema(ev.sio2_irq_sema_id);

	return MODULE_EXIT;
}
module_init(gamepad_init);
