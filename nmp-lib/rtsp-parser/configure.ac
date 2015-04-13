#                                              -*- autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libhmrtspparser], [1.2.0], [bugs@himonkey.com])
AC_CONFIG_SRCDIR([src/rtspparse.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(foreign)

#: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LN_S

# Checks for libraries.
AC_PROG_RANLIB
#LT_INIT

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lev':
AC_CHECK_LIB([ev], [ev_io_init])

# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([bzero memmove memset select socket strchr strpbrk strstr strtol strtoul])

PKG_PROG_PKG_CONFIG()
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.2 gthread-2.0 gobject-2.0])

CFLAGS="$CFLAGS $GLIB_CFLAGS"
LIBS="$LIBS $GLIB_LIBS"

AC_MSG_NOTICE([FOUND CFLAGS DEFINES : "$CFLAGS"])
AC_MSG_NOTICE([FOUND CPPFLAGS DEFINES : "$CPPFLAGS"])
AC_MSG_NOTICE([FOUND LIBS DEFINES : "$LIBS"])

AC_CONFIG_FILES([Makefile
		src/Makefile
		test/Makefile])

AC_OUTPUT
