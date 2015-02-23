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

struct _JNetIO
{
    atomic_t   ref_count;

    JWatch    *io_watch;
    JNet      *net;
    JMutex    *lock;

	JIOReader  on_read;
	JIOEst	   on_est;		/* On established, after connect. */

	JNetIOFin  finalize;	/* used by scheduler */
	void      *private;

	void	  *init_data;

	JNetIOFin  user_fin;
	void      *user_data;
};

extern void j_net_establish_io(JNet *net, JNetIO *net_io);
extern int j_net_add_io(JNet *net, JNetIO *net_io, void *init_data, int notify);
extern JNet *j_net_ref(JNet *net);
extern void j_net_unref(JNet *net);
extern void j_net_async_kill_io(JNet *net, JNetIO *net_io, void *init_data, int err);

static atomic_t total_net_io_count = ATOMIC_INIT;

static __inline__ JWatch *
j_net_io_create_watch(JConnection *conn, 
    JPacketProto *ll_proto, JPayloadProto *hl_proto)
{
    return (JWatch*)j_hl_io_new(conn, ll_proto, hl_proto);
}


static __inline__ JWatch *
j_net_io_create_listen_watch(JConnection *conn, 
    JPacketProto *ll_proto, JPayloadProto *hl_proto)
{
    return (JWatch*)j_hl_listen_io_new(conn, ll_proto, hl_proto);
}


static __inline__ void
j_net_io_release(JNetIO *net_io)
{
    atomic_add(&total_net_io_count, -1);
	j_debug("Net net_io '%p' finalized, total %d left.\n", 
			net_io, atomic_get(&total_net_io_count));
	
    BUG_ON(net_io->io_watch);
    BUG_ON(net_io->net);
    j_mutex_free(net_io->lock);

    j_del(net_io, JNetIO, 1);
}


__export void
j_net_io_attach(JNetIO *net_io, JEventLoop *loop)
{
    J_ASSERT(net_io != NULL);

    j_mutex_lock(net_io->lock);
    if (net_io->io_watch)
        j_watch_attach(net_io->io_watch, loop);
    j_mutex_unlock(net_io->lock);
}


__export JNetIO *
j_net_io_ref(JNetIO *net_io)
{
    J_ASSERT(net_io != NULL && 
        atomic_get(&net_io->ref_count) > 0);

    atomic_inc(&net_io->ref_count);
    return net_io;
}


__export void
j_net_io_unref(JNetIO *net_io)
{
    J_ASSERT(net_io != NULL && 
        atomic_get(&net_io->ref_count) > 0);

    if (atomic_dec_and_test_zero(&net_io->ref_count))
    {
    	if (net_io->finalize)
    		(*net_io->finalize)(net_io->private);
    	if (net_io->user_fin)
    		(*net_io->user_fin)(net_io->user_data);
        j_net_io_release(net_io);
    }
}


/*
 * kill a net-io object without unref.
*/
__export void
j_net_io_kill(JNetIO *net_io)
{
    JWatch *watch;
    JNet *net;
    J_ASSERT(net_io != NULL);

    j_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    j_mutex_unlock(net_io->lock);

    if (watch)
    {
        j_watch_kill(watch);
        j_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        j_net_unref(net);
    }
}


__export void
j_net_io_async_kill(JNetIO *net_io, int err)
{
    JWatch *watch;
    JNet *net;
    J_ASSERT(net_io != NULL);

    j_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    j_mutex_unlock(net_io->lock);

    if (watch)
    {
        j_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        j_net_async_kill_io(net, net_io, net_io->init_data, err);
        j_net_unref(net);
    }
}


__export JNetIO *
j_net_io_new(JConnection *conn, JPacketProto *ll_proto,
    JPayloadProto *hl_proto, void *init_data, int *err)
{
    JNetIO *net_io;
    J_ASSERT(conn != NULL && ll_proto != NULL 
        && hl_proto != NULL);

    net_io = j_new0(JNetIO, 1);
    atomic_set(&net_io->ref_count, 1);
    net_io->io_watch = j_net_io_create_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        j_del(net_io, JNetIO, 1);
        return NULL;
    }

    j_watch_set_private(net_io->io_watch, net_io);
    j_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = j_mutex_new();
    net_io->init_data = init_data;
    atomic_add(&total_net_io_count, 1);

    return net_io;
}


__export JNetIO *
j_net_listen_io_new(JConnection *conn, JPacketProto *ll_proto,
    JPayloadProto *hl_proto, void *init_data, int *err)
{
    JNetIO *net_io;
    J_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

    net_io = j_new0(JNetIO, 1);
    //net_io->ref_count = 1;
    atomic_set(&net_io->ref_count, 1);
    net_io->io_watch = j_net_io_create_listen_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        j_del(net_io, JNetIO, 1);
        return NULL;
    }

    j_watch_set_private(net_io->io_watch, net_io);
    j_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = j_mutex_new();
	net_io->init_data = init_data;
    atomic_add(&total_net_io_count, 1);

    return net_io;
}


static __inline__ void
__j_net_io_set_owner(JNetIO *net_io, JNet *owner)
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
j_net_io_set_owner(JNetIO *net_io, void *owner)
{
    J_ASSERT(net_io != NULL);

    j_mutex_lock(net_io->lock);
    __j_net_io_set_owner(net_io, (JNet*)owner);
    j_mutex_unlock(net_io->lock);
}


