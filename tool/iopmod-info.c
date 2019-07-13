// SPDX-License-Identifier: GPL-2.0
/*
 * Print input/output processor (IOP) module specific information
 *
 * IOP modules are ELF, so most information is available from standard tools
 * such as readelf and objdump. This tool prints the .iopmod section and the
 * import and export libraries that are specific to IOP modules.
 *
 * Copyright (C) 2019 Fredrik Noring
 */

#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <stdio.h>
#include <stdlib.h>

#include "iopmod/tool/file.h"
#include "iopmod/tool/irx.h"
#include "iopmod/tool/print.h"
#include "iopmod/tool/symtab.h"
#include "iopmod/tool/tool.h"

#include "iopmod/asm/macro.h"

#include "iopmod/types.h"
#include "iopmod/version.h"

char progname[] = "iopmod-info";

/*
 * FIXME: iopdep tool
 * FIXME: lexer-c.c
 */

static struct {
	int identify;

	int list_modules;
	int list_libraries;
	int list_functions;

	const char *filepath;
} option;

static void show_iopmod(Elf_Ehdr *ehdr)
{
	struct irx_iopmod *iopmod = irx_iopmod(ehdr);
	struct irx_iopmod_id iopmod_id = irx_iopmod_id(ehdr);

	if (!iopmod)	/* FIXME: done in iop_valid */
		return pr_warn("%s: Missing .iopmod section", option.filepath);

	pr_info("iopmod id addr		0x%x\n"
		"iopmod id name		%s\n"
		"iopmod id version	0x%04x\n"
		"iopmod entry addr	0x%x\n"
		"iopmod unknown addr	0x%x\n"
		"iopmod text size	%u\n"
		"iopmod data size	%u\n"
		"iopmod bss size		%u\n"
		"iopmod version		0x%04x\n"
		"iopmod name		%s",
		iopmod->id_addr,
		iopmod_id.name,
		iopmod_id.version,
		iopmod->entry_addr,
		iopmod->unknown,
		iopmod->text_size,
		iopmod->data_size,
		iopmod->bss_size,
		iopmod->version,
		iopmod->name);
}

static void show_import_links(
	struct irx_import_library *library, Elf_Ehdr *ehdr)
{
	struct irx_import_link *link;
	int i = 0;

	irx_for_each_import_link (link, library) {
		const int link_id = irx_id_for_import_link(link);
		const struct module_func_symbol *decl =
			func_symbol_from_index(library->name, link_id);
		const Elf_Addr link_addr = elf_addr_for_ent(link, ehdr);
		const char *name = elf_symbol_for_addr(link_addr, ehdr);

		/* FIXME: Deduplication of names */

		pr_info("import %2d\t%2d\t%s\t%s",
			i++,
			link_id,
			name ? name : "",
			decl ? decl->function.name : "");
	}
}

static void show_import_libraries(Elf_Ehdr *ehdr)
{
	struct irx_import_library *library;

	irx_for_each_import_library (library, ehdr) {
		pr_info("import library\t0x%04x\t%.8s",
			library->version, library->name);

		show_import_links(library, ehdr);
	}
}

static void show_export_links(
	struct irx_export_library *library, Elf_Ehdr *ehdr)
{
	struct irx_iopmod *iopmod = irx_iopmod(ehdr);
	struct irx_export_link *link;
	int i = 0;

	irx_for_each_export_link (link, library) {
		const struct module_func_symbol *decl =
			func_symbol_from_index(library->name, i);
		const char *name = "";

		if (decl)
			name = decl->function.name;
		else if (link->offset == iopmod->entry_addr)
			name = "_init_module";
		else
			name = "";

		pr_info("export %2d\t0x%04x\t%s",
			i, link->offset, name);

		i++;
	}
}

static void show_export_libraries(Elf_Ehdr *ehdr)
{
	struct irx_export_library *library;

	irx_for_each_export_library (library, ehdr) {
		pr_info("export library\t0x%04x\t%.8s",
			library->version, library->name);

		show_export_links(library, ehdr);
	}
}

static void show_info(Elf_Ehdr *ehdr)
{
	show_iopmod(ehdr);
	show_import_libraries(ehdr);
	show_export_libraries(ehdr);
}

static bool print_module(const struct symtab_module *module, void *arg)
{
	const struct symtab_formating *format = arg;

	printf("%*s 0x%04x\n",
		format->module_name_width,
		module->name,
		module->version);

	return true;
}

