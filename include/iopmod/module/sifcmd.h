// SPDX-License-Identifier: GPL-2.0

MODULE_ID(IOP_SIF_rpc_interface, 0x0101);
LIBRARY_ID(sifcmd, 0x0101);

id_(4) void sifcmd_init_cmd(void)
	alias_(sceSifInitCmd);
id_(5) void sifcmd_exit_cmd(void)
	alias_(sceSifExitCmd);
id_(6) void sifcmd_get_sreg(int index)
	alias_(sceSifGetSreg);
id_(7) void sifcmd_set_sreg(int index, unsigned int value)
	alias_(sceSifSetSreg);
id_(8) void sifcmd_set_cmd_buffer(
		struct sifcmd_handler_data *cmdBuffer, int size)
	alias_(sceSifSetCmdBuffer);
id_(9) void sifcmd_set_sys_cmd_buffer(
		struct sifcmd_handler_data *sysCmdBuffer, int size)
	alias_(sceSifSetSysCmdBuffer);

/**
 * sif_request_cmd - register SIF command
 * @cid: command id, normally limited to 0..31, with bit 31 indicating system
 * @handler: function to be called back for the command
 * @arg: optional argument passed back to the callback function, can be %NULL
 *
 * Context: thread
 */
id_(10) void sif_request_cmd(int cid, sifcmd_handler handler, void *arg)
	alias_(sceSifAddCmdHandler);

/**
 * sif_release_cmd - deregister SIF command
 * @cid: command id, normally limited to 0..31, with bit 31 indicating system
 *
 * Context: thread
 */
id_(11) void sif_release_cmd(int cid)
	alias_(sceSifRemoveCmdHandler);

/**
 * sifcmd_send_cmd - send command over the SIF in a thread context
 * @cmd: command number
 * @packet: packet pointer
 * @packet_size: size in bytes of packet
 * @src: sub data to copy from, must be aligned with a 4-byte DMA boundary
 * @dst: main data to copy to, must be aligned with a 16-byte DMA boundary
 * @nbytes: size in bytes to copy, will be rounded up to multiple of 16 bytes
 *
 * A @packet begins with a &struct sif_cmd_header, optionally followed by a
 * small packet payload.
 *
 * @packet_size must not be larger than 112 (%CMD_PACKET_PAYLOAD_MAX) bytes.
 * @packet must be 4-byte boundary aligned. @src may be %NULL if @nbytes is 0.
 *
 * This call is asynchronous and both the @packet and @src buffers must remain
 * valid until the DMA transfer has completed, when sifman_dma_stat() returns
 * %DMA_STATUS_COMPLETED.
 *
 * For interrupt context, use sifcmd_send_cmd_irq() instead.
 *
 * Context: thread
 * Return: nonzero DMA transfer id, or zero if no DMA id could be allocated
 * 	and a retry may be necessary
 */
id_(12) unsigned int sifcmd_send_cmd(u32 cmd,
		void *packet, size_t packet_size,
		const void *src, main_addr_t dst, size_t nbytes)
	alias_(sceSifSendCmd);

/**
 * sifcmd_send_cmd_irq - send command over the SIF in an interrupt context
 * @cmd: command number
 * @packet: packet pointer
 * @packet_size: size in bytes of packet
 * @src: sub data to copy from, must be aligned with a 4-byte DMA boundary
 * @dst: main data to copy to, must be aligned with a 16-byte DMA boundary
 * @nbytes: size in bytes to copy, will be rounded up to multiple of 16 bytes
 *
 * A @packet begins with a &struct sif_cmd_header, optionally followed by a
 * small packet payload.
 *
 * @packet_size must not be larger than 112 (%CMD_PACKET_PAYLOAD_MAX) bytes.
 * @packet must be 4-byte boundary aligned. @src may be %NULL if @nbytes is 0.
 *
 * This call is asynchronous and both the @packet and @src buffers must remain
 * valid until the DMA transfer has completed, when sifman_dma_stat() returns
 * %DMA_STATUS_COMPLETED.
 *
 * For thread context, use sifcmd_send_cmd() instead.
 *
 * Context: irq
 * Return: nonzero DMA transfer id, or zero if no DMA id could be allocated
 * 	and a retry may be necessary
 */
id_(13) unsigned int sifcmd_send_cmd_irq(u32 cmd,
		void *packet, size_t packet_size,
		const void *src, main_addr_t dst, size_t nbytes)
	alias_(isceSifSendCmd);

id_(14) void sifcmd_init_rpc(int mode)
	alias_(sceSifInitRpc);
id_(15) int sifcmd_bind_rpc(struct sifcmd_rpc_client_data *client,
		int rpc_number, int mode)
	alias_(sceSifBindRpc);
id_(16) int sifcmd_call_rpc(struct sifcmd_rpc_client_data *client,
		int rpc_number, int mode, void *send, int ssize,
		void *receive, int rsize,
		sifcmd_rpc_end_func end_function, void *end_param)
	alias_(sceSifCallRpc);

id_(17) void sifcmd_register_rpc(struct sifcmd_rpc_server_data *sd, int sid,
		sifcmd_rpc_func func,
		void *buf, sifcmd_rpc_func cfunc, void *cbuf,
		struct sifcmd_rpc_data_queue *qd)
	alias_(sceSifRegisterRpc);

id_(18) int sifcmd_check_stat_rpc(struct sifcmd_rpc_client_data *cd)
	alias_(sceSifCheckStatRpc);

id_(19) struct sifcmd_rpc_data_queue *sifcmd_set_rpc_queue(
		struct sifcmd_rpc_data_queue *q, int thread_id)
	alias_(sceSifSetRpcQueue);
id_(20) struct sifcmd_rpc_server_data *sifcmd_get_next_request(
		struct sifcmd_rpc_data_queue *qd)
	alias_(sceSifGetNextRequest);
id_(21) void sifcmd_exec_request(struct sifcmd_rpc_server_data *srv)
	alias_(sceSifExecRequest);
id_(22) void sifcmd_rpc_loop(struct sifcmd_rpc_data_queue *qd)
	alias_(sceSifRpcLoop);

id_(23) int sifcmd_get_other_data(struct sifcmd_rpc_receive_data *rd,
		void *src, void *dest, int size, int mode)
	alias_(sceSifGetOtherData);

id_(24) struct sifcmd_rpc_server_data *sifcmd_remove_rpc(
		struct sifcmd_rpc_server_data *sd,
		struct sifcmd_rpc_data_queue *qd)
	alias_(sceSifRemoveRpc);
id_(25) struct sifcmd_rpc_data_queue *sifcmd_remove_rpc_queue(
		struct sifcmd_rpc_data_queue *qd)
	alias_(sceSifRemoveRpcQueue);
id_(26) void sifcmd_set_sif1_cb(void *func, int param)
	alias_(sceSifSetSif1CB);
id_(27) void sifcmd_clear_sif1_cb(void)
	alias_(sceSifClearSif1CB);
id_(28) unsigned int sifcmd_send_cmd_intr(unsigned int, void *, int,
		void *, void *, int, void (*func)(void), void *)
	alias_(sceSifSendCmdIntr);
id_(29) unsigned int sifcmd_isend_cmd_intr(unsigned int, void *, int,
		void *, void *, int, void (*func)(void), void *)
	alias_(isceSifSendCmdIntr);
