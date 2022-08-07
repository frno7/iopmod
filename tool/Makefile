# SPDX-License-Identifier: GPL-2.0

TOOL_CFLAGS += -O2 -g $(CFLAGS) $(BASIC_CFLAGS)

TOOL_LDFLAGS += $(TOOL_CFLAGS) $(LDFLAGS)

VERSION_SRC = tool/version.c
IOPMOD_INFO = tool/iopmod-info
IOPMOD_LINK = tool/iopmod-link
IOPMOD_SYMC = tool/iopmod-symc
IOPMOD_MODC = script/iopmod-modc

TOOL_LIB = tool/tool.a

TOOL_C_SRC = $(addprefix tool/,						\
	elf32.c								\
	file.c								\
	irx.c								\
	lexc.c								\
	print.c								\
	string.c)							\
	$(VERSION_SRC)

TOOL_C_OBJ = $(TOOL_C_SRC:%.c=%.o)

SYMTAB_C_SRC = tool/symtab.c
SYMTAB_C_SYM = tool/symtab.sym.h
SYMTAB_C_OBJ = $(SYMTAB_C_SRC:%.c=%.o)

$(TOOL_LIB): $(TOOL_C_OBJ)
	$(QUIET_AR)$(AR) rc $@ $^

.PHONY: tool
tool: $(IOPMOD_INFO) $(IOPMOD_LINK) $(IOPMOD_SYMC)

$(SYMTAB_C_OBJ): $(SYMTAB_C_SYM)
$(SYMTAB_C_SYM): $(IOPMOD_SYMC)
$(SYMTAB_C_SYM): $(MODULE_H_ALL)
	$(QUIET_GEN)$(IOPMOD_SYMC) -o $@ $(MODULE_H_ALL)

$(IOPMOD_INFO): $(SYMTAB_C_OBJ)

$(IOPMOD_INFO) $(IOPMOD_LINK) $(IOPMOD_SYMC): %: %.o $(TOOL_LIB)
	$(QUIET_LINK)$(CC) $(TOOL_LDFLAGS) -o $@ $^

$(IOPMOD_SYMC_OBJ):

$(TOOL_C_OBJ)								\
$(IOPMOD_INFO).o $(IOPMOD_LINK).o $(IOPMOD_SYMC).o			\
$(SYMTAB_C_OBJ): %.o : %.c
	$(QUIET_CC)$(CC) $(TOOL_CFLAGS) -c -o $@ $<

$(TOOL_S_OBJ): %.o : %.S
	$(QUIET_AS)$(CC) $(TOOL_CFLAGS) -c -o $@ $<

.PHONY: $(shell script/version $(VERSION_SRC))
$(VERSION_SRC):
	@script/version $@
