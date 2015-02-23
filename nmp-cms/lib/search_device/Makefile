#
#        Author:
#        Sep 9th, 2010
#

include $(PROJ_ROOT)/Rules.mk

LIB_TARG = libjpfsearch.a
TARG = $(LIB_OUTPUT)/$(LIB_TARG)

LOCAL_INC := $(INCS)
LOCAL_INC += -I ./include
LOCAL_SRC := ./src

LIBS += -lrt

CFLAGS += -rdynamic $(LOCAL_INC)
SOURCE_FILE := $(wildcard $(LOCAL_SRC)/*.c)
OBJECT_FILE := $(patsubst %.c, %.o, $(SOURCE_FILE))
DEPS_OUTPUT := Makefile.deps

$(TARG):$(OBJECT_FILE)
	$(AR) -r $@ $^

$(DEPS_OUTPUT):$(SOURCE_FILE)
	$(CC) -MM $(LOCAL_INC) $^ > $(DEPS_OUTPUT)
	
-include $(DEPS_OUTPUT)

.PHONY:clean
clean:
	$(RM) -rf $(LIB_TARG) $(OBJECT_FILE) $(DEPS_OUTPUT)

