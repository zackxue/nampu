AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libnmpxml.la
libnmpxml_la_SOURCES=nmp_xmlmem.c xml-tree.c xml-merge.c xml-parse.c nmp_transform.c nmp_xmlhandler.c nmp_xmlmsg.c 
#libnmpxml_la_LIBADD=libmxml.a
libnmpxml_la_LDFLAGS=-no-undefined -version-info $(LIBNMPXML_SO_VERSION) -L$(NMP_LIB_INSTALL)/lib -lmxml
libnmpxml_la_CFLAGS=-fPIC
libnmpxml_la_CXXFLAGS=-fPIC


libnmpxmlincludedir=$(prefix)/include/nampu
libnmpxmlinclude_HEADERS=$(top_srcdir)/include/nmp_sdk.h \
		       $(top_srcdir)/include/nmp_packet.h \
		       $(top_srcdir)/include/nmp_xmlmsg.h \
		       $(top_srcdir)/include/nmp_transform.h \
 		       $(top_srcdir)/include/proxy_info.h 

