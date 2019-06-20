// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_PROCESSOR_H
#define IOPMOD_PROCESSOR_H

#include "iopmod/barrier.h"

/* The barrier ensures pending writes will become visible as expected. */
#define cpu_relax() ({ barrier(); asm volatile ("nop;nop;nop;nop;"); })

#endif /* IOPMOD_PROCESSOR_H */
