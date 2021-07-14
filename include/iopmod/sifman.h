// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_SIFMAN_H
#define IOPMOD_SIFMAN_H

#include <iopmod/types.h>

/**
 * enum sif_dma_attr - DMA transfer completion attributes
 * @SIF_DMA_ATTR_INT_I: FIXME
 * @SIF_DMA_ATTR_INT_O: assert IOP interrupt on completion
 * @SIF_DMA_ATTR_ERT: FIXME
 *
 * Attributes may be combined into a set by logical or.
 */
enum sif_dma_attr {
	SIF_DMA_ATTR_INT_I = 0x02,
	SIF_DMA_ATTR_INT_O = 0x04,
	SIF_DMA_ATTR_ERT   = 0x40,
};

/**
 * struct sif_dma_transfer - SIF DMA transfer structure
 * @src: sub data to copy from, must be aligned with a 4-byte DMA boundary
 * @dst: main data to copy to, must be aligned with a 16-byte DMA boundary
 * @nbytes: size in bytes to copy, will be rounded up to multiple of 16 bytes
 * @attr: zero, or a set of DMA transfer completion attributes
 */
struct sif_dma_transfer {
	void *src;
	main_addr_t dst;
	size_t nbytes;
	enum sif_dma_attr attr;
};

/**
 * enum sif_dma_status - status of DMA transfer
 * @SIF_DMA_STATUS_PENDING: DMA transfer has not yet started
 * @SIF_DMA_STATUS_BUSY: DMA transfer is in progress
 * @SIF_DMA_STATUS_COMPLETED: DMA transfer has completed
 */
enum sif_dma_status {
	SIF_DMA_STATUS_PENDING   =  1,
	SIF_DMA_STATUS_BUSY      =  0,
	SIF_DMA_STATUS_COMPLETED = -1
};

#include "iopmod/module-prototype.h"
#include "iopmod/module/sifman.h"

#endif /* IOPMOD_SIFMAN_H */
