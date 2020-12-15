// SPDX-License-Identifier: GPL-2.0
/*
 * Serial I/O 2 (SIO2).
 */

#ifndef SIO2_H
#define SIO2_H

#include "iopmod/build-bug.h"
#include "iopmod/string.h"
#include "iopmod/types.h"

#define SIO2_MEM_FIFO_TX	0xbf808000	/* (RW) 256 bytes */
#define SIO2_MEM_FIFO_RX	0xbf808100	/* (RW) 256 bytes */

#define SIO2_REG_CMD_QUEUE	0xbf808200	/* (RW) 16 commands */
#define SIO2_REG_PORT0_CTRL0	0xbf808240	/* (RW) Controller 0 */
#define SIO2_REG_PORT0_CTRL1	0xbf808244	/* (RW) Controller 0 */
#define SIO2_REG_PORT1_CTRL0	0xbf808248	/* (RW) Controller 1 */
#define SIO2_REG_PORT1_CTRL1	0xbf80824c	/* (RW) Controller 1 */
#define SIO2_REG_PORT2_CTRL0	0xbf808250	/* (RW) Memory card 0 */
#define SIO2_REG_PORT2_CTRL1	0xbf808254	/* (RW) Memory card 0 */
#define SIO2_REG_PORT3_CTRL0	0xbf808258	/* (RW) Memory card 1 */
#define SIO2_REG_PORT3_CTRL1	0xbf80825c	/* (RW) Memory card 1 */
#define SIO2_REG_TX		0xbf808260	/* (W) 8/16/32-bit transmit */
#define SIO2_REG_RX		0xbf808264	/* (R) 8/16/32-bit receive */
#define SIO2_REG_CTRL		0xbf808268	/* (RW) Control */
#define SIO2_REG_CMD_STAT	0xbf80826c	/* (R) Command status */
#define SIO2_REG_PORT_STAT	0xbf808270	/* (R) Port status */
#define SIO2_REG_FIFO_STAT	0xbf808274	/* (R) FIFO status */
#define SIO2_REG_FIFO_TX	0xbf808278	/* (RW) FIFO TX head and tail */
#define SIO2_REG_FIFO_RX	0xbf80827c	/* (RW) FIFO RX head and tail */
#define SIO2_REG_IRQ_STAT	0xbf808280	/* (RW) Interrupt status */
#define SIO2_REG_REMOTE_CTRL	0xbf808284	/* (RW) For IOP rev >= 0x23 */

/**
 * struct sio2_cmd - SIO2 command register
 * @port: port 0-3 where 0-1 are controllers and 2-3 are memory cards
 * @pause: FIXME
 * @cfg: FIXME
 * @tx_size: FIXME
 * @tx_unknown: FIXME
 * @rx_size: FIXME
 * @rx_unknown: FIXME
 * @clk_div_sel: FIXME
 * @ack_end_wait: FIXME
 */
struct sio2_cmd {
	u32 port : 2;
	u32 pause : 1;
	u32 : 1;
	u32 cfg : 4;
	u32 tx_size : 9;
	u32 tx_unknown : 1;
	u32 rx_size : 9;
	u32 rx_unknown : 1;
	u32 : 2;
	u32 clk_div_sel : 1;
	u32 ack_end_wait : 1;
};

/**
 * struct sio2_ctrl0 - SIO2 control 0 register
 * @att_active_cycles: typically 0x5
 * @att_inactive_cycles: typically 0x5
 * @baud_div0: 48 MHz divisor with 0x60 for 500 kHz and 0xc0 for 250 kHz
 * @baud_div1: typically 0xff FIXME: only valid in PS1 mode, with 0x60?
 *
 * Typical controller values for CTRL0 and CTRL1 are:
 *
 * 0xffc00505 0x00020014
 * 0xff060505 0x0002012c
 *
 * Analogue controllers:
 *
 * 0xffc00505 0x00020014
 * 0xff060505 0x0002012c
 * 0xff600a0a 0x00020014
 *
 * Finding:
 *
 * 0xffc00505 0x0002000a
 * 0xff060505 0x0002012c
 *
 * PS2 memory card:
 *
 * 0xff020405 0x0005ffff
 *
 * PS1:
 *
 * 0xffc00505 0x000201f4
 *
 * PS1 SIO0 mode:
 *
 * 0x06060a05 0x00050400
 */
