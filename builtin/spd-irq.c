// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Fredrik Noring
 */

#include "iopmod/bits.h"
#include "iopmod/build-bug.h"
#include "iopmod/dev9.h"
#include "iopmod/errno.h"
#include "iopmod/interrupt.h"
#include "iopmod/io.h"
#include "iopmod/iop-error.h"
#include "iopmod/irqs.h"
#include "iopmod/printk.h"
#include "iopmod/processor.h"
#include "iopmod/spd.h"
#include "iopmod/spd-irq.h"
#include "iopmod/string.h"
#include "iopmod/types.h"

static int spd_irq_count;

#define SPD_IRQ_TYPE(irq_) [irq_ - IRQ_IOP_SPD] = { }

static struct spd_irq_desc {
	irq_handler_t cb;
	void *arg;
} spd_irqs[16] = {
	SPD_IRQ_TYPE(IRQ_IOP_SPD_ATA0),
	SPD_IRQ_TYPE(IRQ_IOP_SPD_ATA1),
	SPD_IRQ_TYPE(IRQ_IOP_SPD_TXDNV),
	SPD_IRQ_TYPE(IRQ_IOP_SPD_RXDNV),
	SPD_IRQ_TYPE(IRQ_IOP_SPD_TXEND),
	SPD_IRQ_TYPE(IRQ_IOP_SPD_RXEND),
	SPD_IRQ_TYPE(IRQ_IOP_SPD_EMAC3),
	SPD_IRQ_TYPE(IRQ_IOP_SPD_DVR),
	SPD_IRQ_TYPE(IRQ_IOP_SPD_UART),
};

static struct spd_irq_desc *spd_irq(unsigned int irq)
{
	if (spd_valid_irq(irq))
		return &spd_irqs[irq - IRQ_IOP_SPD];

	pr_warn("IRQ %d not SPD\n", irq);

	return NULL;
}

static void spd_ack_irq____(unsigned int irq)
{
	const u16 m = BIT(irq - IRQ_IOP_SPD);

	iowr16(m, SPD_REG(SPD_REG_INTR_STAT));
}

static void spd_enable_irq____(unsigned int irq)
{
	const u16 m = BIT(irq - IRQ_IOP_SPD);

	iowr16(iord16(SPD_REG(SPD_REG_INTR_MASK)) | m, SPD_REG(SPD_REG_INTR_MASK));
}

static void spd_disable_irq____(unsigned int irq)
{
	const u16 m = BIT(irq - IRQ_IOP_SPD);

	iowr16(iord16(SPD_REG(SPD_REG_INTR_MASK)) & ~m, SPD_REG(SPD_REG_INTR_MASK));
}

static enum irq_status spd_handle_irq(void *arg)
{
	u16 pending = iord16(SPD_REG(SPD_REG_INTR_STAT)) &
		      iord16(SPD_REG(SPD_REG_INTR_MASK));

	iowr16(pending, SPD_REG(SPD_REG_INTR_STAT));  /* Acknowledge IRQs. */

	while (pending) {
		const unsigned int irq_spd = fls(pending) - 1;

		if (spd_irqs[irq_spd].cb)
			spd_irqs[irq_spd].cb(spd_irqs[irq_spd].arg);

		pending &= ~BIT(irq_spd);
	}

	return IRQ_HANDLED;
}

/**
 * spd_request_irq__ - allocate a DEV9 SPD interrupt line and enable it
 * @irq: interrupt line to allocate
 * @cb: function to be called back when the IRQ occurs
 * @arg: optional argument passed back to the callback function, can be %NULL
 *
 * Drivers should use request_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Context: thread
 * Return: 0 on success, negative errno on error
 */
int spd_request_irq__(unsigned int irq, irq_handler_t cb, void *arg)
{
	struct spd_irq_desc *desc = spd_irq(irq);
	unsigned int flags;
	int ioperr = 0;

	if (!desc || !cb || desc->cb)
		return -EINVAL;

	irq_save(flags);

	if (!spd_irq_count)
		ioperr = request_irq(IRQ_IOP_DEV9, spd_handle_irq, NULL);

	if (ioperr >= 0) {
		spd_irq_count++;

		desc->cb = cb;
		desc->arg = arg;

		spd_ack_irq____(irq);
		spd_enable_irq____(irq);
	}

	irq_restore(flags);

	return ioperr < 0 ? errno_for_iop_error(ioperr) : 0;
}

/**
 * spd_release_irq__ - disable and free an allocated DEV9 SPD interrupt
 * @irq: interrupt line to release
 *
 * Drivers should use release_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Context: thread
 * Return: 0 on success, negative errno on error
 */
int spd_release_irq__(unsigned int irq)
{
	struct spd_irq_desc *desc = spd_irq(irq);
	unsigned int flags;
	int ioperr = 0;

	if (!desc || !desc->cb)
		return -EINVAL;

	irq_save(flags);

	spd_disable_irq____(irq);

	if (spd_irq_count == 1)
		ioperr = release_irq(IRQ_IOP_DEV9);

	if (ioperr >= 0)
		spd_irq_count--;

	desc->cb = NULL;
	desc->arg = NULL;

	irq_restore(flags);

	return ioperr < 0 ? errno_for_iop_error(ioperr) : 0;
}

/**
 * spd_enable_irq__ - enable handling of the selected DEV9 SPD interrupt line
 * @irq: interrupt to enable
 *
 * Drivers should use enable_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Note that spd_enable_irq__() and spd_disable_irq__() do not nest, so
 * spd_enable_irq__() will enable regardless of the number of previous calls
 * to spd_disable_irq__().
 *
 * Context: any
 */
void spd_enable_irq__(unsigned int irq)
{
	unsigned int flags;

	if (!spd_valid_irq(irq))
		return;

	irq_save(flags);

	spd_enable_irq____(irq);

	irq_restore(flags);
}

/**
 * spd_disable_irq__ - disable the selected DEV9 SPD interrupt line
 * @irq: interrupt to disable
 *
 * Drivers should use disable_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Note that spd_enable_irq__() and spd_disable_irq__() do not nest, so
 * spd_enable_irq__() will enable regardless of the number of previous calls
 * to spd_disable_irq__().
 *
 * Context: any
 */
void spd_disable_irq__(unsigned int irq)
{
	unsigned int flags;

	if (!spd_valid_irq(irq))
		return;

	irq_save(flags);

	spd_disable_irq____(irq);

	irq_restore(flags);
}
