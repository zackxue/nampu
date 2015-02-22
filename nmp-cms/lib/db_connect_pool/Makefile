#
#        Author:
#        Sep 9th, 2010
#

include $(PROJ_ROOT)/Rules.mk
LIB_TARG = libdbc.a
TARG = $(LIB_OUTPUT)/$(LIB_TARG)

LOCAL_INC := -I ./inc
LOCAL_SRC := ./src

$(INCS) += $(LOCAL_INC)

SOURCE_FILE := $(wildcard $(LOCAL_SRC)/*.c)
OBJECT_FILE := $(patsubst %.c, %.o, $(SOURCE_FILE))
DEPS_OUTPUT := Makefile.deps

$(TARG):$(OBJECT_FILE)
	$(AR) -r $@ $^
	
$(DEPS_OUTPUT):$(SOURCE_FILE)
	$(CC) -MM $(INCS) $^ > $(DEPS_OUTPUT)

-include $(DEPS_OUTPUT)

.PHONY:clean
clean:
	$(RM) -rf $(LIBDBC) $(OBJECT_FILE) $(DEPS_OUTPUT)

