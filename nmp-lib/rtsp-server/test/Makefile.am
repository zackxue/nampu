noinst_PROGRAMS=test

test_SOURCES=main.c
test_CFLAGS=-fPIC -I../include
test_LDADD=-L../src -lnmprtspsvr -L$(HOME)/usr/lib -lnetembryo -lm -lpthread -lev -lnmpgev -lnmprtspparser
test_LDFLAGS=-D_GNU_SOURCE -fPIC
