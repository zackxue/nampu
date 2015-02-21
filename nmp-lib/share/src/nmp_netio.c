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

struct _HmNetIO
{
    gint        ref_count;

    HmWatch    *io_watch;
    HmNet      *net;
    GMutex      *lock;

	HmIOEst	on_est;		/* On established, after connect. */

	gpointer    priv_data;
	HmDesFun   priv_destroy;
};

extern gint hm_net_recv_message(HmNet *net, HmNetIO *net_io, gpointer msg);
extern gint hm_net_add_io(HmNet *net, HmNetIO *net_io, gint notify);
extern HmNet *hm_net_ref(HmNet *net);
extern void hm_net_unref(HmNet *net);
extern void hm_net_async_kill_io(HmNet *net, HmNetIO *net_io, gint err);
extern void hm_net_wakeup_context(HmNet *net);
extern void hm_net_establish_io(HmNet *net, HmNetIO *net_io);

static gint total_net_io_count = 0;

static __inline__ HmWatch *
hm_net_io_create_watch(HmConnection *conn, 
    HmPacketProto *ll_proto, HmPayloadProto *hl_proto)
{
    return (HmWatch*)hm_hl_io_new(conn, ll_proto, hl_proto);
}


static __inline__ HmWatch *
hm_net_io_create_listen_watch(HmConnection *conn, 
    HmPacketProto *ll_proto, HmPayloadProto *hl_proto)
{
    return (HmWatch*)hm_hl_listen_io_new(conn, ll_proto, hl_proto);
}


static __inline__ void
hm_net_io_release(HmNetIO *net_io)
{
    g_atomic_int_add(&total_net_io_count, -1);
hm_debug("Net net_io '%p' finalized, total %d left.", net_io, g_atomic_int_get(&total_net_io_count));
    BUG_ON(net_io->io_watch);
    BUG_ON(net_io->net);
    g_mutex_free(net_io->lock);

	if (net_io->priv_destroy)
		(*net_io->priv_destroy)(net_io->priv_data);

    g_free(net_io);
}


__export void
hm_net_io_attach(HmNetIO *net_io, GMainContext *context)
{
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    if (net_io->io_watch)
        hm_watch_attach(net_io->io_watch, context);
    g_mutex_unlock(net_io->lock);
}


__export HmNetIO *
hm_net_io_ref(HmNetIO *net_io)
{
    G_ASSERT(net_io != NULL && 
        g_atomic_int_get(&net_io->ref_count) > 0);

    g_atomic_int_add(&net_io->ref_count, 1);
    return net_io;
}


__export void
hm_net_io_unref(HmNetIO *net_io)
{
    G_ASSERT(net_io != NULL && 
        g_atomic_int_get(&net_io->ref_count) > 0);

    if (g_atomic_int_dec_and_test(&net_io->ref_count))
    {
        hm_net_io_release(net_io);
    }
}


/*
 * kill a net-io object without unref.
*/
__export void
hm_net_io_kill(HmNetIO *net_io)
{
    HmWatch *watch;
    HmNet *net;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        hm_watch_kill(watch);
        hm_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        hm_net_unref(net);
    }
}


__export void
hm_net_io_async_kill(HmNetIO *net_io, gint err)
{
    HmWatch *watch;
    HmNet *net;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        hm_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        hm_net_async_kill_io(net, net_io, err);
        hm_net_unref(net);
    }
}


__export HmNetIO *
hm_net_io_new(HmConnection *conn, HmPacketProto *ll_proto,
    HmPayloadProto *hl_proto, gint *err)
{
    HmNetIO *net_io;
    G_ASSERT(conn != NULL && ll_proto != NULL 
        && hl_proto != NULL);

    net_io = g_new0(HmNetIO, 1);
    net_io->ref_count = 1;
    net_io->io_watch = hm_net_io_create_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        g_free(net_io);
        return NULL;
    }

    hm_watch_set_private(net_io->io_watch, net_io);
    hm_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = g_mutex_new();
    g_atomic_int_add(&total_net_io_count, 1);

    return net_io;
}


__export HmNetIO *
hm_net_listen_io_new(HmConnection *conn, HmPacketProto *ll_proto,
    HmPayloadProto *hl_proto, gint *err)
{
    HmNetIO *net_io;
    G_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

    net_io = g_new0(HmNetIO, 1);
    net_io->ref_count = 1;
    net_io->io_watch = hm_net_io_create_listen_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        g_free(net_io);
        return NULL;
    }

    hm_watch_set_private(net_io->io_watch, net_io);
    hm_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = g_mutex_new();
    g_atomic_int_add(&total_net_io_count, 1);

    return net_io;
}


