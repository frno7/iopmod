// SPDX-License-Identifier: GPL-2.0

MODULE_ID(dmacman, 0x0101);
LIBRARY_ID(dmacman, 0x0101);

id_(4) void dmac_ch_set_madr(u32 channel, u32 val);
id_(5) u32 dmac_ch_get_madr(u32 channel);

id_(6) void dmac_ch_set_bcr(u32 channel, u32 val);
id_(7) u32 dmac_ch_get_bcr(u32 channel);

id_(8) void dmac_ch_set_chcr(u32 channel, u32 val);
id_(9) u32 dmac_ch_get_chcr(u32 channel);

id_(10) void dmac_ch_set_tadr(u32 channel, u32 val);
id_(11) u32 dmac_ch_get_tadr(u32 channel);

id_(14) void dmac_set_dpcr(u32 val);
id_(15) u32 dmac_get_dpcr(void);
id_(16) void dmac_set_dpcr2(u32 val);
id_(17) u32 dmac_get_dpcr2(void);
id_(18) void dmac_set_dpcr3(u32 val);
id_(19) u32 dmac_get_dpcr3(void);

id_(20) void dmac_set_dicr(u32 val);
id_(21) u32 dmac_get_dicr(void);
id_(22) void dmac_set_dicr2(u32 val);
id_(23) u32 dmac_get_dicr2(void);

id_(28) int dmac_request(u32 channel, void * addr, u32 size, u32 count, int dir);

id_(32) void dmac_transfer(u32 channel);

id_(33) void dmac_ch_set_dpcr(u32 channel, u32 val) alias_(sceSetDMAPriority);

id_(34) void dmac_enable(u32 channel) alias_(sceEnableDMAChannel);
id_(35) void dmac_disable(u32 channel) alias_(sceDisableDMAChannel);
