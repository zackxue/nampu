/*
 * nmp_net.c
 *
 * This file implements net, used for net io objects management.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#include <string.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_net.h"
#include "nmp_netproto.h"


#define GATHER_SLEEP_MSEC 	 50
#define GATHER_INTERVAL_MSEC 1


typedef struct _HmSchedLoop HmSchedLoop;
struct _HmSchedLoop
{
    GMainContext    *context;   /* loop context */
    GMainLoop       *loop;
    GThread         *loop_thread;   
};


typedef struct _NmpNetSched NmpNetSched;
struct _NmpNetSched
{
    gint nr_loops;
    gint last_loop;
    HmSchedLoop loops[0];
};


struct _NmpNet
{
    gint            ref_count;
    gint            killed;     /* state */

    GHashTable      *io_table;  /* net-io hash table */
    GMutex          *lock;

    NmpNetSched     *scheduler; /* io scheduler */

    NmpPacketProto  *ll_proto;  /* Low Level: packet proto */
    NmpPayloadProto *hl_proto;  /* High Level: payload proto */

    NmpIOInit       on_init;
    NmpIOFin        on_fin;
    NmpIOReader     on_read;
    gpointer        priv_data;
};


static gpointer
nmp_net_scheduler_loop_thread(gpointer user_data)
{
    HmSchedLoop *l = (HmSchedLoop*)user_data;

    g_main_loop_run(l->loop);
    BUG_ON( TRUE );
    return NULL;
}


static gboolean
nmp_net_loop_gather(gpointer data)
{
	usleep(GATHER_SLEEP_MSEC*1000);
	return TRUE;
}


static __inline__ void
nmp_net_scheduler_init(NmpNetSched *sched, guint n_loops,
	gboolean gather)
{
    gint index;
    GSource *tsource;

    sched->nr_loops = n_loops;
    sched->last_loop = 0;

    for (index = 0; index < n_loops; ++index)
    {
        HmSchedLoop *l = &sched->loops[index];

        l->context = g_main_context_new();
        l->loop = g_main_loop_new(
            l->context, FALSE
        );
        l->loop_thread = g_thread_create(
            nmp_net_scheduler_loop_thread,
            l,
            FALSE,
            NULL
        );

		if (gather)
		{
			tsource = g_timeout_source_new(GATHER_INTERVAL_MSEC);
			if (tsource)
			{
				g_source_set_callback(tsource, nmp_net_loop_gather, NULL, NULL);
				g_source_attach(tsource, l->context);
				g_source_unref(tsource);
			}
		}
    }
}


static __inline__ NmpNetSched *
nmp_net_scheduler_new(guint n_loops, gboolean gather)
{
    gint mem_size;
    NmpNetSched *sched;

	if (!n_loops)
		n_loops = 1;
    mem_size = sizeof(NmpNetSched) + (n_loops * sizeof(HmSchedLoop));
    sched = (NmpNetSched *)g_malloc(mem_size);
    nmp_net_scheduler_init(sched, n_loops, gather);
    return sched;
}


static __inline__ void
nmp_net_scheduler_free(NmpNetSched *sched)
{
	BUG();
}


static __inline__ void
nmp_net_scheduler_wakup(NmpNetSched *sched)
{
	gint index;

	for (index = 0; index < sched->nr_loops; ++index)
	{
		HmSchedLoop *l = &sched->loops[index];
		g_main_context_wakeup(l->context);
	}
}


static __inline__ GMainContext *
nmp_net_scheduler_get_context(NmpNetSched *sched)
{
    HmSchedLoop *l;

    l = &sched->loops[sched->last_loop];
    ++sched->last_loop;
    if (sched->last_loop >= sched->nr_loops)
        sched->last_loop = 0;
    return l->context;
}


static guint
nmp_net_io_hash_func(gconstpointer key)
{
    return (guint)key;
}


static gboolean
nmp_net_io_hash_key_equal(gconstpointer a, gconstpointer b)
{
    return a == b;
}


static __inline__ void
nmp_net_init(NmpNet *self, guint nloops, gboolean gather,
	NmpPacketProto *ll_proto, NmpPayloadProto *hl_proto,
	gpointer priv_data)
{
    self->ref_count = 1;
    self->killed = 0;
    self->io_table = g_hash_table_new(
        nmp_net_io_hash_func,
        nmp_net_io_hash_key_equal
    );

    self->lock = g_mutex_new();

    self->scheduler = nmp_net_scheduler_new(nloops, gather);
    self->ll_proto = ll_proto;
    self->hl_proto = hl_proto;

    self->on_init = NULL;
    self->on_fin = NULL;
    self->on_read = NULL;

    self->priv_data = priv_data;
}


__export NmpNet *
nmp_net_new(NmpPacketProto *ll_proto,
    NmpPayloadProto *hl_proto, gpointer priv_data)
{
    NmpNet *net;
    G_ASSERT(ll_proto != NULL && hl_proto != NULL);

    net = g_new0(NmpNet, 1);
    nmp_net_init(net, 1, FALSE, ll_proto, hl_proto, priv_data);

    return net;
}


