AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libnmpnet.a
libnmpnet_a_SOURCES=nmp_conn.c \
		   nmp_hlio.c \
		   nmp_io.c \
		   nmp_netbuf.c \
		   nmp_net.c \
		   nmp_netio.c \
	           nmp_scheduler.c \
		   nmp_timer.c \
		   nmp_watch.c
libnmpnet_a_CFLAGS=-fPIC -pthread
libnmpnet_a_CXXFLAGS=

libnmpnetincludedir=$(prefix)/include/nampu/nmpnet
libnmpnetinclude_HEADERS=$(top_srcdir)/include/*.h 
