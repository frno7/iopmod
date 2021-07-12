// SPDX-License-Identifier: GPL-2.0
/*
 * IRQ module.
 *
 * Copyright (C) 2021 Fredrik Noring
 */

#include "iopmod/interrupt.h"
#include "iopmod/irq.h"
#include "iopmod/module.h"

int request_irq(unsigned int irq, irq_handler_t cb, void *arg)
{
	return request_irq__(irq, cb, arg);
}

int release_irq(unsigned int irq)
{
	return release_irq__(irq);
}
