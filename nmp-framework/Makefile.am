SUBDIRS = src
CURRENTPATH=$(PWD)

INCLUDES = -I$(CURRENTPATH)/include \
           -I$(HOME)/usr/include \
	   -I$(HOME)/usr/include/nampu \
	   $(shell pkg-config --cflags glib-2.0 gobject-2.0)

export INCLUDES
