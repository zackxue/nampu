#SUBDIRS = log media rtp rtsp utils reverse_connector
AUTOMAKE_OPTIONS=foreign subdir-objects
lib_LIBRARIES=libnmprtspsvr.a
libnmprtspsvr_a_SOURCES=rtsp-server.c \
			./log/rc_log.c \
			./media/demuxer/demuxer_nmp.c \
			./media/demuxer/demuxer_ls.c \
			./media/parser/nmpma.c \
			./media/parser/nmpmv.c \
			./media/demuxer.c \
			./media/demuxer_module.h \
			./media/media.c \
			./media/mediaparser.c \
			./media/mediautils.c \
			./reverse_connector/reverse_connector.c \
			./rtp/rtp.c \
			./rtp/rtp_port.c \
			./rtp/rtp_watch.c \
			./rtp/rtp_watch.h \
			./rtsp/rtsp.c \
			./rtsp/rtsp_method_describe.c \
			./rtsp/rtsp_method_gsparam.c \
			./rtsp/rtsp_method_options.c \
			./rtsp/rtsp_method_pause.c \
			./rtsp/rtsp_method_play.c \
			./rtsp/rtsp_method_setup.c \
			./rtsp/rtsp_method_teardown.c \
			./rtsp/rtsp_methods.h \
			./utils/bufferqueue.c \
			./utils/fdset.c \
			./utils/ports.c

#libnmprtspsvr_a_LIBADD=
libnmprtspsvr_a_CFLAGS=-Wall -g -fno-inline -fstack-protector -fPIC
#nmprtspsvr_a_CXXFLAGS=

libnmprtspsvrincludedir=$(prefix)/include/nampu/rtsp-server
libnmprtspsvrinclude_HEADERS=$(top_srcdir)/include/*.h
