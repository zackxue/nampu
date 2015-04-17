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
#include "nmp_scheduler.h"

struct _nmp_net
{
    int			ref_count;
    int			killed;     	/* state */

    nmp_list_t	*io_list;  		/* net-io object list */
    nmp_mutex_t	*lock;
	nmp_scheduler_t	*scheduler;		/* IO scheduler */

    nmp_io_init_func		on_init;
    nmp_io_fin_func		on_fin;

};


static __inline__ void
nmp_net_init(nmp_net_t *self, int loops)
{
    atomic_set(&self->ref_count, 1);
    self->killed = 0;
    self->io_list = NULL;
    self->lock = nmp_mutex_new();
    self->scheduler = nmp_scheduler_new(loops);

    self->on_init = NULL;
    self->on_fin = NULL;
}


__export nmp_net_t *
nmp_net_new(int loops)
{
    nmp_net_t *net;

    net = nmp_new0(nmp_net_t, 1);
    nmp_net_init(net, loops);

    return net;
}

static __inline__ void
nmp_net_free(nmp_net_t *net)
{
    BUG_ON(!net->killed);
    BUG_ON(net->io_list);
    BUG_ON(net->io_list);

    nmp_mutex_free(net->lock);
    nmp_del(net, nmp_net_t, 1);
}


__export nmp_net_t *
nmp_net_ref(nmp_net_t *net)
{
    NMP_ASSERT(net != NULL && 
        atomic_get(&net->ref_count) > 0);

    atomic_inc(&net->ref_count);
    return net;
}


__export void
nmp_net_unref(nmp_net_t *net)
{
    NMP_ASSERT(net != NULL && 
        atomic_get(&net->ref_count) > 0);

    if (atomic_dec_and_test_zero(&net->ref_count))
    {
        nmp_net_free(net);
    }   
}


static __inline__ void
nmp_net_kill(nmp_net_t *net)
{
	//BUG();
}


__export void
nmp_net_release(nmp_net_t *net)
{//TODO:
    NMP_ASSERT(net != NULL);

    nmp_net_kill(net);
    nmp_net_unref(net);
}


__export void 
nmp_net_set_funcs(nmp_net_t *net, nmp_io_init_func init, nmp_io_fin_func fin)
{
    NMP_ASSERT(net != NULL);

    net->on_init = init;
    net->on_fin = fin;
}


static __inline__ int
__nmp_net_kill_io(nmp_net_t *net, nmp_netio_t *net_io, void *init_data, 
	int notify, int err)
{
    nmp_list_t *list;

    if (net->killed)
        return 0;

    list = nmp_list_find(net->io_list, net_io);
    if (NMP_LIKELY(list))
    {
        net->io_list = nmp_list_delete_link(net->io_list, list);
        if (notify && net->on_fin)
        {
        	nmp_mutex_unlock(net->lock);
            (*net->on_fin)(net_io, err, init_data);
            nmp_mutex_lock(net->lock);
        }
        return 1;
    }

    return 0;
}


static __inline__ int
_nmp_net_kill_io(nmp_net_t *net, nmp_netio_t *net_io,
    int notify, int err)
{
    int found;

    nmp_mutex_lock(net->lock);
    found = __nmp_net_kill_io(net, net_io, NULL, notify, err);
    nmp_mutex_unlock(net->lock);

    if (found)
    {
        nmp_net_io_kill(net_io);
        nmp_net_io_unref(net_io);   /* decrease ref owned by #net::io_list */
        return 0;
    }

    return -E_NONETIO;
}


static __inline__ int
__nmp_net_add_io(nmp_net_t *net, nmp_netio_t *net_io, void *init_data, int notify)
{
	nmp_list_t *list;
    int rc = 0;

    if (net->killed)
    {
        return -E_NETDIE;
    }

#ifdef NMP_DEBUG
    BUG_ON(nmp_list_find(net->io_list, net_io));
#else
	list = nmp_list_find(net->io_list, net_io);
	if (list)
	{
		return -EEXIST;	
	}
#endif

	net->io_list = nmp_list_add(net->io_list, net_io);
    nmp_net_io_set_owner(net_io, net);
    nmp_net_ref(net);
    nmp_net_io_ref(net_io);

    if (notify && net->on_init)
    {
    	nmp_mutex_unlock(net->lock);

        rc = (*net->on_init)(net_io, init_data);
		if (rc)
		{
			_nmp_net_kill_io(net, net_io, 0, 0);
		}

        nmp_mutex_lock(net->lock);
    }

    return rc;
}


__export int
nmp_net_add_io(nmp_net_t *net, nmp_netio_t *net_io, void *init_data, int notify)
{
    int rc;
    NMP_ASSERT(net != NULL && net_io != NULL);

    nmp_mutex_lock(net->lock);
    rc = __nmp_net_add_io(net, net_io, init_data, notify);
    nmp_mutex_unlock(net->lock);

    if (!rc)
    {
   	 	nmp_scheduler_sched_io(net->scheduler, net_io);
   	}

    return rc;
}


