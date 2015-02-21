AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=librtsp.a
librtsp_a_SOURCES=rtsp_mem.c rtsp_msg.c rtsp_defs.c rtsp_sdp.c rtsp_ctx.c rtsp_parse.c rtsp_url.c rtsp_transport.c
#librtsp_la_LIBADD=
#librtsp_la_LIBFLAGS=-shared
librtsp_a_CFLAGS=-fPIC -fvisibility=hidden
librtsp_a_CXXFLAGS=-fPIC -fvisibility=hidden

librtspincludedir=$(prefix)/inc
librtspinclude_HEADERS=$(top_srcdir)/src/inc/*.h 


