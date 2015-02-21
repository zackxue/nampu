noinst_PROGRAMS=test
test_SOURCES=main.c
test_CFLAGS=-fPIC
test_LDADD=-L$(top_srcdir)/src -lnmprtspclient \
	-L$(HOME)/usr/lib -lnmprtspparser \
	-lnmpev -lev

test_LDFLAGS = -D_GNU_SOURCE -fPIC
