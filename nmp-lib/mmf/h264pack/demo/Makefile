# Makefile for demo
#

include ../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}
SELF_INCLUDES	+= -I../include
SELF_INCLUDES	+= -I./

SELF_LIBS	+= -L../lib -lmmfh264pack
SELF_LIBS	+= ${PRJ_LIBS}

vpath %.h ../include
vpath %.h ./

TARGET		= ./demo
DEPS		= Makefile.dep

.PHONY: clean install uninstall

${TARGET}: ${C_OBJS}
	$(CC) -o $@ ${C_OBJS} ${SELF_LIBS}

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} ${SELF_DEFINE} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
