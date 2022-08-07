# SPDX-License-Identifier: GPL-2.0
#
# Define V=1 for more verbose compile.
# Define S=1 for sanitation checks.
# Define LDFLAGS=-static for statically linked tools.

CFLAGS += -Wall -Iinclude

ifeq (1,$(S))
TOOL_CFLAGS = -fsanitize=address -fsanitize=leak -fsanitize=undefined	\
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
include module/Makefile

ALL_DEP = $(sort $(ALL_OBJ:%=%.d))

.PHONY: check
check: test

.PHONY: gtags
gtags:
	$(QUIET_GEN)gtags
OTHER_CLEAN += GPATH GRTAGS GTAGS

.PHONY: clean
clean:
	$(QUIET_RM)$(RM) -f $(ALL_OBJ) $(ALL_DEP) $(OTHER_CLEAN)

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
