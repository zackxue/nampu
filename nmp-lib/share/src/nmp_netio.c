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

struct _JpfNetIO
{
    gint        ref_count;

    HmWatch    *io_watch;
    JpfNet      *net;
    GMutex      *lock;

	HmIOEst	on_est;		/* On established, after connect. */

	gpointer    priv_data;
	HmDesFun   priv_destroy;
};

extern gint jpf_net_recv_message(JpfNet *net, JpfNetIO *net_io, gpointer msg);
extern gint jpf_net_add_io(JpfNet *net, JpfNetIO *net_io, gint notify);
extern JpfNet *jpf_net_ref(JpfNet *net);
extern void jpf_net_unref(JpfNet *net);
extern void jpf_net_async_kill_io(JpfNet *net, JpfNetIO *net_io, gint err);
extern void jpf_net_wakeup_context(JpfNet *net);
extern void jpf_net_establish_io(JpfNet *net, JpfNetIO *net_io);

static gint total_net_io_count = 0;

static __inline__ HmWatch *
jpf_net_io_create_watch(HmConnection *conn, 
    JpfPacketProto *ll_proto, JpfPayloadProto *hl_proto)
{
    return (HmWatch*)jpf_hl_io_new(conn, ll_proto, hl_proto);
}


static __inline__ HmWatch *
jpf_net_io_create_listen_watch(HmConnection *conn, 
    JpfPacketProto *ll_proto, JpfPayloadProto *hl_proto)
{
    return (HmWatch*)jpf_hl_listen_io_new(conn, ll_proto, hl_proto);
}


static __inline__ void
jpf_net_io_release(JpfNetIO *net_io)
{
    g_atomic_int_add(&total_net_io_count, -1);
jpf_debug("Net net_io '%p' finalized, total %d left.", net_io, g_atomic_int_get(&total_net_io_count));
    BUG_ON(net_io->io_watch);
    BUG_ON(net_io->net);
    g_mutex_free(net_io->lock);

	if (net_io->priv_destroy)
		(*net_io->priv_destroy)(net_io->priv_data);

    g_free(net_io);
}


__export void
jpf_net_io_attach(JpfNetIO *net_io, GMainContext *context)
{
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    if (net_io->io_watch)
        jpf_watch_attach(net_io->io_watch, context);
    g_mutex_unlock(net_io->lock);
}


__export JpfNetIO *
jpf_net_io_ref(JpfNetIO *net_io)
{
    G_ASSERT(net_io != NULL && 
        g_atomic_int_get(&net_io->ref_count) > 0);

    g_atomic_int_add(&net_io->ref_count, 1);
    return net_io;
}


__export void
jpf_net_io_unref(JpfNetIO *net_io)
{
    G_ASSERT(net_io != NULL && 
        g_atomic_int_get(&net_io->ref_count) > 0);

    if (g_atomic_int_dec_and_test(&net_io->ref_count))
    {
        jpf_net_io_release(net_io);
    }
}


/*
 * kill a net-io object without unref.
*/
__export void
jpf_net_io_kill(JpfNetIO *net_io)
{
    HmWatch *watch;
    JpfNet *net;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        jpf_watch_kill(watch);
        jpf_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        jpf_net_unref(net);
    }
}


__export void
jpf_net_io_async_kill(JpfNetIO *net_io, gint err)
{
    HmWatch *watch;
    JpfNet *net;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        jpf_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        jpf_net_async_kill_io(net, net_io, err);
        jpf_net_unref(net);
    }
}


__export JpfNetIO *
jpf_net_io_new(HmConnection *conn, JpfPacketProto *ll_proto,
    JpfPayloadProto *hl_proto, gint *err)
{
    JpfNetIO *net_io;
    G_ASSERT(conn != NULL && ll_proto != NULL 
        && hl_proto != NULL);

    net_io = g_new0(JpfNetIO, 1);
    net_io->ref_count = 1;
    net_io->io_watch = jpf_net_io_create_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        g_free(net_io);
        return NULL;
    }

    jpf_watch_set_private(net_io->io_watch, net_io);
    jpf_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = g_mutex_new();
    g_atomic_int_add(&total_net_io_count, 1);

    return net_io;
}


__export JpfNetIO *
jpf_net_listen_io_new(HmConnection *conn, JpfPacketProto *ll_proto,
    JpfPayloadProto *hl_proto, gint *err)
{
    JpfNetIO *net_io;
    G_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

    net_io = g_new0(JpfNetIO, 1);
    net_io->ref_count = 1;
    net_io->io_watch = jpf_net_io_create_listen_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        g_free(net_io);
        return NULL;
    }

    jpf_watch_set_private(net_io->io_watch, net_io);
    jpf_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = g_mutex_new();
    g_atomic_int_add(&total_net_io_count, 1);

    return net_io;
}


static __inline__ void
__jpf_net_io_set_owner(JpfNetIO *net_io, JpfNet *owner)
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
jpf_net_io_set_owner(JpfNetIO *net_io, gpointer owner)
{
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    __jpf_net_io_set_owner(net_io, (JpfNet*)owner);
    g_mutex_unlock(net_io->lock);
}


