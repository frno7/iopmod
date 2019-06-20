// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "iopmod/compare.h"
#include "iopmod/struct.h"

#include "iopmod/asm/macro.h"

#include "iopmod/tool/symtab.h"

#include "symtab.sym.h"

#define MODULE_PROLOGUE(module_name, module_version)			\
	{								\
		.module = {						\
			.name = #module_name,				\
			.version = module_version,			\
		},							\
		.library = {						\
			.name = NULL,					\
			.version = 0,					\
		},							\
		.function = {						\
			.name = NULL,					\
			.index = 0,					\
		},							\
	},
#define MODULE_EPILOGUE(module_name, module_version)

#define LIBRARY_PROLOGUE(						\
		module_name, module_version,				\
		library_name, library_version)				\
	{								\
		.module = {						\
			.name = #module_name,				\
			.version = module_version,			\
		},							\
		.library = {						\
			.name = #library_name,				\
			.version = library_version,			\
		},							\
		.function = {						\
			.name = NULL,					\
			.index = 0,					\
		},							\
	},

#define LIBRARY_EPILOGUE(						\
		module_name, module_version,				\
		library_name, library_version)

#define MODULE_FUNC(alias_,						\
		module_name, module_version,				\
		library_name, library_version,				\
		index_,	name_)				\
	{								\
		.module = {						\
			.name = #module_name,				\
			.version = module_version,			\
		},							\
		.library = {						\
			.name = #library_name,				\
			.version = library_version,			\
		},							\
		.function = {						\
			.name = #name_,					\
			.index = index_,				\
			.alias = alias_,				\
		},							\
	},

#define MODULE_FUNC_PROLOGUE(						\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)				\
	MODULE_FUNC(false,						\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)

#define MODULE_FUNC_ALIAS(						\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)				\
	MODULE_FUNC(true,						\
		module_name, module_version,				\
		library_name, library_version,				\
		index_,	name_)

#define MODULE_UNDF(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_)

#define MODULE_FUNC_EPILOGUE(						\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)

static const struct module_func_symbol symtab[] = {
#if defined(SYMTAB)
	SYMTAB(MODULE_PROLOGUE, MODULE_EPILOGUE,
	       LIBRARY_PROLOGUE, LIBRARY_EPILOGUE,
	       MODULE_FUNC_PROLOGUE,
	       MODULE_FUNC_ALIAS,
	       MODULE_FUNC_EPILOGUE,
	       MODULE_UNDF)
#endif /* defined(SYMTAB) */
	{ }
};

const struct module_func_symbol *symtab_first_symbol(void)
{
	return &symtab[0];
}

const struct module_func_symbol *symtab_next_symbol(
	const struct module_func_symbol *symbol)
{
	return symbol->module.name ? &symbol[1] : symbol;
}

int symtab_module_count(void)
{
	const struct symtab_module *module;
	int n = 0;

	symtab_for_each_module (module)
		n++;

	return n;
}

int symtab_library_count(void)
{
	const struct symtab_library *library;
	int n = 0;

	symtab_for_each_library (library)
		n++;

	return n;
}

const struct module_func_symbol *symtab_symbol_for_module(
	const struct symtab_module *module)
{
	return container_of(module, struct module_func_symbol, module);
}

const struct module_func_symbol *symtab_symbol_for_library(
	const struct symtab_library *library)
{
	return container_of(library, struct module_func_symbol, library);
}

const struct module_func_symbol *symtab_symbol_for_function(
	const struct symtab_function *function)
{
	return container_of(function, struct module_func_symbol, function);
}

const struct symtab_module *symtab_module_for_library(
	const struct symtab_library *library)
{
	return &symtab_symbol_for_library(library)->module;
}

const struct symtab_module *symtab_first_module(void)
{
	return &symtab_first_symbol()->module;
}

static bool symtab_end(const struct module_func_symbol *symbol)
{
	return !symbol->module.name;
}

static bool symtab_library_end(const struct module_func_symbol *symbol)
{
	return symtab_end(symbol) || !symbol->library.name;
}

static bool symtab_function_end(const struct module_func_symbol *symbol)
{
	return symtab_end(symbol) || !symbol->function.name;
}

const struct symtab_module *symtab_next_module(
	const struct symtab_module *module)
{
	const struct module_func_symbol *symbol =
		&symtab_symbol_for_module(module)[1];

	while (!symtab_end(symbol) && symbol->library.name)
		symbol++;

	return &symtab_next_symbol(symbol)->module;
}

