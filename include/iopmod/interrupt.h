// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_INTERRUPT_H
#define IOPMOD_INTERRUPT_H

#include <stddef.h>

#include "iopmod/irqs.h"

#include "iopmod/module-prototype.h"
#include "iopmod/module/intrman.h"
#include "iopmod/module/irq.h"

#include "iopmod/typecheck.h"

/**
 * irq_save - prevent preemption by disabling interrupts
 * @flags: variable to store the current interrupt state
 *
 * irq_save() and irq_restore() can be nested.
 *
 * Context: any
 */
#define irq_save(flags)							\
	do {								\
		typecheck(unsigned int, flags);				\
		intrman_cpu_suspend_irq(&(flags));			\
	} while (0)

/**
 * irq_restore - restore a previous interrupt state
 * @flags: variable having the previous interrupt state
 *
 * irq_save() and irq_restore() can be nested.
 *
 * Context: any
 */
#define irq_restore(flags)						\
	do {								\
		typecheck(unsigned int, flags);				\
		intrman_cpu_resume_irq(flags);				\
	} while (0)

int request_irq__(unsigned int irq, irq_handler_t cb, void *arg);

int release_irq__(unsigned int irq);

/**
 * enable_irq__ - enable handling of the selected intrman interrupt line
 * @irq: interrupt to enable
 *
 * Drivers should use enable_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Note that enable_irq__() and disable_irq__() do not nest, so enable_irq__()
 * will enable regardless of the number of previous calls to disable_irq__().
 *
 * Context: any
 */
static inline void enable_irq__(unsigned int irq)
{
	intrman_enable_irq(irq);
}

/**
 * disable_irq__ - disable the selected intrman interrupt line
 * @irq: interrupt to disable
 *
 * Drivers should use disable_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Note that enable_irq__() and disable_irq__() do not nest, so enable_irq__()
 * will enable regardless of the number of previous calls to disable_irq__().
 *
 * Context: any
 */
static inline void disable_irq__(unsigned int irq)
{
	intrman_disable_irq(irq, NULL);
}

/**
 * in_irq - are we in an interrupt context?
 *
 * Context: any
 * Return: %true for interrupt context, %false otherwise
 */
#define in_irq() intrman_in_irq()

#endif /* IOPMOD_INTERRUPT_H */
