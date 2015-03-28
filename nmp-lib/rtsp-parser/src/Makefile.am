AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libnmprtspparser.a
libnmprtspparser_a_SOURCES=base64.c \
			rtspctx.c \
			rtspdefs.c \
			rtspmsg.c \
			rtspparse.c \
			rtsptransport.c \
			rtspurl.c \
			sdp.c

#nmprtspparser_a_LIBADD=

#libnmprtspparser_a_LDFLAGS=
#libnmprtspparser_a_LIBFLAGS=

libnmprtspparser_a_CFLAGS=-Wall -fno-inline -fstack-protector -fPIC
libnmprtspparser_a_CXXFLAGS=

libnmprtspparserincludedir=$(prefix)/include/nampu/rtsp-parser
libnmprtspparserinclude_HEADERS=$(top_srcdir)/include/*.h
