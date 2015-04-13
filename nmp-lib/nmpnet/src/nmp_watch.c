/*
 * nmp_watch.c
 *
 * This file implements net watch.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/


#include "nmp_watch.h"
#include "nmp_netio.h"
#include "nmp_errno.h"

#define BUFFER_BLOCKS		8

static int
nmp_watch_write_out(char *buf, size_t size, void *w)
{
    nmp_watch_t *watch = (nmp_watch_t*)w;

    return nmp_conn_write(watch->conn, buf, size);
}


static __inline__ void
nmp_watch_add_child(nmp_watch_t *watch, nmp_watch_t *child)
{
    nmp_netio_t *net_io;

    net_io = (nmp_netio_t*)watch->priv_data;
    if (net_io)
    {
        nmp_net_io_add_child_watch(net_io, child);
    }
    else
    {
    	BUG();
	}
}


static __inline__ void
nmp_watch_on_establish(nmp_watch_t *watch)
{
    nmp_netio_t *net_io;

    net_io = (nmp_netio_t*)watch->priv_data;
    if (net_io)
    {
        nmp_net_io_establish(net_io);
    }
    else
    {
    	BUG();
	}
}


static __inline__ void
nmp_watch_on_clear(nmp_watch_t *watch, int err)
{
    nmp_netio_t *net_io;

    net_io = (nmp_netio_t*)watch->priv_data;
    if (net_io)
    {
        nmp_mutex_unlock(watch->lock);    /* drop the lock */
        nmp_net_io_async_kill(net_io, err);
        nmp_mutex_lock(watch->lock);
    }
}


static __inline__ void
__nmp_watch_close(nmp_watch_t *watch, int async)
{
    nmp_watch_funcs *funcs;

    if (watch->killed++)
        return;

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (funcs->close)
    {
        (*funcs->close)(watch, async);
    }

    if (async)
    {
        nmp_watch_on_clear(watch, 0);
    }
}


static __inline__ void
__nmp_watch_error(nmp_watch_t *watch, int rw, int err)
{
    nmp_watch_funcs *funcs;

    if (watch->killed++)
        return;

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (funcs->error)
    {
        (*funcs->error)(watch, rw, err);
    }

    nmp_watch_on_clear(watch, err);
}


static __inline__ void
nmp_watch_destroy_private(void *priv_data)
{
    nmp_netio_t *net_io;

    net_io = (nmp_netio_t*)priv_data;
    if (net_io)
    {
        nmp_net_io_unref(net_io);
    }	
}


static __inline__ nmp_bool_t
__j_listen_watch_dispatch(nmp_watch_t *watch, int revents, void *user_data)
{
	nmp_conn_t *conn;
	nmp_watch_t *child_watch;
	nmp_watch_funcs *funcs;
	int err;
	socklen_t len = sizeof(err);

	funcs = watch->funcs;
	BUG_ON(!funcs);

    if (revents & EV_ERROR)
    {
        getsockopt(nmp_event_fd(watch), SOL_SOCKET, SO_ERROR, &err, &len);
        err = -err;
        goto listen_io_error;
    }

	if (revents & EV_READ)
	{
        conn = nmp_conn_accept(watch->conn, &err);
        if (!conn)
        {
        	if (E_AGAIN == err)
				return TRUE;

            goto listen_io_error;
        }

        if (funcs->create)
        {
            child_watch = (*funcs->create)(watch, conn);
            if (child_watch)
            {
                nmp_mutex_unlock(watch->lock);    /* drop the lock */
                nmp_watch_add_child(watch, child_watch);
                nmp_mutex_lock(watch->lock);
            }
        }
        else
        {
            nmp_conn_close(conn);
        }
	}

    return TRUE;

listen_io_error:
    __nmp_watch_error(watch, 0, err);
    return FALSE;
}


