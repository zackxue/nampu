# Makefile for lib process_xml and rtsp-watch

EXCLUDE_DIRS := include bin

DIRS := ${shell find . -maxdepth 1 -type d}
DIRS := ${basename ${patsubst ./%, %, ${DIRS}}}
DIRS := ${filter-out ${EXCLUDE_DIRS}, ${DIRS}}

SUB_DIRS	:= ${DIRS}
CLEAN_DIRS 	:= $(addprefix _cls_, $(SUB_DIRS))

.PHONY: all clean ${SUB_DIRS}

all: ${SUB_DIRS}

${SUB_DIRS}:
	${MAKE} -C $@

clean: ${CLEAN_DIRS}

${CLEAN_DIRS}: 
	${MAKE} -C $(patsubst _cls_%, %, $@) clean
