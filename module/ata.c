// SPDX-License-Identifier: GPL-2.0
/*
 * ATA
 *
 * Copyright (C) 2021 Fredrik Noring
 */

#include <string.h>
#include <sys/types.h>

#include "iopmod/bits.h"
#include "iopmod/build-bug.h"
#include "iopmod/dev9.h"
#include "iopmod/compare.h"
#include "iopmod/errno.h"
#include "iopmod/interrupt.h"
#include "iopmod/io.h"
#include "iopmod/iop-error.h"
#include "iopmod/irq.h"
#include "iopmod/module.h"
#include "iopmod/printk.h"
#include "iopmod/processor.h"
#include "iopmod/sif.h"
#include "iopmod/sifcmd.h"
#include "iopmod/sifman.h"
#include "iopmod/spd.h"
#include "iopmod/thread.h"

#include "iopmod/asm/macro.h"

#define MAX_ATA_SIF_SG (CMD_PACKET_PAYLOAD_MAX / sizeof(struct ata_sif_sg_entry))

/**
 * enum iop_ata_ops - IOP ATA remote operations
 * @rop_bb: Announce bounce buffer for unaligned addresses and sizes
 * @rop_sg: Request scatter-gather transfers
 * @rop_rd: Read request
 * @rop_wr: Write request
 */
enum iop_ata_rops {
	rop_bb  = 0,
	rop_sg  = 1,
	rop_rd  = 2,
	rop_wr  = 3,
};

union ata_sif_opt {
	u32 raw;
	struct {
		u32 op : 3;
		u32 count : 8;
		u32 write : 1;
		u32 : 20;
	};
};

struct ata_sif_bb {
	u32 addr;
	u32 size;
};

struct ata_sif_sg {
	struct ata_sif_sg_entry {
		u32 addr;
		u32 size;
	} entry[MAX_ATA_SIF_SG];
};

struct ata_sif_rd {
	u32 src;
	u32 dst;
	u32 size;
};

enum ata_dir {
	ata_dir_undetermined = -1,
	ata_dir_read = 0,
	ata_dir_write = 1,
};

struct ata_dev {
	u8 buffer[16384] __attribute((aligned(64)));

	union ata_sif_opt opt;
	struct ata_sif_sg sg;
	u32 index;

	enum ata_dir dir;

	struct ata_sif_bb bb;

	int sg_stid;
	int sg_sema_id;
};

static void ata_direction(struct ata_dev *dev, const bool write)
{
	if (dev->dir == write)
		return;

	dev->dir = write;

	/* FIXME: 0x38? It also holds the number of blocks ready for DMA. */
	iowr16(3, SPD_REG(SPD_REG_0x38));

	const u16 udma = iord16(SPD_REG(SPD_REG_IF_CTRL)) & 1; /* 0=MWDMA, 1=UDMA */
	const u16 ctrl = udma | (write ? 0x4c : 0x4e);

	iowr16(ctrl, SPD_REG(SPD_REG_IF_CTRL));
	iowr16(write | 0x6, SPD_REG(SPD_REG_XFR_CTRL));
}

static ssize_t ata_write(struct ata_dev *dev, u32 addr, size_t size)
{
	/* FIXME: Write from main: sif_cmd(SIF_CMD_ATA, ...); */

	/* FIXME: Write to disk */

	return size;
}

/* CHCR flags */
#define DMAC_CHCR_30 (1<<30)
/** TRansfer */
#define DMAC_CHCR_TR (1<<24)
/** LInked list (GPU, SPU and SIF0) */
#define DMAC_CHCR_LI (1<<10)
/** COntinuous (?) */
#define DMAC_CHCR_CO (1<<9)
#define DMAC_CHCR_08 (1<<8)
/** DiRection; 0 = to RAM, 1 = from RAM */
#define DMAC_CHCR_DR (1<<0)

#define DMAC_TO_MEM     0
#define DMAC_FROM_MEM   1

static size_t max_buf_size(struct ata_dev *dev, size_t size)
{
	return min(size, sizeof(dev->buffer));
}

static size_t max_bb_size(struct ata_dev *dev, size_t size)
{
	return min(max_buf_size(dev, size), dev->bb.size);
}

static void wait_for_ata_dma()
{
	/* FIXME: Sleep waiting for a DMA completion interrupt here. */

	for (;;) {
		if ((iord32(DEV9_DMAC_BCR) >> 16) <= 0)
			break;

		cpu_relax();
	}

	iowr16(iord16(SPD_REG(SPD_REG_XFR_CTRL)) & ~0x80, SPD_REG(SPD_REG_XFR_CTRL));
}

