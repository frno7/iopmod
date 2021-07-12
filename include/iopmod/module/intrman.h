// SPDX-License-Identifier: GPL-2.0

MODULE_ID(Interrupt_Manager, 0x0102);
LIBRARY_ID(intrman, 0x0102);

id_(4) enum irq_status intrman_request_irq(unsigned int irq,
		int mode, irq_handler_t handler, void *arg)
	alias_(RegisterIntrHandler);
id_(5) int intrman_release_irq(unsigned int irq)
	alias_(ReleaseIntrHandler);

id_(6) int intrman_enable_irq(unsigned int irq)
	alias_(EnableIntr);

id_(7) int intrman_disable_irq(unsigned int irq, int *res)
	alias_(DisableIntr);

id_(8) int intrman_cpu_disable_irqs(void)
	alias_(CpuDisableIntr);
id_(9) int intrman_cpu_enable_irqs(void)
	alias_(CpuEnableIntr);

id_(14) int intrman_cpu_invoke_in_kmode(void *function, ...)
	alias_(CpuInvokeInKmode);
id_(15) void intrman_disable_dispatch(unsigned int irq)
	alias_(DisableDispatchIntr);
id_(16) void intrman_enable_dispatch(unsigned int irq)
	alias_(EnableDispatchIntr);

id_(17) int intrman_cpu_suspend_irq(unsigned int *flags)
	alias_(CpuSuspendIntr);
id_(18) int intrman_cpu_resume_irq(unsigned int flags)
	alias_(CpuResumeIntr);

id_(23) int intrman_in_irq(void)
	alias_(QueryIntrContext);

id_(24) int intrman_query_stack(void *sp)
	alias_(QueryIntrStack);
id_(25) int intrman_icatch_multi(void)
	alias_(iCatchMultiIntr);
