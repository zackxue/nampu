/*
 * nmp_watch.h
 *
 * This file declares watch structure and interfaces.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_WATCH_H__
#define __NMP_WATCH_H__

#include <glib.h>
#include "nmp_conn.h"
#include "nmp_netbuf.h"

G_BEGIN_DECLS

#define USE_MONOTONIC_CLOCK 1
#define NET_IO(watch) (((HmWatch*)watch)->priv_data)

typedef struct _HmWatch HmWatch;
typedef struct _HmWatchFuncs HmWatchFuncs;

struct _HmWatchFuncs
{
    HmWatch *(*create)(HmWatch *w, HmConnection *conn);

    gint (*recv)(HmWatch *w, gchar *buf, gsize size);
    void (*error)(HmWatch *w, gint rw, gint err);
    void (*close)(HmWatch *w, gint async);

    gint (*format)(HmWatch *w, gpointer msg, gchar buf[], gsize size);
    void (*finalize)(HmWatch *w);
};


struct _HmWatch
{
    GSource             source;

    GMutex              *lock;

    JpfNetBuf           *buffer;
    HmConnection       *conn;
    HmWatchFuncs       *funcs;

    GPollFD             r_fd;
    GPollFD             w_fd;

    GTimeVal            next_timeout;       /* timeout point, realtime */
#ifdef USE_MONOTONIC_CLOCK
    gint                delta_time;			/* realtime - monotonic */
#endif

    gint                w_pending;
    gint                killed;
	gint				heavy_io_load;		/* need more memory for caching */
	gint                block_size;		    /* buffer block size*/

    gpointer            priv_data;
};


HmWatch *jpf_watch_create(HmConnection *conn,
    HmWatchFuncs *funcs, gint size);

HmWatch *jpf_listen_watch_create(HmConnection *conn,
    HmWatchFuncs *funcs, gint size);

void jpf_watch_attach(HmWatch *watch, GMainContext *context);

gint jpf_watch_recv_message(HmWatch *watch, gpointer msg);
gint jpf_watch_write_message(HmWatch *watch, gpointer msg);

void jpf_watch_kill(HmWatch *watch);

void jpf_watch_ref(HmWatch *watch);
void jpf_watch_unref(HmWatch *watch);

void jpf_watch_set_private(HmWatch *watch, gpointer priv_data);
gboolean jpf_watch_set_conn_ttd(HmWatch *watch, gint milli_sec);

gchar *jpf_watch_get_peer(HmWatch *watch);
void jpf_watch_set_heavy_load(HmWatch *watch);
void jpf_watch_set_block_size(HmWatch *watch, gint block_size);

G_END_DECLS

#endif  //__NMP_WATCH_H__
