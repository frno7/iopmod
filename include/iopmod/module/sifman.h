// SPDX-License-Identifier: GPL-2.0

MODULE_ID(IOP_SIF_manager, 0x0101);
LIBRARY_ID(sifman, 0x0101);

id_(4) void sifman_dma2_init(void)
	alias_(sceSifDma2Init);
id_(5) void sifman_init(void)
	alias_(sceSifInit);

id_(6) void sifman_set_dchain(void)
	alias_(sceSifSetDChain);

/**
 * sifman_set_dma - request SIF DMA transfer from sub to main
 * @dma_tr: array of DMA transfers
 * @count: number of array elements
 *
 * Note that SIF DMA transfers in the opposite direction, from main to sub,
 * must be initiated by main.
 *
 * Context: any with interrupts disabled
 * Return: DMA transfer id, or zero if no DMA id could be allocated and a
 * 	retry may be necessary
 */
id_(7) int sifman_set_dma(const struct sifman_dma_transfer *dma_tr, int count)
	alias_(sceSifSetDma);

/**
 * sifman_dma_stat - status of DMA transfer
 * @dma_id: id of DMA transfer to check
 *
 * Context: any
 * Return: status of given DMA transfer
 */
id_(8) enum sifman_dma_status sifman_dma_stat(int dma_id)
	alias_(sceSifDmaStat);

id_(9) void sifman_set_one_dma(struct sifman_dma_transfer dma_tr)
	alias_(sceSifSetOneDma);

id_(12) void sifman_dma0_transfer(void *addr, int size, int mode)
	alias_(sceSifDma0Transfer);
id_(13) void sifman_dma0_sync(void)
	alias_(sceSifDma0Sync);
id_(14) int sifman_dma0_sending(void)
	alias_(sceSifDma0Sending);

id_(15) void sifman_dma1_transfer(void *addr, int size, int mode)
	alias_(sceSifDma1Transfer);
id_(16) void sifman_dma1_sync(void)
	alias_(sceSifDma1Sync);
id_(17) int sifman_dma1_sending(void)
	alias_(sceSifDma1Sending);

id_(18) void sifman_dma2_transfer(void *addr, int size, int mode)
	alias_(sceSifDma2Transfer);
id_(19) void sifman_dma2_sync(void)
	alias_(sceSifDma2Sync);
id_(20) int sifman_dma2_sending(void)
	alias_(sceSifDma2Sending);

id_(21) unsigned int sifman_get_ms_flag(void)
	alias_(sceSifGetMSFlag);
id_(22) unsigned int sifman_set_ms_flag(unsigned int val)
	alias_(sceSifSetMSFlag);

id_(23) u32 sifman_get_sm_flag(void)
	alias_(sceSifGetSMFlag);
id_(24) u32 sifman_set_sm_flag(u32 val)
	alias_(sceSifSetSMFlag);

id_(25) unsigned long sifman_get_main_addr(void)
	alias_(sceSifGetMainAddr);

id_(26) unsigned long sifman_get_sub_addr(void)
	alias_(sceSifGetSubAddr);
id_(27) unsigned long sifman_set_sub_addr(unsigned long addr)
	alias_(sceSifSetSubAddr);

id_(28) void sifman_intr_main(void)
	alias_(sceSifIntrMain);

id_(29) int sifman_check_init(void)
	alias_(sceSifCheckInit);

id_(30) void sifman_set_dma_intr_handler(void (*handler)(void *), void *arg)
	alias_(sceSifSetDmaIntrHandler);
id_(31) void sifman_reset_dma_intr_handler(void)
	alias_(sceSifResetDmaIntrHandler);

id_(32) unsigned int sifman_set_dma_intr(struct sifman_dma_transfer *dma_tr,
		int len, void (*func)(void), void *data)
	alias_(sceSifSetDmaIntr);
