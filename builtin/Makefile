# SPDX-License-Identifier: GPL-2.0

BUILTIN_C_SRC = $(addprefix builtin/,					\
	interrupt.c							\
	ioperr.c							\
	memcpy.c							\
	memset.c							\
	printk.c							\
	sif.c								\
	spd-irq.c							\
	strlen.c							\
	vsnprintf.c							\
	__udivdi3.c							\
	__udivmoddi4.c							\
	__umoddi3.c)

BUILTIN_C_OBJ = $(BUILTIN_C_SRC:%.c=%.o)

ALL_OBJ += $(BUILTIN_C_OBJ)

builtin/memcpy.o							\
builtin/memset.o:							\
	IOP_CFLAGS += -fno-tree-loop-distribute-patterns

$(BUILTIN_C_OBJ): %.o : %.c
	$(QUIET_CC)$(CCC) $(IOP_CFLAGS) -c -o $@ $<

BUILTIN_LIB = builtin/builtin.a

$(BUILTIN_LIB): $(BUILTIN_C_OBJ)
	$(QUIET_AR)$(AR) rc $@ $^

OTHER_CLEAN += $(BUILTIN_LIB)

.PHONY: builtin
builtin: $(BUILTIN_LIB)