__export int
j_net_io_read_message(JNetIO *net_io, void *msg)
{
    int rc = -E_NETIODIE;
    JIOReader on_read = NULL;
    J_ASSERT(net_io != NULL);

    j_mutex_lock(net_io->lock);
    if (net_io->io_watch)
		on_read = net_io->on_read;
    j_mutex_unlock(net_io->lock);

	/* fixme: race-condition*/
	if (on_read)
        rc = (*on_read)(net_io, msg, net_io->init_data);

    return rc;
}


__export int
j_net_io_write_message(JNetIO *net_io, void *msg)
{
    JWatch *watch;
    int rc = 0;
    J_ASSERT(net_io != NULL);

    j_mutex_lock(net_io->lock);

    watch = net_io->io_watch;

    if (watch)
    {
        j_watch_ref(watch);
    }

    j_mutex_unlock(net_io->lock);

    if (watch)
    {
        rc = j_watch_write_message(watch, msg);
        j_watch_unref(watch);
    }

    return rc;
}


static __inline__ int
j_net_io_add_child(JNetIO *net_io, JNetIO *new_io)
{
    JNet *net = NULL;
    int rc = -E_NETIODIE;

    j_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = j_net_ref(net_io->net);
    }
    j_mutex_unlock(net_io->lock);

    if (net)
    {
        rc = j_net_add_io(net, new_io, net_io->init_data, 1);
        j_net_unref(net);
    }

    return rc;
}


__export int
j_net_io_add_child_watch(JNetIO *net_io, void *watch)
{
    JNetIO *new_io;
    int rc;
    J_ASSERT(net_io != NULL && watch != NULL);

    new_io = j_new0(JNetIO, 1);
    atomic_set(&new_io->ref_count, 1); /* 1 returned to user */
    new_io->io_watch = (JWatch*)watch;
    new_io->net = NULL;
    new_io->lock = j_mutex_new();
	new_io->init_data = net_io->init_data;

    j_watch_set_private(watch, new_io);
    j_net_io_ref(new_io); 		/* increased by watch */

    atomic_add(&total_net_io_count, 1);
    rc = j_net_io_add_child(net_io, new_io);
    if (rc)
    {
        j_net_io_kill(new_io);
        j_net_io_unref(new_io);
    }

    return rc;
}


__export void
j_net_io_establish(JNetIO *net_io)
{
	JNet *net = NULL;
	J_ASSERT(net_io != NULL);

	j_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = j_net_ref(net_io->net);
    }
	j_mutex_unlock(net_io->lock);

	if (net)
	{
		j_net_establish_io(net, net_io);
		j_net_unref(net);
	}
}


__export void
j_net_io_on_establish(JNetIO *net_io)
{
	J_ASSERT(net_io != NULL);

	if (net_io->on_est)
	{
		(*net_io->on_est)(net_io, net_io->init_data);
	}
}


__export void
j_net_io_set_ttd(JNetIO *net_io, int milli_secs)
{
    JWatch *watch;
    J_ASSERT(net_io != NULL);

    j_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    if (watch)
    {
        j_watch_ref(watch);
    }

    j_mutex_unlock(net_io->lock);

    if (watch)
    {
        j_watch_set_conn_ttd(watch, milli_secs);
        j_watch_unref(watch);
    }
}


__export void
j_net_io_set_reader(JNetIO *net_io, JIOReader reader, void *init_data)
{
	J_ASSERT(net_io != NULL);

	net_io->on_read = reader;
	net_io->init_data = init_data;
}


__export void
j_net_io_set_ester(JNetIO *net_io, JIOEst on_est)
{
	J_ASSERT(net_io != NULL);

	net_io->on_est = on_est;	
}


__export void
j_net_io_on_destroy(JNetIO *net_io, JNetIOFin fin, void *data)
{
	J_ASSERT(net_io != NULL);

	BUG_ON(net_io->finalize);
	net_io->finalize = fin;
	net_io->private = data;
}


__export void
j_net_io_set_u(JNetIO *net_io, void *user, JNetIOFin user_fin)
{
	net_io->user_fin = user_fin;
	net_io->user_data = user;
}


__export void *
j_net_io_get_u(JNetIO *net_io)
{
	return net_io->user_data;
}


__export char *
j_net_io_get_host(JNetIO *net_io, char *ip)
{
	JWatch *watch;
	char *addr = NULL;
	J_ASSERT(net_io != NULL);

	j_mutex_lock(net_io->lock);
	watch = net_io->io_watch;
	if (watch)
	{
	    j_watch_ref(watch);
	}
	j_mutex_unlock(net_io->lock);

	if (watch)
	{
	    addr = j_watch_get_host(watch, ip);
	    j_watch_unref(watch);
	}

	return addr;
}


__export char *
j_net_io_get_peer(JNetIO *net_io, char *ip)
{
	JWatch *watch;
	char *addr = NULL;
	J_ASSERT(net_io != NULL);

	j_mutex_lock(net_io->lock);
	watch = net_io->io_watch;
	if (watch)
	{
	    j_watch_ref(watch);
	}
	j_mutex_unlock(net_io->lock);

	if (watch)
	{
	    addr = j_watch_get_peer(watch, ip);
	    j_watch_unref(watch);
	}

	return addr;
}

//:~ End
