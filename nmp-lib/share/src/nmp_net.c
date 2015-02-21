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


typedef struct _JpfNetSched JpfNetSched;
struct _JpfNetSched
{
    gint nr_loops;
    gint last_loop;
    HmSchedLoop loops[0];
};


struct _JpfNet
{
    gint            ref_count;
    gint            killed;     /* state */

    GHashTable      *io_table;  /* net-io hash table */
    GMutex          *lock;

    JpfNetSched     *scheduler; /* io scheduler */

    JpfPacketProto  *ll_proto;  /* Low Level: packet proto */
    JpfPayloadProto *hl_proto;  /* High Level: payload proto */

    HmIOInit       on_init;
    HmIOFin        on_fin;
    HmIOReader     on_read;
    gpointer        priv_data;
};


static gpointer
jpf_net_scheduler_loop_thread(gpointer user_data)
{
    HmSchedLoop *l = (HmSchedLoop*)user_data;

    g_main_loop_run(l->loop);
    BUG_ON( TRUE );
    return NULL;
}


static gboolean
jpf_net_loop_gather(gpointer data)
{
	usleep(GATHER_SLEEP_MSEC*1000);
	return TRUE;
}


static __inline__ void
jpf_net_scheduler_init(JpfNetSched *sched, guint n_loops,
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
            jpf_net_scheduler_loop_thread,
            l,
            FALSE,
            NULL
        );

		if (gather)
		{
			tsource = g_timeout_source_new(GATHER_INTERVAL_MSEC);
			if (tsource)
			{
				g_source_set_callback(tsource, jpf_net_loop_gather, NULL, NULL);
				g_source_attach(tsource, l->context);
				g_source_unref(tsource);
			}
		}
    }
}


static __inline__ JpfNetSched *
jpf_net_scheduler_new(guint n_loops, gboolean gather)
{
    gint mem_size;
    JpfNetSched *sched;

	if (!n_loops)
		n_loops = 1;
    mem_size = sizeof(JpfNetSched) + (n_loops * sizeof(HmSchedLoop));
    sched = (JpfNetSched *)g_malloc(mem_size);
    jpf_net_scheduler_init(sched, n_loops, gather);
    return sched;
}


static __inline__ void
jpf_net_scheduler_free(JpfNetSched *sched)
{
	BUG();
}


static __inline__ void
jpf_net_scheduler_wakup(JpfNetSched *sched)
{
	gint index;

	for (index = 0; index < sched->nr_loops; ++index)
	{
		HmSchedLoop *l = &sched->loops[index];
		g_main_context_wakeup(l->context);
	}
}


static __inline__ GMainContext *
jpf_net_scheduler_get_context(JpfNetSched *sched)
{
    HmSchedLoop *l;

    l = &sched->loops[sched->last_loop];
    ++sched->last_loop;
    if (sched->last_loop >= sched->nr_loops)
        sched->last_loop = 0;
    return l->context;
}


static guint
jpf_net_io_hash_func(gconstpointer key)
{
    return (guint)key;
}


static gboolean
jpf_net_io_hash_key_equal(gconstpointer a, gconstpointer b)
{
    return a == b;
}


static __inline__ void
jpf_net_init(JpfNet *self, guint nloops, gboolean gather,
	JpfPacketProto *ll_proto, JpfPayloadProto *hl_proto,
	gpointer priv_data)
{
    self->ref_count = 1;
    self->killed = 0;
    self->io_table = g_hash_table_new(
        jpf_net_io_hash_func,
        jpf_net_io_hash_key_equal
    );

    self->lock = g_mutex_new();

    self->scheduler = jpf_net_scheduler_new(nloops, gather);
    self->ll_proto = ll_proto;
    self->hl_proto = hl_proto;

    self->on_init = NULL;
    self->on_fin = NULL;
    self->on_read = NULL;

    self->priv_data = priv_data;
}


__export JpfNet *
jpf_net_new(JpfPacketProto *ll_proto,
    JpfPayloadProto *hl_proto, gpointer priv_data)
{
    JpfNet *net;
    G_ASSERT(ll_proto != NULL && hl_proto != NULL);

    net = g_new0(JpfNet, 1);
    jpf_net_init(net, 1, FALSE, ll_proto, hl_proto, priv_data);

    return net;
}


