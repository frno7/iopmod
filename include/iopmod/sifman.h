// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_SIFMAN_H
#define IOPMOD_SIFMAN_H

#include <iopmod/types.h>

/**
 * struct sif_dma_transfer - SIF DMA transfer structure
 * @src: sub data to copy from, must be aligned with a 4-byte DMA boundary
 * @dst: main data to copy to, must be aligned with a 16-byte DMA boundary
 * @nbytes: size in bytes to copy, will be rounded up to multiple of 16 bytes
 * @attr: zero or DMA completion attributes
 */
struct sif_dma_transfer {
	void *src;
	main_addr_t dst;
	size_t nbytes;
	u32 attr;
};

/**
 * enum sifman_dma_status - status of DMA transfer
 * @DMA_STATUS_PENDING: DMA transfer has not yet started
 * @DMA_STATUS_BUSY: DMA transfer is in progress
 * @DMA_STATUS_COMPLETED: DMA transfer has completed
 */
enum sifman_dma_status {
	DMA_STATUS_PENDING   =  1,
	DMA_STATUS_BUSY      =  0,
	DMA_STATUS_COMPLETED = -1
};

#include "iopmod/module-prototype.h"
#include "iopmod/module/sifman.h"

#endif /* IOPMOD_SIFMAN_H */