__export NmpNet *
nmp_net_new_full(guint nloops, gboolean gather,
	NmpPacketProto *ll_proto, NmpPayloadProto *hl_proto,
	gpointer priv_data)
{
    NmpNet *net;
    G_ASSERT(ll_proto != NULL && hl_proto != NULL);

    net = g_new0(NmpNet, 1);
    nmp_net_init(net, nloops, gather, ll_proto, hl_proto, priv_data);

    return net;
}


static __inline__ void
nmp_net_free(NmpNet *net)
{
    BUG_ON(!net->killed);
    nmp_net_scheduler_free(net->scheduler);

    g_mutex_free(net->lock);
    g_hash_table_unref(net->io_table);
    g_free(net);
}


__export NmpNet *
nmp_net_ref(NmpNet *net)
{
    G_ASSERT(net != NULL && 
        g_atomic_int_get(&net->ref_count) > 0);

    g_atomic_int_add(&net->ref_count, 1);
    return net;
}


__export void
nmp_net_unref(NmpNet *net)
{
    G_ASSERT(net != NULL && 
        g_atomic_int_get(&net->ref_count) > 0);

    if (g_atomic_int_dec_and_test(&net->ref_count))
    {
        nmp_net_free(net);
    }   
}


static __inline__ void
nmp_net_kill(NmpNet *net)
{
    BUG();
}


__export void
nmp_net_release(NmpNet *net)
{//TODO:
    G_ASSERT(net != NULL);

    nmp_net_kill(net);
    nmp_net_unref(net);
}


__export void
nmp_net_set_reader(NmpNet *net, NmpIOReader reader)
{
    G_ASSERT(net != NULL);

    net->on_read = reader;
}


__export gpointer
nmp_net_get_private(NmpNet *net)
{
    G_ASSERT(net != NULL);

    return net->priv_data;
}


__export void 
nmp_net_set_funcs(NmpNet *net, NmpIOInit init, NmpIOFin fin)
{
    G_ASSERT(net != NULL);

    net->on_init = init;
    net->on_fin = fin;
}


static __inline__ gint
__nmp_net_add_io(NmpNet *net, NmpNetIO *net_io, gint notify)
{
    gint rc = 0;

    if (net->killed)
        return -E_NETDIE;

#ifdef HM_DEBUG
    BUG_ON(g_hash_table_lookup(net->io_table, net_io));
#endif

    if (notify && net->on_init)
        rc = (*net->on_init)(net->priv_data, net_io);

    if (!rc)
    {
        g_hash_table_insert(net->io_table, net_io, net_io);
        nmp_net_io_set_owner(net_io, net);
        nmp_net_ref(net);
        nmp_net_io_ref(net_io);
    }

    return rc;
}


__export gint
nmp_net_add_io(NmpNet *net, NmpNetIO *net_io, gint notify)
{
    gint rc;
    GMainContext *context;
    G_ASSERT(net != NULL && net_io != NULL);

    g_mutex_lock(net->lock);
    rc = __nmp_net_add_io(net, net_io, notify);
    g_mutex_unlock(net->lock);

    if (!rc)
    {
        context = nmp_net_scheduler_get_context(net->scheduler);
        nmp_net_io_attach(net_io, context);
    }

    return rc;
}


__export NmpNetIO *
nmp_net_create_io(NmpNet *net, NmpConnection *conn, NmpIOEst on_est,
    gint *err)
{
    NmpNetIO *io;
    gint rc;
    G_ASSERT(net != NULL && conn != NULL);

    io = nmp_net_io_new(conn, net->ll_proto, 
        net->hl_proto, err);
    if (G_UNLIKELY(!io))
    {
        nmp_connection_close(conn);
        return NULL;
    }

    nmp_net_io_set_ester(io, on_est);
    rc = nmp_net_add_io(net, io, 0);
    if (rc)
    {
        if (err)
            *err = rc;

        nmp_net_io_kill(io);
        nmp_net_io_unref(io);
        return NULL;
    }

    return io;
}


__export NmpNetIO *
nmp_net_create_listen_io(NmpNet *net, NmpConnection *conn,
    gint *err)
{
    NmpNetIO *io;
    gint rc;
    G_ASSERT(net != NULL && conn != NULL);

    io = nmp_net_listen_io_new(conn, net->ll_proto,
        net->hl_proto, err);
    if (G_UNLIKELY(!io))
    {
        nmp_connection_close(conn);
        return NULL;
    }

    rc = nmp_net_add_io(net, io, 0);
    if (rc)
    {
        if (err)
            *err = rc;

        nmp_net_io_kill(io);
        nmp_net_io_unref(io);
        return NULL;
    }

    return io;
}