static nmp_bool_t
j_listen_watch_dispatch(nmp_event_t *ev, int revents, void *user_data)
{
	nmp_watch_t *watch;
	nmp_bool_t ret;
	NMP_ASSERT(ev != NULL);

	watch = (nmp_watch_t*)ev;

	nmp_mutex_lock(watch->lock);
	ret = __j_listen_watch_dispatch(watch, revents, user_data);
	nmp_mutex_unlock(watch->lock);

	return ret;
}


static __inline__ nmp_bool_t
__nmp_watch_rw_dispatch(nmp_watch_t *watch, int revents, void *user_data)
{
    nmp_watch_funcs *funcs;
    int err = 0;
    socklen_t len = sizeof(err);
    int buf[PAGE_SIZE/sizeof(int)]; // 4 ×Ö½Ú¶ÔÆë

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (revents & EV_ERROR)
    {
        getsockopt(nmp_event_fd(watch), SOL_SOCKET, SO_ERROR, &err, &len);
        err = -err;
        goto read_error;
    }

    if (revents & EV_READ)
    {
        for (;;)
        {
            if (watch->killed)
            { /* end the loop if killed */
                return FALSE;
            }

            err = nmp_conn_read(watch->conn, (char*)buf, PAGE_SIZE);
            if (err == 0)
            {
                goto conn_reset;
            }
            else if (err > 0)
            {
                if (funcs->recv)
                {/* funcs->recv() may drop lock, for msg delivering */
                    if ((err = (*funcs->recv)(watch, (char*)buf, err))) // watch->recv
                    {
                        goto read_error;
                    }
                }
                else
                {
                	nmp_event_remove_events((nmp_event_t*)watch, EV_READ);     /* no reader */
                    break;  /* data will stay in the sock buffer */
                }
            }
            else
            {
                if (err == -EAGAIN)
                    break;

                goto read_error;
            }
        }
    }

    if (revents & EV_WRITE)
    {
		if (NMP_UNLIKELY(nmp_conn_is_ingrogress(watch->conn, 1)))
		{
			if (getsockopt(nmp_event_fd(watch), SOL_SOCKET, SO_ERROR, &err, &len))
			{
				nmp_warning("getsockopt() after connect() failed\n");
				err = -errno;
				goto write_error;
			}

			if (err)
			{
				err = -err;
				goto write_error;
			}

			nmp_event_remove_events((nmp_event_t*)watch, EV_WRITE); 
			nmp_event_add_events((nmp_event_t*)watch, EV_READ);

			nmp_mutex_unlock(watch->lock);	/* drop the lock*/
			nmp_watch_on_establish(watch);
			nmp_mutex_lock(watch->lock);
		}
		else
		{
	        err = nmp_net_buf_flush(watch->buffer, watch);
	        if (!err)   /* all data flushed */
	        {
				nmp_event_remove_events((nmp_event_t*)watch, EV_WRITE);
	            watch->w_pending = 0;
	        }
	        else
	        {
	            if (err < 0)
	                goto write_error;
	        }
	    }
    }

    return TRUE;

conn_reset:
    __nmp_watch_close(watch, 1);
    return FALSE;

read_error:
    __nmp_watch_error(watch, 0, err);
    return FALSE;

write_error:
    __nmp_watch_error(watch, 1, err);
    return FALSE;
}


static nmp_bool_t
nmp_watch_rw_dispatch(nmp_event_t *ev, int revents, void *user_data)
{
	nmp_bool_t ret;
	nmp_watch_t *watch;
	NMP_ASSERT(ev != NULL);

	watch = (nmp_watch_t *)ev;

	nmp_mutex_lock(watch->lock);
	ret = __nmp_watch_rw_dispatch(watch, revents, user_data);
	nmp_mutex_unlock(watch->lock);

	return ret;
}


static void
nmp_watch_finalize(nmp_event_t *ev)
{
	nmp_watch_t *watch;
	NMP_ASSERT(ev != NULL);

	watch = (nmp_watch_t*)ev;

/*    jpf_debug(
        "Net watch '%p' finalized. total %d left.",
        source, g_atomic_int_get(&total_watch_count)
    );
*/
    nmp_watch_destroy_private(watch->priv_data);
    watch->priv_data = NULL;

    if (watch->conn)    /* we need to flush the rest of data!!! */
    {
        nmp_conn_close(watch->conn);
    }

    if (watch->buffer)
    {
        nmp_net_buf_free(watch->buffer);
    }

    nmp_mutex_free(watch->lock);
	//TODO;
}


