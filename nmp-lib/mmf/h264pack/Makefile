#20101015 by maohw
#=============================================#
#CROSS:	指定使用的编译工具前缀, 如果CROSS不存在，则为空。

#CFLAGS31 = -march=armv7-a -mcpu=cortex-a9 -mfloat-abi=softfp -mfpu=vfpv3-d16
#CROSS ?= arm-hisiv100nptl-linux-
#CROSS ?= arm-hismall-linux-

export CC := $(shell echo ${CC})
export CXX := $(shell echo ${CXX})
export AR := $(shell echo ${AR})
export AS := $(shell echo ${AS})
export LD := $(shell echo ${LD})

export CFLAGS := $(shell echo ${CFLAGS})

SHARE_DIR := $(shell echo ${SHARE_PRX})

ifneq ($(CROSS),)
	CPU_TYPE := _ARM_ 
else
	CPU_TYPE := _X86_ 
endif

#指定使用的shell命令。
export CP = cp
export RM = rm
export INSTALL = cp -rfv

# ============= 输出目录定义 ===========# 
#EXP_TARGET_DIR: 如果 EXP_TARGET_DIR不存在，则为 ./lib
#EXP_INC_DIR:    如果 EXP_INC_DIR不存在，则为 ./inc

EXP_TARGET_DIR ?= $(PWD)/lib
EXP_INC_DIR    ?= $(PWD)/inc


# ============= demo compile option ===========#
DEMO_TARGET := demo
DEMO_SRC    := ./demo
DEMO_CFLAGS := $(CFLAGS31) -D$(CPU_TYPE) -g -Wall -I./ -I$(EXP_INC_DIR) -DMODULE_NAME="\"\\\"demo\"\\\""
DEMO_CPPFLAGS:=$(DEMO_CFLAGS)
DEMO_LDFLAGS:= -lpthread -L./ -L$(EXP_TARGET_DIR) -lh264pack -lrt
# ---------------------------------------------#
DEMO_MAKE_PARAM := -C $(DEMO_SRC) \
		TARGET=$(DEMO_TARGET)\
		EXPORT_TARGET_DIR=$(EXP_TARGET_DIR)\
		CFLAGS="$(DEMO_CFLAGS)"\
		CXXFLAGS="$(DEMO_CPPFLAGS)"\
		LDFLAGS="$(DEMO_LDFLAGS)"



# ============= lib compile option ===========#
LIB_TARGET  := libh264pack.a
EXP_LIB_INC := h264pack.h nalu.h rtp.h buf_pool.h h264_filter.h media_structs.h h264_framer.h
LIB_SRC     := ./src
LIB_CFLAGS  := $(CFLAGS31) -D$(CPU_TYPE) -g -Wall -I./ -I$(EXP_INC_DIR) \
	-DMODULE_NAME="\"\\\"h264pack\"\\\""

TAR_LIB := $(EXP_TARGET_DIR)/$(LIB_TARGET)
TAR_INC := $(EXP_INC_DIR)/*.h
INS_DIR := $(TAR_DIR)/h264pack

LIB_CPPFLAGS:=$(LIB_CFLAGS)
LIB_LDFLAGS := -lpthread -L./ 
# ---------------------------------------------#
LIB_MAKE_PARAM := -C $(LIB_SRC) \
		TARGET=$(LIB_TARGET)\
		EXPORT_INC_FILE="$(EXP_LIB_INC)"\
		EXPORT_TARGET_DIR=$(EXP_TARGET_DIR)\
		EXPORT_INC_DIR=$(EXP_INC_DIR)\
		CFLAGS="$(LIB_CFLAGS)"\
		CXXFLAGS="$(LIB_CFLAGS)"\
		LDFLAGS="$(LIB_LDFLAGS)"


.PHONY: default clean install uninstall

default:
	make $(LIB_MAKE_PARAM)
	make $(DEMO_MAKE_PARAM)
clean:
	make clean NODEP=yes $(LIB_MAKE_PARAM)
	make clean NODEP=yes $(DEMO_MAKE_PARAM)

install:
	test -z $(INS_DIR)/lib || mkdir -p $(INS_DIR)/lib
	install -m 644 $(TAR_LIB) $(INS_DIR)/lib
	test -z $(INS_DIR)/inc || mkdir -p $(INS_DIR)/inc
	install -m 644 $(TAR_INC) $(INS_DIR)/inc

uninstall:
	rm -f $(INS_DIR)/lib/$(TAR_LIB)
	rm -f $(INS_DIR)/inc/$(TAR_INC)




