AUTOMAKE_OPTIONS=foreign subdir-objects
lib_LTLIBRARIES=libnmpframework.la
libnmpframework_la_SOURCES=./app/nmp_appcore.c \
			./app/nmp_appmod.c \
			./app/nmp_appobj.c \
			./app/nmp_mod_access.c \
			./app/nmp_object.c \
			./islot/nmp_busslot.c \
			./islot/nmp_islot.c \
			./islot/nmp_modio.c \
			./mm/nmp_memory.c \
			./msg/nmp_data.c \
			./msg/nmp_etable.c \
			./msg/nmp_msgbus.c \
			./msg/nmp_msgengine.c \
			./msg/nmp_sysmsg.c

libnmpframework_la_LIBADD=

libnmpframework_la_LDFLAGS=-no-undefined -version-info $(LIBNMPFRAMEWORK_SO_VERSION) $(shell pkg-config --libs glib-2.0 gobject-2.0 gthread-2.0) -lrt
libnmpframework_la_LIBFLAGS=


libnmpframework_la_CFLAGS=-Wall -fno-inline -fstack-protector -pthread

libnmpframeworkincludedir=$(prefix)/include/nampu/framework
libnmpframeworkinclude_HEADERS=$(top_srcdir)/include/*.h
