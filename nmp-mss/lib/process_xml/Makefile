# Makefile for process_xml
# Author:
# Sep 9th, 2012
#

include ../../Makefile.def

SELF_CFLAGS	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}
SELF_INCLUDES	+= -I/usr/include/libxml2

SELF_INCLUDES	+= -I./include
SELF_INCLUDES	+= -I../include
SELF_INCLUDES	+= -I../../include

vpath %.h ./include
vpath %.h ../include
vpath %.h ../../include

TARGET	= ../bin/libprocessxml.a
DEPS	= Makefile.deps

.PHONY: clean install uninstall

${TARGET}: ${C_OBJS} 
	$(AR) -rs ${TARGET} ${C_OBJS}

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} -o $@ -c $< ${SELF_INCLUDES}

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

