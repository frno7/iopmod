// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include "iopmod/interrupt.h"
#include "iopmod/iop-error.h"
#include "iopmod/types.h"

/**
 * request_irq__ - allocate an intrman interrupt line and enable it
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
int request_irq__(unsigned int irq, irq_handler_t cb, void *arg)
{
	int ioperr;

	ioperr = intrman_request_irq(irq, IRQ_MODE_MORE, cb, arg);
	if (ioperr < 0)
		return errno_for_iop_error(ioperr);

	enable_irq(irq);

	return 0;
}

/**
 * release_irq__ - disable and free an allocated intrman interrupt
 * @irq: interrupt line to release
 *
 * Drivers should use release_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Context: thread
 * Return: 0 on success, negative errno on error
 */
int release_irq__(unsigned int irq)
{
	int ioperr;

	disable_irq(irq);

	ioperr = intrman_release_irq(irq);

	return ioperr < 0 ? errno_for_iop_error(ioperr) : 0;
}
