noinst_PROGRAMS=test

test_SOURCES=main.c
test_LDADD=$(top_srcdir)/src/lib/librtsp.a -L$(TAR_DIR)/ev/lib -lev
test_LDFLAGS=-D_GNU_SOURCE -fPIC -lm
test_CFLAGS=-I$(TAR_DIR)/ev/inc


