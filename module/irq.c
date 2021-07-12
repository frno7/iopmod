// SPDX-License-Identifier: GPL-2.0
/*
 * IRQ module.
 *
 * Copyright (C) 2021 Fredrik Noring
 */

#include "iopmod/interrupt.h"
#include "iopmod/irq.h"
#include "iopmod/module.h"

struct intc {
	int (*request_irq)(unsigned int irq, irq_handler_t cb, void *arg);
	int (*release_irq)(unsigned int irq);
};

static const struct intc *intc(unsigned int irq)
{
	static const struct intc intrman = {
		.request_irq = request_irq__,
		.release_irq = release_irq__,
	};

	return &intrman;	/* Default to intrman interrupt controller. */
}

int request_irq(unsigned int irq, irq_handler_t cb, void *arg)
{
	return intc(irq)->request_irq(irq, cb, arg);
}

int release_irq(unsigned int irq)
{
	return intc(irq)->release_irq(irq);
}
