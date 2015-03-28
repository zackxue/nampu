AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libnmpshare.la
libnmpshare_la_SOURCES=nmp_debug.c	\
                      nmp_signal.c \
                      nmp_conn.c \
                      nmp_fragments.c \
                      nmp_fragments.h \
                      nmp_hlio.c \
                      nmp_hlio.h \
                      nmp_io.c \
                      nmp_io.h \
                      nmp_netbuf.c \
                      nmp_netbuf.h \
                      nmp_net.c \
                      nmp_netio.c \
                      nmp_watch.c \
                      nmp_watch.h \
                      nmp_rw_file.c \
                      nmp_timer.c

libnmpshare_la_LIBADD=

libnmpshare_la_LDFLAGS=-no-undefined -version-info $(LIBNMPSHARE_SO_VERSION) $(shell pkg-config --libs glib-2.0 gobject-2.0 gthread-2.0) -lrt
libnmpshare_la_LIBFLAGS=

libnmpshare_la_CFLAGS=-Wall -g -fno-inline -fstack-protector -fPIC
libnmpshare_la_CXXFLAGS=

libnmpshareincludedir=$(prefix)/include/nampu/share
libnmpshareinclude_HEADERS=$(top_srcdir)/include/*.h

