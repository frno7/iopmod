/* SPDX-License-Identifier: GPL-2.0 */

OUTPUT_FORMAT("elf32-tradlittlemips")

ENTRY(_module_init)

PHDRS
{
	iopmod 0x70000080 FLAGS (4);	/* SHT_IOPMOD */
	text PT_LOAD FLAGS(7);
}

SECTIONS
{
	.iopmod 0 (COPY) : ALIGN(4) {
		LONG (DEFINED(_module_id) ? _module_id : 0xffffffff);
		LONG (_module_init);
		LONG (_unknown);	/* FIXME */
		LONG (SIZEOF(.text));
		LONG (SIZEOF(.data));
		LONG (SIZEOF(.bss));
		KEEP(*(.module.version))
		KEEP(*(.module.name))
	} :iopmod

	.text 0 : ALIGN(16) {
		KEEP(*(.module.init))
		KEEP(*(.module.start))
		*(.text)
		*(.text.*)
		KEEP(*(.export.*))
	} :text = 0

	.rodata : ALIGN(16) {
		KEEP(*(.module.id))
		KEEP(*(.module.id_name))
		*(.rodata)
		*(.rodata.*)
	} = 0

	.data : ALIGN(16) {
		*(.data)
		*(.data.*)
	} = 0

	.bss : ALIGN(16) {
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}

	_unknown = ALIGN(16);

	/DISCARD/ : {
		*(.comment)
		*(.gnu.attributes)
		*(.mdebug.*)
		*(.MIPS.*)
		*(.pdr)
		*(.reginfo)
		*(.rel.dyn)
	}
}
