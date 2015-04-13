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

struct _nmp_netio
{
    atomic_t   ref_count;

    nmp_watch_t  *io_watch;
    nmp_net      *net;
    nmp_mutex_t  *lock;

	nmp_io_reader_func  on_read;
	nmp_io_est_func	   on_est;		/* On established, after connect. */

	nmp_netio_fin_func  finalize;	/* used by scheduler */
	void      *private;

	void	  *init_data;

	nmp_netio_fin_func  user_fin;
	void      *user_data;
};

extern void nmp_net_establish_io(nmp_net *net, nmp_netio_t *net_io);
extern int nmp_net_add_io(nmp_net *net, nmp_netio_t *net_io, void *init_data, int notify);
extern nmp_net *nmp_net_ref(nmp_net *net);
extern void nmp_net_unref(nmp_net *net);
extern void nmp_net_async_kill_io(nmp_net *net, nmp_netio_t *net_io, void *init_data, int err);

static atomic_t total_net_io_count = ATOMIC_INIT;

static __inline__ nmp_watch_t *
nmp_net_io_create_watch(nmp_conn_t *conn, 
    nmp_packet_proto_t *ll_proto, nmp_payload_proto_t *hl_proto)
{
    return (nmp_watch_t*)nmp_hlio_io_new(conn, ll_proto, hl_proto);
}


static __inline__ nmp_watch_t *
nmp_net_io_create_listen_watch(nmp_conn_t *conn, 
    nmp_packet_proto_t *ll_proto, nmp_payload_proto_t *hl_proto)
{
    return (nmp_watch_t*)nmp_hlio_listen_io_new(conn, ll_proto, hl_proto);
}


static __inline__ void
nmp_net_io_release(nmp_netio_t *net_io)
{
    atomic_add(&total_net_io_count, -1);
	nmp_debug("Net net_io '%p' finalized, total %d left.\n", 
			net_io, atomic_get(&total_net_io_count));
	
    BUG_ON(net_io->io_watch);
    BUG_ON(net_io->net);
    nmp_mutex_free(net_io->lock);

    nmp_del(net_io, nmp_netio_t, 1);
}


__export void
nmp_net_io_attach(nmp_netio_t *net_io, nmp_event_loop_t *loop)
{
    NMP_ASSERT(net_io != NULL);

    nmp_mutex_lock(net_io->lock);
    if (net_io->io_watch)
        nmp_watch_attach(net_io->io_watch, loop);
    nmp_mutex_unlock(net_io->lock);
}


__export nmp_netio_t *
nmp_net_io_ref(nmp_netio_t *net_io)
{
    NMP_ASSERT(net_io != NULL && 
        atomic_get(&net_io->ref_count) > 0);

    atomic_inc(&net_io->ref_count);
    return net_io;
}


__export void
nmp_net_io_unref(nmp_netio_t *net_io)
{
    NMP_ASSERT(net_io != NULL && 
        atomic_get(&net_io->ref_count) > 0);

    if (atomic_dec_and_test_zero(&net_io->ref_count))
    {
    	if (net_io->finalize)
    		(*net_io->finalize)(net_io->private);
    	if (net_io->user_fin)
    		(*net_io->user_fin)(net_io->user_data);
        nmp_net_io_release(net_io);
    }
}


/*
 * kill a net-io object without unref.
*/
__export void
nmp_net_io_kill(nmp_netio_t *net_io)
{
    nmp_watch_t *watch;
    nmp_net *net;
    NMP_ASSERT(net_io != NULL);

    nmp_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    nmp_mutex_unlock(net_io->lock);

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
nmp_net_io_async_kill(nmp_netio_t *net_io, int err)
{
    nmp_watch_t *watch;
    nmp_net *net;
    NMP_ASSERT(net_io != NULL);

    nmp_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    net_io->io_watch = NULL;
    net = net_io->net;
    net_io->net = NULL;

    nmp_mutex_unlock(net_io->lock);

    if (watch)
    {
        nmp_watch_unref(watch);     /* what we unref is watch */
    }

    if (net)
    {
        nmp_net_async_kill_io(net, net_io, net_io->init_data, err);
        nmp_net_unref(net);
    }
}


__export nmp_netio_t *
nmp_net_io_new(nmp_conn_t *conn, nmp_packet_proto_t *ll_proto,
    nmp_payload_proto_t *hl_proto, void *init_data, int *err)
{
    nmp_netio_t *net_io;
    NMP_ASSERT(conn != NULL && ll_proto != NULL 
        && hl_proto != NULL);

    net_io = nmp_new0(nmp_netio_t, 1);
    atomic_set(&net_io->ref_count, 1);
    net_io->io_watch = nmp_net_io_create_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        nmp_del(net_io, nmp_netio_t, 1);
        return NULL;
    }

    nmp_watch_set_private(net_io->io_watch, net_io);
    nmp_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = nmp_mutex_new();
    net_io->init_data = init_data;
    atomic_add(&total_net_io_count, 1);

    return net_io;
}


