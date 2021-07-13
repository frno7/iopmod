// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_SIFCMD_H
#define IOPMOD_SIFCMD_H

/*
 * The Linux kernel defines the corresponding SIF commands in
 * arch/mips/include/asm/mach-ps2/sif.h and drivers/ps2/sif.c.
 */

#define SIF_MAINADDR 		0x1000f200	/* EE to IOP command buffer */
#define SIF_SUBADDR  		0x1000f210	/* IOP to EE command buffer */
#define SIF_MSFLAG   		0x1000f220	/* EE to IOP flag */
#define SIF_SMFLAG   		0x1000f230	/* IOP to EE flag */
#define SIF_SUBCTRL  		0x1000f240
#define SIF_UNKNF260		0x1000f260

#define	SIF_CMD_ID_SYS		0x80000000
#define	SIF_CMD_ID_USR		0x00000000

#define SIF_CMD_CHANGE_SADDR	(SIF_CMD_ID_SYS | 0x00)
#define SIF_CMD_WRITE_SREG	(SIF_CMD_ID_SYS | 0x01)
#define SIF_CMD_INIT_CMD	(SIF_CMD_ID_SYS | 0x02)
#define SIF_CMD_RESET_CMD	(SIF_CMD_ID_SYS | 0x03)
#define SIF_CMD_RPC_END		(SIF_CMD_ID_SYS | 0x08)
#define SIF_CMD_RPC_BIND	(SIF_CMD_ID_SYS | 0x09)
#define SIF_CMD_RPC_CALL	(SIF_CMD_ID_SYS | 0x0a)
#define SIF_CMD_RPC_RDATA	(SIF_CMD_ID_SYS | 0x0c)
#define SIF_CMD_IRQ_RELAY	(SIF_CMD_ID_SYS | 0x20)
#define SIF_CMD_PRINTK		(SIF_CMD_ID_SYS | 0x21)

#define	SIF_SID_ID_SYS		0x80000000
#define	SIF_SID_ID_USR		0x00000000

#define SIF_SID_FILE_IO		(SIF_SID_ID_SYS | 0x01)
#define SIF_SID_HEAP		(SIF_SID_ID_SYS | 0x03)
#define SIF_SID_LOAD_MODULE	(SIF_SID_ID_SYS | 0x06)
#define SIF_SID_IRQ_RELAY	(SIF_SID_ID_SYS | 0x20)

#define CMD_PACKET_MAX		112
#define CMD_PACKET_PAYLOAD_MAX 	96

/**
 * struct sif_cmd_header - 16-byte SIF command header
 * @packet_size: min 1x16 for header only, max 7*16 bytes
 * @data_size: main processor data size in bytes
 * @data_addr: main processor data address or zero
 * @cmd: command number
 * @opt: optional argument
 */
struct sif_cmd_header
{
	u32 packet_size : 8;
	u32 data_size : 24;
	u32 data_addr;
	u32 cmd;
	u32 opt;
} __attribute__((aligned(4)));

struct sifcmd_sreg_data {
	struct sif_cmd_header header;
	int index;
	unsigned int value;
};

/**
 * typedef sifcmd_handler - SIF command handler
 * @header: SIF command header
 * @arg: argument passed to sif_request_cmd()
 *
 * FIXME: Why is @header->packet_size always zero?
 */
typedef void (*sifcmd_handler)(const struct sif_cmd_header *header, void *arg);

struct sifcmd_handler_data {
	sifcmd_handler handler;
	void *arg;
};

typedef void *(*sifcmd_rpc_func)(int rpo, void *buffer, size_t size);
typedef void (*sifcmd_rpc_end_func)(void *end_param);

struct sifcmd_rpc_server_data {
	int sid;

	sifcmd_rpc_func func;
	void *buff;
	int size;

	sifcmd_rpc_func cfunc;
	void *cbuff;
	int size2;

	struct sifcmd_rpc_client_data *client;
	void *pkt_addr;
	int rpc_number;

	void *receive;
	int rsize;
	int rmode;
	int rid;

	struct sifcmd_rpc_server_data *link;
	struct sifcmd_rpc_server_data *next;
	struct sifcmd_rpc_data_queue *base;
};

struct sifcmd_rpc_header {
	void *pkt_addr;
	u32 rpc_id;
	int sema_id;
	u32 mode;
};

struct sifcmd_rpc_client_data {
	struct sifcmd_rpc_header hdr;
	u32 command;
	void *buff;
	void *cbuff;
	sifcmd_rpc_end_func end_function;
	void *end_param;
	struct sifcmd_rpc_server_data *server;
};

struct sifcmd_rpc_receive_data {
	struct sifcmd_rpc_header hdr;
	void *src;
	void *dest;
	int size;
};

struct sifcmd_rpc_data_queue {
	int thread_id;
	int active;
	struct sifcmd_rpc_server_data *link;
	struct sifcmd_rpc_server_data *start;
	struct sifcmd_rpc_server_data *end;
	struct sifcmd_rpc_data_queue *next;
};

#include "iopmod/module-prototype.h"
#include "iopmod/module/sifcmd.h"

#endif /* IOPMOD_SIFCMD_H */
