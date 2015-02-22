#
#        Author:
#        Sep 9th, 2010
#

include $(PROJ_ROOT)/Rules.mk
export PROJ_ROOT

EXCLUDE_DIRS := include bin

DIRS := $(shell find . -maxdepth 1 -type d)
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS := $(filter-out $(EXCLUDE_DIRS), $(DIRS))

SUB_DIRS := $(DIRS)
CLEAN_DIRS := $(addprefix _cls_, $(SUB_DIRS))
LIB_OUTPUT := $(PROJ_ROOT)/lib/bin
export LIB_OUTPUT

TARG := L_TARG

.PHONY: ALL_SUB $(SUB_DIRS) clean 

$(TARG): ALL_SUB
	$(shell find ./*/ -name "Makefile" -print |xargs cat | grep "LIB_TARG[[:space:]]*=" | awk -F= '{print $$2}' | awk '{print $$1}' | awk -F. '{printf " -L$(LIB_OUTPUT) -l"substr($$1, 4)}'> $@)

ALL_SUB: $(SUB_DIRS)

$(SUB_DIRS):
	$(MAKE) -C $@

clean: $(CLEAN_DIRS)
	-rm $(LIB_OUTPUT)/* -rf

$(CLEAN_DIRS):
	-rm $(TARG) -rf
	$(MAKE) -C $(patsubst _cls_%, %, $@) clean