nmp_watch_t *
nmp_watch_create(nmp_conn_t *conn, nmp_watch_funcs *funcs, int size)
{
	nmp_watch_t *watch;
	int events;
	NMP_ASSERT(conn != NULL && funcs != NULL && 
		size >= sizeof(nmp_watch_t));

	events = nmp_conn_is_ingrogress(conn, 0) ? EV_WRITE : EV_READ;

	watch = (nmp_watch_t*)nmp_event_new(size,
		nmp_conn_get_fd(conn), events);
	if (NMP_UNLIKELY(!watch))
		return NULL;

	watch->buffer = nmp_net_buf_alloc(BUFFER_BLOCKS,
		nmp_watch_write_out);
	if (NMP_UNLIKELY(!watch->buffer))
	{
		nmp_warning("nmp_watch_create()->nmp_net_buf_alloc() failed.\n");
		nmp_event_unref((nmp_event_t*)watch);
		return NULL;
	}

	watch->lock = nmp_mutex_new();
	watch->conn = conn;
	watch->funcs = funcs;
	/* watch->next_timeout */
	watch->w_pending = 0;
	watch->killed = 0;
	watch->priv_data = NULL;

	nmp_event_set_callback((nmp_event_t*)watch, nmp_watch_rw_dispatch,
		NULL, nmp_watch_finalize);

	return watch;
}


nmp_watch_t *
j_listen_watch_create(nmp_conn_t *conn,
    nmp_watch_funcs *funcs, int size)
{
	nmp_watch_t *watch;
	NMP_ASSERT(conn != NULL && funcs != NULL &&
		size >= sizeof(nmp_watch_t));

	watch = (nmp_watch_t*)nmp_event_new(size,
		nmp_conn_get_fd(conn), EV_READ);
	if (NMP_UNLIKELY(!watch))
		return NULL;

	watch->buffer = NULL;

	watch->lock = nmp_mutex_new();
	watch->conn = conn;
	watch->funcs = funcs;
	/* watch->next_timeout */
	watch->w_pending = 0;
	watch->killed = 0;
	watch->priv_data = NULL;

	nmp_event_set_callback((nmp_event_t*)watch, j_listen_watch_dispatch,
		NULL, nmp_watch_finalize);

	return watch;
}


void nmp_watch_attach(nmp_watch_t *watch, nmp_event_loop_t *loop)
{
	NMP_ASSERT(watch != NULL && loop != NULL);

	nmp_event_loop_attach(loop, (nmp_event_t*)watch);
}


static __inline__ int
nmp_watch_deliver_message(void *to, void *msg)
{
    nmp_netio_t *net_io = (nmp_netio_t*)to;

    return nmp_net_io_read_message(net_io, msg);
}


int nmp_watch_recv_message(nmp_watch_t *watch, void *msg)
{
    int rc;
    NMP_ASSERT(watch != NULL && msg != NULL);

    if (!watch->priv_data)
        return -E_WATCHDIE;

    nmp_mutex_unlock(watch->lock);    /* drop the lock, watch->priv_data is destroyed in watch finalize() */
    rc = nmp_watch_deliver_message(watch->priv_data, msg);
    nmp_mutex_lock(watch->lock);

    return rc;
}


