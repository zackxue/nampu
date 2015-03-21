# Makefile for process_xml

include ../../Makefile.def

SELF_CFLAGS   	+= ${PRJ_CFLAGS}
SELF_DEFINE	+= ${PRJ_DEFINE} 

SELF_INCLUDES	+= ${PRJ_INCLUDES}

SELF_INCLUDES	+= -I/usr/include/mysql
SELF_INCLUDES	+= -I../../include
SELF_INCLUDES	+= -I../../include/message
SELF_INCLUDES	+= -I../lib/include
SELF_INCLUDES	+= -I./include

vpath %.h ../../include
vpath %.h ../../include/message
vpath %.h ../lib/include
vpath %.h ./include

SELF_LIBS	+= -L../mod -lmod
SELF_LIBS	+= -L../lib/bin -lprocessxml -ldbcp -lencrypt -lsearchdevice -ltvwall -lET199
SELF_LIBS	+= -L../proto -lproto
SELF_LIBS	+= -rdynamic -ldl
SELF_LIBS	+= ${PRJ_LIBS} -lxml2 -lmysqlclient

TARGET		= nmp-cms
DEPS		= Makefile.deps

.PHONY: all clean clean_all install uninstall cms_libs cms_mods cms_proto cms_libs_clean cms_mods_clean cms_proto_clean

all: ${C_OBJS} cms_libs cms_mods cms_proto 
	$(CC) -o ${TARGET} ${C_OBJS} ${SELF_LIBS}

%.o: %.c ${DEPS}
	${CC} ${SELF_CFLAGS} -o $@ -c $< ${SELF_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${SELF_CFLAGS} ${SELF_DEFINE} $^ > $@ ${SELF_INCLUDES}

-include $(DEPS)

cms_libs:
	make -C ../lib/db_connect_pool
	make -C ../lib/encrypt
	make -C ../lib/process_xml
	make -C ../lib/search_device
	make -C ../lib/tvwall

cms_mods:
	make -C ../mod

cms_proto:
	make -C ../proto

clean:
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

clean_all: cms_libs_clean cms_mods_clean cms_proto_clean
	@rm ${C_OBJS}
	@rm ${TARGET}
	@rm ${DEPS}

cms_libs_clean:
	make -C ../lib/db_connect_pool clean
	make -C ../lib/encrypt clean
	make -C ../lib/process_xml clean
	make -C ../lib/search_device clean
	make -C ../lib/tvwall clean

cms_mods_clean:
	make -C ../mod clean

cms_proto_clean:
	make -C ../proto clean

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