struct sio2_ctrl0 {
	u32 att_active_cycles : 8;
	u32 att_inactive_cycles : 8;
	u32 baud_div0 : 8;
	u32 baud_div1 : 8;
};

/**
 * struct sio2_ctrl1 - SIO2 control 1 register
 * @sck_active_cycles: FIXME
 * @sck_inactive_cycles: typically 0x2 FIXME
 * @unk: FIXME
 * @mode: FIXME
 */
struct sio2_ctrl1 {
	u32 sck_active_cycles : 16;
	u32 sck_inactive_cycles : 8;
	u32 unk : 1;
	u32 mode : 1;
	u32 : 6;
};

/**
 * struct sio2_trx - SIO2 transmit and receive register FIXME
 * @byte: 8-bit data
 * @halfword: 16-bit data
 * @word: 32-bit data
 */
struct sio2_trx {
	union {
		struct {
			u32 byte : 8;
			u32 : 24;
		};
		struct {
			u32 halfword : 16;
			u32 : 16;
		};
		struct {
			u32 word;
		};
	};
};

/**
 * struct sio2_ctrl - SIO2 control register
 * @start: set to start processing from the beginning of the command queue
 * @resume: set to continue processing commands from command queue index
 * @reset: FIXME
 * @reset_fifo: FIXME
 * @timeout_enable: set to enable timeout, otherwise wait indefinitely
 * @error_proceed: set to continue processing commands regardless of errors
 * @unknown: FIXME
 * @error_irq_enable: FIXME
 * @tx_irq_enable: FIXME
 * @ps1: FIXME
 * @dir: FIXME
 *
 * Reset with 0x3bc. Set bits 0x0c before transaction, and then 0x01 to start.
 */
struct sio2_ctrl {
	u32 start : 1;
	u32 resume : 1;
	u32 reset : 1;
	u32 reset_fifo : 1;
	u32 timeout_enable : 1;
	u32 error_proceed : 1;
	u32 : 1;
	u32 unknown : 1;
	u32 error_irq_enable : 1;
	u32 tx_irq_enable : 1;
	u32 : 2;
	u32 : 18;
	u32 ps1 : 1;
	u32 dir : 1;
};

/**
 * struct sio2_cmd_stat - SIO2 command status
 * @unknown0: FIXME
 * @more_dma_bcr_tx: FIXME
 * @more_dma_bcr_rx: FIXME
 * @unknown1: FIXME
 * @index: command queue index
 * @ready: FIXME
 * @error: FIXME
 * @flow_error: FIXME
 * @missing_ack: FIXME
 * @queue_errors: FIXME
 *
 * Disconnected peripherals are typically 0x1d100. Connected ones 0x1100.
 */
struct sio2_cmd_stat {
	u32 unknown0 : 4;
	u32 more_dma_bcr_tx : 1;
	u32 more_dma_bcr_rx : 1;
	u32 unknown1 : 2;
	u32 index : 4;
	u32 ready : 1;
	u32 error : 1;
	u32 flow_error : 1;
	u32 missing_ack : 1;
	u32 queue_errors : 16;
};

/**
 * struct sio2_port_stat - SIO2 port status
 * @cdc_port0: FIXME
 * @cdc_port1: FIXME
 * @cdc_port2: FIXME
 * @cdc_port3: FIXME
 * @sas_port0: FIXME
 * @sas_port1: FIXME
 * @sas_port2: FIXME
 * @sas_port3: FIXME
 */
struct sio2_port_stat {
	u32 cdc_port0 : 1;
	u32 cdc_port1 : 1;
	u32 cdc_port2 : 1;
	u32 cdc_port3 : 1;
	u32 sas_port0 : 1;
	u32 sas_port1 : 1;
	u32 sas_port2 : 1;
	u32 sas_port3 : 1;
	u32 : 24;
};

/**
 * struct sio2_fifo_stat - SIO2 FIFO status
 * @tx_size: FIXME
 * @rx_size: FIXME
 * @full: FIXME
 * @empty: FIXME
 */
