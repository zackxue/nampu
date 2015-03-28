/*
 * nmp_netio.c
 *
 * This file implements net io. Entity used for network communication.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include "nmp_netio.h"
#include "nmp_hlio.h"
#include "nmp_debug.h"
#include "nmp_errno.h"

struct _NmpNetIO
{
    gint        ref_count;

    HmWatch    *io_watch;
    NmpNet      *net;
    GMutex      *lock;

	HmIOEst	on_est;		/* On established, after connect. */

	gpointer    priv_data;
	HmDesFun   priv_destroy;
};

extern gint nmp_net_recv_message(NmpNet *net, NmpNetIO *net_io, gpointer msg);
extern gint nmp_net_add_io(NmpNet *net, NmpNetIO *net_io, gint notify);
extern NmpNet *nmp_net_ref(NmpNet *net);
extern void nmp_net_unref(NmpNet *net);
extern void nmp_net_async_kill_io(NmpNet *net, NmpNetIO *net_io, gint err);
extern void nmp_net_wakeup_context(NmpNet *net);
extern void nmp_net_establish_io(NmpNet *net, NmpNetIO *net_io);

static gint total_net_io_count = 0;

static __inline__ HmWatch *
nmp_net_io_create_watch(NmpConnection *conn, 
    NmpPacketProto *ll_proto, NmpPayloadProto *hl_proto)
{
    return (HmWatch*)nmp_hl_io_new(conn, ll_proto, hl_proto);
}


static __inline__ HmWatch *
nmp_net_io_create_listen_watch(NmpConnection *conn, 
    NmpPacketProto *ll_proto, NmpPayloadProto *hl_proto)
{
    return (HmWatch*)nmp_hl_listen_io_new(conn, ll_proto, hl_proto);
}


static __inline__ void
nmp_net_io_release(NmpNetIO *net_io)
{
    g_atomic_int_add(&total_net_io_count, -1);
nmp_debug("Net net_io '%p' finalized, total %d left.", net_io, g_atomic_int_get(&total_net_io_count));
    BUG_ON(net_io->io_watch);
    BUG_ON(net_io->net);
    g_mutex_free(net_io->lock);

	if (net_io->priv_destroy)
		(*net_io->priv_destroy)(net_io->priv_data);

    g_free(net_io);
}


__export void
nmp_net_io_attach(NmpNetIO *net_io, GMainContext *context)
{
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    if (net_io->io_watch)
        nmp_watch_attach(net_io->io_watch, context);
    g_mutex_unlock(net_io->lock);
}


__export NmpNetIO *
nmp_net_io_ref(NmpNetIO *net_io)
{
    G_ASSERT(net_io != NULL && 
        g_atomic_int_get(&net_io->ref_count) > 0);

    g_atomic_int_add(&net_io->ref_count, 1);
    return net_io;
}


__export void
nmp_net_io_unref(NmpNetIO *net_io)
{
    G_ASSERT(net_io != NULL && 
        g_atomic_int_get(&net_io->ref_count) > 0);

    if (g_atomic_int_dec_and_test(&net_io->ref_count))
    {
        nmp_net_io_release(net_io);
    }
}


/*
 * kill a net-io object without unref.
*/
__export void
nmp_net_io_kill(NmpNetIO *net_io)
{
    HmWatch *watch;
    NmpNet *net;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        nmp_watch_kill(watch);
        nmp_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        nmp_net_unref(net);
    }
}


__export void
nmp_net_io_async_kill(NmpNetIO *net_io, gint err)
{
    HmWatch *watch;
    NmpNet *net;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        nmp_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        nmp_net_async_kill_io(net, net_io, err);
        nmp_net_unref(net);
    }
}


__export NmpNetIO *
nmp_net_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
    NmpPayloadProto *hl_proto, gint *err)
{
    NmpNetIO *net_io;
    G_ASSERT(conn != NULL && ll_proto != NULL 
        && hl_proto != NULL);

    net_io = g_new0(NmpNetIO, 1);
    net_io->ref_count = 1;
    net_io->io_watch = nmp_net_io_create_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        g_free(net_io);
        return NULL;
    }

    nmp_watch_set_private(net_io->io_watch, net_io);
    nmp_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = g_mutex_new();
    g_atomic_int_add(&total_net_io_count, 1);

    return net_io;
}


__export NmpNetIO *
nmp_net_listen_io_new(NmpConnection *conn, NmpPacketProto *ll_proto,
    NmpPayloadProto *hl_proto, gint *err)
{
    NmpNetIO *net_io;
    G_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

    net_io = g_new0(NmpNetIO, 1);
    net_io->ref_count = 1;
    net_io->io_watch = nmp_net_io_create_listen_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        g_free(net_io);
        return NULL;
    }

    nmp_watch_set_private(net_io->io_watch, net_io);
    nmp_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = g_mutex_new();
    g_atomic_int_add(&total_net_io_count, 1);

    return net_io;
}


static __inline__ void
__nmp_net_io_set_owner(NmpNetIO *net_io, NmpNet *owner)
{
    if (owner)
    {
        BUG_ON(net_io->net);
        net_io->net = owner;
    }
    else
    {
        BUG_ON(!net_io->net);
        net_io->net = NULL;
    }
}


