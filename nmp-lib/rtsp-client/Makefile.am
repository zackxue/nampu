#!/bin/sh
SUBDIRS = src test

CURRENTPATH=$(PWD)
INCLUDES= -I$(CURRENTPATH) \
	-I$(CURRENTPATH)/include \
	-I$(CURRENTPATH)/src/rtsp-watch \
	-I$(NMP_LIB_INSTALL)/include \
	-I$(NMP_LIB_INSTALL)/include/nampu/nmpev \
	-I$(NMP_LIB_INSTALL)/include/nampu/rtsp-parser

export INCLUDES

ACLOCAL_AMFLAGS=-I m4
