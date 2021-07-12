// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_INTERRUPT_H
#define IOPMOD_INTERRUPT_H

#include <stddef.h>

#include "iopmod/irqs.h"

#include "iopmod/module-prototype.h"
#include "iopmod/module/intrman.h"
#include "iopmod/module/irq.h"

#include "iopmod/typecheck.h"

#define irq_save(flags)							\
	do {								\
		typecheck(unsigned int, flags);				\
		intrman_cpu_suspend_irq(&(flags));			\
	} while (0)

#define irq_restore(flags)						\
	do {								\
		typecheck(unsigned int, flags);				\
		intrman_cpu_resume_irq(flags);				\
	} while (0)

static inline int enable_irq(unsigned int irq)
{
	return intrman_enable_irq(irq);
}

static inline int disable_irq(unsigned int irq)
{
	return intrman_disable_irq(irq, NULL);	/* FIXME: What does NULL mean here? */
}

int request_irq__(unsigned int irq, irq_handler_t cb, void *arg);

int release_irq__(unsigned int irq);

/**
 * in_irq - are we in IRQ context?
 *
 * Context: any
 * Return: nonzero for IRQ context, otherwise zero.
 */
#define in_irq() intrman_in_irq()

#endif /* IOPMOD_INTERRUPT_H */
