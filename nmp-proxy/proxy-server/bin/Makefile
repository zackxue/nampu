# Makefile for mod

include ../Makefile.def

CURRENT_PATH	:= $(shell pwd)
SELF_CFLAGS   	+= ${PRJ_CFLAGS}

SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}
SELF_INCLUDES	+= -I../utils
SELF_INCLUDES	+= -I../include

vpath %.h ../include
vpath %.h ../utils

SELF_LIBS	+= -L../mod -lmod
SELF_LIBS	+= -L../core -lcore
SELF_LIBS	+= -L../utils -lutils

SELF_LIBS	+= -Wl,-rpath=../lib -L../lib -lhcnetsdk -lhpr -lStreamTransClient -lSystemTransform
SELF_LIBS	+= -ldhnetsdk -ldhdvr -ldhcfgsdk -lhbnetsdk -lbsmnetsdk -lsunellsdk -lhienetsdk -lxmtnetsdk -ltpsnetsdk

SELF_LIBS	+= ${PRJ_LIBS}
SELF_LIBS	+= -rdynamic -ldl -lnetembryo

SELF_LDFLAGS	+= ${PRJ_LDFLAGS}

TARGET		= proxy-server
DEPS		= Makefile.deps

.PHONY: clean install uninstall

${TARGET}: ${C_OBJS} ../mod/libmod.a ../utils/libutils.a ../core/libcore.a
	$(CC) ${SELF_LDFLAGS} -o $@ $^ ${SELF_LIBS}

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} ${SELF_DEFINE} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

../mod/libmod.a:
	make -C ../mod

../utils/libutils.a:
	make -C ../utils

../core/libcore.a:
	make -C ../core

-include $(DEPS)

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
