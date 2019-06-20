// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_BARRIER_H
#define IOPMOD_BARRIER_H

/* The "volatile" is due to gcc bugs */
#define barrier() asm volatile ("": : :"memory")

#endif /* IOPMOD_BARRIER_H */
