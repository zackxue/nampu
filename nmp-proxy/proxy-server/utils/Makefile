# Makefile for mod

include ../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_CPPFLAGS	+= ${PRJ_CPPFLAGS}

SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}
SELF_INCLUDES	+= -I../utils
SELF_INCLUDES	+= -I../include

vpath %.h ../include
vpath %.h ../utils

TARGET		= libutils.a
DEPS		= Makefile.deps

.PHONY: clean install uninstall

${TARGET}: ${C_OBJS}
	$(AR) -rs $@ $^

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} ${SELF_DEFINE} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(DEPS)

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
