// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#ifndef IOPMOD_LINUX_SIF_H
#define IOPMOD_LINUX_SIF_H

#include "iopmod/types.h"

int sif_cmd_opt_data(u32 cmd, u32 opt,
	const void *payload, size_t payload_size,
	main_addr_t dst, const void *src, size_t nbytes);

/**
 * sif_cmd_data - send command over the SIF
 * @cmd: command number
 * @payload: pointer to payload, any alignment
 * @payload_size: payload size, maximum %CMD_PACKET_PAYLOAD_MAX bytes
 * @dst: destination address on main processor, 16-byte aligned, or zero
 * @src: source address on sub processor, 4-byte aligned, or %NULL
 * @nbytes: number of bytes to copy from @src to @dst, or zero
 *
 * Waits for DMA completion. Note that @nbytes will be rounded up to 16-byte
 * alignment, so the destination buffer must accommodate for that.
 *
 * Context: any
 * Return: 0 on success, otherwise a negative error number
 */
static inline int sif_cmd_data(u32 cmd_id,
	const void *payload, size_t payload_size,
	main_addr_t dst, const void *src, size_t nbytes)
{
	return sif_cmd_opt_data(cmd_id, 0,
		payload, payload_size, dst, src, nbytes);
}

/**
 * sif_cmd - send command over the SIF
 * @cmd: command number
 * @payload: pointer to payload, any alignment
 * @payload_size: payload size, maximum %CMD_PACKET_PAYLOAD_MAX bytes
 *
 * This is the most convenient form of sending SIF commands without data.
 * Waits for DMA completion.
 *
 * Context: any
 * Return: 0 on success, otherwise a negative error number
 */
static inline int sif_cmd(u32 cmd_id,
	const void *payload, size_t payload_size)
{
	return sif_cmd_data(cmd_id, payload, payload_size, 0, NULL, 0);
}

#endif /* IOPMOD_LINUX_SIF_H */
