#
#        Author:
#        Sep 9th, 2010
#

include $(PROJ_ROOT)/Rules.mk

SRCS := $(shell find . -name "*.c")
OBJS := $(patsubst %.c, %.o, $(SRCS))
DEPS := Makfile.deps
TARG := O_TARG
_PWD := $(shell pwd)

$(TARG): $(DEPS) $(OBJS)
	echo $(addprefix "$(_PWD)/", $(OBJS)) > $@

$(DEPS): $(SRCS)
	${CC} -MM ${CFLAGS} $^ > $@ ${INCS}

-include $(DEPS)

.PHONY: clean

clean:
	-rm $(OBJS) $(DEPS) $(TARG) -rf
