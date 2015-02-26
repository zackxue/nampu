SUBDIRS=src/core src/lib/db_connect_pool src/lib/encrypt src/lib/process_xml src/lib/search_device src/lib/tvwall src/mod src/proto
CURRENTPATH=$(shell /bin/pwd)
INCLUDES=-I$(CURRENTPATH) \
	 -I$(CURRENTPATH)/include \
	 -I$(CURRENTPATH)/include/message \
	 -I$(CURRENTPATH)/src/lib/include \
	 -I$(CURRENTPATH)/src/lib/encrypt/inc \
	 -I$(CURRENTPATH)/src/lib/process_xml/include \
	 -I$(CURRENTPATH)/src/lib/tvwall/include \
	 -I$(HOME)/usr/include \
         -I$(HOME)/usr/include/nampu \
	 $(shell pkg-config --cflags glib-2.0 gobject-2.0 gthread-2.0)

export INCLUDES

ACLOCAL_AMFLAGS=-I m4
