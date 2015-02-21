SUBDIRS =	wsocket \
		utils \
		pkgconfig \
		. \
		tests

EXTRA_DIST = Doxyfile

lib_LTLIBRARIES = libnetembryo.la

pkginclude_HEADERS =    netembryo/wsocket.h \
			netembryo/wsocket-internal.h \
			netembryo/url.h \
			netembryo/rtsp.h

libnetembryo_la_SOURCES =

libnetembryo_la_CFLAGS = $(GLIB_CFLAGS)
libnetembryo_la_LIBADD = \
	wsocket/libwsocket.la \
	utils/libutils.la \
	$(GLIB_LIBS)

if HAVE_SCTP
libnetembryo_la_LIBADD += $(SCTP_LIBS)
endif

libnetembryo_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

ACLOCAL_AMFLAGS = -I m4
