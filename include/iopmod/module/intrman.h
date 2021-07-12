// SPDX-License-Identifier: GPL-2.0

MODULE_ID(Interrupt_Manager, 0x0102);
LIBRARY_ID(intrman, 0x0102);

/**
 * intrman_request_irq - allocate an intrman interrupt line
 * @irq: interrupt line to allocate
 * @mode: registers saved and restored during an interrupt
 * @handler: function to be called back when the IRQ occurs
 * @arg: optional argument passed back to the callback function, can be %NULL
 *
 * Drivers should use request_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Context: thread
 * Return: 0 on success, nonzero IOP error otherwise
 */
id_(4) enum irq_status intrman_request_irq(unsigned int irq,
		enum irq_mode mode, irq_handler_t handler, void *arg)
	alias_(RegisterIntrHandler);

/**
 * intrman_release_irq - disable and free an allocated intrman interrupt
 * @irq: interrupt line to release
 *
 * Drivers should use release_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Context: thread
 * Return: 0 on success, nonzero IOP error otherwise
 */
id_(5) int intrman_release_irq(unsigned int irq)
	alias_(ReleaseIntrHandler);

/**
 * intrman_enable_irq - enable handling of the selected intrman interrupt line
 * @irq: interrupt to enable
 *
 * Drivers should use enable_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Note that intrman_enable_irq() and intrman_disable_irq() do not nest, so
 * intrman_enable_irq() will enable regardless of the number of previous calls
 * to intrman_disable_irq().
 *
 * Context: any
 * Return: 0 on success, nonzero IOP error otherwise
 */
id_(6) int intrman_enable_irq(unsigned int irq)
	alias_(EnableIntr);

/**
 * intrman_disable_irq - disable the selected intrman interrupt line
 * @irq: interrupt to disable
 * @res: pointer to store disabled interrupt number, ignored if %NULL
 *
 * Drivers should use disable_irq() instead. This function is used by the main
 * interrupt controller.
 *
 * Note that intrman_enable_irq() and intrman_disable_irq() do not nest, so
 * intrman_enable_irq() will enable regardless of the number of previous calls
 * to intrman_disable_irq().
 *
 * Context: any
 * Return: 0 on success, nonzero IOP error otherwise
 */
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

/**
 * intrman_in_irq - are we in an interrupt context?
 *
 * Drivers should use in_irq() instead.
 *
 * Context: any
 * Return: %true for interrupt context, %false otherwise
 */
id_(23) int intrman_in_irq(void)
	alias_(QueryIntrContext);

id_(24) int intrman_query_stack(void *sp)
	alias_(QueryIntrStack);
id_(25) int intrman_icatch_multi(void)
	alias_(iCatchMultiIntr);