static ssize_t ata_sif_cmd_rd_bounce(struct ata_dev *dev, u32 addr, size_t size)
{
	size_t offset = 0;

	while (offset < size) {
		const size_t wanted_size = max_bb_size(dev, size - offset);
		const struct ata_sif_rd rd = {
			.src = dev->bb.addr,
			.dst = addr + offset,
			.size = wanted_size,
		};

		const uint8_t *buf = dev->buffer;
		int err = sif_cmd_opt_data(SIF_CMD_ATA,
			(union ata_sif_opt) { .op = rop_rd }.raw,
			&rd, sizeof(rd), dev->bb.addr, &buf[offset],
			ALIGN(wanted_size, 16));

		if (err < 0) {
			pr_err("%s: sif_cmd_opt_data failed with %d\n", __func__, err);
			return err;
		}

		offset += wanted_size;
	}

	return size;
}

static ssize_t ata_sif_cmd_rd_direct(struct ata_dev *dev, u32 addr, size_t size)
{
	/* FIXME: Skip read completions, for performance. */

	const struct ata_sif_rd rd = { };

	int err = sif_cmd_opt_data(SIF_CMD_ATA,
		(union ata_sif_opt) { .op = rop_rd }.raw,
		&rd, sizeof(rd), addr, dev->buffer, size);

	if (err < 0) {
		pr_err("%s: sif_cmd_opt_data failed with %d\n", __func__, err);
		return err;
	}

#if 0
	const struct sif_dma_transfer dma_tr = {
		.src = dev->buffer,
		.dst = addr,
		.nbytes = size,
	};

	unsigned int flags;
	int dma_id;
	do {
		irq_save(flags);
		dma_id = sifman_set_dma(&dma_tr, 1);
		irq_restore(flags);
	} while (!dma_id); // FIXME: Restart properly on DMA failure

	sif_dma_relax_for_completion(dma_id);
#endif

	return size;
}

static ssize_t ata_sif_cmd_rd(struct ata_dev *dev, u32 addr, size_t size)
{
	return ata_sif_cmd_rd_bounce(dev, addr, size);	// FIXME

	/*
	 * Both address and size must be multiples of 16 bytes for
	 * direct DMA transfers over the SIF.
	 */
	return ALIGNED(addr, 16) && ALIGNED(size, 16) ?
		ata_sif_cmd_rd_direct(dev, addr, size) :
		ata_sif_cmd_rd_bounce(dev, addr, size);
}

static void ata_read_dma(void *buf, size_t size)
{
	const u16 channel = 0;
	u16 dmactrl =
		channel == 0 || channel == 1 ?      channel :
		channel == 2 || channel == 3 ? 4 << channel : -1;
	dmactrl = (iord16(SPD_REG(SPD_REG_REV_1)) < 17) ?
		(dmactrl & 0x03) | 0x04 : (dmactrl & 0x01) | 0x06;

	const u32 dir = DMAC_TO_MEM;
	const u32 block_shift = (size & 0x7f) ? 3 : 7;
	const u32 block_size4 = (1 << block_shift) / 4;

	const u32 maddr = (u32)buf;
	const u32 bcr = (ALIGN(size, 8) >> block_shift) << 16 | block_size4;
	const u32 chcr = DMAC_CHCR_30 |
			 DMAC_CHCR_TR |
			 DMAC_CHCR_CO | (dir & DMAC_CHCR_DR);

	iowr16(dmactrl, SPD_REG(SPD_REG_DMA_CTRL));
	iowr16(iord16(SPD_REG(SPD_REG_XFR_CTRL)) | 0x80, SPD_REG(SPD_REG_XFR_CTRL));

	iowr32(maddr, DEV9_DMAC_MADR);
	iowr32(bcr,   DEV9_DMAC_BCR);
	iowr32(chcr,  DEV9_DMAC_CHCR);

	wait_for_ata_dma();

	/* FIXME: Handle errors. */
}

static ssize_t ata_read(struct ata_dev *dev, u32 addr, size_t size)
{
	ata_read_dma(dev->buffer, size);

	return ata_sif_cmd_rd(dev, addr, size);
}

static void ata_sif_cmd_bb(struct ata_dev *dev,
	const union ata_sif_opt opt, const struct ata_sif_bb *bb)
{
	/*
	 * A bounce buffer has been announced. It will be used for addresses
	 * and sizes that are nonmultiples of 16 bytes, in which cases data
	 * will be transferred to the final destination by memcpy. SIF DMA
	 * requires 16 byte alignments for both.
	 */
	dev->bb = *bb;

	pr_info("%s: bounce buffer size %u bytes\n", __func__, bb->size);
}

