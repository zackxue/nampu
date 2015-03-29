# Makefile for test

include ../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}

SELF_INCLUDES	+= -I../include

vpath %.h ../include

SELF_LIBS	+= -L../lib -ltinyrain
SELF_LIBS	+= ${PRJ_LIBS}

TARGET		= ./test
DEPS		= Makefile.deps

.PHONY: clean install uninstall 

${TARGET}: ${C_OBJS} ../lib/libtinyrain.a
	$(CC) -o $@ ${C_OBJS} ${SELF_LIBS}

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} ${SELF_DEFINE} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(DEPS)

../lib/libtinyrain.a:
	make -C ../framework

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
