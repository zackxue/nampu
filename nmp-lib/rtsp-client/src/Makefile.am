AUTOMAKE_OPTIONS=foreign subdir-objects
lib_LIBRARIES=libnmprtspclient.a
libnmprtspclient_a_SOURCES=./rtp/rtp_fake.c \
			./rtp/rtpparser.c \
			./rtsp/rtsp.c \
			./rtsp/rtsp_describe.c \
			./rtsp/rtsp_method.h \
			./rtsp/rtsp_option.c \
			./rtsp/rtsp_play.c \
			./rtsp/rtsp_setup.c \
			./rtsp/rtsp_teardown.c \
			./rtsp/rtsp_url.c \
			./rtsp-watch/rtspwatch.c \
			./rtsp-watch/rtspwatch.h \
			./rtsp-watch/io_watch.c \
			./rtsp-watch/io_watch.h \
			fnc_log.c \
			media.c \
			ports.c \
			rtsp_client.c \
			rtsp-watch \
			sock.c

libnmprtspclient_a_CFLAGS=-Wall -fno-inline -fstack-protector -fPIC

libnmprtspclientincludedir=$(prefix)/include/nampu/rtsp-client

libnmprtspclientinclude_HEADERS=$(top_srcdir)/include/*.h \
				$(top_srcdir)/src/rtsp-watch/*.h