struct sio2_fifo_stat {
	u32 tx_size : 16;
	u32 rx_size : 9;
	u32 : 1;
	u32 full : 1;
	u32 empty : 1;
	u32 : 4;
};

/**
 * struct sio2_fifo_tx - SIO2 FIFO transmit
 * @head: FIXME
 * @tail: FIXME
 */
struct sio2_fifo_tx {
	u32 head : 8;
	u32 : 8;
	u32 tail : 8;
	u32 : 8;
};

/**
 * struct sio2_fifo_rx - SIO2 FIFO receive
 * @head: FIXME
 * @tail: FIXME
 */
struct sio2_fifo_rx {
	u32 head : 8;
	u32 : 8;
	u32 tail : 8;
	u32 : 8;
};

/**
 * struct sio2_irq_stat - SIO2 interrupt status
 * @tx: transfer complete and mask interrupt, set to clear
 * @error: transmission error and mask interrupt, set to clear
 */
struct sio2_irq_stat {
	u32 tx : 1;
	u32 error : 1;
	u32 : 30;
};

static inline u32 sio2_rd_cmd(int index)
{
	return iord32(SIO2_REG_CMD_QUEUE + (index << 2));
}

static inline void sio2_wr_cmd(int index, u32 value)
{
	iowr32(value, SIO2_REG_CMD_QUEUE + (index << 2));
}

static inline struct sio2_cmd sio2_rs_cmd(int index)
{
	const u32 v = sio2_rd_cmd(index);
	struct sio2_cmd value;
	BUILD_BUG_ON(sizeof(v) != sizeof(value));
	memcpy(&value, &v, sizeof(v));
	return value;
}

static inline void sio2_ws_cmd(int index, struct sio2_cmd value)
{
	u32 v;
	BUILD_BUG_ON(sizeof(v) != sizeof(value));
	memcpy(&v, &value, sizeof(v));
	sio2_wr_cmd(index, v);
}

static inline void sio2_wr_tx8(u8 value) { iowr8(value, SIO2_REG_TX); }
static inline void sio2_wr_tx16(u16 value) { iowr16(value, SIO2_REG_TX); }
static inline void sio2_wr_tx32(u32 value) { iowr32(value, SIO2_REG_TX); }

static inline u8 sio2_rd_rx8(void) { return iord8(SIO2_REG_RX); }
static inline u16 sio2_rd_rx16(void) { return iord16(SIO2_REG_RX); }
static inline u32 sio2_rd_rx32(void) { return iord32(SIO2_REG_RX); }