static __inline__ void
__hm_net_io_set_owner(HmNetIO *net_io, HmNet *owner)
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
hm_net_io_set_owner(HmNetIO *net_io, gpointer owner)
{
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    __hm_net_io_set_owner(net_io, (HmNet*)owner);
    g_mutex_unlock(net_io->lock);
}


__export void
hm_net_io_set_private(HmNetIO *net_io, gpointer priv_data,
	HmDesFun priv_destroy)
{
	G_ASSERT(net_io != NULL);

	net_io->priv_data = priv_data;
	net_io->priv_destroy = priv_destroy;
}


__export gpointer
hm_net_io_get_private(HmNetIO *net_io)
{
	G_ASSERT(net_io != NULL);

	return net_io->priv_data;
}


__export gint
hm_net_io_read_message(HmNetIO *net_io, gpointer msg)
{
    HmNet *net = NULL;
    gint rc = -E_NETIODIE;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = hm_net_ref(net_io->net);
    }
    g_mutex_unlock(net_io->lock);

    if (net)
    {
        rc = hm_net_recv_message(net, net_io, msg);
        hm_net_unref(net);
    }

    return rc;
}


__export gint
hm_net_io_write_message(HmNetIO *net_io, gpointer msg)
{
    HmWatch *watch;
    gint rc = 0;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;

    if (watch)
    {
        hm_watch_ref(watch);
    }

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        rc = hm_watch_write_message(watch, msg);
        hm_watch_unref(watch);
    }

    return rc;
}


static __inline__ gint
hm_net_io_add_child(HmNetIO *net_io, HmNetIO *new_io)
{
    HmNet *net = NULL;
    gint rc = -E_NETIODIE;

    g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = hm_net_ref(net_io->net);
    }
    g_mutex_unlock(net_io->lock);

    if (net)
    {
        rc = hm_net_add_io(net, new_io, 1);
        hm_net_unref(net);
    }

    return rc;
}


__export gint
hm_net_io_add_child_watch(HmNetIO *net_io, gpointer watch)
{
    HmNetIO *new_io;
    gint rc;
    G_ASSERT(net_io != NULL && watch != NULL);

    new_io = g_new0(HmNetIO, 1);
    new_io->ref_count = 1;  /* 1 returned to user */
    new_io->io_watch = (HmWatch*)watch;
    new_io->net = NULL;
    new_io->lock = g_mutex_new();

    hm_watch_set_private(watch, new_io);
    hm_net_io_ref(new_io); /* increased by watch */

    g_atomic_int_add(&total_net_io_count, 1);
    rc = hm_net_io_add_child(net_io, new_io);
    if (rc)
    {
        hm_net_io_kill(new_io);
        hm_net_io_unref(net_io);
    }

    return rc;
}


__export void
hm_net_io_establish(HmNetIO *net_io)
{
	HmNet *net = NULL;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = hm_net_ref(net_io->net);
    }
	g_mutex_unlock(net_io->lock);

	if (net)
	{
		hm_net_establish_io(net, net_io);
		hm_net_unref(net);
	}
}


__export void
hm_net_io_on_establish(HmNetIO *net_io, gpointer init_data)
{
	G_ASSERT(net_io != NULL);

	if (net_io->on_est)
	{
		(*net_io->on_est)(net_io, init_data);
	}
}


__export gboolean
hm_net_io_set_ttd(HmNetIO *net_io, gint milli_secs)
{
    gboolean set_ok = FALSE;
    HmNet *net = NULL;
    HmWatch *watch;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    if (watch)
    {
        hm_watch_ref(watch);
    }

    if (net_io->net)
    {
        net = hm_net_ref(net_io->net);
    }

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        set_ok = hm_watch_set_conn_ttd(watch, milli_secs);
        hm_watch_unref(watch);
    }

    if (net)
    {
        if (set_ok)
            hm_net_wakeup_context(net);
        hm_net_unref(net);
    }

    return set_ok;
}


__export void
hm_net_io_set_ester(HmNetIO *net_io, HmIOEst on_est)
{
	G_ASSERT(net_io != NULL);

	net_io->on_est = on_est;	
}


__export gchar *
hm_net_io_get_peer(HmNetIO *net_io)
{
	gchar *ip = NULL;
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    hm_watch_ref(watch);
	}

	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		ip = hm_watch_get_peer(watch);
	    hm_watch_unref(watch);
	}

	return ip;
}


__export void
hm_net_io_set_heavy_load(HmNetIO *net_io)
{
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    hm_watch_ref(watch);
	}
	
	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		hm_watch_set_heavy_load(watch);
		hm_watch_unref(watch);
	}
}


__export void
hm_net_io_set_block_size(HmNetIO *net_io, gint size)
{
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    hm_watch_ref(watch);
	}
	
	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		hm_watch_set_block_size(watch, size);
		hm_watch_unref(watch);
	}
}

//:~ End
