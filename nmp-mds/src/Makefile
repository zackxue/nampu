# Makefile for nmp-mds/src

include ../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}

SELF_INCLUDES	+= -I../include
SELF_INCLUDES	+= -I../lib/include

vpath %.h ../include
vpath %.h ../lib/include

SELF_LIBS	+= -lev -lnmpev -lev -L../lib/bin -lprocessxml -lxml2 -lrtspwatch -lnmprtspparser
SELF_LIBS	+= -rdynamic -ldl
SELF_LIBS	+= ${PRJ_LIBS}

TARGET		= ../bin/nmp-mds
DEPS		= Makefile.deps

.PHONY: clean cleanall install uninstall mds_libs  mds_libs_clean

${TARGET}: ${C_OBJS} ../lib/bin/libprocessxml.a ../lib/bin/librtspwatch.a
	$(CC) -o $@ ${C_OBJS} ${SELF_LIBS}

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(DEPS)

../lib/bin/libprocessxml.a:
	make -C ../lib/process_xml

../lib/bin/librtspwatch.a:
	make -C ../lib/rtsp-watch

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

cleanall: mds_libs_clean
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

mds_libs_clean:
	make -C ../lib clean

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
