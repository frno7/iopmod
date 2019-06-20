// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include "iopmod/interrupt.h"
#include "iopmod/iop-error.h"
#include "iopmod/types.h"

int request_irq(unsigned int irq, irq_handler_t cb, void *arg)
{
	unsigned int mode = 1;	/* FIXME: Mode? */
	int ioperr;

	ioperr = intrman_request_irq(irq, mode, cb, arg);
	if (ioperr < 0)
		return errno_for_iop_error(ioperr);

	enable_irq(irq);

	return 0;
}

int release_irq(unsigned int irq)
{
	int ioperr;

	disable_irq(irq);

	ioperr = intrman_release_irq(irq);

	return ioperr < 0 ? errno_for_iop_error(ioperr) : 0;
}
