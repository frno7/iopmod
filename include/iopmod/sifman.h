// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_SIFMAN_H
#define IOPMOD_SIFMAN_H

struct sifman_dma_transfer
{
	void	*src;
	void	*dst;
	int	size;
	int	attr;
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
