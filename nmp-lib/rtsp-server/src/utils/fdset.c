/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#include <glib.h>
#include "fdset.h"

struct _FD_set
{
	GMutex *mutex;
	GHashTable	*fd_table;
};

typedef struct _FD_value FD_value;
struct _FD_value
{
	gint fd;
	gpointer user_data;
	fd_callback_t call;
	fd_callback_t fin;
};

static guint hash_function(gconstpointer key)
{
	return (guint)key;
}

static gboolean key_equal(gconstpointer a, gconstpointer b)
{
	return a == b;
}

static void value_destroy(gpointer data)
{
	FD_value *v = (FD_value*)data;

	if (v->fin)
		(*v->fin)(v->fd, v->user_data);
	g_free(v);
}

FD_set *fd_set_new( void )
{
	FD_set *set;

	set = g_new0(FD_set, 1);
	set->mutex = g_mutex_new();
	set->fd_table = g_hash_table_new_full(hash_function,
		key_equal, NULL, value_destroy);

	return set;
}

void fd_set_insert(FD_set *set, gint fd, gpointer user_data,
	fd_callback_t call, fd_callback_t fin)
{
	FD_value *v;
	g_assert(set != NULL);

	v = g_new0(FD_value, 1);
	v->fd = fd;
	v->user_data = user_data;
	v->call = call;
	v->fin = fin;

	g_mutex_lock(set->mutex);
	g_hash_table_insert(set->fd_table, (gpointer)fd, v);
	g_mutex_unlock(set->mutex);
}

gboolean fd_set_remove(FD_set *set, gint fd)
{
	gboolean ret;
	g_assert(set != NULL);

	g_mutex_lock(set->mutex);
	ret = g_hash_table_remove(set->fd_table, (gpointer)fd);
	g_mutex_unlock(set->mutex);

	return ret;
}

static void for_each_call(gpointer key, gpointer value, gpointer user_data)
{
	FD_value *v = (FD_value*)value;
	if (v->call)
		(*v->call)((gint)key, v->user_data);
}

void fd_set_call(FD_set *set)
{
	g_assert(set != NULL);

	g_mutex_lock(set->mutex);
	g_hash_table_foreach(set->fd_table, for_each_call, NULL);
	g_mutex_unlock(set->mutex);
}

void fd_set_free(FD_set *set)
{
	g_assert(set != NULL);

	g_hash_table_destroy(set->fd_table);
	g_mutex_free(set->mutex);
	g_free(set);
}

//:~ End
