// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_IRQS_H
#define IOPMOD_IRQS_H

enum {
	IRQ_IOP_VBLANK       =  0,
	IRQ_IOP_SBUS         =  1,
	IRQ_IOP_CDVD         =  2,
	IRQ_IOP_DMA          =  3,
	IRQ_IOP_RTC0         =  4,
	IRQ_IOP_RTC1         =  5,
	IRQ_IOP_RTC2         =  6,
	IRQ_IOP_SIO0         =  7,
	IRQ_IOP_SIO1         =  8,
	IRQ_IOP_SPU          =  9,
	IRQ_IOP_PIO          = 10,
	IRQ_IOP_EVBLANK      = 11,
	IRQ_IOP_DVD          = 12,
	IRQ_IOP_DEV9         = 13,
	IRQ_IOP_RTC3         = 14,
	IRQ_IOP_RTC4         = 15,
	IRQ_IOP_RTC5         = 16,
	IRQ_IOP_SIO2         = 17,
	IRQ_IOP_HTR0         = 18,
	IRQ_IOP_HTR1         = 19,
	IRQ_IOP_HTR2         = 20,
	IRQ_IOP_HTR3         = 21,
	IRQ_IOP_USB          = 22,
	IRQ_IOP_EXTR         = 23,
	IRQ_IOP_ILINK        = 24,
	IRQ_IOP_ILNKDMA      = 25,

	IRQ_IOP_DMA_MDEC_IN  = 32,		/* Channel 0 */
	IRQ_IOP_DMA_MDEC_OUT = 33,		/* Channel 1 */
	IRQ_IOP_DMA_SIF2     = 34,		/* Channel 2 */
	IRQ_IOP_DMA_CDVD     = 35,		/* Channel 3 */
	IRQ_IOP_DMA_SPU      = 36,		/* Channel 4 */
	IRQ_IOP_DMA_PIO      = 37,		/* Channel 5 */
	IRQ_IOP_DMA_GPU_OTC  = 38,		/* Channel 6 */
	IRQ_IOP_DMA_BE       = 39,		/* Bus error */
	IRQ_IOP_DMA_SPU2     = 40,		/* Channel 7 */
	IRQ_IOP_DMA_DEV9     = 41,		/* Channel 8 */
	IRQ_IOP_DMA_SIF0     = 42,		/* Channel 9 */
	IRQ_IOP_DMA_SIF1     = 43,		/* Channel 10 */
	IRQ_IOP_DMA_SIO2_IN  = 44,		/* Channel 11 */
	IRQ_IOP_DMA_SIO2_OUT = 45,		/* Channel 12 */

	IRQ_IOP_SPD_BASE     = 46,
	IRQ_IOP_SPD_ATA0     = IRQ_IOP_SPD_BASE,/* Virtual SPD IRQ */
	IRQ_IOP_SPD_ATA1     = 47,		/* Virtual SPD IRQ */
	IRQ_IOP_SPD_TXDNV    = 48,		/* Virtual SPD IRQ */
	IRQ_IOP_SPD_RXDNV    = 49,		/* Virtual SPD IRQ */
	IRQ_IOP_SPD_TXEND    = 50,		/* Virtual SPD IRQ */
	IRQ_IOP_SPD_RXEND    = 51,		/* Virtual SPD IRQ */
	IRQ_IOP_SPD_EMAC3    = 52,		/* Virtual SPD IRQ */
	IRQ_IOP_SPD_DVR      = 55,		/* Virtual SPD IRQ */
	IRQ_IOP_SPD_UART     = 58,		/* Virtual SPD IRQ */
	IRQ_IOP_SPD_LAST     = IRQ_IOP_SPD_UART,

	IRQ_IOP_SW1          = 62,		/* R3000A software interrupt 1 */
	IRQ_IOP_SW2          = 63,		/* R3000A software interrupt 2 */
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
