AUTOMAKE_OPTIONS=foreign subdir-objects
lib_LIBRARIES=libnmpframework.a
libnmpframework_a_SOURCES=./app/nmp_appcore.c \
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
libnmpframework_a_CFLAGS=-Wall -g -fno-inline -fstack-protector

libnmpframeworkincludedir=$(prefix)/include/nampu
libnmpframeworkinclude_HEADERS=$(top_srcdir)/include/*.h
