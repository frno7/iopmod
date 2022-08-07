# SPDX-License-Identifier: GPL-2.0

TOOL_CFLAGS = -O2 -g $(S_CFLAGS) $(BASIC_CFLAGS)

TOOL_LDFLAGS = $(TOOL_CFLAGS) $(LDFLAGS)

IOPMOD_INFO = tool/iopmod-info
IOPMOD_LINK = tool/iopmod-link
IOPMOD_SYMC = tool/iopmod-symc

IOPMOD_MODC = script/iopmod-modc

TOOL = $(IOPMOD_INFO) $(IOPMOD_LINK) $(IOPMOD_SYMC)

TOOL_LIB = tool/tool.a

OTHER_CLEAN += $(TOOL) $(TOOL_LIB)

VERSION_SRC = tool/version.c

TOOL_LIB_SRC = $(addprefix tool/,					\
	elf32.c								\
	file.c								\
	irx.c								\
	lexc.c								\
	print.c								\
	string.c)							\
	$(VERSION_SRC)

TOOL_LIB_OBJ = $(TOOL_LIB_SRC:%.c=%.o)

TOOL_OBJ = $(TOOL:%=%.o)

SYMTAB_SRC = tool/symtab.c
SYMTAB_SYM = tool/symtab.sym.h
SYMTAB_OBJ = $(SYMTAB_SRC:%.c=%.o)

ALL_OBJ += $(TOOL_LIB_OBJ) $(SYMTAB_OBJ) $(TOOL_OBJ)

OTHER_CLEAN += $(SYMTAB_SYM) $(VERSION_SRC)

$(TOOL_LIB_OBJ) $(TOOL_OBJ) $(SYMTAB_OBJ): %.o: %.c
	$(QUIET_CC)$(CC) $(TOOL_CFLAGS) -c -o $@ $<

$(TOOL_LIB): $(TOOL_LIB_OBJ)
	$(QUIET_AR)$(AR) rc $@ $^

$(SYMTAB_OBJ): $(SYMTAB_SYM)
$(SYMTAB_SYM): $(IOPMOD_SYMC)
$(SYMTAB_SYM): $(MODULE_H_ALL)
	$(QUIET_GEN)$(IOPMOD_SYMC) -o $@ $(MODULE_H_ALL)

$(IOPMOD_INFO): $(SYMTAB_OBJ)

$(TOOL): %: %.o $(TOOL_LIB)
	$(QUIET_LINK)$(CC) $(TOOL_LDFLAGS) -o $@ $^

.PHONY: tool
tool: $(TOOL)

.PHONY: $(shell script/version --check $(VERSION_SRC))
$(VERSION_SRC):
	$(QUIET_GEN)script/version $@