__export JpfNet *
jpf_net_new_full(guint nloops, gboolean gather,
	JpfPacketProto *ll_proto, JpfPayloadProto *hl_proto,
	gpointer priv_data)
{
    JpfNet *net;
    G_ASSERT(ll_proto != NULL && hl_proto != NULL);

    net = g_new0(JpfNet, 1);
    jpf_net_init(net, nloops, gather, ll_proto, hl_proto, priv_data);

    return net;
}


static __inline__ void
jpf_net_free(JpfNet *net)
{
    BUG_ON(!net->killed);
    jpf_net_scheduler_free(net->scheduler);

    g_mutex_free(net->lock);
    g_hash_table_unref(net->io_table);
    g_free(net);
}


__export JpfNet *
jpf_net_ref(JpfNet *net)
{
    G_ASSERT(net != NULL && 
        g_atomic_int_get(&net->ref_count) > 0);

    g_atomic_int_add(&net->ref_count, 1);
    return net;
}


__export void
jpf_net_unref(JpfNet *net)
{
    G_ASSERT(net != NULL && 
        g_atomic_int_get(&net->ref_count) > 0);

    if (g_atomic_int_dec_and_test(&net->ref_count))
    {
        jpf_net_free(net);
    }   
}


static __inline__ void
jpf_net_kill(JpfNet *net)
{
    BUG();
}


__export void
jpf_net_release(JpfNet *net)
{//TODO:
    G_ASSERT(net != NULL);

    jpf_net_kill(net);
    jpf_net_unref(net);
}


__export void
jpf_net_set_reader(JpfNet *net, HmIOReader reader)
{
    G_ASSERT(net != NULL);

    net->on_read = reader;
}


__export gpointer
jpf_net_get_private(JpfNet *net)
{
    G_ASSERT(net != NULL);

    return net->priv_data;
}


__export void 
jpf_net_set_funcs(JpfNet *net, HmIOInit init, HmIOFin fin)
{
    G_ASSERT(net != NULL);

    net->on_init = init;
    net->on_fin = fin;
}


static __inline__ gint
__jpf_net_add_io(JpfNet *net, JpfNetIO *net_io, gint notify)
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
        jpf_net_io_set_owner(net_io, net);
        jpf_net_ref(net);
        jpf_net_io_ref(net_io);
    }

    return rc;
}


__export gint
jpf_net_add_io(JpfNet *net, JpfNetIO *net_io, gint notify)
{
    gint rc;
    GMainContext *context;
    G_ASSERT(net != NULL && net_io != NULL);

    g_mutex_lock(net->lock);
    rc = __jpf_net_add_io(net, net_io, notify);
    g_mutex_unlock(net->lock);

    if (!rc)
    {
        context = jpf_net_scheduler_get_context(net->scheduler);
        jpf_net_io_attach(net_io, context);
    }

    return rc;
}


__export JpfNetIO *
jpf_net_create_io(JpfNet *net, HmConnection *conn, HmIOEst on_est,
    gint *err)
{
    JpfNetIO *io;
    gint rc;
    G_ASSERT(net != NULL && conn != NULL);

    io = jpf_net_io_new(conn, net->ll_proto, 
        net->hl_proto, err);
    if (G_UNLIKELY(!io))
    {
        jpf_connection_close(conn);
        return NULL;
    }

    jpf_net_io_set_ester(io, on_est);
    rc = jpf_net_add_io(net, io, 0);
    if (rc)
    {
        if (err)
            *err = rc;

        jpf_net_io_kill(io);
        jpf_net_io_unref(io);
        return NULL;
    }

    return io;
}


__export JpfNetIO *
jpf_net_create_listen_io(JpfNet *net, HmConnection *conn,
    gint *err)
{
    JpfNetIO *io;
    gint rc;
    G_ASSERT(net != NULL && conn != NULL);

    io = jpf_net_listen_io_new(conn, net->ll_proto,
        net->hl_proto, err);
    if (G_UNLIKELY(!io))
    {
        jpf_connection_close(conn);
        return NULL;
    }

    rc = jpf_net_add_io(net, io, 0);
    if (rc)
    {
        if (err)
            *err = rc;

        jpf_net_io_kill(io);
        jpf_net_io_unref(io);
        return NULL;
    }

    return io;
}


__export JpfNetIO *
jpf_net_create_listen_io_2(JpfNet *net, struct sockaddr *sa, gint *err)
{
    HmConnection *conn;
    gint rc;

    conn = jpf_connection_new(sa, CF_TYPE_TCP|CF_FLGS_NONBLOCK, err);
    if (conn)
    {
        rc = jpf_connection_listen(conn);
        if (G_UNLIKELY(rc < 0))
        {
            jpf_connection_close(conn);
            if (err)
                *err = rc;
            return NULL;
        }

        return jpf_net_create_listen_io(net, conn, err);
    }

    return NULL;
}


