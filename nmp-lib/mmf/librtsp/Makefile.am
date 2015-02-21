#!/bin/sh

SUBDIRS=src/lib src/test
CURRENTPATH=$(shell /bin/pwd)
INCLUDES=-I$(CURRENTPATH) \
	 -I$(CURRENTPATH)/src/inc 

export INCLUDES

ACLOCAL_AMFLAGS=-I m4

#pkgconfigdir=$(libdir)/pkgconfig
#pkgconfig_DATA=librtsp.pc

