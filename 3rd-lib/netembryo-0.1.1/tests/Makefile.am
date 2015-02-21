if BUILD_TESTS
TESTS = testsuite

check_PROGRAMS = $(TESTS)
endif

INCLUDES = -I$(top_srcdir) $(GLIB_CFLAGS)
LDADD = ../libnetembryo.la $(GLIB_LIBS)

BUILT_SOURCES = gtestmain.c

testsuite_SOURCES = \
	test_hostname.c \
	test_url.c \
	test_sock.c

dist_testsuite_SOURCES = gtestmain.c

noinst_HEADERS = gtest-extra.h

gtestmain.c: $(testsuite_SOURCES)
	$(EXUBERANT_CTAGS) -f - --file-scope=no $^ | \
		sort -k 2 | \
		$(GAWK) -f $(srcdir)/genmain.awk \
		> $@
