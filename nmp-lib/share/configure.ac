#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libnmpshare], [2.8.0], [bugs@nampu.com])
AC_CONFIG_SRCDIR([src/nmp_io.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(foreign)

: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_PROG_LIBTOOL
LT_INIT

PKG_CHECK_MODULES([libgthread], [gthread-2.0])
PKG_CHECK_MODULES([libglib], [glib-2.0 >= 2.28])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_MODE_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([bzero clock_gettime ftruncate getcwd localtime_r memset mkdir socket])

dnl initialize pkg-config so that we can use it within if else fi statements.
PKG_PROG_PKG_CONFIG()

AC_MSG_NOTICE([FOUND CFLAGS DEFINES : "$CFLAGS"])
AC_MSG_NOTICE([FOUND CPPFLAGS DEFINES : "$CPPFLAGS"])
AC_MSG_NOTICE([FOUND LIBS DEFINES : "$LIBS"])

AC_CONFIG_FILES([Makefile
           libnmpshare.pc
           src/Makefile])

AC_OUTPUT
