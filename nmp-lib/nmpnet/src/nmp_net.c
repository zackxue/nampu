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

struct _JNet
{
    int			ref_count;
    int			killed;     	/* state */

    JList		*io_list;  		/* net-io object list */
    JMutex		*lock;
	JScheduler	*scheduler;		/* IO scheduler */

    JIOInit		on_init;
    JIOFin		on_fin;

};


static __inline__ void
j_net_init(JNet *self, int loops)
{
    atomic_set(&self->ref_count, 1);
    self->killed = 0;
    self->io_list = NULL;
    self->lock = j_mutex_new();
    self->scheduler = j_scheduler_new(loops);

    self->on_init = NULL;
    self->on_fin = NULL;
}


__export JNet *
j_net_new(int loops)
{
    JNet *net;

    net = j_new0(JNet, 1);
    j_net_init(net, loops);

    return net;
}

static __inline__ void
j_net_free(JNet *net)
{
    BUG_ON(!net->killed);
    BUG_ON(net->io_list);
    BUG_ON(net->io_list);

    j_mutex_free(net->lock);
    j_del(net, JNet, 1);
}


__export JNet *
j_net_ref(JNet *net)
{
    J_ASSERT(net != NULL && 
        atomic_get(&net->ref_count) > 0);

    atomic_inc(&net->ref_count);
    return net;
}


__export void
j_net_unref(JNet *net)
{
    J_ASSERT(net != NULL && 
        atomic_get(&net->ref_count) > 0);

    if (atomic_dec_and_test_zero(&net->ref_count))
    {
        j_net_free(net);
    }   
}


static __inline__ void
j_net_kill(JNet *net)
{
	//BUG();
}


__export void
j_net_release(JNet *net)
{//TODO:
    J_ASSERT(net != NULL);

    j_net_kill(net);
    j_net_unref(net);
}


__export void 
j_net_set_funcs(JNet *net, JIOInit init, JIOFin fin)
{
    J_ASSERT(net != NULL);

    net->on_init = init;
    net->on_fin = fin;
}


static __inline__ int
__j_net_kill_io(JNet *net, JNetIO *net_io, void *init_data, 
	int notify, int err)
{
    JList *list;

    if (net->killed)
        return 0;

    list = j_list_find(net->io_list, net_io);
    if (J_LIKELY(list))
    {
        net->io_list = j_list_delete_link(net->io_list, list);
        if (notify && net->on_fin)
        {
        	j_mutex_unlock(net->lock);
            (*net->on_fin)(net_io, err, init_data);
            j_mutex_lock(net->lock);
        }
        return 1;
    }

    return 0;
}


static __inline__ int
_j_net_kill_io(JNet *net, JNetIO *net_io,
    int notify, int err)
{
    int found;

    j_mutex_lock(net->lock);
    found = __j_net_kill_io(net, net_io, NULL, notify, err);
    j_mutex_unlock(net->lock);

    if (found)
    {
        j_net_io_kill(net_io);
        j_net_io_unref(net_io);   /* decrease ref owned by #net::io_list */
        return 0;
    }

    return -E_NONETIO;
}


static __inline__ int
__j_net_add_io(JNet *net, JNetIO *net_io, void *init_data, int notify)
{
	JList *list;
    int rc = 0;

    if (net->killed)
    {
        return -E_NETDIE;
    }

#ifdef JPF_DEBUG
    BUG_ON(j_list_find(net->io_list, net_io));
#else
	list = j_list_find(net->io_list, net_io);
	if (list)
	{
		return -EEXIST;	
	}
#endif

	net->io_list = j_list_add(net->io_list, net_io);
    j_net_io_set_owner(net_io, net);
    j_net_ref(net);
    j_net_io_ref(net_io);

    if (notify && net->on_init)
    {
    	j_mutex_unlock(net->lock);

        rc = (*net->on_init)(net_io, init_data);
		if (rc)
		{
			_j_net_kill_io(net, net_io, 0, 0);
		}

        j_mutex_lock(net->lock);
    }

    return rc;
}


__export int
j_net_add_io(JNet *net, JNetIO *net_io, void *init_data, int notify)
{
    int rc;
    J_ASSERT(net != NULL && net_io != NULL);

    j_mutex_lock(net->lock);
    rc = __j_net_add_io(net, net_io, init_data, notify);
    j_mutex_unlock(net->lock);

    if (!rc)
    {
   	 	j_scheduler_sched_io(net->scheduler, net_io);
   	}

    return rc;
}


