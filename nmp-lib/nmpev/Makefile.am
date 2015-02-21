SUBDIRS = src
CURRENTPATH=$(PWD)

GLIB_VERSION = 2.0
GLIB_LIBS = glib-${GLIB_VERSION} gobject-${GLIB_VERSION}

INCLUDES = -I$(CURRENTPATH)/include \
	$(shell pkg-config --cflags ${GLIB_LIBS}) \
	-I$(HOME)/usr/include \
	-I$(HOME)/usr/include/himickey

export INCLUDES

#pkgconfigdir=$(libdir)/pkgconfig
#pkgconfig_DATA=libhmgev.pc
