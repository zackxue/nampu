#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT(libnmpnet, 1.2.0, bugs@himickey.com)
AC_CONFIG_SRCDIR(src/nmp_watch.c)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(foreign)

: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_PROG_RANLIB
#LT_INIT
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

#PKG_CHECK_MODULES(GLIB, [glib-2.0])
#CFLAGS="$CFLAGS $GLIB_CFLAGS"
#LIBS="$LIBS $GLIB_LIBS"

dnl initialize pkg-config so that we can use it within if else fi statements.
#PKG_PROG_PKG_CONFIG()

AC_MSG_NOTICE([FOUND CFLAGS DEFINES : "$CFLAGS"])
AC_MSG_NOTICE([FOUND CPPFLAGS DEFINES : "$CPPFLAGS"])
AC_MSG_NOTICE([FOUND LIBS DEFINES : "$LIBS"])

AC_OUTPUT([Makefile
	   src/Makefile
           test/Makefile])
