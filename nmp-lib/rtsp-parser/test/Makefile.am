noinst_PROGRAMS=test

test_SOURCES=main.c
test_CFLAGS=-fPIC
test_LDADD=-L$(top_srcdir)/src -lnmprtspparser -L$(HOME)/usr/lib -lev -lpthread -lm
test_LDFLAGS=-D_GNU_SOURCE -fPIC