__export gint
jpf_net_write_io(JpfNetIO *net_io, gpointer msg)
{
    return jpf_net_io_write_message(net_io, msg);
}


__export JpfNetIO *
jpf_net_ref_io(JpfNetIO *net_io)
{
    return jpf_net_io_ref(net_io);
}


__export void
jpf_net_unref_io(JpfNetIO *net_io)
{
    jpf_net_io_unref(net_io);
}


__export void
jpf_net_set_io_private(JpfNetIO *net_io, gpointer priv_data,
    HmDesFun des)
{
    jpf_net_io_set_private(net_io, priv_data, des);
}


__export gpointer
jpf_net_get_io_private(JpfNetIO *net_io)
{
    return jpf_net_io_get_private(net_io);
}


__export gboolean
jpf_net_set_io_ttd(JpfNetIO *net_io, gint milli_sec)
{
    return jpf_net_io_set_ttd(net_io, milli_sec);
}


static __inline__ void
__jpf_net_establish_io(JpfNet *net, JpfNetIO *net_io)
{
    gpointer io;

    io = g_hash_table_lookup(net->io_table, net_io);
    if (G_LIKELY(io))
    {
        jpf_net_io_on_establish(net_io, net->priv_data);
    }
}


__export void
jpf_net_establish_io(JpfNet *net, JpfNetIO *net_io)
{
    G_ASSERT(net != NULL && net_io != NULL);

    g_mutex_lock(net->lock);
    __jpf_net_establish_io(net, net_io);
    g_mutex_unlock(net->lock);
}


static __inline__ gint
__jpf_net_kill_io(JpfNet *net, JpfNetIO *net_io,
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
_jpf_net_kill_io(JpfNet *net, JpfNetIO *net_io,
    gint notify, gint err)
{
    gint found;

    g_mutex_lock(net->lock);
    found = __jpf_net_kill_io(net, net_io, notify, err);
    g_mutex_unlock(net->lock);

    if (found)
    {
        jpf_net_io_kill(net_io);
        jpf_net_io_unref(net_io);   /* decrease ref owned by #net::io_table */
        return 0;
    }

    return -E_NONETIO;
}


__export void
jpf_net_async_kill_io(JpfNet *net, JpfNetIO *net_io, gint err)
{
    gint found;

    g_mutex_lock(net->lock);
    found = __jpf_net_kill_io(net, net_io, 1, err);
    g_mutex_unlock(net->lock);

    if (found)
    {   /* decrease ref owned by #net::io_table */
        jpf_net_io_unref(net_io);       
    }
}


__export gint
jpf_net_kill_io(JpfNet *net, JpfNetIO *net_io)
{
    G_ASSERT(net != NULL && net_io != NULL);

    return _jpf_net_kill_io(net, net_io, 0, 0);
}


static __inline__ gint
__jpf_net_recv_message(JpfNet *net, JpfNetIO *net_io, gpointer msg)
{
    gint rc = -E_NETNOREADER;

    if (net->on_read)
    {
        rc = (*net->on_read)(net->priv_data, net_io, msg);
    }

    return rc;
}


__export gint
jpf_net_recv_message(JpfNet *net, JpfNetIO *net_io, gpointer msg)
{
    gint rc = -E_NETDIE;
    G_ASSERT(net != NULL);

    if (!net->killed)       //Fixme: race condition!
    {
        rc = __jpf_net_recv_message(net, net_io, msg);
    }

    return rc;
}


__export void
jpf_net_wakeup_context(JpfNet *net)
{
    G_ASSERT(net != NULL);

    if (net->scheduler)
    {
        jpf_net_scheduler_wakup(net->scheduler);
    }
}


__export gchar *
jpf_net_get_io_peer_name(JpfNetIO *net_io)
{
    if (!net_io)
        return NULL;
    return jpf_net_io_get_peer(net_io);
}


__export void
jpf_net_set_heavy_io_load(JpfNetIO *net_io)
{
    if (!net_io)
        return;

    jpf_net_io_set_heavy_load(net_io);
}


__export void
jpf_net_set_io_block_size(JpfNetIO *net_io, gint size)
{
    if (!net_io)
        return;

    jpf_net_io_set_block_size(net_io, size);    
}

//:~ End
