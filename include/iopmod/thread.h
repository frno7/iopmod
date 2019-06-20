// SPDX-License-Identifier: GPL-2.0

#ifndef IOPMOD_THREAD_H
#define IOPMOD_THREAD_H

#define THREAD_ATTR_ASM			0x01000000
#define THREAD_ATTR_C			0x02000000
#define THREAD_ATTR_UMODE		0x00000008
#define THREAD_ATTR_NO_FILLSTACK	0x00100000
#define THREAD_ATTR_CLEAR_STACK		0x00200000

struct iop_thread {
	u32 attr;
	u32 option;
	void (*thread)(void *);
	u32 stacksize;
	u32 priority;
};

#define THREAD_SELF			0 /* Not supported by all functions */

#define	THREAD_STATUS_RUN	 	0x01
#define THREAD_STATUS_READY		0x02
#define THREAD_STATUS_WAIT		0x04
#define THREAD_STATUS_SUSPEND		0x08
#define THREAD_STATUS_WAITSUSPEND	0x0c
#define THREAD_STATUS_DORMANT		0x10

#define THREAD_WAIT_STATUS_SLEEP	1
#define THREAD_WAIT_STATUS_DELAY	2
#define THREAD_WAIT_STATUS_SEMA		3
#define THREAD_WAIT_STATUS_EVENTFLAG	4
#define THREAD_WAIT_STATUS_MBX		5
#define THREAD_WAIT_STATUS_VPL		6
#define THREAD_WAIT_STATUS_FPL		7

struct iop_thread_status {
	u32 attr;
	u32 option;
	int status;
	void *entry;
	void *stack;
	int stack_size;
	void *gp_reg;
	int init_priority;
	int current_priority;
	int wait_type;
	int wait_id;
	int wakeup_count;
	long int *reg_context;	/* Only valid with thbase_irefer_status */
	u32 reserved[4];
};

struct iop_sys_clock {
	u32 lo;
	u32 hi;
};

struct iop_thread_run_status {
	int status;
	int current_priority;
	int wait_type;
	int wait_id;
	int wakeup_count;
	long int *reg_context;
	struct iop_sys_clock run_clocks;
	u32 intr_preempt_count;
	u32 thread_preempt_count;
	u32 release_count;
};

struct iop_sys_status {
	u32 status;
	int system_low_timer_width;
	struct iop_sys_clock idle_clocks;
	struct iop_sys_clock kernel_clocks;
	u32 comes_out_of_idle_count;
	u32 thread_switch_count;
	u32 reserved[8];
};

/* Defines for wait_event_flag */

#define WEF_AND		0
#define WEF_OR		1
#define WEF_CLEAR	0x10

#define EA_SINGLE	0
#define EA_MULTI	2

struct iop_event {
	u32 attr;
	u32 option;
	u32 bits;
};

struct iop_event_status {
	u32 attr;
	u32 option;
	u32 init_bits;
	u32 curr_bits;
	u32 num_threads;
	u32 reserved1;
	u32 reserved2;
};

/* Semaphore attributes */

#define SA_THFIFO	0x000
#define SA_THPRI	0x001
#define SA_IHTHPRI	0x100

struct iop_sema {
	u32 attr;
	u32 option;
	u32 initial;
	u32 max;
};

struct iop_sema_status {
	u32 attr;
	u32 option;
	u32 initial;
	u32 max;
	u32 current;
	u32 num_wait_threads;
	u32 reserved[2];
};

/* Message box attributes */

#define MBA_THFIFO	0x000
#define MBA_THPRI	0x001
#define MBA_MSFIFO	0x000
#define MBA_MSPRI	0x004

struct iop_mbx {
	u32 attr;
	u32 option;
};

struct iop_message {
	struct iop_message *next;
	u8 priority;
	u8 unused[3];
};

struct iop_mbx_status {
	u32 attr;
	u32 option;
	u32 num_wait_threads;
	u32 num_message;
	struct iop_message *top_packet;
	u32 reserved[2];
};

/* Fixed-length pool attributes */

#define FA_THFIFO	0x000
#define FA_THPRI	0x001
#define FA_MEMBTM	0x200

/*
 * Fixed length memory pool
*/

struct iop_fpl_param {
	u32 attr;
	u32 option;
	u32 block_size;
	u32 blocks;
};

struct iop_fpl_status {
	u32 attr;
	u32 option;
	u32 block_size;
	u32 num_blocks;
	u32 free_blocks;
	u32 num_wait_threads;
	u32 reserved[4];
};

/* Variable-length pool attributes */

#define VA_THFIFO	0x000
#define VA_THPRI	0x001
#define VA_MEMBTM	0x200

struct iop_vpl_param {
	u32 attr;
	u32 option;
	int size;
};

struct iop_vpl_status {
	u32 attr;
	u32 option;
	u32 size;
	u32 free_size;
	u32 num_wait_threads;
	u32 reserved[3];
};

#include "iopmod/module-prototype.h"
#include "iopmod/module/thread.h"

#endif /* IOPMOD_THREAD_H */