const struct symtab_library *symtab_first_library(void)
{
	const struct module_func_symbol *symbol = symtab_first_symbol();

	while (!symtab_end(symbol) && !symbol->library.name)
		symbol++;

	return &symbol->library;
}

const struct symtab_library *symtab_next_library(
	const struct symtab_library *library)
{
	const struct module_func_symbol *symbol =
		symtab_symbol_for_library(library);

	if (!symtab_library_end(symbol))
		symbol++;

	while (!symtab_end(symbol) &&
			(!symbol->library.name || symbol->function.name))
		symbol++;

	return &symbol->library;
}

const struct symtab_function *symtab_first_function(
	const struct symtab_library *library)
{
	return &symtab_symbol_for_library(library)[1].function;
}

const struct symtab_function *symtab_next_function(
	const struct symtab_function *function)
{
	const struct module_func_symbol *symbol =
		symtab_symbol_for_function(function);

	if (!symtab_function_end(symbol))
		symbol++;

	while (!symtab_function_end(symbol) && symbol->function.alias)
		symbol++;

	return &symbol->function;
}

const struct symtab_function *symtab_first_alias(
	const struct symtab_function *function)
{
	const struct module_func_symbol *symbol =
		symtab_symbol_for_function(function);

	if (symtab_function_end(symbol))
		return &symbol->function;

	return &symbol[symbol[1].function.alias ? 1 : 0].function;
}

const struct symtab_function *symtab_next_alias(
	const struct symtab_function *alias)
{
	const struct module_func_symbol *symbol =
		symtab_symbol_for_function(alias);

	if (symtab_function_end(symbol))
		return &symbol->function;

	return &symbol[1].function;
}

static int module_name_cmp(const void *a__, const void *b__)
{
	const struct symtab_module * const *a_ = a__, *a = *a_;
	const struct symtab_module * const *b_ = b__, *b = *b_;

	return strcasecmp(a->name, b->name);
}

static void order_modules(const struct symtab_module *modules[], int n)
{
	qsort(modules, n, sizeof(*modules), module_name_cmp);
}

bool symtab_iterate_module_ordered(
	bool (*cb)(const struct symtab_module *module, void *arg), void *arg)
{
	const struct symtab_module *module;
	const int n = symtab_module_count();
	const struct symtab_module *modules[n];
	int i = 0;

	symtab_for_each_module (module)
		modules[i++] = module;

	order_modules(modules, n);

	for (i = 0; i < n; i++)
		if (!cb(modules[i], arg))
			return false;

	return true;
}

static int library_name_cmp(const void *a__, const void *b__)
{
	const struct symtab_library * const *a_ = a__, *a = *a_;
	const struct symtab_library * const *b_ = b__, *b = *b_;

	return strcasecmp(a->name, b->name);
}

static void order_libraries(const struct symtab_library *libraries[], int n)
{
	qsort(libraries, n, sizeof(*libraries), library_name_cmp);
}

bool symtab_iterate_library_ordered(
	bool (*cb)(const struct symtab_library *library, void *arg), void *arg)
{
	const struct symtab_library *library;
	const int n = symtab_library_count();
	const struct symtab_library *libraries[n];
	int i = 0;

	symtab_for_each_library (library)
		libraries[i++] = library;

	order_libraries(libraries, n);

	for (i = 0; i < n; i++)
		if (!cb(libraries[i], arg))
			return false;

	return true;
}

struct symtab_formating symtab_formating(void)
{
	const struct symtab_library *library;
	struct symtab_formating format = { };

	symtab_for_each_library (library) {
		const struct symtab_module *module =
			symtab_module_for_library(library);

		format.library_name_width = max_t(int,
			format.library_name_width, strlen(library->name));
		format.module_name_width = max_t(int,
			format.module_name_width, (int)strlen(module->name));
	}

	return format;
}

const struct symtab_library *symtab_library_for_name(const char *name)
{
	const struct symtab_library *library;

	symtab_for_each_library (library)
		if (strcmp(library->name, name) == 0)
			return library;

	return NULL;
}

const struct module_func_symbol *func_symbol_from_index(
	const char *name, const int index)
{
	const struct symtab_library *library = symtab_library_for_name(name);
	const struct symtab_function *function;

	if (library)
		symtab_for_each_library_function (function, library)
			if (function->index == index)
				return symtab_symbol_for_function(function);

	return NULL;
}