static void list_modules_exit(void)
{
	struct symtab_formating format = symtab_formating();

	symtab_iterate_module_ordered(print_module, &format);

	exit(EXIT_SUCCESS);
}

static bool print_library(const struct symtab_library *library, void *arg)
{
	const struct symtab_module *module = symtab_module_for_library(library);
	const struct symtab_formating *format = arg;

	if (option.list_modules)
		printf("%*s 0x%04x ",
			format->module_name_width,
			module->name,
			module->version);

	printf("%*s 0x%04x\n",
		format->library_name_width,
		library->name,
		library->version);

	return true;
}

static void list_libraries_exit(void)
{
	struct symtab_formating format = symtab_formating();

	symtab_iterate_library_ordered(print_library, &format);

	exit(EXIT_SUCCESS);
}

static bool print_functions(const struct symtab_library *library, void *arg)
{
	const struct symtab_module *module = symtab_module_for_library(library);
	const struct symtab_formating *format = arg;
	const struct symtab_function *function;

	symtab_for_each_library_function (function, library) {
		const struct symtab_function *alias;

		if (option.list_modules)
			printf("%*s 0x%04x ",
				format->module_name_width,
				module->name,
				module->version);

		if (option.list_libraries)
			printf("%*s 0x%04x ",
				format->library_name_width,
				library->name,
				library->version);

		printf("%2d %s",
				function->index,
				function->name);

		symtab_for_each_function_alias (alias, function)
			printf(" %s", alias->name);

		printf("\n");
	}

	return true;
}

static void list_functions_exit(void)
{
	struct symtab_formating format = symtab_formating();

	symtab_iterate_library_ordered(print_functions, &format);

	exit(EXIT_SUCCESS);
}

static void help(FILE *file)
{
	fprintf(file,
"usage: %s [option]... [infile]\n"
"\n"
"options:\n"
"    -h, --help              display this help and exit\n"
"    --version               display version and exit\n"
"\n"
"    --identify              exit sucessfully if the file is an IOP module\n"
"\n"
"the following list options can be combined:\n"
"    --list-modules          list recognised IOP modules and exit\n"
"    --list-libraries        list recognised IOP libraries and exit\n"
"    --list-functions        list recognised IOP functions and exit\n"
"\n",
		progname);
}

static void NORETURN help_exit(void)
{
	help(stdout);
	exit(EXIT_SUCCESS);
}

static void NORETURN version_exit(void)
{
	printf("%s version %s\n", progname, program_version());

	exit(EXIT_SUCCESS);
}

static void parse_options(int argc, char **argv)
{
	static const struct option options[] = {
		{ "help",           no_argument, NULL,                   0 },
		{ "version",        no_argument, NULL,                   0 },
		{ "identify",       no_argument, &option.identify,       1 },
		{ "list-modules",   no_argument, &option.list_modules,   1 },
		{ "list-libraries", no_argument, &option.list_libraries, 1 },
		{ "list-functions", no_argument, &option.list_functions, 1 },
		{ NULL, 0, NULL, 0 }
	};

	argv[0] = progname;	/* For better getopt_long error messages. */

#define OPT(option) (strcmp(options[index].name, (option)) == 0)
	for (;;) {
		int index = 0;

		switch (getopt_long(argc, argv, "h", options, &index)) {
		case -1:
			goto out;

		case 0:
			if (OPT("help"))
				help_exit();
			else if (OPT("version"))
				version_exit();
			break;

		case 'h':
			help_exit();

		case '?':
			exit(EXIT_FAILURE);
		}
	}
#undef OPT

out:
	if (option.list_functions)
		list_functions_exit();
	if (option.list_libraries)
		list_libraries_exit();
	if (option.list_modules)
		list_modules_exit();

	if (argc < optind + 1)
		pr_fatal_error("no input file");
	if (argc > optind + 1)
		pr_fatal_error("too many input files");
	option.filepath = argv[optind];
}

int main(int argc, char **argv)
{
	int status = EXIT_FAILURE;

	parse_options(argc, argv);

	const struct file f = file_read(option.filepath);
	if (!file_valid(f))
		pr_fatal_errno(option.filepath);

	if (option.identify) {
		if (irx_identify(f.data, f.size))
			status = EXIT_SUCCESS;
		goto out;
	}

	if (!irx_identify(f.data, f.size))
		pr_fatal_error("%s: Invalid IRX file", option.filepath);

	show_info(f.data);

	status = EXIT_SUCCESS;
out:
	file_free(f);

	return status;
}
