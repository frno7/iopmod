// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include "iopmod/build-bug.h"
#include "iopmod/compare.h"
#include "iopmod/errno.h"
#include "iopmod/interrupt.h"
#include "iopmod/processor.h"
#include "iopmod/sifcmd.h"
#include "iopmod/sifman.h"
#include "iopmod/string.h"
#include "iopmod/types.h"

/**
 * sif_dma_relax_for_completion - relax processor waiting for DMA completion
 * @dma_id: id of DMA transfer to wait for completion
 *
 * The relaxation is a busy-wait that is loose enough to allow status changes,
 * but is still only suitable for very short intervals, or when no other
 * way of waiting can be done, such as in an interrupt context. For a thread
 * context, it's often much better to trigger completion by an interrupt or
 * yield by putting the thread to sleep.
 *
 * Context: any
 */
void sif_dma_relax_for_completion(int dma_id)
{
	while (sifman_dma_stat(dma_id) != SIF_DMA_STATUS_COMPLETED)
		cpu_relax();
}

/**
 * sif_cmd_opt_data - send command over the SIF with optional argument
 * @cmd: command number
 * @opt: optional number
 * @payload: pointer to payload, any alignment
 * @payload_size: payload size, maximum %CMD_PACKET_PAYLOAD_MAX bytes
 * @dst: destination address on main processor, 16-byte aligned, or zero
 * @src: source address on sub processor, 4-byte aligned, or %NULL
 * @nbytes: number of bytes to copy from @src to @dst, or zero
 *
 * This is the most general form of sending SIF commands. Waits for DMA
 * completion. Note that @nbytes will be rounded up to 16-byte alignment,
 * so the destination buffer must accommodate for that.
 *
 * Context: any
 * Return: 0 on success, otherwise a negative error number
 */
int sif_cmd_opt_data(u32 cmd, u32 opt,
	const void *payload, size_t payload_size,
	main_addr_t dst, const void *src, size_t nbytes)
{
	unsigned int (*send_cmd)(u32 cmd, void *packet, size_t packet_size,
		const void *src, main_addr_t dst, size_t nbytes) =
		in_irq() ? sifcmd_send_cmd_irq : sifcmd_send_cmd;
	struct {
		struct sif_cmd_header header;
		u8 payload[CMD_PACKET_PAYLOAD_MAX];
	} packet;
	int dma_id;

	BUILD_BUG_ON(sizeof(packet.header) != 16);
	BUILD_BUG_ON(sizeof(packet.payload) != CMD_PACKET_PAYLOAD_MAX);
	BUILD_BUG_ON(sizeof(packet) != CMD_PACKET_MAX);

	if (payload_size > CMD_PACKET_PAYLOAD_MAX)
		return -EINVAL;

	packet.header.opt = opt;
	memcpy(packet.payload, payload, payload_size);

	do {
		dma_id = send_cmd(cmd,
			&packet, sizeof(packet.header) + payload_size,
			src, dst, nbytes);
	} while (!dma_id);

	sif_dma_relax_for_completion(dma_id);

	return 0;
}
