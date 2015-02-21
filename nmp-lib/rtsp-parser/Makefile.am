SUBDIRS = src test
CURRENTPATH=$(PWD)

GLIB_VERSION = 2.0
GLIB_LIBS = glib-${GLIB_VERSION} gobject-${GLIB_VERSION} gthread-${GLIB_VERSION}

INCLUDES = -I$(CURRENTPATH)/include -I$(shell pkg-config --cflags ${GLIB_LIBS})

export INCLUDES
