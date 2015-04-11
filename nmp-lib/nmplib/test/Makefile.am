noinst_PROGRAMS=test

test_SOURCES=main.c
test_CFLAGS=-Wall -g -fno-inline -fstack-protector -fPIC -I../include -I$(NMP_LIB_INSTALL)/include
test_LDADD=-L$(top_srcdir)/src -lnmplib -lpthread -lrt
test_LDFLAGS=-D_GNU_SOURCE -fPIC
