# Makefile for core

include ../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}

SELF_INCLUDES	+= -I../include
SELF_INCLUDES	+= -I../include/message
SELF_INCLUDES	+= -I../lib/include
SELF_INCLUDES	+= -I./include

vpath %.h ../include
vpath %.h ../include/message
vpath %.h ../lib/include
vpath %.h ./include

SELF_LIBS	+= -L../mod -lmod
SELF_LIBS	+= -L../proto -lproto
SELF_LIBS	+= -L../lib/bin -lprocessxml

SELF_LIBS	+= -L../lib/storage/lib -lstor
SELF_LIBS	+= -L../lib/storage/lib/lib -lmk2fs -lcom_err -lblkid -luuid -lquota -lext2fs -le2p
SELF_LIBS	+= -rdynamic -ldl
SELF_LIBS	+= ${PRJ_LIBS} -lxml2

TARGET		= ../bin/nmp-mss
DEPS		= Makefile.deps

.PHONY: clean cleanall install uninstall mss_libs_clean mss_mods_clean mss_proto_clean

${TARGET}: ${C_OBJS} ../mod/libmod.a ../proto/libproto.a ../lib/bin/libprocessxml.a
	$(CC) -o $@ ${C_OBJS} ${SELF_LIBS}

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(DEPS)

../lib/bin/libprocessxml.a:
	make -C ../lib/process_xml

../mod/libmod.a:
	make -C ../mod

../proto/libproto.a:
	make -C ../proto

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

cleanall: mss_libs_clean mss_mods_clean mss_proto_clean
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

mss_libs_clean:
	make -C ../lib clean

mss_mods_clean:
	make -C ../mod clean

mss_proto_clean:
	make -C ../proto clean

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
