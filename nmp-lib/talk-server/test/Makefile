#
#        Author:
#        October 9th, 2012
#

include ../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= -I../include

vpath %.h ../include

SELF_LIBS	+= -L../core -ltalksvr

SELF_LIBS	+= ${PRJ_LIBS}
SELF_LIBS	+= -lnmplib

SELF_LDFLAGS	+= ${PRJ_LDFLAGS}

TARGET	= ./test
DEPS	= Makefile.deps

.PHONY: clean

$(TARGET): $(C_OBJS)
	${CC} -o $@ ${SELF_LDFLAGS} $^ ${SELF_LIBS}

%.o : %.c ${DEPS}
	${CC} ${SELF_CFLAGS} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): ${C_SRCS}
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(DEPS)

clean:
	@rm -rf ${C_OBJS} ${DEPS} ${TARGET}
