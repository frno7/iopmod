// SPDX-License-Identifier: GPL-2.0

MODULE_ID(Multi_Thread_Manager, 0x0101);
LIBRARY_ID(thbase, 0x0101);

id_(4) int thbase_create(const struct iop_thread *thread)
	alias_(CreateThread);
id_(5) int thbase_delete(int thid)
	alias_(DeleteThread);

id_(6) int thbase_start(int thid, void *arg)
	alias_(StartThread);
id_(7) int thbase_start_args(int thid, int args, void *argp)
	alias_(StartThreadArgs);

id_(8) int thbase_exit(void)
	alias_(ExitThread);
id_(9) int thbase_exit_delete(void)
	alias_(ExitDeleteThread);
id_(10) int thbase_terminate(int thid)
	alias_(TerminateThread);
id_(11) int thbase_iterminate(int thid)
	alias_(iTerminateThread);

id_(14) int thbase_change_priority(int thid, int priority)
	alias_(ChangeThreadPriority);
id_(15) int thbase_ichange_priority(int thid, int priority)
	alias_(iChangeThreadPriority);

id_(16) int thbase_rotate_ready_queue(int priority)
	alias_(RotateThreadReadyQueue);
id_(17) int thbase_irotate_ready_queue(int priority)
	alias_(iRotateThreadReadyQueue);

id_(18) int thbase_release_wait(int thid)
	alias_(ReleaseWaitThread);
id_(19) int thbase_irelease_wait(int thid)
	alias_(iReleaseWaitThread);

id_(20) int thbase_get_id(void)
	alias_(GetThreadId);
id_(21) int thbase_check_stack(void)
	alias_(CheckThreadStack);
id_(22) int thbase_refer_status(int thid, struct iop_thread_status *status)
	alias_(ReferThreadStatus);
id_(23) int thbase_irefer_status(int thid, struct iop_thread_status *status)
	alias_(iReferThreadStatus);

id_(24) int thbase_sleep(void)
	alias_(SleepThread);
id_(25) int thbase_wakeup(int thid)
	alias_(WakeupThread);
id_(26) int thbase_iwakeup(int thid)
	alias_(iWakeupThread);
id_(27) int thbase_cancel_wakeup(int thid)
	alias_(CancelWakeupThread);
id_(28) int thbase_icancel_wakeup(int thid)
	alias_(iCancelWakeupThread);

id_(33) int thbase_delay(int usec)
	alias_(DelayThread);

id_(34) int thbase_get_system_time(struct iop_sys_clock *sys_clock)
	alias_(GetSystemTime);
id_(35) int thbase_set_alarm(struct iop_sys_clock *sys_clock,
		unsigned int (*alarm_cb)(void *), void *arg)
	alias_(SetAlarm);
id_(36) int thbase_iset_alarm(struct iop_sys_clock *sys_clock,
		unsigned int (*alarm_cb)(void *), void *arg)
	alias_(iSetAlarm);
id_(37) int thbase_cancel_alarm(
		unsigned int (*alarm_cb)(void *), void *arg)
	alias_(CancelAlarm);
id_(38) int thbase_icancel_alarm(
		unsigned int (*alarm_cb)(void *), void *arg)
	alias_(iCancelAlarm);
id_(39) void thbase_us_to_sys_clock(u32 usec, struct iop_sys_clock *sys_clock)
	alias_(USec2SysClock);
id_(40) void thbase_sys_clock_to_us(struct iop_sys_clock *sys_clock,
		u32 *sec, u32 *usec)
	alias_(SysClock2USec);

id_(41) int thbase_get_system_status_flag(void)
	alias_(GetSystemStatusFlag);

LIBRARY_ID(thevent, 0x0101);

id_(4) int thevent_create_event_flag(struct iop_event *event)
	alias_(CreateEventFlag);
id_(5) int thevent_delete_event_flag(int ef)
	alias_(DeleteEventFlag);

id_(6) int thevent_set_event_flag(int ef, u32 bits)
	alias_(SetEventFlag);
id_(7) int thevent_iset_event_flag(int ef, u32 bits)
	alias_(iSetEventFlag);

id_(8) int thevent_clear_event_flag(int ef, u32 bits)
	alias_(ClearEventFlag);
id_(9) int thevent_iclear_event_flag(int ef, u32 bits)
	alias_(iClearEventFlag);

