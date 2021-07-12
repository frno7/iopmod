// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_IRQS_H
#define IOPMOD_IRQS_H

enum {
	IRQ_IOP_VBLANK = 0,
	IRQ_IOP_SBUS,
	IRQ_IOP_CDVD,
	IRQ_IOP_DMA,
	IRQ_IOP_RTC0,
	IRQ_IOP_RTC1,
	IRQ_IOP_RTC2,
	IRQ_IOP_SIO0,
	IRQ_IOP_SIO1,
	IRQ_IOP_SPU,
	IRQ_IOP_PIO,
	IRQ_IOP_EVBLANK,
	IRQ_IOP_DVD,
	IRQ_IOP_DEV9,
	IRQ_IOP_RTC3,
	IRQ_IOP_RTC4,
	IRQ_IOP_RTC5,
	IRQ_IOP_SIO2,
	IRQ_IOP_HTR0,
	IRQ_IOP_HTR1,
	IRQ_IOP_HTR2,
	IRQ_IOP_HTR3,
	IRQ_IOP_USB,
	IRQ_IOP_EXTR,
	IRQ_IOP_ILINK,
	IRQ_IOP_ILNKDMA,

	IRQ_IOP_DMA_MDEC_IN = 32,	/* Ch0 */
	IRQ_IOP_DMA_MDEC_OUT,		/* Ch1 */
	IRQ_IOP_DMA_SIF2,		/* Ch2 */
	IRQ_IOP_DMA_CDVD,		/* Ch3 */
	IRQ_IOP_DMA_SPU,		/* Ch4 */
	IRQ_IOP_DMA_PIO,		/* Ch5 */
	IRQ_IOP_DMA_GPU_OTC,		/* Ch6 */
	IRQ_IOP_DMA_BE,			/* Bus error */
	IRQ_IOP_DMA_SPU2,		/* Ch7 */
	IRQ_IOP_DMA_DEV9,		/* Ch8 */
	IRQ_IOP_DMA_SIF0,		/* Ch9 */
	IRQ_IOP_DMA_SIF1,		/* Ch10 */
	IRQ_IOP_DMA_SIO2_IN,		/* Ch11 */
	IRQ_IOP_DMA_SIO2_OUT,		/* Ch12 */

	IRQ_IOP_SPD_BASE  = 46,
	IRQ_IOP_SPD_ATA0  = IRQ_IOP_SPD_BASE,
	IRQ_IOP_SPD_ATA1  = 47,
	IRQ_IOP_SPD_TXDNV = 48,
	IRQ_IOP_SPD_RXDNV = 49,
	IRQ_IOP_SPD_TXEND = 50,
	IRQ_IOP_SPD_RXEND = 51,
	IRQ_IOP_SPD_EMAC3 = 52,
	IRQ_IOP_SPD_DVR   = 55,
	IRQ_IOP_SPD_UART  = 58,
	IRQ_IOP_SPD_LAST  = IRQ_IOP_SPD_UART,

	IRQ_IOP_SW1 = 62,		/* R3000A software interrupt 1 */
	IRQ_IOP_SW2,			/* R3000A software interrupt 2 */
};

/**
 * irq_mode - mode of registers saved and restored during an interrupt
 * @IRQ_MODE_SOME: $at, $v0, $v1, $a0, $a1, $a2, $a3, $sp and $ra
 * @IRQ_MODE_MORE: like @IRQ_MODE_SOME with $t0..$t9, $gp and $fp
 * @IRQ_MODE_MOST: like @IRQ_MODE_MORE with $s0..$s7
 *
 * Additionally note that $hi, $lo, $t0..$t9 normally are caller saved, and
 * that $s0..$s8 normally are callee saved, by normal calling conventions.
 *
 * Finally, $k0 and $k1 are reserved for use by interrupt handlers, and can
 * randomly change at any time, unless all interrupts are disabled by for
 * instance irq_save().
 */
enum irq_mode {
	IRQ_MODE_SOME = 0,
	IRQ_MODE_MORE = 1,
	IRQ_MODE_MOST = 2,
};

/** FIXME */
enum irq_status {
	IRQ_HANDLED = 1,
};

typedef enum irq_status (*irq_handler_t)(void *arg);

#endif /* IOPMOD_IRQS_H */
