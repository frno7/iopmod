// SPDX-License-Identifier: GPL-2.0

#include "iopmod/asm/irx.h"
#include "iopmod/asm/macro.h"

#include "__MODULE_NAME__.sym.h"

	.macro	ascia size, string
1:	.ascii	"\string"
2:	.iflt	\size - (2b - 1b)
	.error	"\string longer than maximum \size characters"
	.endif

	.ifgt	\size - (2b - 1b)
	.zero	\size - (2b - 1b)
	.endif
	.endm

/*
 * Enable garbage collection (Gnu LD option --gc-sections) of unused external
 * functions by placing them in their own text sections.
 *
 * Unfortunately this prevents grouping of several functions under a common
 * module, taking somewhat more storage space when more than one is imported.
 */

#define IMPORT_MP(module_name, module_version)
#define IMPORT_ME(module_name, module_version)

#define IMPORT_LP(							\
		module_name, module_version,				\
		library_name, library_version)

#define IMPORT_LE(							\
		module_name, module_version,				\
		library_name, library_version)

#define IMPORT_FP(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)						\
	.pushsection .text.import.#name_;				\
		.set	push;						\
		.set	noreorder;					\
		.4byte	IOPMOD_IMPORT_MAGIC;				\
		.4byte	0;						\
		.4byte	library_version;				\
		ascia	8, #library_name;				\
		.globl	name_;						\
		.weak	name_;						\
		.type	name_, @function;				\
	name_:	.insn;

#define IMPORT_FA(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)						\
		.globl	name_;						\
		.weak	name_;						\
		.type	name_, @function;				\
	name_:	.insn;

#define IMPORT_FE(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)						\
		jr	$ra;						\
		li	$0, index_;					\
		.4byte	0;						\
		.4byte	0;						\
		.set	pop;						\
	.popsection;

#define IMPORT_FU(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_)

#if defined(SYMTAB_IMPORT)
SYMTAB_IMPORT(
	IMPORT_MP, IMPORT_ME,
	IMPORT_LP, IMPORT_LE,
	IMPORT_FP, IMPORT_FA, IMPORT_FE, IMPORT_FU)
#endif

#define EXPORT_MP(module_name, module_version)
#define EXPORT_ME(module_name, module_version)

#define EXPORT_LP(							\
		module_name, module_version,				\
		library_name, library_version)				\
	.pushsection .export.library_name;				\
		.set	push;						\
		.set	noreorder;					\
		.globl _module_export;					\
	_module_export:;						\
		.4byte IOPMOD_EXPORT_MAGIC;				\
		.4byte 0;						\
		.4byte library_version;					\
		ascia	8, #library_name;

#define EXPORT_LE(							\
		module_name, module_version,				\
		library_name, library_version)				\
		.4byte 0;						\
		.set	pop;						\
	.popsection;

#define EXPORT_FP(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)						\
		.4byte name_;

#define EXPORT_FA(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)

#define EXPORT_FE(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_, name_)

#define EXPORT_FU(							\
		module_name, module_version,				\
		library_name, library_version,				\
		index_)

#if defined(SYMTAB_EXPORT)
SYMTAB_EXPORT(
	EXPORT_MP, EXPORT_ME,
	EXPORT_LP, EXPORT_LE,
	EXPORT_FP, EXPORT_FA, EXPORT_FE, EXPORT_FU)
#endif

#define MODULE_VERSION(module_version)					\
	.pushsection .module.version;					\
		.set	push;						\
		.set	noreorder;					\
		.globl _module_version;					\
	_module_version:;						\
		.2byte module_version;					\
		.set	pop;						\
	.popsection;

#define MODULE_NAME(module_name)					\
	.pushsection .module.name;					\
		.set	push;						\
		.set	noreorder;					\
		.globl _module_name;					\
	_module_name:;							\
		.asciz #module_name;					\
		.set	pop;						\
	.popsection;

#define MODULE_ID(module_name, module_version)				\
	MODULE_NAME(module_name);					\
	MODULE_VERSION(module_version);					\
	.pushsection .module.id_name;					\
		.set	push;						\
		.set	noreorder;					\
		.globl _module_id_name;					\
	_module_id_name:;						\
		.asciz #module_name;					\
		.set	pop;						\
	.popsection;							\
	.pushsection .module.id;					\
		.set	push;						\
		.set	noreorder;					\
		.globl _module_id;					\
	_module_id:;							\
		.4byte _module_id_name;					\
		.2byte module_version;					\
		.set	pop;						\
	.popsection;

#if defined(SYMTAB_MODULE_ID)
SYMTAB_MODULE_ID(MODULE_ID)
#endif