__export NmpNetIO *
nmp_net_create_listen_io_2(NmpNet *net, struct sockaddr *sa, gint *err)
{
    NmpConnection *conn;
    gint rc;

    conn = nmp_connection_new(sa, CF_TYPE_TCP|CF_FLGS_NONBLOCK, err);
    if (conn)
    {
        rc = nmp_connection_listen(conn);
        if (G_UNLIKELY(rc < 0))
        {
            nmp_connection_close(conn);
            if (err)
                *err = rc;
            return NULL;
        }

        return nmp_net_create_listen_io(net, conn, err);
    }

    return NULL;
}


__export gint
nmp_net_write_io(NmpNetIO *net_io, gpointer msg)
{
    return nmp_net_io_write_message(net_io, msg);
}


__export NmpNetIO *
nmp_net_ref_io(NmpNetIO *net_io)
{
    return nmp_net_io_ref(net_io);
}


__export void
nmp_net_unref_io(NmpNetIO *net_io)
{
    nmp_net_io_unref(net_io);
}


__export void
nmp_net_set_io_private(NmpNetIO *net_io, gpointer priv_data,
    NmpDesFun des)
{
    nmp_net_io_set_private(net_io, priv_data, des);
}


__export gpointer
nmp_net_get_io_private(NmpNetIO *net_io)
{
    return nmp_net_io_get_private(net_io);
}


__export gboolean
nmp_net_set_io_ttd(NmpNetIO *net_io, gint milli_sec)
{
    return nmp_net_io_set_ttd(net_io, milli_sec);
}


static __inline__ void
__nmp_net_establish_io(NmpNet *net, NmpNetIO *net_io)
{
    gpointer io;

    io = g_hash_table_lookup(net->io_table, net_io);
    if (G_LIKELY(io))
    {
        nmp_net_io_on_establish(net_io, net->priv_data);
    }
}


__export void
nmp_net_establish_io(NmpNet *net, NmpNetIO *net_io)
{
    G_ASSERT(net != NULL && net_io != NULL);

    g_mutex_lock(net->lock);
    __nmp_net_establish_io(net, net_io);
    g_mutex_unlock(net->lock);
}


static __inline__ gint
__nmp_net_kill_io(NmpNet *net, NmpNetIO *net_io,
    gint notify, gint err)
{
   if (net->killed)
        return 0;

    if (g_hash_table_remove(net->io_table, net_io))
    {
        if (notify && net->on_fin)
            (*net->on_fin)(net->priv_data, net_io, err);
        return 1;
    }

    return 0;
}


static __inline__ gint
_nmp_net_kill_io(NmpNet *net, NmpNetIO *net_io,
    gint notify, gint err)
{
    gint found;

    g_mutex_lock(net->lock);
    found = __nmp_net_kill_io(net, net_io, notify, err);
    g_mutex_unlock(net->lock);

    if (found)
    {
        nmp_net_io_kill(net_io);
        nmp_net_io_unref(net_io);   /* decrease ref owned by #net::io_table */
        return 0;
    }

    return -E_NONETIO;
}


__export void
nmp_net_async_kill_io(NmpNet *net, NmpNetIO *net_io, gint err)
{
    gint found;

    g_mutex_lock(net->lock);
    found = __nmp_net_kill_io(net, net_io, 1, err);
    g_mutex_unlock(net->lock);

    if (found)
    {   /* decrease ref owned by #net::io_table */
        nmp_net_io_unref(net_io);       
    }
}


__export gint
nmp_net_kill_io(NmpNet *net, NmpNetIO *net_io)
{
    G_ASSERT(net != NULL && net_io != NULL);

    return _nmp_net_kill_io(net, net_io, 0, 0);
}


static __inline__ gint
__nmp_net_recv_message(NmpNet *net, NmpNetIO *net_io, gpointer msg)
{
    gint rc = -E_NETNOREADER;

    if (net->on_read)
    {
        rc = (*net->on_read)(net->priv_data, net_io, msg);
    }

    return rc;
}


__export gint
nmp_net_recv_message(NmpNet *net, NmpNetIO *net_io, gpointer msg)
{
    gint rc = -E_NETDIE;
    G_ASSERT(net != NULL);

    if (!net->killed)       //Fixme: race condition!
    {
        rc = __nmp_net_recv_message(net, net_io, msg);
    }

    return rc;
}


__export void
nmp_net_wakeup_context(NmpNet *net)
{
    G_ASSERT(net != NULL);

    if (net->scheduler)
    {
        nmp_net_scheduler_wakup(net->scheduler);
    }
}


__export gchar *
nmp_net_get_io_peer_name(NmpNetIO *net_io)
{
    if (!net_io)
        return NULL;
    return nmp_net_io_get_peer(net_io);
}


__export void
nmp_net_set_heavy_io_load(NmpNetIO *net_io)
{
    if (!net_io)
        return;

    nmp_net_io_set_heavy_load(net_io);
}


__export void
nmp_net_set_io_block_size(NmpNetIO *net_io, gint size)
{
    if (!net_io)
        return;

    nmp_net_io_set_block_size(net_io, size);    
}

//:~ End
