# Makefile for encrypt

include ../Makefile.def

SELF_CFLAGS   	+= $(PRJ_CFLAGS)
SELF_DEFINE	+= $(PRJ_DEFINE) 

SELF_INCLUDES	+= $(PRJ_INCLUDES)
SELF_INCLUDES	+= -I../include

vpath %.h ../include

TARGET		:= ../lib/libtinyrain.a
DEPS		:= Makefile.deps

.PHONY: clean cleanall install uninstall

$(TARGET): $(C_OBJS) ../lib/rtsp-objs.lst ../lib/sip-objs.lst
	@test -z ../lib || mkdir -p ../lib
	$(AR) -rs $@ $^ $(shell find ../lib -name "*.lst" -print | xargs cat)

%.o: %.c $(DEPS)
	$(CC) $(SELF_CFLAGS) $(SELF_DEFINE) -o $@ -c $< $(SELF_INCLUDES)

$(DEPS): $(C_SRCS)
	$(CC) -MM $(SELF_CFLAGS) $(SELF_DEFINE) $^ > $@ $(SELF_INCLUDES)

-include $(DEPS)

../lib/rtsp-objs.lst:
	make -C ../rtsp

../lib/sip-objs.lst:
	make -C ../sip

clean:
	@rm $(C_OBJS)
	@rm $(DEPS)
	@rm $(TARGET)

cleanall:
	make -C ../rtsp clean
	make -C ../sip clean
	@rm $(C_OBJS)
	@rm $(DEPS)
	@rm $(TARGET)

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"

