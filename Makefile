# SPDX-License-Identifier: GPL-2.0
#
# Define V=1 for more verbose compile.

CFLAGS += -Wall -Iinclude

TOOL_CFLAGS += -O2 -g $(CFLAGS) $(BASIC_CFLAGS)

MODULE_LD := script/iop.ld
# FIXME: -O0 -> -O2
IOP_CFLAGS += -O0 -march=r3000 -EL -msoft-float -fomit-frame-pointer	\
       -fno-pic -mno-abicalls -fno-common -ffreestanding -static	\
       -fno-strict-aliasing -nostdlib -mlong-calls -mno-gpopt		\
       -mno-shared -G0 -ffunction-sections -fdata-sections		\
       $(CFLAGS) $(BASIC_CFLAGS)
IOP_LDFLAGS += -O2 --gpsize=0 -G0 --nmagic --orphan-handling=error	\
	--discard-all --gc-sections --emit-relocs -nostdlib		\
	-z max-page-size=4096 --no-relax --script=$(MODULE_LD)

CCC = $(CROSS_COMPILE)gcc
CLD = $(CROSS_COMPILE)ld

.PHONY: all
all: module tool

#
# Tool
#

IOPMOD_INFO := tool/iopmod-info
IOPMOD_LINK := tool/iopmod-link
IOPMOD_SYMC := tool/iopmod-symc
IOPMOD_MODC := script/iopmod-modc

TOOL_LIB := tool/tool.a
TOOL_C_SRC := tool/elf32.c tool/file.c tool/irx.c tool/lexc.c		\
	tool/print.c tool/string.c
TOOL_C_OBJ = $(patsubst %.c, %.o, $(TOOL_C_SRC))

SYMTAB_C_SRC := tool/symtab.c
SYMTAB_C_SYM := tool/symtab.sym.h
SYMTAB_C_OBJ = $(patsubst %.c, %.o, $(SYMTAB_C_SRC))

.PHONY: tool
tool: $(IOPMOD_INFO) $(IOPMOD_LINK) $(IOPMOD_SYMC)

$(SYMTAB_C_OBJ): $(SYMTAB_C_SYM)
$(SYMTAB_C_SYM): $(IOPMOD_SYMC)
$(SYMTAB_C_SYM): $(MODULE_H_ALL)
	$(QUIET_GEN)$(IOPMOD_SYMC) -o $@ $(MODULE_H_ALL)

$(IOPMOD_INFO): $(IOPMOD_INFO).o $(TOOL_LIB) $(SYMTAB_C_OBJ)
	$(QUIET_LINK)$(CC) $(TOOL_CFLAGS) -o $@ $^

$(IOPMOD_LINK): $(IOPMOD_LINK).o $(TOOL_LIB)
	$(QUIET_LINK)$(CC) $(TOOL_CFLAGS) -o $@ $^

$(IOPMOD_SYMC): $(IOPMOD_SYMC).o $(TOOL_LIB)
	$(QUIET_LINK)$(CC) $(TOOL_CFLAGS) -o $@ $^

$(TOOL_LIB): $(TOOL_C_OBJ)
	$(QUIET_AR)$(AR) rc $@ $^

$(IOPMOD_SYMC_OBJ):

$(TOOL_C_OBJ)								\
$(IOPMOD_INFO).o $(IOPMOD_LINK).o $(IOPMOD_SYMC).o			\
$(SYMTAB_C_OBJ): %.o : %.c
	$(QUIET_CC)$(CC) $(TOOL_CFLAGS) -c -o $@ $<

$(TOOL_S_OBJ): %.o : %.S
	$(QUIET_AS)$(CC) $(TOOL_CFLAGS) -c -o $@ $<

#
# Builtin
#

