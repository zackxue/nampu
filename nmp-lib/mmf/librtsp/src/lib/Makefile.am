AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libmmfrtsp.a
libmmfrtsp_a_SOURCES=rtsp_mem.c rtsp_msg.c rtsp_defs.c rtsp_sdp.c rtsp_ctx.c rtsp_parse.c rtsp_url.c rtsp_transport.c

libmmfrtsp_a_CFLAGS=-fPIC -fvisibility=hidden
libmmfrtsp_a_CXXFLAGS=-fPIC -fvisibility=hidden

libmmfrtspincludedir=$(prefix)/include/nampu/mmfrtsp
libmmfrtspinclude_HEADERS=$(top_srcdir)/src/inc/*.h 
