#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(cms, 2.1.0, bugs@nampu.com)
AC_CONFIG_SRCDIR(src/core/main.c)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE(foreign)

: ${CFLAGS=""}

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
LT_INIT
AC_PROG_RANLIB

# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [timer_create])
# FIXME: Replace `main' with a function in `-lxml2':
AC_CHECK_LIB([xml2], [UTF8ToHtml])

PKG_CHECK_MODULES([libgobject],[gobject-2.0])
PKG_CHECK_MODULES([libgthread], [gthread-2.0])
PKG_CHECK_MODULES([libglib], [glib-2.0 >= 2.28])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h syslog.h termios.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([bzero clock_gettime inet_ntoa localtime_r memset select setenv socket strerror strstr tzset])

AC_CONFIG_FILES([Makefile
                 src/lib/db_connect_pool/Makefile
                 src/lib/encrypt/Makefile
                 src/lib/process_xml/Makefile
                 src/lib/search_device/Makefile
                 src/lib/tvwall/Makefile
                 src/mod/Makefile
                 src/proto/Makefile
		 src/core/Makefile])
AC_OUTPUT
