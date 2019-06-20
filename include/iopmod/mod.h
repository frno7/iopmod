// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include "iopmod/irx.h"
#include "iopmod/module-init.h"

#include "iopmod/loadcore.h"

#include "__MODULE_NAME__.sym.h"

enum module_init_status _module_start(int argc, char *argv[])
	__attribute__((weak))
	__attribute__((used))
	__attribute__((section(".module.start")));

enum module_init_status _module_start(int argc, char *argv[])
{
	return MODULE_RESIDENT;
}

enum module_init_status _module_init(int argc, char *argv[])
	__attribute__((section(".module.init")));

#if defined(SYMTAB_EXPORT)

extern struct irx_export_library _module_export[];

enum module_init_status _module_init(int argc, char *argv[])
{
	int err = loadcore_register_library(_module_export);
	if (err < 0)
		return MODULE_EXIT;

	enum module_init_status status = _module_start(argc, argv);

	if (status == MODULE_EXIT)
		loadcore_release_library(_module_export);

	return status;
}

#else

enum module_init_status _module_init(int argc, char *argv[])
{
	return _module_start(argc, argv);
}

#endif /* defined(SYMTAB_EXPORT) */