__export nmp_netio_t *
nmp_net_listen_io_new(nmp_conn_t *conn, nmp_packet_proto_t *ll_proto,
    nmp_payload_proto_t *hl_proto, void *init_data, int *err)
{
    nmp_netio_t *net_io;
    NMP_ASSERT(conn != NULL && ll_proto != NULL && hl_proto != NULL);

    net_io = nmp_new0(nmp_netio_t, 1);
    //net_io->ref_count = 1;
    atomic_set(&net_io->ref_count, 1);
    net_io->io_watch = nmp_net_io_create_listen_watch(conn, ll_proto,
        hl_proto);

    if (!net_io->io_watch)
    {
        if (err)
            *err = -E_WATCH;
        nmp_del(net_io, nmp_netio_t, 1);
        return NULL;
    }

    nmp_watch_set_private(net_io->io_watch, net_io);
    nmp_net_io_ref(net_io); /* watch */
    net_io->net = NULL;
    net_io->lock = nmp_mutex_new();
	net_io->init_data = init_data;
    atomic_add(&total_net_io_count, 1);

    return net_io;
}


static __inline__ void
__nmp_net_io_set_owner(nmp_netio_t *net_io, nmp_net *owner)
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
nmp_net_io_set_owner(nmp_netio_t *net_io, void *owner)
{
    NMP_ASSERT(net_io != NULL);

    nmp_mutex_lock(net_io->lock);
    __nmp_net_io_set_owner(net_io, (nmp_net*)owner);
    nmp_mutex_unlock(net_io->lock);
}


__export int
nmp_net_io_read_message(nmp_netio_t *net_io, void *msg)
{
    int rc = -E_NETIODIE;
    nmp_io_reader_func on_read = NULL;
    NMP_ASSERT(net_io != NULL);

    nmp_mutex_lock(net_io->lock);
    if (net_io->io_watch)
		on_read = net_io->on_read;
    nmp_mutex_unlock(net_io->lock);

	/* fixme: race-condition*/
	if (on_read)
        rc = (*on_read)(net_io, msg, net_io->init_data);

    return rc;
}


__export int
nmp_net_io_write_message(nmp_netio_t *net_io, void *msg)
{
    nmp_watch_t *watch;
    int rc = 0;
    NMP_ASSERT(net_io != NULL);

    nmp_mutex_lock(net_io->lock);

    watch = net_io->io_watch;

    if (watch)
    {
        nmp_watch_ref(watch);
    }

    nmp_mutex_unlock(net_io->lock);

    if (watch)
    {
        rc = nmp_watch_write_message(watch, msg);
        nmp_watch_unref(watch);
    }

    return rc;
}


static __inline__ int
nmp_net_io_add_child(nmp_netio_t *net_io, nmp_netio_t *new_io)
{
    nmp_net *net = NULL;
    int rc = -E_NETIODIE;

    nmp_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = nmp_net_ref(net_io->net);
    }
    nmp_mutex_unlock(net_io->lock);

    if (net)
    {
        rc = nmp_net_add_io(net, new_io, net_io->init_data, 1);
        nmp_net_unref(net);
    }

    return rc;
}


