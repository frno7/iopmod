// SPDX-License-Identifier: GPL-2.0
/*
 * Input/output processor (IOP) module symbol table
 *
 * Copyright (C) 2019 Fredrik Noring
 */

#ifndef IOPMOD_TOOL_SYMTAB_H
#define IOPMOD_TOOL_SYMTAB_H

#include <stdbool.h>

/**
 * struct symtab_module - module entry in symbol table
 * @name: module name
 * @version: module version in BCD format
 */
struct symtab_module {
	const char *name;
	int version;
};

/**
 * struct symtab_library - module library entry in symbol table
 * @name: library name
 * @version: library version in BCD format
 */
struct symtab_library {
	const char *name;
	int version;
};

/**
 * struct symtab_function - library function entry in symbol table
 * @name: function name
 * @index: index of function in library export table, starting from 0
 * @alias: %true if function is an alias, otherwise %false
 */
struct symtab_function {
	const char *name;
	int index;
	bool alias;
};

/** FIXME */
struct module_func_symbol {
	struct symtab_module module;
	struct symtab_library library;
	struct symtab_function function;
};

/** FIXME */
const struct module_func_symbol *func_symbol_from_index(
	const char *library, const int index);

/** FIXME */
const struct module_func_symbol *func_first_alias_from_index(
	const char *name, const int index);

#define symtab_for_each_symbol(symbol)					\
	for ((symbol) = symtab_first_symbol();				\
	     (symbol)->module.name;					\
	     (symbol) = symtab_next_symbol(symbol))

const struct module_func_symbol *symtab_first_symbol(void);
const struct module_func_symbol *symtab_next_symbol(
	const struct module_func_symbol *symbol);

#define symtab_for_each_module(module)					\
	for ((module) = symtab_first_module();				\
	     (module)->name;						\
	     (module) = symtab_next_module(module))

const struct symtab_module *symtab_first_module(void);
const struct symtab_module *symtab_next_module(
	const struct symtab_module *module);

#define symtab_for_each_library(library)				\
	for ((library) = symtab_first_library();			\
	     (library)->name;						\
	     (library) = symtab_next_library(library))

const struct symtab_library *symtab_first_library(void);
const struct symtab_library *symtab_next_library(
	const struct symtab_library *library);

#define symtab_for_each_library_function(function, library)		\
	for ((function) = symtab_first_function(library);		\
	     (function)->name;						\
	     (function) = symtab_next_function(function))

const struct symtab_function *symtab_first_function(
	const struct symtab_library *library);
const struct symtab_function *symtab_next_function(
	const struct symtab_function *function);

#define symtab_for_each_function_alias(alias_, function)		\
	for ((alias_) = symtab_first_alias(function);			\
	     (alias_)->alias;		/* FIXME: etc. */		\
	     (alias_) = symtab_next_alias(alias_))

const struct symtab_function *symtab_first_alias(
	const struct symtab_function *function);
const struct symtab_function *symtab_next_alias(
	const struct symtab_function *alias);

const struct module_func_symbol *symtab_symbol_for_library(
	const struct symtab_library *library);
const struct symtab_module *symtab_module_for_library(
	const struct symtab_library *library);

int symtab_module_count(void);
int symtab_library_count(void);

bool symtab_iterate_module_ordered(
	bool (*cb)(const struct symtab_module *module, void *arg), void *arg);

bool symtab_iterate_library_ordered(
	bool (*cb)(const struct symtab_library *library, void *arg), void *arg);

struct symtab_formating {
	int library_name_width;
	int module_name_width;
};

struct symtab_formating symtab_formating(void);

#endif /* IOPMOD_TOOL_SYMTAB_H */