static void ata_sif_cmd_sg_ack(struct ata_dev *dev)
{
	int err = sif_cmd_opt(SIF_CMD_ATA,
		(union ata_sif_opt) { .op = rop_sg }.raw,
		NULL, 0);

	if (err < 0)
		pr_err("%s: sif_cmd_opt failed with %d\n", __func__, err);
}

static void ata_sif_cmd_sg_transfer(struct ata_dev *dev)
{
	while (dev->index < dev->opt.count) {
		struct ata_sif_sg_entry *e = &dev->sg.entry[dev->index];

		if (!e->size) {
			dev->index++;
			continue;
		}

		const size_t wanted_size = max_buf_size(dev, e->size);

		const ssize_t actual_size =
			(dev->opt.write ? ata_write : ata_read)
			(dev, e->addr, wanted_size);

		if (actual_size < 0) {
			dev->index = dev->opt.count;	/* Skip on error. */

			// FIXME: Error handling
		}

		e->addr += actual_size;
		e->size -= actual_size;
	}

	/* Acknowledge that the list of transfers has been processed. */
	ata_sif_cmd_sg_ack(dev);
}

static void ata_sif_cmd_sg(struct ata_dev *dev,
	const union ata_sif_opt opt, const struct ata_sif_sg *sg)
{
	ata_direction(dev, opt.write);

	dev->opt = opt;
	memcpy(&dev->sg.entry[0], &sg->entry[0],
		opt.count * sizeof(sg->entry[0]));
	dev->index = 0;

#if 0
	thsemap_isignal_sema(dev->sg_sema_id);	// FIXME
#else
	ata_sif_cmd_sg_transfer(dev);
#endif
}

static void ata_sif_cmd(const struct sif_cmd_header *header, void *arg)
{
	const union ata_sif_opt opt = { .raw = header->opt };
	void *p = sif_cmd_payload(header);
	struct ata_dev *dev = arg;

	switch (opt.op)
	{
	case rop_bb:
		/* Register bounce buffer for unaligned transfers. */
		ata_sif_cmd_bb(dev, opt, p);
		break;
	case rop_sg:
		/* Process requested list of transfers. */
		ata_sif_cmd_sg(dev, opt, p);
		break;
	default:
		pr_err("%s: Unknown op %d\n", __func__, opt.op);
	}
}

static void sg_event(void *arg)
{
	struct ata_dev *dev = arg;

	for (;;) {
		thsemap_wait_sema(dev->sg_sema_id);

		ata_sif_cmd_sg_transfer(dev);
	}
}

static enum module_init_status ata_init(int argc, char *argv[])
{
	static struct ata_dev dev = {
		.dir = ata_dir_undetermined,
	};

	BUILD_BUG_ON(sizeof(union ata_sif_opt) != sizeof(u32));
	BUILD_BUG_ON(sizeof(struct ata_sif_sg) > CMD_PACKET_PAYLOAD_MAX);

	int err = dev9_request();
	if (err < 0) {
		pr_err("%s: dev9_request failed with %d\n", __func__, err);
		return MODULE_EXIT;
	}

	static const struct iop_thread sg_th = {
		.attr = THREAD_ATTR_C,
		.thread = sg_event,
		.stacksize = 1024,
		.priority = 0x26,	/* FIXME: What are reasonable priorities? */
	};

	dev.sg_stid = thbase_create(&sg_th);
	if (dev.sg_stid < 0) {
		pr_err("%s: thbase_create failed with %d: %s\n",
			__func__, dev.sg_stid, iop_error_message(dev.sg_stid));
		goto err_sg_thread_create;
	}

	const struct iop_sema sg_sema = { .initial = 1, .max = 1 };
	dev.sg_sema_id = thsemap_create_sema(&sg_sema);
	if (dev.sg_sema_id < 0) {
		pr_err("%s: thsemap_create_sema failed with %d: %s\n",
			__func__, dev.sg_sema_id, iop_error_message(dev.sg_sema_id));
		goto err_sema_create;
	}

	int ioperr = thbase_start(dev.sg_stid, &dev);
	if (ioperr < 0) {
		pr_err("%s: thbase_start failed with %d: %s\n",
			__func__, ioperr, iop_error_message(ioperr));
		goto err_sg_thread_start;
	}

	sif_request_cmd(SIF_CMD_ATA, ata_sif_cmd, &dev);

	return MODULE_RESIDENT;

err_sg_thread_start:
	thsemap_delete_sema(dev.sg_sema_id);

err_sema_create:
	thbase_delete(dev.sg_stid);

err_sg_thread_create:
	return MODULE_EXIT;
}
module_init(ata_init);