__export JNetIO *
j_net_create_io(JNet *net, JConnection *conn, J2Proto *proto,
	JIOEst on_est, int *err)
{
    JNetIO *io;
    int rc;
    J_ASSERT(net != NULL && conn != NULL);

    io = j_net_io_new(conn, proto->ll_proto, 
        proto->hl_proto, proto->init_data, err);
    if (J_UNLIKELY(!io))
    {
    	j_connection_close(conn);
        return NULL;
    }

	j_net_io_set_ester(io, on_est);
    rc = j_net_add_io(net, io, proto->init_data, 0);
    if (rc)
    {
        if (err)
            *err = rc;

        j_net_io_kill(io);
        j_net_io_unref(io);
        return NULL;
    }

    return io;
}


__export JNetIO *
j_net_create_listen_io(JNet *net, JConnection *conn, J2Proto *proto,
    int *err)
{
    JNetIO *io;
    int rc;
    J_ASSERT(net != NULL && conn != NULL && proto != NULL);

    io = j_net_listen_io_new(conn, proto->ll_proto,
        proto->hl_proto, proto->init_data, err);
    if (J_UNLIKELY(!io))
    {
    	j_connection_close(conn);
        return NULL;
    }

    rc = j_net_add_io(net, io, proto->init_data, 0);
    if (rc)
    {
        if (err)
            *err = rc;

        j_net_io_kill(io);
        j_net_io_unref(io);
        return NULL;
    }

    return io;
}


__export JNetIO *
j_net_create_listen_io_2(JNet *net, struct sockaddr *sa, J2Proto *proto, int *err)
{
    JConnection *conn;
    int rc;

    conn = j_connection_new(sa, CF_TYPE_TCP|CF_FLGS_NONBLOCK, err);
    if (conn)
    {
        rc = j_connection_listen(conn);
        if (J_UNLIKELY(rc < 0))
        {
            j_connection_close(conn);
            if (err)
                *err = rc;
            return NULL;
        }

        return j_net_create_listen_io(net, conn, proto, err);
    }

    return NULL;
}


__export int
j_net_write_io(JNetIO *net_io, void * msg)
{
    return j_net_io_write_message(net_io, msg);
}


__export JNetIO *
j_net_ref_io(JNetIO *net_io)
{
    return j_net_io_ref(net_io);
}


__export void
j_net_unref_io(JNetIO *net_io)
{
    j_net_io_unref(net_io);
}


__export void
j_net_set_io_ttd(JNetIO *net_io, int milli_sec)
{
    j_net_io_set_ttd(net_io, milli_sec);
}


__export void
j_net_set_io_reader(JNetIO *net_io, JIOReader reader, void *init_data)
{
	j_net_io_set_reader(net_io, reader, init_data);
}


static __inline__ void
__j_net_establish_io(JNet *net, JNetIO *net_io)
{
	JList *list;

    list = j_list_find(net->io_list, net_io);
    if (J_LIKELY(list))
    {
    	j_mutex_unlock(net->lock);
		/* if net_io may be killed in on_establish() */
    	j_net_io_on_establish(net_io);
    	j_mutex_lock(net->lock);
    }
}


__export void
j_net_establish_io(JNet *net, JNetIO *net_io)
{
	J_ASSERT(net != NULL && net_io != NULL);

	j_mutex_lock(net->lock);
	__j_net_establish_io(net, net_io);
	j_mutex_unlock(net->lock);
}


__export void
j_net_async_kill_io(JNet *net, JNetIO *net_io, void *init_data, int err)
{
    int found;

    j_mutex_lock(net->lock);
    found = __j_net_kill_io(net, net_io, init_data, 1, err);
    j_mutex_unlock(net->lock);

    if (found)
    {   /* decrease ref owned by #net::io_list */
        j_net_io_unref(net_io);
    }
}


__export int
j_net_kill_io(JNet *net, JNetIO *net_io)
{
    J_ASSERT(net != NULL && net_io != NULL);

    return _j_net_kill_io(net, net_io, 0, 0);
}


__export void *
j_set_timer(int timeout, int (*on_timer)(void*), void *data)
{
	return j_scheduler_add_timer(timeout, on_timer, data);
}


__export void
j_del_timer(void *handle)
{
	j_scheduler_del_timer(handle);
}


__export void
j_net_set_io_u(JNetIO *net_io, void *u, JIOFinUser *fu)
{
	j_net_io_set_u(net_io, u, (JNetIOFin)fu);
}


__export void *
j_net_get_io_u(JNetIO *net_io)
{
	return j_net_io_get_u(net_io);
}


__export char *
j_net_get_io_host_name(JNetIO *net_io, char *ip)
{
	if (!net_io)
		return NULL;
	return j_net_io_get_host(net_io, ip);
}


__export char *
j_net_get_io_peer_name(JNetIO *net_io, char *ip)
{
	if (!net_io)
		return NULL;
	return j_net_io_get_peer(net_io, ip);
}

//:~ End
