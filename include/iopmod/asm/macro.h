// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_ASM_MACRO_H
#define IOPMOD_ASM_MACRO_H

#define __ALIGN_IOPMOD_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define __ALIGN_IOPMOD(x, a) __ALIGN_IOPMOD_MASK(x, (typeof(x))(a) - 1)
#define ALIGN(x, a) __ALIGN_IOPMOD((x), (a))

#define STR(x) #x
#define XSTR(x) STR(x)

#define CONCAT__(a, b) a ## b
#define CONCATENATE(a, b) CONCAT__(a, b)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define preserve(x)							\
	for (typeof(x) x__ = (x), y__ = 0; !y__; (x) = x__, y__ = !y__)

#endif /* IOPMOD_ASM_MACRO_H */
