# Makefile for encrypt

include ../../../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}
SELF_INCLUDES	+= -I../include
SELF_INCLUDES	+= -I./include

vpath %.h ../include
vpath %.h ./include

TARGET		= libencrypt.a
DEPS		= Makefile.deps

.PHONY: all clean install uninstall

all: ${C_OBJS} 
	${AR} -r ${TARGET} $^
	mv ${TARGET} ../bin/
	cp ./src/libET199.a ../bin/

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(DEPS)

clean:
	@rm ${C_OBJS}
	@rm ../bin/${TARGET}
	@rm ../bin/libET199.a
	@rm ${DEPS}

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