__export void
nmp_net_io_set_owner(NmpNetIO *net_io, gpointer owner)
{
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    __nmp_net_io_set_owner(net_io, (NmpNet*)owner);
    g_mutex_unlock(net_io->lock);
}


__export void
nmp_net_io_set_private(NmpNetIO *net_io, gpointer priv_data,
	HmDesFun priv_destroy)
{
	G_ASSERT(net_io != NULL);

	net_io->priv_data = priv_data;
	net_io->priv_destroy = priv_destroy;
}


__export gpointer
nmp_net_io_get_private(NmpNetIO *net_io)
{
	G_ASSERT(net_io != NULL);

	return net_io->priv_data;
}


__export gint
nmp_net_io_read_message(NmpNetIO *net_io, gpointer msg)
{
    NmpNet *net = NULL;
    gint rc = -E_NETIODIE;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = nmp_net_ref(net_io->net);
    }
    g_mutex_unlock(net_io->lock);

    if (net)
    {
        rc = nmp_net_recv_message(net, net_io, msg);
        nmp_net_unref(net);
    }

    return rc;
}


__export gint
nmp_net_io_write_message(NmpNetIO *net_io, gpointer msg)
{
    HmWatch *watch;
    gint rc = 0;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;

    if (watch)
    {
        nmp_watch_ref(watch);
    }

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        rc = nmp_watch_write_message(watch, msg);
        nmp_watch_unref(watch);
    }

    return rc;
}


static __inline__ gint
nmp_net_io_add_child(NmpNetIO *net_io, NmpNetIO *new_io)
{
    NmpNet *net = NULL;
    gint rc = -E_NETIODIE;

    g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = nmp_net_ref(net_io->net);
    }
    g_mutex_unlock(net_io->lock);

    if (net)
    {
        rc = nmp_net_add_io(net, new_io, 1);
        nmp_net_unref(net);
    }

    return rc;
}


__export gint
nmp_net_io_add_child_watch(NmpNetIO *net_io, gpointer watch)
{
    NmpNetIO *new_io;
    gint rc;
    G_ASSERT(net_io != NULL && watch != NULL);

    new_io = g_new0(NmpNetIO, 1);
    new_io->ref_count = 1;  /* 1 returned to user */
    new_io->io_watch = (HmWatch*)watch;
    new_io->net = NULL;
    new_io->lock = g_mutex_new();

    nmp_watch_set_private(watch, new_io);
    nmp_net_io_ref(new_io); /* increased by watch */

    g_atomic_int_add(&total_net_io_count, 1);
    rc = nmp_net_io_add_child(net_io, new_io);
    if (rc)
    {
        nmp_net_io_kill(new_io);
        nmp_net_io_unref(net_io);
    }

    return rc;
}


__export void
nmp_net_io_establish(NmpNetIO *net_io)
{
	NmpNet *net = NULL;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = nmp_net_ref(net_io->net);
    }
	g_mutex_unlock(net_io->lock);

	if (net)
	{
		nmp_net_establish_io(net, net_io);
		nmp_net_unref(net);
	}
}


__export void
nmp_net_io_on_establish(NmpNetIO *net_io, gpointer init_data)
{
	G_ASSERT(net_io != NULL);

	if (net_io->on_est)
	{
		(*net_io->on_est)(net_io, init_data);
	}
}


__export gboolean
nmp_net_io_set_ttd(NmpNetIO *net_io, gint milli_secs)
{
    gboolean set_ok = FALSE;
    NmpNet *net = NULL;
    HmWatch *watch;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    if (watch)
    {
        nmp_watch_ref(watch);
    }

    if (net_io->net)
    {
        net = nmp_net_ref(net_io->net);
    }

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        set_ok = nmp_watch_set_conn_ttd(watch, milli_secs);
        nmp_watch_unref(watch);
    }

    if (net)
    {
        if (set_ok)
            nmp_net_wakeup_context(net);
        nmp_net_unref(net);
    }

    return set_ok;
}


__export void
nmp_net_io_set_ester(NmpNetIO *net_io, HmIOEst on_est)
{
	G_ASSERT(net_io != NULL);

	net_io->on_est = on_est;	
}


__export gchar *
nmp_net_io_get_peer(NmpNetIO *net_io)
{
	gchar *ip = NULL;
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    nmp_watch_ref(watch);
	}

	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		ip = nmp_watch_get_peer(watch);
	    nmp_watch_unref(watch);
	}

	return ip;
}


__export void
nmp_net_io_set_heavy_load(NmpNetIO *net_io)
{
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    nmp_watch_ref(watch);
	}
	
	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		nmp_watch_set_heavy_load(watch);
		nmp_watch_unref(watch);
	}
}


__export void
nmp_net_io_set_block_size(NmpNetIO *net_io, gint size)
{
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    nmp_watch_ref(watch);
	}
	
	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		nmp_watch_set_block_size(watch, size);
		nmp_watch_unref(watch);
	}
}

//:~ End
