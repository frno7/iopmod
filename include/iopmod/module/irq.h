// SPDX-License-Identifier: GPL-2.0

MODULE_ID(irq, 0x0100);
LIBRARY_ID(irq, 0x0100);

id_(0) int request_irq(unsigned int irq, irq_handler_t cb, void *arg);

id_(1) int release_irq(unsigned int irq);
