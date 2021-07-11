// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_SPD_IRQ_H
#define IOPMOD_SPD_IRQ_H

#include "iopmod/interrupt.h"

static inline bool spd_valid_irq(unsigned int irq)
{
	return IRQ_IOP_SPD <= irq && irq <= IRQ_IOP_SPD_UART;
}

void spd_enable_irq__(unsigned int irq);

void spd_disable_irq__(unsigned int irq);

int spd_request_irq__(unsigned int irq, irq_handler_t cb, void *arg);

int spd_release_irq__(unsigned int irq);

#endif /* IOPMOD_SPD_IRQ_H */
