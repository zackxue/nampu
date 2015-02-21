#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(libnmpframework, 1.1.0, bugs@nampu.com)
AC_CONFIG_SRCDIR([src/app/nmp_appcore.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(foreign)

: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CC
AM_PROG_AR

# Checks for libraries.
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([memset])

# dnl initialize pkg-config so that we can use it within if else fi statements.
PKG_PROG_PKG_CONFIG()
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.2 gthread-2.0 gobject-2.0], , AC_MSG_ERROR([You need Glib >= 2.2 to build $PACKAGE]))

AC_MSG_NOTICE([FOUND CFLAGS DEFINES : "$CFLAGS"])
AC_MSG_NOTICE([FOUND CPPFLAGS DEFINES : "$CPPFLAGS"])
AC_MSG_NOTICE([FOUND LIBS DEFINES : "$LIBS"])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
