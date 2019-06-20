// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#ifndef IOPMOD_TOOL_IRX_H
#define IOPMOD_TOOL_IRX_H

#include "elf32.h"

#include "iopmod/asm/irx.h"
#include "iopmod/irx.h"

#include "iopmod/types.h"

/**
 * struct irx_iopmod - special .iopmod section with module name, version, etc.
 * @id_addr: address of a special identification structure, or %IOPMOD_NO_ID
 * @entry_addr: module entry address to begin executing code
 * @unknown: FIXME
 * @text_size: size in bytes of text section
 * @data_size: size in bytes of data section
 * @bss_size: size in bytes of BSS section
 * @version: module version in BCD
 * @name: NUL-terminated name of module
 */
struct irx_iopmod {
	u32 id_addr;
	u32 entry_addr;
	u32 unknown;
	u32 text_size;
	u32 data_size;
	u32 bss_size;
	u16 version;
	char name[0];
};

struct irx_iopmod_id {
	char *name;
	u16 version;
};

/**
 * irx_for_each_import_library - iterate over imported libraries
 * @library: &struct irx_import_library loop cursor
 * @ehdr: ELF header of module to iterate
 */
#define irx_for_each_import_library(library, ehdr)			\
	for ((library) = irx_first_import_library((ehdr));		\
	     (library);							\
	     (library) = irx_next_import_library((library), (ehdr)))

struct irx_import_library *irx_first_import_library(Elf_Ehdr *ehdr);
struct irx_import_library *irx_next_import_library(
	struct irx_import_library *library, Elf_Ehdr *ehdr);

/**
 * irx_for_each_export_library - iterate over exported libraries
 * @library: &struct irx_export_library loop cursor
 * @ehdr: ELF header of module to iterate
 */
#define irx_for_each_import_link(link, library)				\
	for ((link) = irx_first_import_link(library);			\
	     (link);							\
	     (link) = irx_next_import_link((library), (link)))

struct irx_import_link *irx_first_import_link(
	struct irx_import_library *library);
struct irx_import_link *irx_next_import_link(
	struct irx_import_library *library,
	struct irx_import_link *link);

#define irx_for_each_export_library(library, ehdr)			\
	for ((library) = irx_first_export_library(ehdr);		\
	     (library);							\
	     (library) = irx_next_export_library((library), (ehdr)))

struct irx_export_library *irx_first_export_library(Elf_Ehdr *ehdr);
struct irx_export_library *irx_next_export_library(
	struct irx_export_library *library, Elf_Ehdr *ehdr);

#define irx_for_each_export_link(link, library)				\
	for ((link) = irx_first_export_link(library);			\
	     (link);							\
	     (link) = irx_next_export_link((library), (link)))

struct irx_export_link *irx_first_export_link(
	struct irx_export_library *library);
struct irx_export_link *irx_next_export_link(
	struct irx_export_library *library,
	struct irx_export_link *link);

const char *irx_section_type_name(u32 sh_type);

struct irx_iopmod *irx_iopmod(Elf_Ehdr *ehdr);

struct irx_iopmod_id irx_iopmod_id(Elf_Ehdr *ehdr);

unsigned int irx_id_for_import_link(struct irx_import_link *link);

/**
 * irx_identify - does the buffer contain an IRX object?
 * @buffer: pointer to data to identify
 * @size: size in bytes of buffer
 *
 * Return: %true if the buffer looks like an IRX object, otherwise %false
 */
bool irx_identify(void *buffer, size_t size);

#endif /* IOPMOD_TOOL_IRX_H */
