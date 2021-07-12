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
	void (*enable_irq)(unsigned int irq);
	void (*disable_irq)(unsigned int irq);
	int (*request_irq)(unsigned int irq, irq_handler_t cb, void *arg);
	int (*release_irq)(unsigned int irq);
};

static const struct intc *intc(unsigned int irq)
{
	static const struct intc intrman = {
		.enable_irq = enable_irq__,
		.disable_irq = disable_irq__,
		.request_irq = request_irq__,
		.release_irq = release_irq__,
	};

	return &intrman;	/* Default to intrman interrupt controller. */
}

void enable_irq(unsigned int irq)
{
	intc(irq)->enable_irq(irq);
}

void disable_irq(unsigned int irq)
{
	intc(irq)->disable_irq(irq);
}

int request_irq(unsigned int irq, irq_handler_t cb, void *arg)
{
	return intc(irq)->request_irq(irq, cb, arg);
}

int release_irq(unsigned int irq)
{
	return intc(irq)->release_irq(irq);
}
