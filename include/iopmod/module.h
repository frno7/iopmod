// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#ifndef IOPMOD_MODULE_H
#define IOPMOD_MODULE_H

#include "iopmod/module-init.h"
#include "iopmod/module-prototype.h"
#include "iopmod/types.h"

/**
 * module_init - module entry point
 * @initfn: function to be run at module insertion
 */
#define module_init(initfn)						\
	static enum module_init_status initfn(int argc, char *argv[])	\
		__attribute__((used))					\
		__attribute__((section(".module.start")));		\
	extern enum module_init_status					\
		_module_start(int argc, char *argv[])			\
		__attribute__((alias(#initfn)))				\

enum module_init_status _module_start(int argc, char *argv[]);

#endif /* IOPMOD_MODULE_H */
