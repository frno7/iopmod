// SPDX-License-Identifier: GPL-2.0
/*
 * Kernel interrupt relay module.
 *
 * Copyright (C) 2019 Fredrik Noring
 */

#include "iopmod/errno.h"
#include "iopmod/interrupt.h"
#include "iopmod/iop-error.h"
#include "iopmod/irqs.h"
#include "iopmod/module.h"
#include "iopmod/printk.h"
#include "iopmod/sif.h"
#include "iopmod/sifcmd.h"
#include "iopmod/sifman.h"
#include "iopmod/thread.h"

#include "iopmod/asm/macro.h"

#define MAX_IRQ_RELAYS 16

/**
 * enum iop_irq_relay_rpc_ops - IOP IRQ relay RPC operations
 * @rpo_request_irq: request IRQ mapping
 * @rpo_release_irq: release IRQ mapping
 * @rpo_remap_irq: remap existing IRQ mapping
 */
enum iop_irq_relay_rpc_ops {
	rpo_request_irq = 1,
	rpo_release_irq = 2,
	rpo_remap_irq   = 3,
};

/**
 * struct iop_rpc_relay_map - IOP IRQ relay mapping
 * @u8 iop: IOP IRQ map source
 * @u8 map: main IRQ map target
 * @u8 rpc: %true for RPC relay, %false for SMFLAG relay
 */
struct iop_rpc_relay_map {
	u8 iop;
	u8 map;
	u8 rpc;
};

/**
 * struct iop_rpc_relay_release - IOP IRQ relay to release
 * @iop: IOP IRQ to release mapping for
 */
struct iop_rpc_relay_release {
	u8 iop;
};

/**
 * struct iop_irq_map - IRQ relay map
 * @set: IRQ is active if %true, otherwise unused
 * @iop: IRQ for the IOP
 * @map: IRQ to map to in main if @rpc is %true, otherwise bit in SMFLAG
 * @rpc: relay via RPC if %true, otherwise via SMFLAG
 */
struct iop_irq_map {
	u16 set : 1;
	u16 iop : 7;
	u16 map : 7;
	u16 rpc : 1;
};

static struct iop_irq_map irqs[MAX_IRQ_RELAYS];

static int rpc_stid;
static struct sifcmd_rpc_data_queue rpc_qdata;
static struct sifcmd_rpc_server_data rpc_sdata;
static u8 rpc_buffer[16] __attribute__((aligned(4)));

static enum irq_status service_irq(void *arg)
{
	struct iop_irq_map *m = arg;

	if (m->rpc) {
		const u32 irq = m->map;
		int err;

		err = sif_cmd(SIF_CMD_IRQ_RELAY, &irq, sizeof(irq));
		if (err < 0)
			pr_err("%s: sif_cmd failed with %d\n", __func__, err);
	} else {
		sifman_set_sm_flag(1 << m->map);
		sifman_intr_main();
	}

	return IRQ_HANDLED;
}

static struct iop_irq_map *find_map(unsigned int iop_irq)
{
	for (int i = 0; i < ARRAY_SIZE(irqs); i++)
		if (irqs[i].set && irqs[i].iop == iop_irq)
			return &irqs[i];

	return NULL;
}

static struct iop_irq_map *find_unset(unsigned int iop_irq)
{
	for (int i = 0; i < ARRAY_SIZE(irqs); i++)
		if (!irqs[i].set)
			return &irqs[i];

	return NULL;
}

static int request_map(unsigned int iop_irq, unsigned map_irq, bool rpc)
{
	const struct iop_irq_map w = {
		.set = true,
		.iop = iop_irq,
		.map = map_irq,
		.rpc = rpc,
	};
	unsigned int flags;
	int err = 0;

	pr_debug("%s: request iop irq %u for %u with %s\n",
		__func__, iop_irq, map_irq, rpc ? "RPC" : "SMFLAG");

	if (w.iop != iop_irq || w.map != map_irq)
		return -EINVAL;

	irq_save(flags);

	if (find_map(iop_irq)) {
		err = -EBUSY;
		goto out;
	}

	struct iop_irq_map *m = find_unset(iop_irq);
	if (!m) {
		err = -ENOMEM;
		goto out;
	}

	*m = w;

	err = request_irq(m->iop, service_irq, m);
	if (err < 0)
		*m = (struct iop_irq_map) { };

out:
	irq_restore(flags);

	return err;
}

