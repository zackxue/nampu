noinst_PROGRAMS=test

test_SOURCES=main.c
test_LDADD=$(top_srcdir)/src/lib/libmmfrtsp.a -L$(NMP_LIB_INSTALL)/lib -lev
test_LDFLAGS=-D_GNU_SOURCE -fPIC -lm
