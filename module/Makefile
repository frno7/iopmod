# SPDX-License-Identifier: GPL-2.0

MODULE_H_ALL = $(wildcard include/iopmod/module/*.h)
MODULE_C_SRC = $(filter-out %.mod.c, $(wildcard module/*.c))

MODULE_C_OBJ = $(MODULE_C_SRC:%.c=%.o)
MODULE_H_SYM = $(MODULE_C_SRC:%.c=%.sym.h)
MODULE_S_SYM = $(MODULE_C_SRC:%.c=%.sym.S)
MODULE_C_MOD = $(MODULE_C_SRC:%.c=%.mod.c)
MODULE_M_OBJ = $(MODULE_C_SRC:%.c=%.mod.o)
MODULE_S_OBJ = $(MODULE_C_SRC:%.c=%.sym.o)
MODULE_IOP   = $(MODULE_C_SRC:%.c=%.iop)
MODULE_IRX   = $(MODULE_C_SRC:%.c=%.irx)

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
		$(@:%.iop=%.mod.o)					\
		$(@:%.iop=%.sym.o) $(BUILTIN_LIB)

$(MODULE_IRX): $(IOPMOD_LINK)

$(MODULE_IRX): %.irx : %.iop
	$(QUIET_LINK)$(IOPMOD_LINK) -o $@ $<
