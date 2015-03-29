# Makefile for sip

include ../../Makefile.def

PWD		:= $(shell pwd)
SELF_CFLAGS   	+= $(PRJ_CFLAGS)
SELF_DEFINE	+= $(PRJ_DEFINE) 

SELF_INCLUDES	+= $(PRJ_INCLUDES)
SELF_INCLUDES	+= -I../../include

vpath %.h ../../include

TARGET		= ../../lib/sip-objs.lst
DEPS		= Makefile.deps

.PHONY: clean install uninstall

$(TARGET): $(C_OBJS)  
	@test -z ../../lib || mkdir -p ../../lib
	@echo $(addprefix "$(PWD)/", $(C_OBJS)) > $@

%.o: %.c $(DEPS)
	$(CC) $(SELF_CFLAGS) -o $@ -c $< $(SELF_INCLUDES)

$(DEPS): $(C_SRCS)
	$(CC) -MM $(SELF_CFLAGS) $(SELF_DEFINE) $^ > $@ $(SELF_INCLUDES)

-include $(DEPS)

clean:
	@rm $(C_OBJS)
	@rm $(DEPS)
	@rm $(TARGET)

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
