AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libnmplib.a
libnmplib_a_SOURCES=nmp_mem.c \
 		   nmp_list.c \
		   nmp_queue.c \
		   nmp_asyncqueue.c \
		   nmp_thread.c \
		   nmp_thread_posix.c \
		   nmp_thread_windows.c \
		   nmp_threadpool.c \
		   nmp_event.c \
		   nmp_sched.c \
		   nmp_base64.c
libnmplib_a_CFLAGS=-Wall -g -fno-inline -fstack-protector -fPIC
#libnmplib_a_LIBADD=$(HMDEV_LIB_INSTALL)/lib/libev.a
#libnmplib_a_LDADD=$(HMDEV_LIB_INSTALL)/lib/libev.a
#libnmplib_a_LDFLAGS=

libnmplibincludedir=$(prefix)/include/nampu
libnmplibinclude_HEADERS=$(top_srcdir)/include/*.h
