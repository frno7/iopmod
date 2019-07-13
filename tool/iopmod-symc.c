// SPDX-License-Identifier: GPL-2.0
/*
 * Generate symbol tables for C header files
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
#include "iopmod/tool/lexc.h"
#include "iopmod/tool/print.h"
#include "iopmod/tool/string.h"
#include "iopmod/tool/tool.h"

#include "iopmod/asm/macro.h"

#include "iopmod/compare.h"
#include "iopmod/types.h"
#include "iopmod/version.h"

char progname[] = "iopmod-symc";

static struct {
	int module;

	const char *outfile;
	char *tmpfile;
} option;

#define for_each_declaration(token, s)					\
	for ((token) = lexc_first_token((s), NULL);			\
	     (token).type != TOKEN_EMPTY;				\
	     (token) = next_declaration_token(token))

#define for_each_token_in_declaration(token, declaration)		\
	for ((token) = (declaration);					\
	     (token).type != TOKEN_EMPTY &&				\
	     (token).type != TOKEN_SEMICOLON;				\
	     (token) = lexc_next_token(token))

static struct lexc_token next_declaration_token(struct lexc_token token)
{
	lexc_for_each_token_after (token)
		if (token.type == TOKEN_SEMICOLON)
			return lexc_next_token(token);

	return token;
}

struct function {
	struct lexc_token name;
	unsigned int index;

	struct {
		size_t count;
		struct lexc_token *entry;
	} alias;
};

struct library {
	struct lexc_token name;
	struct lexc_token version;

	struct {
		size_t count;
		struct function *entry;
	} function;
};

struct module {
	struct lexc_token name;
	struct lexc_token version;

	struct {
		size_t count;
		struct library *entry;
	} library;
};

struct symtab {
	struct {
		size_t count;
		struct module *entry;
	} module;
};

static long token_integer(struct lexc_token token)
{
	if (!token.length || token.length > 16)
		pr_fatal_error("Malformed integer \"%.*s\"\n",
			(int)token.length, token.string);

	char buffer[token.length + 1];
	char *e;

	memcpy(buffer, token.string, token.length);
	buffer[token.length] = '\0';

	long n = strtol(buffer, &e, 0);

	if (e != &buffer[token.length])
		pr_fatal_error("Malformed integer \"%.*s\"\n",
			(int)token.length, token.string);

	return n;
}

static bool vtscanf(struct lexc_token token, const char *fmt, va_list ap)
{
	struct lexc_token fmt_token;

	lexc_for_each_token (fmt_token, fmt) {
		if (fmt_token.type == TOKEN_MOD) {
			fmt_token = lexc_next_token(fmt_token);

			if (fmt_token.type != TOKEN_IDENTIFIER ||
			    fmt_token.length != 1)
				return false;		/* Malformed format */

			if (fmt_token.string[0] == 's') {
				*va_arg(ap, struct lexc_token *) = token;
			} else if (fmt_token.string[0] == 'i') {
				if (token.type != TOKEN_INTEGER)
					return false;

				*va_arg(ap, struct lexc_token *) = token;
			} else
				return false;		/* Malformed format */
		} else if (!lexc_equal_token(fmt_token, token))
			return false;

		token = lexc_next_token(token);
	}

	return true;
}

static bool tscanf(struct lexc_token token, const char *fmt, ...)
{
	va_list ap;
	bool match;

	va_start(ap, fmt);
	match = vtscanf(token, fmt, ap);
	va_end(ap);

	return match;
}

static void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);

	if (!ptr)
		pr_fatal_errno("realloc");

	return ptr;
}

static struct function *current_function(struct library *library)
{
	return !library->function.count ? NULL :
	       &library->function.entry[library->function.count - 1];
}

static struct library *current_library(struct module *module)
{
	return !module->library.count ? NULL :
	       &module->library.entry[module->library.count - 1];
}

static struct module *current_module(struct symtab *symtab)
{
	return !symtab->module.count ? NULL :
	       &symtab->module.entry[symtab->module.count - 1];
}

static void function_alias(struct function *function, struct lexc_token alias)
{
	function->alias.count++;
	function->alias.entry = xrealloc(function->alias.entry,
		function->alias.count * sizeof(*function->alias.entry));

	function->alias.entry[function->alias.count - 1] = alias;
}