__export int
nmp_net_io_add_child_watch(nmp_netio_t *net_io, void *watch)
{
    nmp_netio_t *new_io;
    int rc;
    NMP_ASSERT(net_io != NULL && watch != NULL);

    new_io = nmp_new0(nmp_netio_t, 1);
    atomic_set(&new_io->ref_count, 1); /* 1 returned to user */
    new_io->io_watch = (nmp_watch_t*)watch;
    new_io->net = NULL;
    new_io->lock = nmp_mutex_new();
	new_io->init_data = net_io->init_data;

    nmp_watch_set_private(watch, new_io);
    nmp_net_io_ref(new_io); 		/* increased by watch */

    atomic_add(&total_net_io_count, 1);
    rc = nmp_net_io_add_child(net_io, new_io);
    if (rc)
    {
        nmp_net_io_kill(new_io);
        nmp_net_io_unref(new_io);
    }

    return rc;
}


__export void
nmp_net_io_establish(nmp_netio_t *net_io)
{
	nmp_net *net = NULL;
	NMP_ASSERT(net_io != NULL);

	nmp_mutex_lock(net_io->lock);
    if (net_io->net)
    {
        net = nmp_net_ref(net_io->net);
    }
	nmp_mutex_unlock(net_io->lock);

	if (net)
	{
		nmp_net_establish_io(net, net_io);
		nmp_net_unref(net);
	}
}


__export void
nmp_net_io_on_establish(nmp_netio_t *net_io)
{
	NMP_ASSERT(net_io != NULL);

	if (net_io->on_est)
	{
		(*net_io->on_est)(net_io, net_io->init_data);
	}
}


__export void
nmp_net_io_set_ttd(nmp_netio_t *net_io, int milli_secs)
{
    nmp_watch_t *watch;
    NMP_ASSERT(net_io != NULL);

    nmp_mutex_lock(net_io->lock);

    watch = net_io->io_watch;
    if (watch)
    {
        nmp_watch_ref(watch);
    }

    nmp_mutex_unlock(net_io->lock);

    if (watch)
    {
        nmp_watch_set_conn_ttd(watch, milli_secs);
        nmp_watch_unref(watch);
    }
}


__export void
nmp_net_io_set_reader(nmp_netio_t *net_io, nmp_io_reader_func reader, void *init_data)
{
	NMP_ASSERT(net_io != NULL);

	net_io->on_read = reader;
	net_io->init_data = init_data;
}


__export void
nmp_net_io_set_ester(nmp_netio_t *net_io, nmp_io_est_func on_est)
{
	NMP_ASSERT(net_io != NULL);

	net_io->on_est = on_est;	
}


__export void
nmp_net_io_on_destroy(nmp_netio_t *net_io, nmp_netio_fin_func fin, void *data)
{
	NMP_ASSERT(net_io != NULL);

	BUG_ON(net_io->finalize);
	net_io->finalize = fin;
	net_io->private = data;
}


__export void
nmp_net_io_set_u(nmp_netio_t *net_io, void *user, nmp_netio_fin_func user_fin)
{
	net_io->user_fin = user_fin;
	net_io->user_data = user;
}


__export void *
nmp_net_io_get_u(nmp_netio_t *net_io)
{
	return net_io->user_data;
}


__export char *
nmp_net_io_get_host(nmp_netio_t *net_io, char *ip)
{
	nmp_watch_t *watch;
	char *addr = NULL;
	NMP_ASSERT(net_io != NULL);

	nmp_mutex_lock(net_io->lock);
	watch = net_io->io_watch;
	if (watch)
	{
	    nmp_watch_ref(watch);
	}
	nmp_mutex_unlock(net_io->lock);

	if (watch)
	{
	    addr = nmp_watch_get_host(watch, ip);
	    nmp_watch_unref(watch);
	}

	return addr;
}


__export char *
nmp_net_io_get_peer(nmp_netio_t *net_io, char *ip)
{
	nmp_watch_t *watch;
	char *addr = NULL;
	NMP_ASSERT(net_io != NULL);

	nmp_mutex_lock(net_io->lock);
	watch = net_io->io_watch;
	if (watch)
	{
	    nmp_watch_ref(watch);
	}
	nmp_mutex_unlock(net_io->lock);

	if (watch)
	{
	    addr = nmp_watch_get_peer(watch, ip);
	    nmp_watch_unref(watch);
	}

	return addr;
}

//:~ End
