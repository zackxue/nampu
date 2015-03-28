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
#define NET_IO(watch) (((NmpWatch*)watch)->priv_data)

typedef struct _NmpWatch NmpWatch;
typedef struct _NmpWatchFuncs NmpWatchFuncs;

struct _NmpWatchFuncs
{
    NmpWatch *(*create)(NmpWatch *w, NmpConnection *conn);

    gint (*recv)(NmpWatch *w, gchar *buf, gsize size);
    void (*error)(NmpWatch *w, gint rw, gint err);
    void (*close)(NmpWatch *w, gint async);

    gint (*format)(NmpWatch *w, gpointer msg, gchar buf[], gsize size);
    void (*finalize)(NmpWatch *w);
};


struct _NmpWatch
{
    GSource             source;

    GMutex              *lock;

    NmpNetBuf           *buffer;
    NmpConnection       *conn;
    NmpWatchFuncs       *funcs;

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


NmpWatch *nmp_watch_create(NmpConnection *conn,
    NmpWatchFuncs *funcs, gint size);

NmpWatch *nmp_listen_watch_create(NmpConnection *conn,
    NmpWatchFuncs *funcs, gint size);

void nmp_watch_attach(NmpWatch *watch, GMainContext *context);

gint nmp_watch_recv_message(NmpWatch *watch, gpointer msg);
gint nmp_watch_write_message(NmpWatch *watch, gpointer msg);

void nmp_watch_kill(NmpWatch *watch);

void nmp_watch_ref(NmpWatch *watch);
void nmp_watch_unref(NmpWatch *watch);

void nmp_watch_set_private(NmpWatch *watch, gpointer priv_data);
gboolean nmp_watch_set_conn_ttd(NmpWatch *watch, gint milli_sec);

gchar *nmp_watch_get_peer(NmpWatch *watch);
void nmp_watch_set_heavy_load(NmpWatch *watch);
void nmp_watch_set_block_size(NmpWatch *watch, gint block_size);

G_END_DECLS

#endif  //__NMP_WATCH_H__
