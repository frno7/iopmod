// SPDX-License-Identifier: GPL-2.0
/*
 * IOP modules are IRX objects based on the executable and linkable format
 * (ELF). All valid IOP modules have a special `.iopmod` section containing
 * the module name, version, etc.
 *
 * IOP modules may import and export any number of library functions,
 * including non at all. Imported libraries must be resolved and prelinked
 * before the given module is allowed to link itself. Other modules can link
 * with its exported libraries.
 *
 * IOP modules begin to execute their entry function immediately after linking.
 * The modules can either stay resident in the IOP, and provide services, or
 * unlink themselves when exiting the entry function. Many modules provide
 * remote procedure call (RPC) services via the sub-system interface (SIF).
 *
 * Copyright (C) 2019 Fredrik Noring
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "iopmod/asm/irx.h"

#include "iopmod/tool/elf32.h"
#include "iopmod/tool/irx.h"

const char *irx_section_type_name(u32 sh_type)
{
	switch (sh_type) {
	case SHT_NULL:       return "NULL";
	case SHT_PROGBITS:   return "PROGBITS";
	case SHT_SYMTAB:     return "SYMTAB";
	case SHT_REL:        return "REL";
	case SHT_NOBITS:     return "NOBITS";
	case SHT_STRTAB:     return "STRTAB";
	case SHT_MIPS_DEBUG: return "MIPS_DEBUG";
	case SHT_IOPMOD:     return "IOPMOD";
	}

	return NULL;
}

/**
 * irx_iopmod - give .iopmod section pointer, if it exists
 * @ehdr: ELF header of module
 *
 * The .iopmod section is specific to IOP (IRX) modules.
 *
 * Return: .iopmod section pointer, or %NULL
 */
struct irx_iopmod *irx_iopmod(Elf_Ehdr *ehdr)
{
	Elf_Shdr *shdr = elf_first_section_with_name(".iopmod", ehdr);

	if (!shdr)
		return NULL;

	/* FIXME: Check SHT_IOPMOD etc. */

	return elf_ent_for_offset(shdr->sh_offset, ehdr);
}

struct irx_iopmod_id irx_iopmod_id(Elf_Ehdr *ehdr)
{
	struct irx_iopmod *iopmod = irx_iopmod(ehdr);
	struct irx_iopmod_id iopmod_id = { .name = "" };

	if (!iopmod || iopmod->id_addr == IOPMOD_NO_ID)
		return iopmod_id;

	const struct {
		u32 name;
		u16 version;
	} *id = elf_ent_for_addr(iopmod->id_addr, ehdr);

	iopmod_id.name = elf_ent_for_addr(id->name, ehdr);
	iopmod_id.version = id->version;

	return iopmod_id;
}

static void *library_entry(void *library, u32 magic, Elf_Ehdr *ehdr)
{
	Elf_Shdr *shdr = elf_first_section_with_name(".text", ehdr);
	u32 *text = elf_ent_for_offset(shdr->sh_offset, ehdr);
	unsigned int length = shdr->sh_size / sizeof(*text);
	unsigned int index = library ? ((u32 *)library - text) + 1 : 0;

	for (unsigned int i = index; i < length; i++)
		if (text[i] == magic)
			return &text[i];

	return NULL;
}

/**
 * irx_first_import_library - first imported library entry
 * @ehdr: ELF header of module
 *
 * Return: first imported library entry, or %NULL
 */
struct irx_import_library *irx_first_import_library(Elf_Ehdr *ehdr)
{
	return library_entry(NULL, IOPMOD_IMPORT_MAGIC, ehdr);
}

/**
 * irx_next_import_library - next imported library entry
 * @library: current library entry
 * @ehdr: ELF header of module
 *
 * Return: imported library entry following the current library entry, or %NULL
 */
struct irx_import_library *irx_next_import_library(
	struct irx_import_library *library, Elf_Ehdr *ehdr)
{
	return library_entry(library, IOPMOD_IMPORT_MAGIC, ehdr);
}

/**
 * irx_first_export_library - first exported library entry
 * @ehdr: ELF header of module
 *
 * Return: first exported library entry, or %NULL
 */
struct irx_export_library *irx_first_export_library(Elf_Ehdr *ehdr)
{
	return library_entry(NULL, IOPMOD_EXPORT_MAGIC, ehdr);
}

/**
 * irx_next_export_library - next exported library entry
 * @library: current library entry
 * @ehdr: ELF header of module
 *
 * Return: exported library entry following the current library entry, or %NULL
 */
struct irx_export_library *irx_next_export_library(
	struct irx_export_library *library, Elf_Ehdr *ehdr)
{
	return library_entry(library, IOPMOD_EXPORT_MAGIC, ehdr);
}

struct irx_import_link *irx_first_import_link(
	struct irx_import_library *library)
{
	return library->link;
}

struct irx_import_link *irx_next_import_link(
	struct irx_import_library *library,
	struct irx_import_link *link)
{
	struct irx_import_link *next = &link[1];

	return next->li.op ? next : NULL;
}

struct irx_export_link *irx_first_export_link(
	struct irx_export_library *library)
{
	return library->link;
}

struct irx_export_link *irx_next_export_link(
	struct irx_export_library *library,
	struct irx_export_link *link)
{
	struct irx_export_link *next = &link[1];

	return next->offset ? next : NULL;
}

unsigned int irx_id_for_import_link(struct irx_import_link *link)
{
	return link->li.imm;
}

/**
 * irx_identify - does the buffer contain an IRX object?
 * @buffer: pointer to data to identify
 * @size: size in bytes of buffer
 *
 * Return: %true if the buffer looks like an IRX object, otherwise %false
 */
bool irx_identify(void *buffer, size_t size)
{
	return elf_identify(buffer, size) && irx_iopmod(buffer) != NULL;
}