#define SIO2_DEFINE_RD_REG(reg, addr)					\
	static inline u32 sio2_rd_##reg(void)				\
	{								\
		return iord32(SIO2_REG_##addr);				\
	}

#define SIO2_DEFINE_WR_REG(reg, addr)					\
	static inline void sio2_wr_##reg(u32 value)			\
	{								\
		iowr32(value, SIO2_REG_##addr);				\
	}

/* Read registers as structures, which simplifies notation. */
#define SIO2_DEFINE_RS_REG(reg, type)					\
	static inline struct sio2_##type sio2_rs_##reg(void)		\
	{								\
		const u32 v = sio2_rd_##reg();				\
		struct sio2_##type value;				\
		BUILD_BUG_ON(sizeof(v) != sizeof(value));		\
		memcpy(&value, &v, sizeof(v));				\
		return value;						\
	}

/* Write registers as structures, which simplifies notation. */
#define SIO2_DEFINE_WS_REG(reg, type)					\
	static inline void sio2_ws_##reg(struct sio2_##type value)	\
	{								\
		u32 v;							\
		BUILD_BUG_ON(sizeof(v) != sizeof(value));		\
		memcpy(&v, &value, sizeof(v));				\
		sio2_wr_##reg(v);					\
	}

#define SIO2_DEFINE_RO_REG(reg, type, addr)				\
	SIO2_DEFINE_RD_REG(reg, addr);					\
	SIO2_DEFINE_RS_REG(reg, type)

#define SIO2_DEFINE_WO_REG(reg, type, addr)				\
	SIO2_DEFINE_WR_REG(reg, addr);					\
	SIO2_DEFINE_WS_REG(reg, type)

#define SIO2_DEFINE_RW_REG(reg, type, addr)				\
	SIO2_DEFINE_RO_REG(reg, type, addr);				\
	SIO2_DEFINE_WO_REG(reg, type, addr)

SIO2_DEFINE_RW_REG(port0_ctrl0, ctrl0,     PORT0_CTRL0);
SIO2_DEFINE_RW_REG(port0_ctrl1, ctrl1,     PORT0_CTRL1);
SIO2_DEFINE_RW_REG(port1_ctrl0, ctrl0,     PORT1_CTRL0);
SIO2_DEFINE_RW_REG(port1_ctrl1, ctrl1,     PORT1_CTRL1);
SIO2_DEFINE_RW_REG(port2_ctrl0, ctrl0,     PORT2_CTRL0);
SIO2_DEFINE_RW_REG(port2_ctrl1, ctrl1,     PORT2_CTRL1);
SIO2_DEFINE_RW_REG(port3_ctrl0, ctrl0,     PORT3_CTRL0);
SIO2_DEFINE_RW_REG(port3_ctrl1, ctrl1,     PORT3_CTRL1);
SIO2_DEFINE_RW_REG(ctrl,        ctrl,      CTRL);
SIO2_DEFINE_RO_REG(cmd_stat,    cmd_stat,  CMD_STAT);
SIO2_DEFINE_RO_REG(port_stat,   port_stat, PORT_STAT);
SIO2_DEFINE_RO_REG(fifo_stat,   fifo_stat, FIFO_STAT);
SIO2_DEFINE_RW_REG(fifo_tx,     fifo_tx,   FIFO_TX);
SIO2_DEFINE_RW_REG(fifo_rx,     fifo_rx,   FIFO_RX);
SIO2_DEFINE_RW_REG(irq_stat,    irq_stat,  IRQ_STAT);

static inline void sio2_cl_irq_stat()
{
	sio2_wr_irq_stat(sio2_rd_irq_stat());
}

#define SIO2_WS_CMD(index, ...)						\
	sio2_ws_cmd(index, (struct sio2_cmd) { __VA_ARGS__ })

#define SIO2_WS_REG(reg, str, ...)					\
	sio2_ws_##reg((struct sio2_##str) { __VA_ARGS__ })

#define SIO2_WS_PORT0_CTRL0(...) SIO2_WS_REG(port0_ctrl0, ctrl0,    __VA_ARGS__)
#define SIO2_WS_PORT0_CTRL1(...) SIO2_WS_REG(port0_ctrl1, ctrl1,    __VA_ARGS__)
#define SIO2_WS_PORT1_CTRL0(...) SIO2_WS_REG(port1_ctrl0, ctrl0,    __VA_ARGS__)
#define SIO2_WS_PORT1_CTRL1(...) SIO2_WS_REG(port1_ctrl1, ctrl1,    __VA_ARGS__)
#define SIO2_WS_PORT2_CTRL0(...) SIO2_WS_REG(port2_ctrl0, ctrl0,    __VA_ARGS__)
#define SIO2_WS_PORT2_CTRL1(...) SIO2_WS_REG(port2_ctrl1, ctrl1,    __VA_ARGS__)
#define SIO2_WS_PORT3_CTRL0(...) SIO2_WS_REG(port3_ctrl0, ctrl0,    __VA_ARGS__)
#define SIO2_WS_PORT3_CTRL1(...) SIO2_WS_REG(port3_ctrl1, ctrl1,    __VA_ARGS__)
#define SIO2_WS_CTRL(...)        SIO2_WS_REG(ctrl,        ctrl,     __VA_ARGS__)
#define SIO2_WS_FIFO_TX(...)     SIO2_WS_REG(fifo_tx,     fifo_tx,  __VA_ARGS__)
#define SIO2_WS_FIFO_RX(...)     SIO2_WS_REG(fifo_rx,     fifo_rx,  __VA_ARGS__)
#define SIO2_WS_IRQ_STAT(...)    SIO2_WS_REG(irq_stat,    irq_stat, __VA_ARGS__)

#endif /* SIO2_H */
