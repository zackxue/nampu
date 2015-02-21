#
#        Author:
#        September 6th, 2011
#

include $(PROJ_ROOT)/Rules.mk

INCS += -I$(shell pwd)
SRCS := $(shell find . -name "*.c")
OBJS := $(patsubst %.c, %.o, $(SRCS))
DEPS := Makefile.deps
TARG := O_TARG
_PWD := $(shell pwd)

$(TARG): $(DEPS) $(OBJS)
	echo $(addprefix "$(_PWD)/", $(OBJS)) > $@

$(DEPS): $(SRCS)
#	${CC} -MM ${CFLAGS} $^ > $@ ${INCS}

.PHONY: clean

clean:
	rm $(OBJS) $(DEPS) $(TARG) -rf
