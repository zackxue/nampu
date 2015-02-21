#
#        Author:
#        October 9th, 2012
#

PROJ_ROOT := $(shell pwd)
export PROJ_ROOT
export INCS

include $(PROJ_ROOT)/Rules.mk

EXCLUDE_DIRS := inc lib docs si

DIRS := $(shell find . -maxdepth 1 -type d)
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS := $(filter-out $(EXCLUDE_DIRS), $(DIRS))

SUB_DIRS := $(DIRS)
CLEAN_DIRS := $(addprefix _cls_, $(SUB_DIRS))

TARG := $(PROJ_ROOT)/test/talk_server
TAR_LIB := $(PROJ_ROOT)/lib/libts.a
TAR_INC := $(PROJ_ROOT)/inc/talk_server.h $(PROJ_ROOT)/inc/talk_api.h $(PROJ_ROOT)/inc/media_struct.h

INS_DIR := $(TAR_DIR)/talk-server

.PHONY: ALL_SUB $(SUB_DIRS) clean install uninstall

$(TARG): ALL_SUB
	$(AR) -r $(TAR_LIB) $(shell find . -name "O_TARG" -print | xargs cat) 
	$(CC) -o $@ $(EOBJ) $(shell find . -name "E_TARG" -print | xargs cat) -L$(PROJ_ROOT)/lib -lts $(shell find . -name "L_TARG" -print | xargs cat) $(LIBS)

ALL_SUB: $(SUB_DIRS)

$(SUB_DIRS):
	$(MAKE) -C $@

clean: $(CLEAN_DIRS)
	rm -rf $(TARG) $(TAR_LIB)

$(CLEAN_DIRS):
	$(MAKE) -C $(patsubst _cls_%, %, $@) clean

install:
	test -z $(INS_DIR)/lib || mkdir -p $(INS_DIR)/lib
	install -m 644 $(TAR_LIB) $(INS_DIR)/lib
	test -z $(INS_DIR)/inc || mkdir -p $(INS_DIR)/inc
	install -m 644 $(TAR_INC) $(INS_DIR)/inc

uninstall:
	rm -f $(INS_DIR)/lib/$(TAR_LIB)
	rm -f $(INS_DIR)/inc/*
#$(TAR_INC)

