TARGET_LIB  := ./core/libtalksvr.a
LIB_HEADERS := ./include/*.h	

ifneq (${NMP_LIB_INSTALL},)
	DESTDIR	:= ${NMP_LIB_INSTALL}
else
	DESTDIR	:= /usr/local
endif


.PHONY: talksvr clean install uninstall

talksvr:
	make -C ./core
	make -C ./test

clean:
	make -C ./core clean
	make -C ./test clean

install:
	test -z $(DESTDIR)/lib || mkdir -p $(DESTDIR)/lib
	install -m 644 $(TARGET_LIB) $(DESTDIR)/lib
	test -z $(DESTDIR)/include/nampu/talksvr || mkdir -p $(DESTDIR)/include/nampu/talksvr
	install -m 644 $(LIB_HEADERS) $(DESTDIR)/include/nampu/talksvr

uninstall:
	rm -f $(DESTDIR)/lib/$(TARGET_LIB)
	rm -rf $(DESTDIR)/include/nampu/talksvr
