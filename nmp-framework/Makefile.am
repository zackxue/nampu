SUBDIRS = src
CURRENTPATH=$(PWD)

INCLUDES = -I$(CURRENTPATH)/include \
           -I$(NMP_LIB_INSTALL)/include \
	   -I$(NMP_LIB_INSTALL)/include/nampu/share \
	   $(shell pkg-config --cflags glib-2.0 gobject-2.0 gthread-2.0)

export INCLUDES

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=nmpframework.pc
