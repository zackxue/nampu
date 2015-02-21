/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define this if ATTR_UNUSED is not supported */
#define ATTR_UNUSED __attribute__((__unused__))

/* Debug enabled */
/* #undef ENABLE_DUMA */

/* Define default directory for Feng A/V resources */
#define FENICE_AVROOT_DIR_DEFAULT /var/feng/avroot

/* Define default directory string for Feng A/V resources */
#define FENICE_AVROOT_DIR_DEFAULT_STR "/var/feng/avroot"

/* Define default directory for Feng configuration */
#define FENICE_CONF_DIR_DEFAULT /etc

/* Define default file for Feng configuration */
#define FENICE_CONF_FILE_DEFAULT feng.conf

/* Define absolute path string for Feng configuration file */
#define FENICE_CONF_PATH_DEFAULT_STR "/etc/feng.conf"

/* Define default file for Feng logger */
#define FENICE_LOG_FILE_DEFAULT /var/log/feng.log

/* Define default string for Feng log file */
#define FENICE_LOG_FILE_DEFAULT_STR "/var/log/feng.log"

/* Define max number of RTSP incoming sessions for Feng */
#define FENICE_MAX_SESSION_DEFAULT 100

/* Define default RTSP listening port */
#define FENICE_RTSP_PORT_DEFAULT 554

/* Define default dir for Certificate (pem format) */
#define FENICE_STATE_DIR NONE/var/feng

/* Define default string dir for Certificate (pem format) */
#define FENIC_STATE_DIR_STR "NONE/var/feng"

/* Define if libavformat support is available */
#define HAVE_AVFORMAT /**/

/* Define if libavutil support is available */
#define HAVE_AVUTIL /**/

/* Define this if you have clock_gettime */
#define HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the <ev.h> header file. */
#define HAVE_EV_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `ev' library (-lev). */
#define HAVE_LIBEV 1

/* Define to 1 if you have the `mudflapth' library (-lmudflapth). */
/* #undef HAVE_LIBMUDFLAPTH */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define this if metadata support must be included */
/* #undef HAVE_METADATA */

/* Define to 1 if MySQL libraries are available */
/* #undef HAVE_MYSQL */

/* Define this if you have libsctp */
/* #undef HAVE_SCTP */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if the system has the type `struct sockaddr_storage'. */
#define HAVE_STRUCT_SOCKADDR_STORAGE 1

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define IPv6 support */
#define IPV6 1

/* Define this if live streaming is supported */
#define LIVE_STREAMING 1

/* Name of package */
#define PACKAGE "feng"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "feng"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "feng 2.1.0_rc1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "feng"

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.1.0_rc1"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define this if the compiler supports __attribute__(( ifelse([], ,
   [destructor], []) )) */
#define SUPPORT_ATTRIBUTE_DESTRUCTOR 1

/* Define this if the compiler supports __attribute__(( ifelse([], , [unused],
   []) )) */
#define SUPPORT_ATTRIBUTE_UNUSED 1

/* Trace enabled */
/* #undef TRACE */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "2.1.0_rc1"

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

#if !defined(NDEBUG) && defined(SUPPORT_ATTRIBUTE_DESTRUCTOR)
	   # define CLEANUP_DESTRUCTOR __attribute__((__destructor__))
	   #endif
	  
