noinst_PROGRAMS=test

test_SOURCES=main.c
test_LDADD=../src/libnmpsearchdevice.a -L$(NMP_LIB_INSTALL)/lib -lnmpmb -lpthread -lm -lrt
test_LDFLAGS=-D_GNU_SOURCE -fPIC
