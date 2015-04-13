#!/bin/sh
SUBDIRS=src test
CURRENTPATH=$(shell /bin/pwd)
INCLUDES=-I$(CURRENTPATH) \
	 -I$(CURRENTPATH)/include \
	 -I$(NMP_LIB_INSTALL)/include
export INCLUDES

ACLOCAL_AMFLAGS=-I m4

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libnmpxml.pc