__export nmp_netio_t *
nmp_net_create_io(nmp_net_t *net, nmp_conn_t *conn, nmp_2proto_t *proto,
	nmp_io_est_func on_est, int *err)
{
    nmp_netio_t *io;
    int rc;
    NMP_ASSERT(net != NULL && conn != NULL);

    io = nmp_net_io_new(conn, proto->ll_proto, 
        proto->hl_proto, proto->init_data, err);
    if (NMP_UNLIKELY(!io))
    {
    	nmp_conn_close(conn);
        return NULL;
    }

	nmp_net_io_set_ester(io, on_est);
    rc = nmp_net_add_io(net, io, proto->init_data, 0);
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


__export nmp_netio_t *
nmp_net_create_listen_io(nmp_net_t *net, nmp_conn_t *conn, nmp_2proto_t *proto,
    int *err)
{
    nmp_netio_t *io;
    int rc;
    NMP_ASSERT(net != NULL && conn != NULL && proto != NULL);

    io = nmp_net_listen_io_new(conn, proto->ll_proto,
        proto->hl_proto, proto->init_data, err);
    if (NMP_UNLIKELY(!io))
    {
    	nmp_conn_close(conn);
        return NULL;
    }

    rc = nmp_net_add_io(net, io, proto->init_data, 0);
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


__export nmp_netio_t *
nmp_net_create_listen_io_2(nmp_net_t *net, struct sockaddr *sa, nmp_2proto_t *proto, int *err)
{
    nmp_conn_t *conn;
    int rc;

    conn = nmp_conn_new(sa, CF_TYPE_TCP|CF_FLGS_NONBLOCK, err);
    if (conn)
    {
        rc = nmp_conn_listen(conn);
        if (NMP_UNLIKELY(rc < 0))
        {
            nmp_conn_close(conn);
            if (err)
                *err = rc;
            return NULL;
        }

        return nmp_net_create_listen_io(net, conn, proto, err);
    }

    return NULL;
}


__export int
nmp_net_write_io(nmp_netio_t *net_io, void * msg)
{
    return nmp_net_io_write_message(net_io, msg);
}


__export nmp_netio_t *
nmp_net_ref_io(nmp_netio_t *net_io)
{
    return nmp_net_io_ref(net_io);
}


__export void
nmp_net_unref_io(nmp_netio_t *net_io)
{
    nmp_net_io_unref(net_io);
}


__export void
nmp_net_set_io_ttd(nmp_netio_t *net_io, int milli_sec)
{
    nmp_net_io_set_ttd(net_io, milli_sec);
}


__export void
nmp_net_set_io_reader(nmp_netio_t *net_io, nmp_io_reader_func reader, void *init_data)
{
	nmp_net_io_set_reader(net_io, reader, init_data);
}


static __inline__ void
__nmp_net_establish_io(nmp_net_t *net, nmp_netio_t *net_io)
{
	nmp_list_t *list;

    list = nmp_list_find(net->io_list, net_io);
    if (NMP_LIKELY(list))
    {
    	nmp_mutex_unlock(net->lock);
		/* if net_io may be killed in on_establish() */
    	nmp_net_io_on_establish(net_io);
    	nmp_mutex_lock(net->lock);
    }
}


__export void
nmp_net_establish_io(nmp_net_t *net, nmp_netio_t *net_io)
{
	NMP_ASSERT(net != NULL && net_io != NULL);

	nmp_mutex_lock(net->lock);
	__nmp_net_establish_io(net, net_io);
	nmp_mutex_unlock(net->lock);
}


__export void
nmp_net_async_kill_io(nmp_net_t *net, nmp_netio_t *net_io, void *init_data, int err)
{
    int found;

    nmp_mutex_lock(net->lock);
    found = __nmp_net_kill_io(net, net_io, init_data, 1, err);
    nmp_mutex_unlock(net->lock);

    if (found)
    {   /* decrease ref owned by #net::io_list */
        nmp_net_io_unref(net_io);
    }
}


__export int
nmp_net_kill_io(nmp_net_t *net, nmp_netio_t *net_io)
{
    NMP_ASSERT(net != NULL && net_io != NULL);

    return _nmp_net_kill_io(net, net_io, 0, 0);
}


__export void *
nmp_set_timer(int timeout, int (*on_timer)(void*), void *data)
{
	return nmp_scheduler_add_timer(timeout, on_timer, data);
}


__export void
nmp_del_timer(void *handle)
{
	nmp_scheduler_del_timer(handle);
}


__export void
nmp_net_set_io_u(nmp_netio_t *net_io, void *u, nmp_io_fin_user *fu)
{
	nmp_net_io_set_u(net_io, u, (nmp_netio_fin_func)fu);
}


__export void *
nmp_net_get_io_u(nmp_netio_t *net_io)
{
	return nmp_net_io_get_u(net_io);
}


__export char *
nmp_net_get_io_host_name(nmp_netio_t *net_io, char *ip)
{
	if (!net_io)
		return NULL;
	return nmp_net_io_get_host(net_io, ip);
}


__export char *
nmp_net_get_io_peer_name(nmp_netio_t *net_io, char *ip)
{
	if (!net_io)
		return NULL;
	return nmp_net_io_get_peer(net_io, ip);
}

//:~ End