static void library_function(struct library *library,
	struct lexc_token name, unsigned int index)
{
	library->function.count++;
	library->function.entry = xrealloc(library->function.entry,
		library->function.count * sizeof(*library->function.entry));

	*current_function(library) = (struct function) {
		.name = name,
		.index = index,
	};
}

static void module_library(struct module *module,
	struct lexc_token name, struct lexc_token version)
{
	module->library.count++;
	module->library.entry = xrealloc(module->library.entry,
		module->library.count * sizeof(*module->library.entry));

	*current_library(module) = (struct library) {
		.name = name,
		.version = version,
	};
}

static void symtab_module(struct symtab *symtab,
	struct lexc_token name, struct lexc_token version)
{
	symtab->module.count++;
	symtab->module.entry = xrealloc(symtab->module.entry,
		symtab->module.count * sizeof(*symtab->module.entry));

	*current_module(symtab) = (struct module) {
		.name = name,
		.version = version,
	};
}

static bool read_function(struct library *library,
	struct lexc_token declaration)
{
	struct lexc_token token, name = { }, index = { }, alias = { };

	for_each_token_in_declaration (token, declaration)
		if (tscanf(token, "id_(%i)", &index))
			break;

	if (!index.length)
		return false;

	for_each_token_in_declaration (token, declaration)
		if (tscanf(token, "id_(") || tscanf(token, "alias_("))
			continue;
		else if (tscanf(token, "%s(", &name))
			break;

	if (!name.length)
		return false;

	library_function(library, name, token_integer(index));

	for_each_token_in_declaration (token, declaration)
		if (tscanf(token, "alias_(%s)", &alias))
			function_alias(current_function(library), alias);

	return true;
}

static bool read_declaration(struct symtab *symtab,
	struct lexc_token declaration)
{
	struct lexc_token token;

	for_each_token_in_declaration (token, declaration) {
		struct lexc_token name, version;

		if (tscanf(token, "MODULE_ID(%s, %i)", &name, &version)) {
			symtab_module(symtab, name, version);
			return true;
		}

		struct module *module = current_module(symtab);
		if (!module)
			continue;

		if (tscanf(token, "LIBRARY_ID(%s, %i)", &name, &version)) {
			module_library(module, name, version);
			return true;
		}
	}

	struct module *module = current_module(symtab);
	if (!module)
		return false;

	struct library *library = current_library(module);
	if (!library)
		return false;

	read_function(library, declaration);

	return true;
}

static void read_symtab(struct symtab *symtab, struct file f)
{
	struct lexc_token declaration;

	for_each_declaration (declaration, f.data)
		read_declaration(symtab, declaration);
}

static void print_undefined(FILE *file, const struct function *function,
	const struct library *library, const struct module *module)
{
	fprintf(file, "\tFU(%.*s, %.*s, %.*s, %.*s, %u) \\\n",
		(int)module->name.length, module->name.string,
		(int)module->version.length, module->version.string,
		(int)library->name.length, library->name.string,
		(int)library->version.length, library->version.string,
		function->index);
}

static void print_alias(FILE *file,
	const struct lexc_token alias, const struct function *function,
	const struct library *library, const struct module *module)
{
	fprintf(file, "\tFA(%.*s, %.*s, %.*s, %.*s, %u, %.*s) \\\n",
		(int)module->name.length, module->name.string,
		(int)module->version.length, module->version.string,
		(int)library->name.length, library->name.string,
		(int)library->version.length, library->version.string,
		function->index,
		(int)alias.length, alias.string);
}

static void print_function_statement(FILE *file, const char *prefix,
	const struct function *function, const struct library *library,
	const struct module *module)
{
	fprintf(file, "\t%s(%.*s, %.*s, %.*s, %.*s, %u, %.*s) \\\n",
		prefix,
		(int)module->name.length, module->name.string,
		(int)module->version.length, module->version.string,
		(int)library->name.length, library->name.string,
		(int)library->version.length, library->version.string,
		function->index,
		(int)function->name.length, function->name.string);
}

