/*
 * jpf_rw_file.h
 *
 * This file declares interfaces to read/write config file.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#ifndef __HM_RW_FILE_H__
#define __HM_RW_FILE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _rw_file rw_file;


rw_file *open_rw_file(const char *path, mode_t perm, gint *err);

const gchar *get_value_of(rw_file *fp, const gchar *section,
    gint index, const gchar *name);

gint set_value_of(rw_file *fp, const gchar *section, 
    const gchar *name, const gchar *value);

gint set_multi_value_of(rw_file *fp, const gchar *section,
    const gchar *name, const gchar *value);

gint del_value_of(rw_file *fp, const gchar *section, 
    const gchar *name);

gint del_multi_value_of(rw_file *fp, const gchar *section, 
    const gchar *name);

gint flush_rw_file(rw_file *fp);

void close_rw_file(rw_file *fp);

G_END_DECLS

#endif  //__HM_RW_FILE_H__
