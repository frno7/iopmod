// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#ifndef IOPMOD_IRX_H
#define IOPMOD_IRX_H

#include "iopmod/types.h"

#define IOPMOD_MAX_LIBRARY_NAME	8

/**
 * struct irx_import_link - link entry for an imported library
 * @jr: unconditional MIPS I jump to return address instruction
 * @jr.target: jump target, to be resolved by the IOP linker
 * @jr.op: operation code for the jump register instruction
 * @li: 16-bit load immediate pseudo-instruction
 * @li.imm: index of the imported library link entry
 * @li.rt: `$0` register
 * @li.rs: `$0` register
 * @li.op: operation code for the load immediate pseudo-instruction
 */
struct irx_import_link {
	struct {
		u32 target : 26;
		u32 op : 6;
	} jr;
	struct {
		u32 imm : 16;
		u32 rt : 5;
		u32 rs : 5;
		u32 op : 6;
	} li;
};

/**
 * struct irx_import_library - link entry table for an imported library
 * @magic: %IOPMOD_IMPORT_MAGIC marks the beginning of the link entry table
 * @zero: always zero
 * @version: 16-bit version in BCD, with 8-bit minor and 8-bit major
 * @name: library name, not NUL terminated unless shorter than
 * 	%IOPMOD_MAX_LIBRARY_NAME characters
 * @link: array of imported link entries, with the terminating entry zero
 *
 * The &struct irx_import_library resides in the .text section of the module.
 */
struct irx_import_library {
	u32 magic;
	u32 zero;
	u32 version;
	char name[IOPMOD_MAX_LIBRARY_NAME];
	struct irx_import_link link[0];
};

/**
 * struct irx_export_link - link entry for an exported library
 * @addr: address to link
 */
struct irx_export_link {
	u32 offset;
};

/**
 * struct irx_export_library - link entry table for an exported library
 * @magic: %IOPMOD_EXPORT_MAGIC marks the beginning of the link entry table
 * @zero: always zero
 * @version: 16-bit version in BCD, with 8-bit minor and 8-bit major
 * @name: library name, not NUL terminated unless shorter than
 * 	%IOPMOD_MAX_LIBRARY_NAME characters
 * @link: array of exported link entries, with the terminating entry zero
 *
 * The &struct irx_export_library resides in the .text section of the module.
 */
struct irx_export_library {
	u32 magic;
	u32 zero;
	u32 version;
	char name[IOPMOD_MAX_LIBRARY_NAME];
	struct irx_export_link link[0];
};

#endif /* IOPMOD_IRX_H */
