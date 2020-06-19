// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Fredrik Noring
 */

#ifndef IOPMOD_IO_H
#define IOPMOD_IO_H

#include "iopmod/types.h"

/* Use I/O read and write with KSEG1. */

#define DEFINE_IORD(type, name)						\
static inline type name(const u32 addr)					\
{									\
	const volatile type *__addr = (const volatile type *)addr;	\
									\
	/* FIXME: Crashes? __sync_synchronize(); */			\
									\
	return *__addr;							\
}

DEFINE_IORD(u8,  iord8);
DEFINE_IORD(u16, iord16);
DEFINE_IORD(u32, iord32);

#define DEFINE_IOWR(type, name)						\
static inline void name(type value, u32 addr)				\
{									\
	volatile type *__addr = (volatile type *)addr;			\
									\
	/* FIXME: Crashes? __sync_synchronize(); */			\
									\
	*__addr = value;						\
}

DEFINE_IOWR(u8,  iowr8);
DEFINE_IOWR(u16, iowr16);
DEFINE_IOWR(u32, iowr32);

#endif /* IOPMOD_IO_H */
