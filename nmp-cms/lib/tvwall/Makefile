#
#        zyt, zhengyetao@126.com
#        Oct 9th, 2012
#

include $(PROJ_ROOT)/Rules.mk

LIB_TARG = libtw.a
TARG = $(LIB_OUTPUT)/$(LIB_TARG)

LOCAL_INC := $(INCS)
LOCAL_INC += -I ./include
LOCAL_SRC := ./src

XML_INC_PATH += -I /usr/include/libxml2 
LIBS += -lxml2

CFLAGS += -rdynamic $(XML_INC_PATH)  $(LOCAL_INC)
SOURCE_FILE := $(wildcard $(LOCAL_SRC)/*.c)
OBJECT_FILE := $(patsubst %.c, %.o, $(SOURCE_FILE))
DEPS_OUTPUT := Makefile.deps

$(TARG):$(OBJECT_FILE)
	$(AR) -r $@ $^

$(DEPS_OUTPUT):$(SOURCE_FILE)
	$(CC) -MM $(XML_INC_PATH) $(LOCAL_INC) $^ > $(DEPS_OUTPUT)
	
-include $(DEPS_OUTPUT)

.PHONY:clean
clean:
	$(RM) -rf $(LIB_TARG) $(OBJECT_FILE) $(DEPS_OUTPUT)
