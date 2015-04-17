# Makefile for mod

include ../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_CPPFLAGS	+= ${PRJ_CPPFLAGS}

SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}
SELF_INCLUDES	+= -I../utils
SELF_INCLUDES	+= -I../include
SELF_INCLUDES	+= -I./include

vpath %.h ../include
vpath %.h ../utils
vpath %.h ./include

TARGET		= libmod.a
C_DEPS		= Makefile.c.deps
CPP_DEPS	= Makefile.cpp.deps

.PHONY: clean install uninstall

${TARGET}: ${C_OBJS} ${CPP_OBJS}
	$(AR) -rs $@ $^

%.o: %.c ${C_DEPS}
	${CC} ${SELF_CFLAGS} ${SELF_DEFINE} -o $@ -c $< ${SELF_INCLUDES}

%.o: %.cpp ${CPP_DEPS}
	${C++} ${SELF_CPPFLAGS} ${SELF_DEFINE} -o $@ -c $< ${SELF_INCLUDES}

$(C_DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

$(CPP_DEPS): $(CPP_SRCS)
	${C++} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(C_DEPS)
-include $(CPP_DEPS)

clean:
	@rm ${C_OBJS} ${CPP_OBJS}
	@rm ${TARGET}
	@rm ${C_DEPS} ${CPP_DEPS}

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
