# Makefile for tinyrain
#

TARGET_EXE  := ./test/tiny-rain
TARGET_LIB  := ./lib/libtinyrain.a
LIB_HEADERS := ./include/tr_avs.h


ifneq (${NMP_LIB_INSTALL},)
	DESTDIR	:= ${NMP_LIB_INSTALL}
else
	DESTDIR	:= /usr/local
endif

.PHONY: all clean install uninstall

all:
	make -C ./framework
	make -C ./test

clean: 
	make -C ./framework cleanall
	make -C ./test clean

install:
	test -z $(DESTDIR)/lib || mkdir -p $(DESTDIR)/lib
	install -m 644 $(TARGET_LIB) $(DESTDIR)/lib
	test -z $(DESTDIR)/include/nampu/tinyrain || mkdir -p $(DESTDIR)/include/nampu/tinyrain
	install -m 644 $(LIB_HEADERS) $(DESTDIR)/include/nampu/tinyrain

uninstall:
	rm -f $(DESTDIR)/lib/$(TARGET_LIB)
	rm -rf $(DESTDIR)/include/nampu/tinyrain