static void print_library_statement(FILE *file, const char *prefix,
	const struct library *library, const struct module *module)
{
	fprintf(file, "\t%s(%.*s, %.*s, %.*s, %.*s) \\\n", prefix,
		(int)module->name.length, module->name.string,
		(int)module->version.length, module->version.string,
		(int)library->name.length, library->name.string,
		(int)library->version.length, library->version.string);
}

static void print_module_statement(FILE *file, const char *prefix,
	const struct module *module, bool last)
{
	fprintf(file, "\t%s(%.*s, %.*s)%s\n", prefix,
		(int)module->name.length, module->name.string,
		(int)module->version.length, module->version.string,
		last ? "" : " \\");
}

static void print_function(FILE *file, const struct function *function,
	const struct library *library, const struct module *module)
{
	if (!function->name.length)
		return print_undefined(file, function, library, module);

	print_function_statement(file, "FP", function, library, module);

	for (size_t i = 0; i < function->alias.count; i++)
		print_alias(file, function->alias.entry[i],
			function, library, module);

	print_function_statement(file, "FE", function, library, module);
}

static void print_library(FILE *file, const struct library *library,
	const struct module *module)
{
	print_library_statement(file, "LP", library, module);

	for (size_t i = 0; i < library->function.count; i++)
		print_function(file, &library->function.entry[i], library, module);

	print_library_statement(file, "LE", library, module);
}

static void print_module(FILE *file,
	const struct module *module, bool last)
{
	print_module_statement(file, "MP", module, false);

	for (size_t i = 0; i < module->library.count; i++)
		print_library(file, &module->library.entry[i], module);

	print_module_statement(file, "ME", module, last);
}

static bool module_match(const struct module *module)
{
	if (!option.outfile)
		pr_fatal_error("Missing module output file\n");

	const size_t i = basename_index(option.outfile);
	const size_t e = baseextension_index(option.outfile);
	const size_t length = e - i;

	if (module->name.length != length)
		return false;

	return strncmp(&option.outfile[i], module->name.string, length) == 0;
}

static void print_module_id(FILE *file, const struct module *module)
{
	fprintf(file,
		"#define SYMTAB_MODULE_ID(MODULE_ID) MODULE_ID(%.*s, %.*s)\n",
		(int)module->name.length, module->name.string,
		(int)module->version.length, module->version.string);
}

static void print_module_export(FILE *file, const struct module *module)
{
	if (!module->library.count)
		return;

	fprintf(file, "#define SYMTAB_EXPORT(MP, ME, LP, LE, FP, FA, FE, FU) \\\n");

	print_module(file, module, true);
}

static void print_symtab_module(FILE *file, const struct symtab *symtab)
{
	for (size_t i = 0; i < symtab->module.count; i++) {
		const struct module *module = &symtab->module.entry[i];

		if (module_match(module)) {
			print_module_id(file, module);
			print_module_export(file, module);
			break;
		}
	}

	ssize_t first = -1, last = -1;

	for (size_t i = 0; i < symtab->module.count; i++)
		if (!module_match(&symtab->module.entry[i])) {
			if (first < 0)
				first = i;
			last = i;
		}

	for (size_t i = 0; i < symtab->module.count; i++) {
		const struct module *module = &symtab->module.entry[i];

		if (!module_match(module)) {
			if (i == first)
				fprintf(file, "#define SYMTAB_IMPORT(MP, ME, LP, LE, FP, FA, FE, FU) \\\n");

			print_module(file, module, i == last);
		}
	}
}

static void print_symtab(FILE *file, const struct symtab *symtab)
{
	for (size_t i = 0; i < symtab->module.count; i++) {
		if (!i)
			fprintf(file, "#define SYMTAB(MP, ME, LP, LE, FP, FA, FE, FU) \\\n");

		print_module(file, &symtab->module.entry[i],
			i + 1 == symtab->module.count);
	}
}

static void write_symtab(const struct symtab *symtab)
{
	FILE *file = stdout;

	if (option.outfile) {
		option.tmpfile = strdupcat(option.outfile, ".tmp");

		file = fopen(option.tmpfile, "wb");

		if (!file)
			pr_fatal_errno(option.outfile);
	}

	if (option.module)
		print_symtab_module(file, symtab);
	else
		print_symtab(file, symtab);

	if (option.outfile) {
		if (ferror(file))
			pr_fatal_errno("%s: Write failure %d\n",
				option.outfile, ferror(file));

		if (fclose(file))
			pr_fatal_errno(option.outfile);

		if (rename(option.tmpfile, option.outfile))
			pr_fatal_errno(option.outfile);

		free(option.tmpfile);
	} else if (ferror(file))
		pr_fatal_errno("stdout: Write failure %d\n", ferror(file));
}

