AUTOMAKE_OPTIONS=foreign subdir-objects
noinst_PROGRAMS=cms
cms_SOURCES=./main.c \
	    ./ctl/nmp_args.c \
	    ./ctl/nmp_sysctl.c \
	    ./msg/nmp_message.c

cms_LDADD=../lib/db_connect_pool/src/libdbcp.a \
	   ../lib/encrypt/src/libencrypt.a \
	   ../lib/encrypt/libET199.a
	   ../lib/process_xml/src/libprocessxml.a \
	   ../lib/search_device/src/libsearchdevice.a \
	   ../lib/tvwall/src/libtvwall.a \
	   ../mod/libcmsmod.a

cms_CFLAGS=-Wall -g -fno-inline -fstack-protector -fPIC -lrt
cms_CXXFLAGS=
