#                                              -*- autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libhmrtspsvr], [1.2.0], [bugs@himonkey.com])
AC_CONFIG_SRCDIR([src/rtsp-server.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(foreign)

: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AM_PROG_AR
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h sys/socket.h sys/time.h sys/timeb.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([bzero memset gettimeofday strchr strrchr strstr])

# dnl initialize pkg-config so that we can use it within if else fi statements.
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
