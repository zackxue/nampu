#                                               -*- autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.40])
AC_INIT(nmp-search-device, 2.1.0, bugs@nampu.com)
AC_CONFIG_SRCDIR([src/search_device.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(foreign)

: ${CFLAGS=""}

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S

# Checks for libraries.
AC_PROG_RANLIB
#LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

dnl initialize pkg-config so that we can use it within if else fi statements.
#PKG_PROG_PKG_CONFIG()
AC_MSG_NOTICE([FOUND CFLAGS DEFINES : "$CFLAGS"])
AC_MSG_NOTICE([FOUND CPPFLAGS DEFINES : "$CPPFLAGS"])
AC_MSG_NOTICE([FOUND LIBS DEFINES : "$LIBS"])

AC_CONFIG_FILES([Makefile
		src/Makefile
		test/Makefile])
AC_OUTPUT
