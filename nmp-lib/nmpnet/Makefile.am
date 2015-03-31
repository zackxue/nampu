#!/bin/sh
SUBDIRS = src test
CURRENTPATH=$(PWD)
INCLUDES=-I$(CURRENTPATH) \
	 -I$(CURRENTPATH)/include \
	 -I$(NMP_LIB_INSTALL)/include \
	 -I$(NMP_LIB_INSTALL)/include/nampu/nmplib

export INCLUDES

ACLOCAL_AMFLAGS=-I m4