static __inline__ int
__nmp_watch_write_message(nmp_watch_t *watch, void *msg)
{
    int packet[MAX_IO_BUFFER_SIZE/sizeof(int)];
    int ret = 0, pending = 0;
    nmp_watch_funcs *funcs;

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (funcs->format)      /* if !funcs->format, return 0 */
    {
        ret = (*funcs->format)(watch, msg, (char*)packet,
            MAX_IO_BUFFER_SIZE);
        if (ret > 0)
        {
            ret = nmp_net_buf_write(watch->buffer, (char*)packet, ret, 
                watch, &pending);
            if (!ret)
            {
				nmp_print(
					"Net watch '%p' send buffer full, drop 1 packet.\n",
					watch
				);

#if 0			/* Just report, without killing action */
            	nmp_event_remove((nmp_event_t*)watch);
            	__nmp_watch_error(watch, 1, -E_SNDBUFFULL);
#endif
            }
            else if (ret < 0)
            {
                nmp_print(
                    "Net watch '%p' send failed.\n", watch
                );

                nmp_event_remove((nmp_event_t*)watch);
                __nmp_watch_error(watch, 1, ret);
            }
            else
            {
                if (pending && !watch->w_pending)
                {
                    nmp_event_add_events((nmp_event_t*)watch, EV_WRITE);
                    watch->w_pending = 1;
                }
            }
        }
        else
        {
            nmp_print(
                "Net format packet failed while sending, drop.\n"
            );
        }
    }

    return ret;
}


/**
 * Write a internal msg to network, before that, the msg
 * will be formatted to correct payload and packet data.
 * return the size written.
*/
int nmp_watch_write_message(nmp_watch_t *watch, void *msg)
{
    int ret = -E_WATCHDIE;
    NMP_ASSERT(watch != NULL);

    nmp_mutex_lock(watch->lock);

    /**
     *  if writes failed, __nmp_watch_error() has been invoked,
     *  and the watch may be destroyed (because __finalize() can
     *  be called by this execute flow, just here. so, we must
     *  increase the watch ref-count before writing.
    */

    if (!watch->killed)
    {
        ret = __nmp_watch_write_message(watch, msg);
    }
    nmp_mutex_unlock(watch->lock);

    return ret;
}


void nmp_watch_kill(nmp_watch_t *watch)
{
	NMP_ASSERT(watch != NULL);

    /* remove it from loop-conext */
    nmp_event_remove((nmp_event_t*)watch);

    nmp_mutex_lock(watch->lock);
    __nmp_watch_close(watch, 0);
    nmp_mutex_unlock(watch->lock);	
}


void nmp_watch_ref(nmp_watch_t *watch)
{
	NMP_ASSERT(watch != NULL);

	nmp_event_ref((nmp_event_t*)watch);
}


void nmp_watch_unref(nmp_watch_t *watch)
{
	NMP_ASSERT(watch != NULL);

	nmp_event_unref((nmp_event_t*)watch);
}


void nmp_watch_set_private(nmp_watch_t *watch, void *priv_data)
{
	NMP_ASSERT(watch != NULL);

	nmp_mutex_lock(watch->lock);
	BUG_ON(watch->priv_data);
    watch->priv_data = priv_data;
	nmp_mutex_unlock(watch->lock);
}


nmp_bool_t nmp_watch_set_conn_ttd(nmp_watch_t *watch, int milli_sec)
{
	return TRUE;
}


static __inline__ char *
__nmp_watch_get_host(nmp_watch_t *watch, char *ip)
{
	if (watch->conn)
	{
		return nmp_conn_get_host(watch->conn, ip);
	}

	return NULL;
}


__export char *
nmp_watch_get_host(nmp_watch_t *watch, char *ip)
{
	char *addr = NULL;
	nmp_mutex_lock(watch->lock);
	if (!watch->killed)
	{
		addr = __nmp_watch_get_host(watch, ip);
	}
	nmp_mutex_unlock(watch->lock);

	return addr;
}


static __inline__ char *
__nmp_watch_get_peer(nmp_watch_t *watch, char *ip)
{
	if (watch->conn)
	{
		return nmp_conn_get_peer(watch->conn, ip);
	}

	return NULL;
}


__export char *
nmp_watch_get_peer(nmp_watch_t *watch, char *ip)
{
	char *addr = NULL;
	nmp_mutex_lock(watch->lock);
	if (!watch->killed)
	{
		addr = __nmp_watch_get_peer(watch, ip);
	}
	nmp_mutex_unlock(watch->lock);

	return addr;
}


//:~ End