__export void
jpf_net_io_set_private(JpfNetIO *net_io, gpointer priv_data,
	HmDesFun priv_destroy)
{
	G_ASSERT(net_io != NULL);

	net_io->priv_data = priv_data;
	net_io->priv_destroy = priv_destroy;
}


__export gpointer
jpf_net_io_get_private(JpfNetIO *net_io)
{
	G_ASSERT(net_io != NULL);

	return net_io->priv_data;
}


__export gint
jpf_net_io_read_message(JpfNetIO *net_io, gpointer msg)
{
    JpfNet *net = NULL;
    gint rc = -E_NETIODIE;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = jpf_net_ref(net_io->net);
    }
    g_mutex_unlock(net_io->lock);

    if (net)
    {
        rc = jpf_net_recv_message(net, net_io, msg);
        jpf_net_unref(net);
    }

    return rc;
}


__export gint
jpf_net_io_write_message(JpfNetIO *net_io, gpointer msg)
{
    HmWatch *watch;
    gint rc = 0;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;

    if (watch)
    {
        jpf_watch_ref(watch);
    }

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        rc = jpf_watch_write_message(watch, msg);
        jpf_watch_unref(watch);
    }

    return rc;
}


static __inline__ gint
jpf_net_io_add_child(JpfNetIO *net_io, JpfNetIO *new_io)
{
    JpfNet *net = NULL;
    gint rc = -E_NETIODIE;

    g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = jpf_net_ref(net_io->net);
    }
    g_mutex_unlock(net_io->lock);

    if (net)
    {
        rc = jpf_net_add_io(net, new_io, 1);
        jpf_net_unref(net);
    }

    return rc;
}


__export gint
jpf_net_io_add_child_watch(JpfNetIO *net_io, gpointer watch)
{
    JpfNetIO *new_io;
    gint rc;
    G_ASSERT(net_io != NULL && watch != NULL);

    new_io = g_new0(JpfNetIO, 1);
    new_io->ref_count = 1;  /* 1 returned to user */
    new_io->io_watch = (HmWatch*)watch;
    new_io->net = NULL;
    new_io->lock = g_mutex_new();

    jpf_watch_set_private(watch, new_io);
    jpf_net_io_ref(new_io); /* increased by watch */

    g_atomic_int_add(&total_net_io_count, 1);
    rc = jpf_net_io_add_child(net_io, new_io);
    if (rc)
    {
        jpf_net_io_kill(new_io);
        jpf_net_io_unref(net_io);
    }

    return rc;
}


__export void
jpf_net_io_establish(JpfNetIO *net_io)
{
	JpfNet *net = NULL;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = jpf_net_ref(net_io->net);
    }
	g_mutex_unlock(net_io->lock);

	if (net)
	{
		jpf_net_establish_io(net, net_io);
		jpf_net_unref(net);
	}
}


__export void
jpf_net_io_on_establish(JpfNetIO *net_io, gpointer init_data)
{
	G_ASSERT(net_io != NULL);

	if (net_io->on_est)
	{
		(*net_io->on_est)(net_io, init_data);
	}
}


__export gboolean
jpf_net_io_set_ttd(JpfNetIO *net_io, gint milli_secs)
{
    gboolean set_ok = FALSE;
    JpfNet *net = NULL;
    HmWatch *watch;
    G_ASSERT(net_io != NULL);

    g_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    if (watch)
    {
        jpf_watch_ref(watch);
    }

    if (net_io->net)
    {
        net = jpf_net_ref(net_io->net);
    }

    g_mutex_unlock(net_io->lock);

    if (watch)
    {
        set_ok = jpf_watch_set_conn_ttd(watch, milli_secs);
        jpf_watch_unref(watch);
    }

    if (net)
    {
        if (set_ok)
            jpf_net_wakeup_context(net);
        jpf_net_unref(net);
    }

    return set_ok;
}


__export void
jpf_net_io_set_ester(JpfNetIO *net_io, HmIOEst on_est)
{
	G_ASSERT(net_io != NULL);

	net_io->on_est = on_est;	
}


__export gchar *
jpf_net_io_get_peer(JpfNetIO *net_io)
{
	gchar *ip = NULL;
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    jpf_watch_ref(watch);
	}

	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		ip = jpf_watch_get_peer(watch);
	    jpf_watch_unref(watch);
	}

	return ip;
}


__export void
jpf_net_io_set_heavy_load(JpfNetIO *net_io)
{
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    jpf_watch_ref(watch);
	}
	
	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		jpf_watch_set_heavy_load(watch);
		jpf_watch_unref(watch);
	}
}


__export void
jpf_net_io_set_block_size(JpfNetIO *net_io, gint size)
{
	HmWatch *watch;
	G_ASSERT(net_io != NULL);

	g_mutex_lock(net_io->lock);

	watch = net_io->io_watch;
	if (watch)
	{
	    jpf_watch_ref(watch);
	}
	
	g_mutex_unlock(net_io->lock);

	if (watch)
	{
		jpf_watch_set_block_size(watch, size);
		jpf_watch_unref(watch);
	}
}

//:~ End