BUILTIN_LIB := builtin/builtin.a
BUILTIN_C_SRC = $(wildcard builtin/*.c)
BUILTIN_C_OBJ = $(patsubst %.c, %.o, $(BUILTIN_C_SRC))

.PHONY: builtin
builtin: $(BUILTIN_LIB)

$(BUILTIN_LIB): $(BUILTIN_C_OBJ)
	$(QUIET_AR)$(AR) rc $@ $^

$(BUILTIN_C_OBJ): %.o : %.c
	$(QUIET_CC)$(CCC) $(IOP_CFLAGS) -c -o $@ $<

#
# Module
#

MODULE_H_ALL = $(wildcard include/iopmod/module/*.h)
MODULE_C_SRC = $(filter-out %.mod.c, $(wildcard module/*.c))
MODULE_C_OBJ = $(patsubst %.c, %.o, $(MODULE_C_SRC))
MODULE_H_SYM = $(patsubst %.c, %.sym.h, $(MODULE_C_SRC))
MODULE_S_SYM = $(patsubst %.c, %.sym.S, $(MODULE_C_SRC))
MODULE_C_MOD = $(patsubst %.c, %.mod.c, $(MODULE_C_SRC))
MODULE_M_OBJ = $(patsubst %.c, %.mod.o, $(MODULE_C_SRC))
MODULE_S_OBJ = $(patsubst %.c, %.sym.o, $(MODULE_C_SRC))
MODULE_IOP   = $(patsubst %.c, %.iop, $(MODULE_C_SRC))
MODULE_IRX   = $(patsubst %.c, %.irx, $(MODULE_C_SRC))

.PHONY: module
module: $(MODULE_IRX)

$(MODULE_H_SYM): $(IOPMOD_SYMC)

$(MODULE_H_SYM): $(MODULE_H_ALL)
	$(QUIET_GEN)$(IOPMOD_SYMC) -m -o $@ $(MODULE_H_ALL)

$(MODULE_S_SYM): $(IOPMOD_MODC)

$(MODULE_S_SYM): $(IOPMOD_MODC) include/iopmod/module-symbol.S
	$(QUIET_GEN)$(IOPMOD_MODC) -o $@ $^

$(MODULE_C_MOD): $(IOPMOD_MODC) include/iopmod/mod.h
	$(QUIET_GEN)$(IOPMOD_MODC) -o $@ $^

$(MODULE_M_OBJ): %.mod.o : %.sym.h

%.mod.o : %.mod.c
	$(QUIET_CC)$(CCC) $(IOP_CFLAGS) -c -o $@ $<

$(MODULE_C_OBJ): %.o : %.c
	$(QUIET_CC)$(CCC) $(IOP_CFLAGS) -c -o $@ $<

$(MODULE_S_OBJ): %.sym.o : %.sym.h

$(MODULE_S_OBJ): %.o : %.S
	$(QUIET_AS)$(CCC) $(IOP_CFLAGS) -c -o $@ $<

$(MODULE_IOP): $(MODULE_LD) $(BUILTIN_LIB)
$(MODULE_IOP): %.iop : %.mod.o
$(MODULE_IOP): %.iop : %.sym.o
$(MODULE_IOP): %.iop : %.o
	$(QUIET_LINK)$(CLD) $(IOP_LDFLAGS) -o $@ $<			\
		$(patsubst %.iop, %.mod.o, $@)				\
		$(patsubst %.iop, %.sym.o, $@) $(BUILTIN_LIB)

$(MODULE_IRX): $(IOPMOD_LINK)

$(MODULE_IRX): %.irx : %.iop
	$(QUIET_LINK)$(IOPMOD_LINK) -o $@ $<

#
# test
#

.PHONY: check
check: test

#
# Clean
#

.PHONY: clean
clean:
	$(QUIET_RM)$(RM) -f */*.a */*.o */*.o.d				\
		*/*.mod.* */*.sym.* */*.iop */*.irx */*.tmp		\
		$(IOPMOD_INFO) $(IOPMOD_LINK) $(IOPMOD_SYMC)

#
# General
#

V             = @
Q             = $(V:1=)
QUIET_AR      = $(Q:@=@echo    '  AR      '$@;)
QUIET_AS      = $(Q:@=@echo    '  AS      '$@;)
QUIET_CC      = $(Q:@=@echo    '  CC      '$@;)
QUIET_GEN     = $(Q:@=@echo    '  GEN     '$@;)
QUIET_LINK    = $(Q:@=@echo    '  LD      '$@;)
QUIET_TEST    = $(Q:@=@echo    '  TEST    '$@;)
QUIET_RM      = $(Q:@=@echo    '  RM      '$@;)

BASIC_CFLAGS += -Wp,-MMD,$(@D)/$(@F).d -MT $(@D)/$(@F)

ifneq "$(MAKECMDGOALS)" "clean"
   DEP_FILES := $(wildcard */*.d)
   $(if $(DEP_FILES),$(eval -include $(DEP_FILES)))
endif
