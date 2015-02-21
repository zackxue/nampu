#
#        Author:
#        October 9th, 2012
#

vpath %.h $(PROJ_ROOT)/inc


export CC := $(shell echo ${CC})
export CXX := $(shell echo ${CXX})
export AR := $(shell echo ${AR})
export AS := $(shell echo ${AS})
export LD := $(shell echo ${LD})
export CFLAGS := $(shell echo ${CFLAGS})

SHARE_DIR := $(shell echo ${SHARE_PRX})

INCS += -I$(PROJ_ROOT)/inc
LIBS += -rdynamic -ldl -lpthread -lm

#INCS += -I$(TAR_DIR)/h264pack/inc
#LIBS += -L$(TAR_DIR)/h264pack/lib -lh264pack

#INCS += -I$(TAR_DIR)/rtsp/inc
#LIBS += -L$(TAR_DIR)/rtsp/lib -lrtsp

INCS += -I$(TAR_DIR)/jlib/inc
LIBS += -L$(TAR_DIR)/jlib/lib -ljlib

OUTPUT_DIR := $(PROJ_ROOT)/test

.c.o:
	${CC} ${CFLAGS} -o $@ -c $< ${INCS}