id_(10) int thevent_wait_event_flag(int ef, u32 bits, int mode, u32 *resbits)
	alias_(WaitEventFlag);

id_(11) int thevent_poll_event_flag(int ef, u32 bits, int mode, u32 *resbits)
	alias_(PollEventFlag);

id_(13) int thevent_refer_event_flag_status(int ef, struct iop_event_status* status)
	alias_(ReferEventFlagStatus);
id_(14) int thevent_irefer_event_flag_status(int ef, struct iop_event_status* status)
	alias_(iReferEventFlagStatus);

LIBRARY_ID(thsemap, 0x0101);

id_(4) int thsemap_create_sema(struct iop_sema *sema)
	alias_(CreateSema);
id_(5) int thsemap_delete_sema(int semid)
	alias_(DeleteSema);

id_(6) int thsemap_signal_sema(int semid)
	alias_(SignalSema);
id_(7) int thsemap_isignal_sema(int semid)
	alias_(iSignalSema);
id_(8) int thsemap_wait_sema(int semid)
	alias_(WaitSema);
id_(9) int thsemap_poll_sema(int semid)
	alias_(PollSema);

id_(11) int thsemap_refer_sema_status(int semid, struct iop_sema_status *status)
	alias_(ReferSemaStatus);
id_(12) int thsemap_irefer_sema_status(int semid, struct iop_sema_status *status)
	alias_(iReferSemaStatus);

LIBRARY_ID(thmsgbx, 0x0101);

id_(4) int thmsgbx_create_mbx(struct iop_mbx *mbx)
	alias_(CreateMbx);
id_(5) int thmsgbx_delete_mbx(int mbxid)
	alias_(DeleteMbx);

id_(6) int thmsgbx_send_mbx(int mbxid, void *msg)
	alias_(SendMbx);
id_(7) int thmsgbx_isend_mbx(int mbxid, void *msg)
	alias_(iSendMbx);
id_(8) int thmsgbx_receive_mbx(void **msgvar, int mbxid)
	alias_(ReceiveMbx);
id_(9) int thmsgbx_poll_mbx(void **msgvar, int mbxid)
	alias_(PollMbx);

id_(11) int thmsgbx_refer_mbxstatus(int mbxid, struct iop_mbx_status *status)
	alias_(ReferMbxStatus);
id_(12) int thmsgbx_irefer_mbx_status(int mbxid, struct iop_mbx_status *status)
	alias_(iReferMbxStatus);

LIBRARY_ID(thfpool, 0x0101);

id_(4) int thfpool_create_fpl(struct iop_fpl_param *param)
	alias_(CreateFpl);
id_(5) int thfpool_delete_fpl(int fplId)
	alias_(DeleteFpl);
id_(6) void *thfpool_allocate_fpl(int fplId)
	alias_(AllocateFpl);
id_(7) void *thfpool_pallocate_fpl(int fplId)
	alias_(pAllocateFpl);
id_(8) void *thfpool_ipallocate_fpl(int fplId)
	alias_(ipAllocateFpl);
id_(9) int thfpool_free_fpl(int fplId, void *memory)
	alias_(FreeFpl);
id_(11) int thfpool_refer_fpl_status(int fplId, struct iop_fpl_status *status)
	alias_(ReferFplStatus);
id_(12) int thfpool_irefer_fpl_status(int fplId, struct iop_fpl_status *status)
	alias_(iReferFplStatus);

LIBRARY_ID(thvpool, 0x0101);

id_(4) int thvpool_create_vpl(struct iop_vpl_param *param)
	alias_(CreateVpl);
id_(5) int thvpool_delete_vpl(int vplId)
	alias_(DeleteVpl);
id_(6) void *thvpool_allocate_vpl(int vplId, int size)
	alias_(AllocateVpl);
id_(7) void *thvpool_pallocate_vpl(int vplId, int size)
	alias_(pAllocateVpl);
id_(8) void *thvpool_ipallocate_vpl(int vplId, int size)
	alias_(ipAllocateVpl);
id_(9) int thvpool_free_vpl(int vplId, void *memory)
	alias_(FreeVpl);
id_(11) int thvpool_refer_vpl_status(int vplId, struct iop_vpl_status *status)
	alias_(ReferVplStatus);
id_(12) int thvpool_irefer_vpl_status(int vplId, struct iop_vpl_status *status)
	alias_(iReferVplStatus);

LIBRARY_ID(thrdman, 0x0102);
