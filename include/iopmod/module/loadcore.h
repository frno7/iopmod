// SPDX-License-Identifier: GPL-2.0

MODULE_ID(loadcore, 0x0101);
LIBRARY_ID(loadcore, 0x0101);

id_(4) void loadcore_flush_icache(void)
	alias_(FlushIcache);
id_(5) void loadcore_flush_dcache(void)
	alias_(FlushDcache);

id_(6) int loadcore_register_library(
		struct irx_export_library *module_export)
	alias_(RegisterLibraryEntries);
id_(7) int loadcore_release_library(
		struct irx_export_library *module_export)
	alias_(ReleaseLibraryEntries);
