AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libnmpshare.la
libnmpshare_la_SOURCES=nmp_share_debug.c	\
                      nmp_share_signal.c \
                      nmp_share_conn.c \
                      nmp_share_fragments.c \
                      nmp_share_fragments.h \
                      nmp_share_hlio.c \
                      nmp_share_hlio.h \
                      nmp_share_io.c \
                      nmp_share_io.h \
                      nmp_share_netbuf.c \
                      nmp_share_netbuf.h \
                      nmp_share_net.c \
                      nmp_share_netio.c \
                      nmp_share_watch.c \
                      nmp_share_watch.h \
                      nmp_share_rw_file.c \
                      nmp_share_timer.c

libnmpshare_la_LIBADD=

libnmpshare_la_LDFLAGS=-no-undefined -version-info $(LIBNMPSHARE_SO_VERSION) $(shell pkg-config --libs glib-2.0 gobject-2.0 gthread-2.0) -lrt
libnmpshare_la_LIBFLAGS=

libnmpshare_la_CFLAGS=-Wall -fno-inline -fstack-protector -fPIC -pthread
libnmpshare_la_CXXFLAGS=

libnmpshareincludedir=$(prefix)/include/nampu/share
libnmpshareinclude_HEADERS=$(top_srcdir)/include/*.h

