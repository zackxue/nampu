#
#        Author:
#        October 9th, 2012
#

include $(PROJ_ROOT)/Rules.mk

PWD := $(shell pwd)
SRCS := $(shell find . -maxdepth 1 -name "*.c")
OBJS := $(patsubst %.c, %.o, $(SRCS))
DEPS := Makfile.deps
TARG := E_TARG
INCS += -I$(PWD)
INCS += -I$(PWD)/inc

EXCLUDE_DIRS := inc lib docs
DIRS := $(filter-out $(EXCLUDE_DIRS), $(basename $(patsubst ./%, %, $(shell find . -maxdepth 1 -type d))))
CLEAN_DIRS := $(addprefix _cls_, $(DIRS))

$(TARG): $(DEPS) $(OBJS) ALL_DIR
	echo $(addprefix "$(PWD)/", $(OBJS)) > $@

$(DEPS): $(SRCS)
	-${CC} -MM ${CFLAGS} $^ > $@ ${INCS}

.PHONY: ALL_DIR $(DIRS) clean $(CLEAN_DIRS)

ALL_DIR: $(DIRS)

$(DIRS):
	$(MAKE) -C $@

-include $(DEPS)

clean: $(CLEAN_DIRS)
	-rm $(OBJS) $(DEPS) $(TARG) -rf

$(CLEAN_DIRS):
	$(MAKE) -C $(patsubst _cls_%, %, $@) clean