static struct function *function_for_index(int index, struct library *library)
{
	for (size_t i = 0; i < library->function.count; i++)
		if (index == library->function.entry[i].index)
			return &library->function.entry[i];

	return NULL;
}

static void fill_library(struct library *library, struct module *module)
{
	static const struct lexc_token empty = { .string = "" };
	unsigned int max_index = 0;

	for (size_t i = 0; i < library->function.count; i++) {
		const int index = library->function.entry[i].index;

		if (!i || max_index < index)
			max_index = index;
	}

	for (size_t i = 0; i < max_index; i++)
		if (!function_for_index(i, library))
			library_function(library, empty, i);
}

static void fill_module(struct module *module)
{
	for (size_t i = 0; i < module->library.count; i++)
		fill_library(&module->library.entry[i], module);
}

static void fill_symtab(struct symtab *symtab)
{
	for (size_t i = 0; i < symtab->module.count; i++)
		fill_module(&symtab->module.entry[i]);
}

static int function_index_cmp(const void *a_, const void *b_)
{
	const struct function * const a = a_;
	const struct function * const b = b_;

	return a->index > b->index;
}
static void order_library(struct library *library, struct module *module)
{
	qsort(library->function.entry, library->function.count,
		sizeof(*library->function.entry), function_index_cmp);
}

static void order_module(struct module *module)
{
	for (size_t i = 0; i < module->library.count; i++)
		order_library(&module->library.entry[i], module);
}

static void order_symtab(struct symtab *symtab)
{
	for (size_t i = 0; i < symtab->module.count; i++)
		order_module(&symtab->module.entry[i]);
}

static void help(FILE *file)
{
	fprintf(file,
"usage: %s [-h | --help] <-o <outfile>> <infile>\n"
"\n"
"options:\n"
"    -h, --help              display this help and exit\n"
"    --version               display version and exit\n"
"\n"
"    -m, --module            generate symbols for a module\n"
"    -o, --output <outfile>  name for the IOP module produced by %s\n"
"\n",
		progname, progname);
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
#define OPT(option) (strcmp(options[index].name, (option)) == 0)

	static const struct option options[] = {
		{ "help",    no_argument,       NULL,           0 },
		{ "version", no_argument,       NULL,           0 },
		{ "output",  required_argument, NULL,           0 },
		{ "module",  no_argument,       &option.module, 1 },
		{ NULL, 0, NULL, 0 }
	};

	argv[0] = progname;	/* For better getopt_long error messages. */

	for (;;) {
		int index = 0;

		switch (getopt_long(argc, argv, "ho:m", options, &index)) {
		case -1:
			return;

		case 0:
			if (OPT("help"))
				help_exit();
			else if (OPT("version"))
				version_exit();
			else if (OPT("output"))
				option.outfile = optarg;
			break;

		case 'h':
			help_exit();

		case 'o':
			option.outfile = optarg;
			break;

		case 'm':
			option.module = 1;
			break;

		case '?':
			exit(EXIT_FAILURE);
		}
	}

#undef OPT
}

static void atexit_(void)
{
	if (option.tmpfile)
		unlink(option.tmpfile);
}

int main(int argc, char **argv)
{
	struct symtab symtab = { };

	parse_options(argc, argv);

	struct file file[argc - optind];

	if (atexit(atexit_))
		pr_fatal_errno("atexit");

	for (int i = optind; i < argc; i++) {
		struct file f = file[i - optind] = file_read(argv[i]);

		if (!file_valid(f))
			pr_fatal_errno(argv[i]);

		read_symtab(&symtab, f);
	}

	fill_symtab(&symtab);
	order_symtab(&symtab);
	write_symtab(&symtab);

	for (int i = optind; i < argc; i++)
		file_free(file[i - optind]);

	return EXIT_SUCCESS;
}
