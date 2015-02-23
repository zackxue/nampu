/*
 * jpf_debug.c
 *
 * This file include routes and macros for debugging 
 * and system running information logging purpose.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <sys/wait.h>
#include "nmp_errno.h"
#include "nmp_debug.h"


#define MAX_FILE_NAME               128
#define PATH_FOLDER_PERM            0777
#define HM_LOG_LINE_SIZE           256

#define MAX_LOG_SIZE                500  /* top limit */
#define MIN_LOG_SIZE                1

#define AQUIRE_LOG_LOCK \
    g_mutex_lock(log_mutex)
#define RELEASE_LOG_LOCK    \
    g_mutex_unlock(log_mutex)

static FILE *log_file = NULL;
static GMutex *log_mutex = NULL;
static gsize max_log_size = 10; /* default: 10MB */

static __inline__ gint
jpf_debug_get_sub_str(const gchar *start, const gchar *end, 
    gchar buf[], gsize size)
{
    gint len;

    len = end - start;
    if (len < 0 || len >= size)
        return -E_STRINGLEN;

    while (start != end)
        *buf++ = *start++;
    *buf = 0;

    return 0;
}


static __inline__ gint
jpf_debug_create_path(const gchar *path, mode_t mode, gchar abs_p[])
{
    const gchar *start, *end;
    gchar c;
    gchar file_name[MAX_FILE_NAME];
    gchar abs_path[PATH_MAX];

    if (!path)
        return -E_INVAL;

    while ((c = *path) != 0)
    {
        if (c == ' ' || c == '\t')
            ++path;
        else
            break;
    }

    c = *path;

    if (c == 0)
        return -E_INVAL;
    else
    {
        if (c == '/' || c == '\\')
            strcpy(abs_path, "/");
        else
        {
            if(!getcwd(abs_path, PATH_MAX))
                return -errno;
        }
    }

    end = path;

    while ( TRUE )
    {
        while ((c = *end) != 0)
        {
            if (c == '/' || c == '\\')
                ++end;
            else
                break;
        }

        start = end;

        if (*start == 0)
            break;

        while ((c = *end) != 0)
        {
            if (c != '/' && c != '\\')
                ++end;
            else
                break;
        }

        memset(file_name, 0, sizeof(file_name));

        if (jpf_debug_get_sub_str(start, end, 
            file_name, MAX_FILE_NAME))
        {
            return -ENAMETOOLONG;
        }

        if (strlen(file_name) + 
                strlen(abs_path) + 2 > PATH_MAX)
        {
            return -ENAMETOOLONG;
        }

        strcat(abs_path, "/");
        strcat(abs_path, file_name);

        if (mkdir(abs_path, mode))
        {
            if (errno != EEXIST)
                return -errno;
        }
    }

    if (abs_p)
        strcpy(abs_p, abs_path);

    return 0;
}


static __inline__ gint
jpf_debug_create_log_file(const gchar *file_folder, const gchar *name)
{
    gint err;
    time_t time_now;
    struct tm tm_now;
    gchar file_name[MAX_FILE_NAME];
    gchar log_file_path[PATH_MAX];
    gchar old_file_path[PATH_MAX];

    if (!file_folder || !name)
        return -E_INVAL;

    if ((err = jpf_debug_create_path(file_folder,
        PATH_FOLDER_PERM, log_file_path)))
        return err;

	memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, name, MAX_FILE_NAME - 1);
    strcpy(old_file_path, log_file_path);

    if (strlen(log_file_path) + strlen(file_name)
        + 2 > PATH_MAX)
        return -ENAMETOOLONG;

    time_now = time(NULL);

    strcat(log_file_path, "/");
    strcat(log_file_path, file_name);

    log_file = fopen(log_file_path, "r");
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;

        if (!localtime_r(&time_now, &tm_now))
            return -errno;

        sprintf(file_name + strlen(file_name), 
            "."HM_LOG_TIME_FORMAT, 
            tm_now.tm_year + 1900, 
            tm_now.tm_mon + 1, 
            tm_now.tm_mday, 
            tm_now.tm_hour, 
            tm_now.tm_min, 
            tm_now.tm_sec
        );

        if (strlen(old_file_path) + strlen(file_name)
            + 2 > PATH_MAX)
            return -ENAMETOOLONG;

        strcat(old_file_path, "/");
        strcat(old_file_path, file_name);
        rename(log_file_path, old_file_path);
    }

    log_file = fopen(log_file_path, "w");
    if (log_file)
    {
        fprintf(log_file, HM_LOG_TITLE, ctime(&time_now));
        fflush(log_file);
    }

    return log_file ? 0 : -errno;
}


