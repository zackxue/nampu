/*
 * nmp_rw_file.c
 *
 * This file implements routes to read/write config file.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "nmp_errno.h"
#include "nmp_rw_file.h"

#define FILE_TITLE_START    "#:{File Begin:\r\n"
#define FILE_TITLE_END      "\r\n#:The End!}"
#define MAX_FILE_SIZE       (2 << 20)

#define ALIGN(n, align) ((n + ((align) - 1)) & (~((align) - 1)))

struct _rw_file
{
    gint        fd;
    gint        err;

    GList       *list_items;

    gchar       *mm_start;
    gchar       *mm_end;
    GMutex      *lock;
};

typedef struct _rw_node
{
    gchar           *name;
    gchar           *value; 
}rw_node;


typedef struct _rw_section
{
    gchar           *name;
    GList           *list;
}rw_section;


static __inline__ gchar *
trim_left_right_space(gchar *str)
{
    gchar c, *p, *l;

    p = str;

    for (;;)
    {
        c = *p;
        if (c && (c == ' ' || c == '\t'))
        {
            ++p;
            continue;
        }

        break;
    }

    if (c)
    {
        l = p + strlen(p) - 1;
        for (; l > p;)
        {
            if (*l != ' ' && *l != '\t')
                break;
            *l-- = 0;
        }
    }

    return p;
}


static gint
is_the_same_section(gconstpointer _sec, gconstpointer name)
{
    rw_section *rw_secion;

    rw_secion = (rw_section*)_sec;
    return strcmp(rw_secion->name, (gchar*)name);
}


static gint
is_the_right_node(gconstpointer _node, gconstpointer name)
{
    rw_node *node = (rw_node*)_node;

    return strcmp(node->name, name);
}


static __inline__ void
add_one_section(rw_file *fp, rw_section *sec)
{
    GList *list;
    rw_section *orig_sec;

    list = g_list_find_custom(fp->list_items, sec->name, 
        is_the_same_section);
    if (list)
    {
        orig_sec = (rw_section*)list->data;
        orig_sec->list = g_list_concat(
            orig_sec->list, sec->list);

        if (sec->name < fp->mm_start || sec->name >= fp->mm_end)
            g_free(sec->name);
        g_free(sec);
    }
    else
    {
        fp->list_items = g_list_append(fp->list_items, sec);
    }
}


static __inline__ void
add_to_section(rw_section *sec, gchar *name, gchar *value)
{
    rw_node *node;

    if (!name)
        return;

	if (!value)
		value = g_strdup("");

    name = trim_left_right_space(name);
    value = trim_left_right_space(value);

    if (!*name)
        return;

    node = g_new0(rw_node, 1);
    node->name = name;
    node->value = value;
    sec->list = g_list_append(sec->list, node);
}


static void
free_nodes(gpointer _node, gpointer _file)
{
    rw_node *node = (rw_node*)_node;
    rw_file *fp = (rw_file*)_file;

    if (node->name < fp->mm_start ||
        node->name >= fp->mm_end)
    {
        g_free(node->name);
    }

    if (node->value < fp->mm_start ||
        node->value >= fp->mm_end)
    {
        g_free(node->value);
    }

    g_free(node);
}


static void
free_sections(gpointer _sec, gpointer _file)
{
    rw_section *sec = (rw_section*)_sec;
    rw_file *fp = (rw_file*)_file;

    g_list_foreach(sec->list, free_nodes, fp);

    if (sec->name < fp->mm_start ||
        sec->name >= fp->mm_end)
    {
        g_free(sec->name);
    }

    g_list_free(sec->list);
    g_free(sec);
}


static __inline__ void
parse_rw_file(rw_file *fp, gchar *p)
{
    gint line_terminated = 0;
    rw_section *rw_sec = NULL;
    gchar *sec_b = NULL, *sec_e = NULL; 
    gchar *name = NULL, *value = NULL;

    while (*p)
    {
        if (*p == ' ' || *p == '\t')
        {
            ++p;
            continue;
        }

        if (*p == '\r' || *p == '\n')
        {
            *p = 0;

            if (name && rw_sec)
                add_to_section(rw_sec, name, value);

            if (sec_b)
            {
                if (!sec_e)
                {
                    g_free(rw_sec);
                    rw_sec = NULL;
                }
                else
                    *sec_e = 0;
            }

            sec_b = NULL;
            sec_e = NULL;
            name = NULL;
            value = NULL;
            line_terminated = 0;

            ++p;
            continue;
        }

        if (line_terminated)
        {
            ++p;
            continue;
        }

        if (*p == '[')
        {
            if (!name && !sec_b)    //'[' not in name or value.
            {
                if (rw_sec)
                    add_one_section(fp, rw_sec);

                rw_sec = g_new0(rw_section, 1);
                sec_b = p + 1;
                rw_sec->name = sec_b;
            }
            ++p;
            continue;
        }

        if (*p == ']')
        {
            sec_e = p;
            ++p;
            continue;
        }

        if (*p == '=')
        {
            *p = 0;
            if (name)
                value = p + 1;
            ++p;
            continue;
        }

        if (*p == '#')
        {
            line_terminated = 1;
            *p++ = 0;
            continue;
        }

        if (!sec_b && !name)
            name = p;
        ++p;
        continue;
    }

    if (rw_sec)
        add_one_section(fp, rw_sec);
}


static __inline__ gint
load_rw_file(rw_file *fp)
{
    gint ret, size;
    gchar *ptr;
    struct stat s;

    if (fstat(fp->fd, &s))
        return -errno;

    if (!s.st_size)
        return 0;

    if (s.st_size > MAX_FILE_SIZE)
        return -EFBIG;

    size = ALIGN(s.st_size + 2, 8);
    ptr = g_malloc(size);

    if (read(fp->fd, ptr, s.st_size) <= 0)  //read block-device.
    {
        ret = -errno;
        g_free(ptr);
        return ret;
    }

    ptr[s.st_size] = '\n';
    ptr[s.st_size + 1] = 0;

    fp->mm_start = ptr;
    fp->mm_end = ptr + size;
    parse_rw_file(fp, ptr);

    return 0;
}


rw_file *
open_rw_file(const char *path, mode_t perm, gint *err)
{
    gint fd, flags, rc = 0;
    rw_file *fp = NULL;

    if (!path)
    {
        rc = -EINVAL;
        goto open_exit;
    }

    flags = O_RDWR;
    if (perm)
    {
        flags |= O_CREAT;
    }

    fd = open(path, flags, perm);
    if (fd < 0)
    {
        rc = -errno;
        goto open_exit;
    }

    fp = g_new0(rw_file, 1);
    fp->fd = fd;
    fp->lock = g_mutex_new();

    rc = load_rw_file(fp);
    if (rc)
    {
        g_free(fp);
        fp = NULL;
        close(fd);
    }

open_exit:
    if (rc && err)
    {
        *err = rc;
    }

    return fp;
}


static __inline__ const gchar *
__get_value_of(rw_file *fp, const gchar *section, gint index, 
    const gchar *name)
{
    GList *list;
    rw_node *node;
    rw_section *sec;

    if (index < 0)
        return NULL;

    list = g_list_find_custom(fp->list_items, section, 
        is_the_same_section);
    if (list)
    {
        sec = (rw_section*)list->data;

        list = g_list_find_custom(sec->list, name,
                is_the_right_node);

        while (--index >= 0)
        {
            list = g_list_next(list);

            list = g_list_find_custom(list, name,
                is_the_right_node);
            if (!list)
                return NULL;
        }

        if (list)
        {
            node = (rw_node*)list->data;
            return node->value;
        }
    }

    return NULL;
}


const gchar *
get_value_of(rw_file *fp, const gchar *section, gint index, 
    const gchar *name)
{
    const gchar *ret;

    g_assert(fp != NULL && section != NULL && name != NULL);

    g_mutex_lock(fp->lock);
    ret = __get_value_of(fp, section, index, name);
    g_mutex_unlock(fp->lock);

    return ret;
}


static __inline__ gint
__set_multi_value_of(rw_file *fp, const gchar *section, 
    const gchar *name, const gchar *value)
{
    GList *list;
    rw_section *sec;
    rw_node *node;

    list = g_list_find_custom(fp->list_items, section, 
        is_the_same_section);
    if (list)
    {
        sec = (rw_section*)list->data;
    }
    else
    {
        sec = g_new0(rw_section, 1);
        sec->name = g_strdup(section);

        fp->list_items = g_list_append(fp->list_items, sec);
    }

    node = g_new0(rw_node, 1);
    node->name = g_strdup(name);
    node->value = g_strdup(value);

    sec->list = g_list_append(sec->list, node);

    return 0;
}


gint
set_multi_value_of(rw_file *fp, const gchar *section, 
    const gchar *name, const gchar *value)
{
    gint rc;

    g_assert(fp != NULL && section != NULL 
        && name != NULL && value != NULL);

    g_mutex_lock(fp->lock);
    rc = __set_multi_value_of(fp, section, name, value);
    g_mutex_unlock(fp->lock);

    return rc;
}


static __inline__ gint
__set_value_of(rw_file *fp, const gchar *section, 
    const gchar *name, const gchar *value)
{
    if (__get_value_of(fp, section, 0, name))
        return -1;

    return __set_multi_value_of(fp, section, name, value);
}


gint
set_value_of(rw_file *fp, const gchar *section, 
    const gchar *name, const gchar *value)
{
    gint rc;

    g_assert(fp != NULL && section != NULL 
        && name != NULL && value != NULL);

    g_mutex_lock(fp->lock);
    rc = __set_value_of(fp, section, name, value);
    g_mutex_unlock(fp->lock);

    return rc;
}


static __inline__ gint
__del_value_internal(rw_file *fp, const gchar *section, 
    const gchar *name, gboolean all)
{
    GList *list;
    rw_node *node;
    rw_section *sec;
    gint n = 0;

    list = g_list_find_custom(fp->list_items, section, 
        is_the_same_section);
    if (list)
    {
        sec = (rw_section*)list->data;

		while ( TRUE )
		{
	        list = g_list_find_custom(sec->list, name,
	                is_the_right_node);
	        
			if (!list)
				break;

			++n;
			node = (rw_node*)list->data;
			sec->list = g_list_delete_link(sec->list, list);
			free_nodes(node, fp);

			if (!all)
				return n;
		}
    }

    return n;	
}


static __inline__ gint
__del_value_of(rw_file *fp, const gchar *section, 
    const gchar *name)
{
	return __del_value_internal(fp, section, name, FALSE);
}


gint
del_value_of(rw_file *fp, const gchar *section, 
    const gchar *name)
{
	gint ret;
	g_assert(fp != NULL && section != NULL && name != NULL);

	g_mutex_lock(fp->lock);
	ret = __del_value_of(fp, section, name);
	g_mutex_unlock(fp->lock);

	return ret;
}


gint
__del_multi_value_of(rw_file *fp, const gchar *section, 
    const gchar *name)
{
	return __del_value_internal(fp, section, name, TRUE);
}

gint
del_multi_value_of(rw_file *fp, const gchar *section, 
    const gchar *name)
{
	gint ret;
	g_assert(fp != NULL && section != NULL && name != NULL);

	g_mutex_lock(fp->lock);
	ret = __del_multi_value_of(fp, section, name);
	g_mutex_unlock(fp->lock);

	return ret;
}

static __inline__ void
file_write(rw_file *fp, gchar *buf, gsize size)
{
    if (fp->err)
        return;

    if (write(fp->fd, buf, size) < 0)
        fp->err = -errno;
}


static void
flush_node(gpointer _node, gpointer _file)
{
    rw_node *node = (rw_node*)_node;
    rw_file *fp = (rw_file*)_file;

    file_write(fp, node->name, strlen(node->name));
    file_write(fp, " = ", 3);
    file_write(fp, node->value, strlen(node->value));
    file_write(fp, "\r\n", 2);
}


static void
flush_sections(gpointer _sec, gpointer _file)
{
    rw_section *sec = (rw_section*)_sec;
    rw_file *fp = (rw_file*)_file;

    file_write(fp, "\r\n[", 3);
    file_write(fp, sec->name, strlen(sec->name));
    file_write(fp, "]\r\n", 3);

    g_list_foreach(sec->list, flush_node, fp);
}


static __inline__ gint
__flush_rw_file(rw_file *fp)
{
    if (lseek(fp->fd, 0, SEEK_SET))
        return -errno;

    if (ftruncate(fp->fd, 0))
        return -errno;

    file_write(fp, FILE_TITLE_START, strlen(FILE_TITLE_START));
    g_list_foreach(fp->list_items, flush_sections, fp);
    file_write(fp, FILE_TITLE_END, strlen(FILE_TITLE_END));

    return fp->err;
}


gint
flush_rw_file(rw_file *fp)
{
    gint rc;
    g_assert(fp != NULL);

    g_mutex_lock(fp->lock);
    rc = __flush_rw_file(fp);
    g_mutex_unlock(fp->lock);

    return rc;
}


static __inline__ void
clear_rw_file(rw_file *fp)
{
    g_mutex_lock(fp->lock);
    g_list_foreach(fp->list_items, free_sections, fp);
    g_list_free(fp->list_items);
    fp->list_items = NULL;
    g_mutex_unlock(fp->lock);
}


void
close_rw_file(rw_file *fp)
{
    g_assert(fp != NULL);

    clear_rw_file(fp);
    g_free(fp->mm_start);
    close(fp->fd);
    g_mutex_free(fp->lock); 

    g_free(fp);
}


//: ~End
