// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_SPD_IRQ_H
#define IOPMOD_SPD_IRQ_H

#include "iopmod/interrupt.h"

/**
 * spd_valid_irq - does the interrupt line belong to DEV9 SPD?
 * @irq: interrupt line to check
 *
 * Return: %true if the IRQ belongs to the DEV9 SPD, %false otherwise
 */
static inline bool spd_valid_irq(unsigned int irq)
{
	return IRQ_IOP_SPD_BASE <= irq && irq <= IRQ_IOP_SPD_LAST;
}

int spd_request_irq__(unsigned int irq, irq_handler_t cb, void *arg);

int spd_release_irq__(unsigned int irq);

void spd_enable_irq__(unsigned int irq);

void spd_disable_irq__(unsigned int irq);

#endif /* IOPMOD_SPD_IRQ_H */
