#!/bin/sh

SUBDIRS=src/lib src/test
CURRENTPATH=$(shell /bin/pwd)
INCLUDES=-I$(CURRENTPATH) \
	 -I$(CURRENTPATH)/src/inc \
	 -I$(NMP_LIB_INSTALL)/include

export INCLUDES

ACLOCAL_AMFLAGS=-I m4
