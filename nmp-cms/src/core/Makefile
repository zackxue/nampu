# Makefile for process_xml

include ../../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}

SELF_INCLUDES	+= -I/usr/include/mysql
SELF_INCLUDES	+= -I../../include
SELF_INCLUDES	+= -I../../include/message
SELF_INCLUDES	+= -I../lib/include
SELF_INCLUDES	+= -I./include

vpath %.h ../../include
vpath %.h ../../include/message
vpath %.h ../lib/include
vpath %.h ./include

SELF_LIBS	+= -L../mod -lmod
SELF_LIBS	+= -L../lib/bin -lprocessxml -ldbcp -lencrypt -lET199 -lsearchdevice -ltvwall
SELF_LIBS	+= -L../proto -lproto
SELF_LIBS	+= -rdynamic -ldl
SELF_LIBS	+= ${PRJ_LIBS} -lxml2 -lmysqlclient

TARGET		= ../../bin/nmp-cms
DEPS		= Makefile.deps

.PHONY: clean cleanall install uninstall cms_libs_clean cms_mods_clean cms_proto_clean

${TARGET}: ${C_OBJS} ../mod/libmod.a ../proto/libproto.a ../lib/bin/libdbcp.a ../lib/bin/libencrypt.a \
	../lib/bin/libprocessxml.a ../lib/bin/libsearchdevice.a ../lib/bin/libtvwall.a
	$(CC) -o $@ ${C_OBJS} ${SELF_LIBS}

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(DEPS)

../lib/bin/libdbcp.a:
	make -C ../lib/db_connect_pool

../lib/bin/libencrypt.a:
	make -C ../lib/encrypt

../lib/bin/libprocessxml.a:
	make -C ../lib/process_xml

../lib/bin/libsearchdevice.a:
	make -C ../lib/search_device

../lib/bin/libtvwall.a:
	make -C ../lib/tvwall

../mod/libmod.a:
	make -C ../mod

../proto/libproto.a:
	make -C ../proto

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

cleanall: cms_libs_clean cms_mods_clean cms_proto_clean
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

cms_libs_clean:
	make -C ../lib clean

cms_mods_clean:
	make -C ../mod clean

cms_proto_clean:
	make -C ../proto clean

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
