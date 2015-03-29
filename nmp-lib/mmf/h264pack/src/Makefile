# Makefile for libh264pack
#

include ../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}
SELF_INCLUDES	+= -I../include
SELF_INCLUDES	+= -I./

vpath %.h ../include
vpath %.h ./

TARGET	:= ../lib/libmmfh264pack.a
DEPS	:= Makefile.dep

ifneq (${NMP_LIB_INSTALL},)
	DESTDIR	:= ${NMP_LIB_INSTALL}
else
	DESTDIR	:= /usr/local
endif

LIB_HEADERS = ../include/*.h

.PHONY: clean install uninstall

${TARGET}: ${C_OBJS}
	test -z ../lib || mkdir -p ../lib
	${AR} -rs $@ $^ 

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} ${SELF_DEFINE} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

install: ${TARGET}
	test -z ${DESTDIR}/lib || mkdir -p ${DESTDIR}/lib
	install -m 644 ${TARGET} ${DESTDIR}/lib
	test -z ${DESTDIR}/include/nampu/mmfh264pack || mkdir -p ${DESTDIR}/include/nampu/mmfh264pack
	install -m 644 ${LIB_HEADERS} ${DESTDIR}/include/nampu/mmfh264pack

uninstall:
	rm -rf ${DESTDIR}/include/nampu/mmfh264pack
	rm -f ${DESTDIR}/lib/${TARGET}
