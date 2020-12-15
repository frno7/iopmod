// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Fredrik Noring
 */

#ifndef IOPMOD_IO_H
#define IOPMOD_IO_H

#include "iopmod/addrspace.h"
#include "iopmod/types.h"

/* Use I/O read and write with KSEG1. */

/* Macro definition from the Linux kernel. */
#define __fast_iob()							\
	__asm__ __volatile__(						\
		".set   push\n\t"					\
		".set   noreorder\n\t"					\
		"lw     $0,%0\n\t"					\
		"nop\n\t"						\
		".set   pop"						\
		: /* No output. */					\
		: "m" (*(int *)CKSEG1)					\
		: "memory")

#define wbflush() __fast_iob()

#define mb()		wbflush()
#define iobarrier_rw()	mb()

#define DEFINE_IORD(type, name)						\
static inline type name(const u32 addr)					\
{									\
	const volatile type *__addr = (const volatile type *)addr;	\
									\
	iobarrier_rw();							\
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
	iobarrier_rw();							\
									\
	*__addr = value;						\
}

DEFINE_IOWR(u8,  iowr8);
DEFINE_IOWR(u16, iowr16);
DEFINE_IOWR(u32, iowr32);

#endif /* IOPMOD_IO_H */
