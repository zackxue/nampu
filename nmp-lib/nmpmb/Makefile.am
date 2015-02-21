#!/bin/sh
SUBDIRS=src test

CURRENTPATH=$(shell /bin/pwd)
INCLUDES=-I$(CURRENTPATH) \
	 -I$(CURRENTPATH)/include \
	 -I$(NMP_LIB_INSTALL)/include/nampu

export INCLUDES

ACLOCAL_AMFLAGS=-I m4
