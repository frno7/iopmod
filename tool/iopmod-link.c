// SPDX-License-Identifier: GPL-2.0
/*
 * Link input/output processor (IOP) modules
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
#include "iopmod/tool/tool.h"

#include "iopmod/asm/macro.h"
#include "iopmod/types.h"
#include "iopmod/version.h"

char progname[] = "iopmod-link";

static struct {
	const char *input;
	const char *output;
} option;

static void link_module(struct file f)
{
	Elf_Ehdr *ehdr = f.data;

	ehdr->e_type = ET_LOPROC + ET_IOPMOD;
	ehdr->e_flags = EF_MIPS_NOREORDER;

	/* FIXME: link_compact: compact ELF */

	Elf_Shdr *shdr = elf_first_section_with_name(".iopmod", ehdr);	/* FIXME: irx_iopmod_section */
	if (!shdr)	/* FIXME: done in iop_valid */
		pr_fatal_error("%s: Missing .iopmod section", f.path);
	shdr->sh_type = SHT_IOPMOD;
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
		{ "help",    no_argument,       NULL, 0 },
		{ "version", no_argument,       NULL, 0 },
		{ "output",  required_argument, NULL, 0 },
		{ NULL, 0, NULL, 0 }
	};

	argv[0] = progname;	/* For better getopt_long error messages. */

	for (;;) {
		int index = 0;

		switch (getopt_long(argc, argv, "ho:", options, &index)) {
		case -1:
			if (argc < optind + 1)
				pr_fatal_error("no input file");
			if (argc > optind + 1)
				pr_fatal_error("too many input files");
			option.input = argv[optind];
			return;

		case 0:
			if (OPT("help"))
				help_exit();
			else if (OPT("version"))
				version_exit();
			else if (OPT("output"))
				option.output = optarg;
			break;

		case 'h':
			help_exit();

		case 'o':
			option.output = optarg;
			break;

		case '?':
			exit(EXIT_FAILURE);
		}
	}

#undef OPT
}

int main(int argc, char **argv)
{
	int status = EXIT_FAILURE;

	parse_options(argc, argv);

	struct file f = file_read(option.input);
	if (!file_valid(f))
		pr_fatal_errno(option.input);

	link_module(f);

	if (!file_rename(option.output, &f))
		pr_fatal_errno(option.output);

	if (!file_write(f, 0644))
		pr_fatal_errno(f.path);

	status = EXIT_SUCCESS;

	file_free(f);

	return status;
}