static int remap(unsigned int iop_irq, unsigned map_irq, bool rpc)
{
	const struct iop_irq_map w = {
		.map = map_irq,
		.rpc = rpc,
	};
	unsigned int flags;
	int err = 0;

	pr_debug("%s: remap iop irq %u to %u with %s\n",
		__func__, iop_irq, map_irq, rpc ? "RPC" : "SMFLAG");

	if (w.map != map_irq)
		return -EINVAL;

	irq_save(flags);

	struct iop_irq_map *m = find_map(iop_irq);
	if (!m) {
		err = -ENXIO;
		goto out;
	}

	m->map = w.map;
	m->rpc = w.rpc;

out:
	irq_restore(flags);

	return err;
}

static int release_map(unsigned int iop_irq)
{
	unsigned int flags;
	int err = 0;

	pr_debug("%s: release %u\n", __func__, iop_irq);

	irq_save(flags);

	struct iop_irq_map *m = find_map(iop_irq);
	if (!m) {
		err = -ENXIO;
		goto out;
	}

	err = release_irq(iop_irq);
	if (!err)
		*m = (struct iop_irq_map) { };

out:
	irq_restore(flags);

	return err;
}

static void *irqrelay_service_rpc(int rpo, void *buffer, size_t size)
{
	static int status;

	switch (rpo) {
	case rpo_request_irq: {
		const struct iop_rpc_relay_map *request = buffer;

		if (size != sizeof(*request)) {
			status = -EINVAL;
			break;
		}

		status = request_map(request->iop, request->map, request->rpc);
		break;
	}

	case rpo_remap_irq: {
		const struct iop_rpc_relay_map *request = buffer;

		if (size != sizeof(*request)) {
			status = -EINVAL;
			break;
		}

		status = remap(request->iop, request->map, request->rpc);
		break;
	}

	case rpo_release_irq: {
		const struct iop_rpc_relay_release *request = buffer;

		if (size != sizeof(*request)) {
			status = -EINVAL;
			break;
		}

		status = release_map(request->iop);
		break;
	}

	default:
		pr_err("%s: Invalid RPC %d size %zu\n", __func__, rpo, size);
		status = -EINVAL;
	}

	return &status;
}

static void irqrelay_rpc_server(void *arg)
{
	pr_debug("%s: Calling sifcmd_rpc_loop\n", __func__);

	sifcmd_rpc_loop(&rpc_qdata);
}

static enum module_init_status irqrelay_init(int argc, char *argv[])
{
	int ioperr;

	const struct iop_thread th = {
		.attr = THREAD_ATTR_C,
		.thread = irqrelay_rpc_server,
		.stacksize = 1024,
		.priority = 0x27,	/* FIXME: What are reasonable priorities? */
	};

	rpc_stid = thbase_create(&th);
	if (rpc_stid < 0) {
		pr_err("%s: thbase_create failed with %d: %s\n",
			__func__, rpc_stid, iop_error_message(rpc_stid));
		goto err_create;
	}

	sifcmd_set_rpc_queue(&rpc_qdata, rpc_stid);
	sifcmd_register_rpc(&rpc_sdata, SIF_SID_IRQ_RELAY,
		irqrelay_service_rpc, rpc_buffer, NULL, NULL, &rpc_qdata);

	ioperr = thbase_start(rpc_stid, NULL);
	if (ioperr < 0) {
		pr_err("%s: thbase_start failed with %d: %s\n",
			__func__, ioperr, iop_error_message(ioperr));
		goto err_start;
	}

	pr_info("irqrelay: IRQ relay ready\n");

	return MODULE_RESIDENT;

err_start:
	sifcmd_remove_rpc(&rpc_sdata, &rpc_qdata);
	sifcmd_remove_rpc_queue(&rpc_qdata);
	thbase_delete(rpc_stid);

err_create:
	return MODULE_EXIT;
}
module_init(irqrelay_init);
