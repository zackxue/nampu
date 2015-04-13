#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT(libnmplib, 1.1.0, bugs@nampu.com)
AC_CONFIG_SRCDIR(test/main.c)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE(foreign)

#: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_PROG_RANLIB
#AC_PROG_LIBTOOL
#LT_INIT

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

PKG_PROG_PKG_CONFIG()

AC_MSG_NOTICE([FOUND CFLAGS DEFINES : "$CFLAGS"])
AC_MSG_NOTICE([FOUND CPPFLAGS DEFINES : "$CPPFLAGS"])
AC_MSG_NOTICE([FOUND LIBS DEFINES : "$LIBS"])

AC_OUTPUT([Makefile
	   src/Makefile
	   test/Makefile])
