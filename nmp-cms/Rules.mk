#
#        Author: HiMickey
#        Sep 9th, 2010
#

vpath %.h &(PROJ_SHARE)/inc
vpath %.h $(PROJ_ROOT)/include
vpath %.h $(PROJ_ROOT)/lib/include

CC := gcc
CFLAGS := -Wall -g -fno-inline -fstack-protector

GLIB_VERSION := 2.0
GLIB_LIBS := glib-${GLIB_VERSION} gobject-${GLIB_VERSION} gthread-${GLIB_VERSION}
MYSQL_INC := -I/usr/include/mysql
MYSQL_LIB := -L/usr/lib/i386-linux-gnu -lmysqlclient

INCS += $(shell pkg-config --cflags ${GLIB_LIBS})
INCS += -I$(PROJ_ROOT)/include -I$(PROJ_ROOT)/lib/include -I$(PROJ_ROOT)/include/message -I$(NMP_LIB_INSTALL)/include/nampu -I$(NMP_LIB_INSTALL)/include/nampu
INCS += -I /usr/include/libxml2 
INCS += $(MYSQL_INC)

#LIBS += -rdynamic -ldl
LIBS += $(shell pkg-config --libs ${GLIB_LIBS})
LIBS += -lxml2
LIBS += $(MYSQL_LIB)
LIBS += -L$(NMP_LIB_INSTALL)/lib -lnmpshare
LIBS += -L$(NMP_LIB_INSTALL)/lib -lnmpframework

OUTPUT_DIR := $(PROJ_ROOT)/bin

.c.o:
	${CC} ${CFLAGS} -o $@ -c $< ${INCS}
