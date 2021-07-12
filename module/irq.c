// SPDX-License-Identifier: GPL-2.0
/*
 * Main IRQ module. This is the interrupt controller drivers and almost any
 * other module should use. It cascades multiple hardware controllers
 * including DEV9 SPD IRQs by assigning virtual IRQs for them, which unifies
 * and simplifies IRQ handling.
 *
 * The available IRQs are defined in <include/iopmod/irqs.h>.
 *
 * Copyright (C) 2021 Fredrik Noring
 */

#include "iopmod/interrupt.h"
#include "iopmod/irq.h"
#include "iopmod/module.h"
#include "iopmod/spd-irq.h"

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

	static const struct intc spd = {
		.enable_irq = spd_enable_irq__,
		.disable_irq = spd_disable_irq__,
		.request_irq = spd_request_irq__,
		.release_irq = spd_release_irq__,
	};

	if (spd_valid_irq(irq))
		return &spd;

	return &intrman;	/* Default to intrman interrupt controller. */
}

/**
 * enable_irq - enable handling of the selected interrupt line
 * @irq: interrupt to enable
 *
 * Note that enable_irq() and disable_irq() do not nest, so enable_irq() will
 * enable regardless of the number of previous calls to disable_irq().
 *
 * Context: any
 */
void enable_irq(unsigned int irq)
{
	intc(irq)->enable_irq(irq);
}

/**
 * disable_irq - disable the selected interrupt line
 * @irq: interrupt to disable
 *
 * Note that enable_irq() and disable_irq() do not nest, so enable_irq() will
 * enable regardless of the number of previous calls to disable_irq().
 *
 * Context: any
 */
void disable_irq(unsigned int irq)
{
	intc(irq)->disable_irq(irq);
}

/**
 * request_irq - allocate an interrupt line and enable it
 * @irq: interrupt line to allocate
 * @cb: function to be called back when the IRQ occurs
 * @arg: optional argument passed back to the callback function, can be %NULL
 *
 * Context: thread
 * Return: 0 on success, negative errno on error
 */
int request_irq(unsigned int irq, irq_handler_t cb, void *arg)
{
	return intc(irq)->request_irq(irq, cb, arg);
}

/**
 * release_irq - disable and free an allocated interrupt
 * @irq: interrupt line to release
 *
 * Context: thread
 * Return: 0 on success, negative errno on error
 */
int release_irq(unsigned int irq)
{
	return intc(irq)->release_irq(irq);
}
