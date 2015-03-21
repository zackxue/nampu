/*
 * nmp_debug.h
 *
 * This file declares interfaces and macros for debugging 
 * and system running information logging purpose.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_DEBUG_H__
#define __NMP_DEBUG_H__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

#undef  G_LOG_DOMAIN
#define G_LOG_DOMAIN             "Nampu"

#define HM_LOG_TITLE            "# Server Started At %s"
#define HM_LOG_TIME_FORMAT      "%04d-%02d-%02d-%02d:%02d:%02d"

#define HM_LOG_TITLE_SIZE      45

#define FATAL_ERR_CODE          -1
#define HM_DEBUG               1
#define MAX_MSG_SIZE            256

#if defined HM_DEBUG && !defined G_OS_WIN32
/* snprintf() has different behavior on windows */
#define nmp_print(...)   \
G_STMT_START {\
    gchar ________msg[MAX_MSG_SIZE];    \
    snprintf(________msg , MAX_MSG_SIZE, __VA_ARGS__);   \
    snprintf(________msg + strlen(________msg), MAX_MSG_SIZE  - strlen(________msg),  \
        " [%s:%d]", __FILE__, __LINE__);   \
    g_log (G_LOG_DOMAIN,         \
        G_LOG_LEVEL_MESSAGE,  \
        "%s", ________msg); \
} G_STMT_END


#define nmp_warning(...)   \
G_STMT_START {\
    gchar ________msg[MAX_MSG_SIZE];    \
    snprintf(________msg , MAX_MSG_SIZE, __VA_ARGS__);   \
    snprintf(________msg + strlen(________msg), MAX_MSG_SIZE  - strlen(________msg),  \
        " [%s:%d]", __FILE__, __LINE__);   \
    g_log (G_LOG_DOMAIN,         \
        G_LOG_LEVEL_WARNING,  \
        "%s", ________msg); \
} G_STMT_END


#define nmp_error(...)   \
G_STMT_START {\
    gchar ________msg[MAX_MSG_SIZE];    \
    snprintf(________msg , MAX_MSG_SIZE, __VA_ARGS__);   \
    snprintf(________msg + strlen(________msg), MAX_MSG_SIZE  - strlen(________msg),  \
        " [%s:%d]", __FILE__, __LINE__);   \
    g_log (G_LOG_DOMAIN,         \
        G_LOG_LEVEL_ERROR,  \
        "%s", ________msg); \
} G_STMT_END


#else

#define nmp_print(...)   \
    g_log (G_LOG_DOMAIN,         \
        G_LOG_LEVEL_MESSAGE,  \
        __VA_ARGS__)


#define nmp_warning(...)   \
    g_log (G_LOG_DOMAIN,         \
        G_LOG_LEVEL_WARNING,  \
        __VA_ARGS__)


#define nmp_error(...)  \
    G_STMT_START {                 \
        g_log (G_LOG_DOMAIN,         \
            G_LOG_LEVEL_ERROR,    \
            __VA_ARGS__);         \
            for (;;) ;                 \
        } G_STMT_END

#endif  /* HM_DEBUG & G_OS_WIN32 */

#define nmp_debug nmp_print

#define bug_print(...)   \
    g_log (G_LOG_DOMAIN,         \
        G_LOG_LEVEL_MESSAGE,  \
        __VA_ARGS__)

#define __export
#define G_ASSERT    g_assert

/*
 * force SIG-SEGV, then dump stack. 
*/
#define BUG() \
G_STMT_START {\
    bug_print( \
    "****<BUG>****\n\t" \
    "!! BUG() AT FILE:%s LINE:%d FUNCTION:%s().", \
        __FILE__, __LINE__, __FUNCTION__ \
    ); \
    \
    gchar *_______________p= 0; *_______________p = 0;\
} G_STMT_END


/*
 * force SIG-SEGV, then dump stack. 
*/
#define BUG_ON(exp) \
G_STMT_START {\
    if (exp) \
    { \
        bug_print( \
        "****<BUG>****\n\t" \
        "!! BUG_ON(%s) AT FILE:%s LINE:%d FUNCTION:%s().", \
            #exp, __FILE__, __LINE__, __FUNCTION__ \
        ); \
        \
        gchar *_______________p= 0; *_______________p = 0;\
    } \
} G_STMT_END


#define FATAL_ERROR_EXIT \
G_STMT_START {\
    bug_print( \
    "<ERROR> _SERVER_FATAL_ERROR_ AT FILE:%s LINE:%d FUNCTION:%s()!!", \
        __FILE__, __LINE__, __FUNCTION__ \
    ); \
    \
    exit(FATAL_ERR_CODE);\
} G_STMT_END


gint nmp_debug_log_facility_init(const gchar *folder_path,
    const gchar *name);

void nmp_debug_set_log_size(gint size);


#endif  //__NMP_DEBUG_H__