static __inline__ void
jpf_debug_check_rewind( void )
{
    static glong file_size = 0;
    glong now_size, limit_size;

    if (G_UNLIKELY(!log_file))
        return;

    AQUIRE_LOG_LOCK;

    limit_size = (max_log_size << 20) - HM_LOG_LINE_SIZE;

    now_size = ftell(log_file);
    if (G_UNLIKELY(now_size < 0))
    {
        fseek(log_file, HM_LOG_TITLE_SIZE, SEEK_SET);
        RELEASE_LOG_LOCK;
        return;
    }

    if (G_UNLIKELY(now_size >= limit_size))
    {
        if (file_size && file_size > now_size)
            ftruncate(fileno(log_file), now_size);

        file_size = now_size;
        fseek(log_file, HM_LOG_TITLE_SIZE, 
                SEEK_SET);
    }

    RELEASE_LOG_LOCK;
}


FILE *
jpf_debug_get_log_file( void )
{
    if (G_UNLIKELY(!log_file))
        return stderr;

    jpf_debug_check_rewind();

    return log_file;
}


static __inline__ const gchar *
jpf_debug_get_level_str(GLogLevelFlags log_level)
{
    switch (log_level)
    {
    case G_LOG_LEVEL_MESSAGE:
        return "MESSAGE :";

    case G_LOG_LEVEL_WARNING:
        return "WARNING **:";

    case G_LOG_LEVEL_ERROR:
        return "ERROR *****";

    default:
        return "UNKNOWN :";
    }
}


static void
jpf_debug_log_assertion(const gchar *msg)
{
    FILE *fp;

    fp = jpf_debug_get_log_file();

    AQUIRE_LOG_LOCK;
    fprintf(fp, "%s", msg);
    fflush(fp);
    RELEASE_LOG_LOCK;
}


static void
jpf_debug_log_to_file(const gchar *log_domain, GLogLevelFlags log_level,
    const gchar *msg, gpointer user_data)
{
    time_t t;
    struct tm tm;
    FILE *fp;
    const gchar *level;

    t = time(NULL);
    localtime_r(&t, &tm);

    fp = jpf_debug_get_log_file();
    level = jpf_debug_get_level_str(log_level);

    AQUIRE_LOG_LOCK;

    fprintf(
        fp,
        "[%04d-%02d-%02d %02d:%02d:%02d] %s-%s%s\n", 
        tm.tm_year + 1900, 
        tm.tm_mon + 1, 
        tm.tm_mday,
        tm.tm_hour, 
        tm.tm_min, 
        tm.tm_sec,
        log_domain,
        level,
        msg
    );

    if (log_level == G_LOG_LEVEL_WARNING ||
        log_level == G_LOG_LEVEL_ERROR ||
        log_level == G_LOG_LEVEL_MESSAGE)
    {
        fflush(fp);
    }

    RELEASE_LOG_LOCK;

    return ;
}


static __inline__ void
jpf_debug_on_exit(gint status, gpointer arg)
{
    jpf_print("<EXIT> SERVER EXITED.");
}


static __inline__ void
jpf_debug_log_set_exit_handler( void )
{
    on_exit(jpf_debug_on_exit, NULL);
}


__export void
jpf_debug_set_log_size(gint size)
{
    if (size > MAX_LOG_SIZE)
        size = MAX_LOG_SIZE;

    if (size < MIN_LOG_SIZE)
        size = MIN_LOG_SIZE;

    max_log_size = size;
}


static __inline__ gint
jpf_debug_set_fp_nonblock(FILE *fp)
{
	gint fd, old_flgs;

	if (!fp)
		return -EINVAL;

	fd = fileno(fp);
	if (fd > 0)
	{
	    old_flgs = fcntl(fd, F_GETFL, 0);
	    if (G_UNLIKELY(old_flgs < 0))
	        return -errno;

	    old_flgs |= O_NONBLOCK;
	    if (fcntl(fd, F_SETFL, old_flgs) < 0)
	        return -errno;		
	}

	return 0;
}


static __inline__ void
jpf_debug_set_fps_nonblock( void )
{
	jpf_debug_set_fp_nonblock(stdout);
	jpf_debug_set_fp_nonblock(stderr);
	jpf_debug_set_fp_nonblock(log_file);
}


__export gint
jpf_debug_log_facility_init(const gchar *folder_path, const gchar *name)
{
    int err;
    
    log_mutex = g_mutex_new();
    if (G_UNLIKELY(!log_mutex))
        return -E_NEWMUTEX;

    err = jpf_debug_create_log_file(folder_path, name);
    if (G_UNLIKELY(err))
    {
        g_mutex_free(log_mutex);
        log_mutex = NULL;
        return err;
    }

	jpf_debug_set_fps_nonblock();

    g_log_set_handler(
        G_LOG_DOMAIN,
        G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING |G_LOG_LEVEL_ERROR,
        jpf_debug_log_to_file,
        NULL
    );

    g_set_printerr_handler(jpf_debug_log_assertion);

    jpf_debug_log_set_exit_handler();

    return 0;
}


//:~ End
