// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_ADDRSPACE_H
#define IOPMOD_ADDRSPACE_H

/* Macro definitions from the Linux kernel. */

/* Returns the kernel segment base of a given address. */
#define KSEGX(a)		((a) & 0xe0000000)

/* Returns the physical address of a CKSEGx address. */
#define CPHYSADDR(a)		((a) & 0x1fffffff)

#define CKSEG0ADDR(a)		(CPHYSADDR(a) | KSEG0)
#define CKSEG1ADDR(a)		(CPHYSADDR(a) | KSEG1)
#define CKSEG2ADDR(a)		(CPHYSADDR(a) | KSEG2)
#define CKSEG3ADDR(a)		(CPHYSADDR(a) | KSEG3)

/* Map an address to a certain kernel segment. */
#define KSEG0ADDR(a)		(CPHYSADDR(a) | KSEG0)
#define KSEG1ADDR(a)		(CPHYSADDR(a) | KSEG1)
#define KSEG2ADDR(a)		(CPHYSADDR(a) | KSEG2)
#define KSEG3ADDR(a)		(CPHYSADDR(a) | KSEG3)

/* Traditional memory segments (32-bit kernel mode addresses). */
#define KUSEG			0x00000000
#define KSEG0			0x80000000
#define KSEG1			0xa0000000
#define KSEG2			0xc0000000
#define KSEG3			0xe0000000

#define CKUSEG			0x00000000
#define CKSEG0			0x80000000
#define CKSEG1			0xa0000000
#define CKSEG2			0xc0000000
#define CKSEG3			0xe0000000

#endif /* IOPMOD_ADDRSPACE_H */
