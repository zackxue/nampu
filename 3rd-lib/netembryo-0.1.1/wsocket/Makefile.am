
netembryoincludedir = $(top_srcdir)/netembryo

noinst_LTLIBRARIES = libwsocket.la

libwsocket_la_LIBADD =
libwsocket_la_CFLAGS =

libwsocket_la_SOURCES =		multicast.c \
				socket.c \
				sock_ntop_host.c \
				get_info.c \
				Sock.c

if ENABLE_SSL
libwsocket_la_SOURCES += ssl.c ssl.h
libwsocket_la_CFLAGS += $(OPENSSL_CFLAGS)
libwsocket_la_LIBADD += $(OPENSSL_LIBS)
endif

INCLUDES = -I$(netembryoincludedir) -I$(top_srcdir)
