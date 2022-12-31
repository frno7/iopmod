// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_SPD_H
#define IOPMOD_SPD_H

#define SPD_REGBASE		0xb0000000
#define SPD_REG(offset)		(SPD_REGBASE + (offset))

#define SPD_REG_REV		0x00
#define SPD_REG_REV_1		0x02
#define SPD_REG_REV_3		0x04
#define SPD_REG_REV_8		0x0e

#define SPD_REG_DMA_CTRL	0x24
#define SPD_REG_INTR_STAT	0x28
#define SPD_REG_INTR_MASK	0x2a

#define SPD_REG_XFR_CTRL	0x32
#define SPD_REG_0x38		0x38
#define SPD_REG_IF_CTRL		0x64
#define   SPD_IF_ATA_RESET	  0x80
#define   SPD_IF_DMA_ENABLE	  0x04
#define SPD_REG_PIO_MODE	0x70
#define SPD_REG_MWDMA_MODE	0x72
#define SPD_REG_UDMA_MODE	0x74

#endif /* IOPMOD_SPD_H */
