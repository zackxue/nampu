#
#        Author:
#        Sep 9th, 2010
#

PROJ_ROOT := $(shell pwd)
#PROJ_SHARE:= /home/share/share/pd-libs/share
#PROJ_AFX:= /home/share/share/pd-framework
export PROJ_ROOT
#export PROJ_SHARE
#export PROJ_AFX

include $(PROJ_ROOT)/Rules.mk

EXCLUDE_DIRS := include bin docs

DIRS := $(shell find . -maxdepth 1 -type d)
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS := $(filter-out $(EXCLUDE_DIRS), $(DIRS))

SUB_DIRS := $(DIRS)
CLEAN_DIRS := $(addprefix _cls_, $(SUB_DIRS))

TARG := $(OUTPUT_DIR)/cms

.PHONY: ALL_SUB $(SUB_DIRS) clean 

$(TARG): ALL_SUB
	$(CC) -o $@ $(shell find . -name "O_TARG" -print | xargs cat) $(LIBS) $(shell find . -name "L_TARG" -print | xargs cat) -lxml2

ALL_SUB: $(SUB_DIRS)

$(SUB_DIRS):
	$(MAKE) -C $@

clean: $(CLEAN_DIRS)
	-rm $(OUTPUT_DIR)/* -rf
	
$(CLEAN_DIRS):
	$(MAKE) -C $(patsubst _cls_%, %, $@) clean
