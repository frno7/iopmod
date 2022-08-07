# SPDX-License-Identifier: GPL-2.0
#
# Define V=1 for more verbose compile.
# Define S=1 for sanitation checks.
# Define LDFLAGS=-static for statically linked tools.

CFLAGS += -Wall -Iinclude

ifeq "$(S)" "1"
TOOL_CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined	\
	  -fsanitize-address-use-after-scope -fstack-check
endif

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

include tool/Makefile
include builtin/Makefile

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
	$(QUIET_RM)$(RM) -f $(VERSION_SRC) */*.a */*.o */*.o.d		\
		*/*.mod.* */*.sym.* */*.iop */*.irx */*.tmp		\
		$(IOPMOD_INFO) $(IOPMOD_LINK) $(IOPMOD_SYMC)		\
		GPATH GRTAGS GTAGS

#
# Global tags
#

.PHONY: gtags
gtags:
	$(QUIET_GEN)gtags

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
