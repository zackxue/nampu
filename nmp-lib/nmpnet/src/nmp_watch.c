/*
 * nmp_watch.c
 *
 * This file implements net watch.
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/


#include "nmp_watch.h"
#include "nmp_netio.h"
#include "nmp_errno.h"

#define BUFFER_BLOCKS		8

static int
j_watch_write_out(char *buf, size_t size, void *w)
{
    JWatch *watch = (JWatch*)w;

    return j_connection_write(watch->conn, buf, size);
}


static __inline__ void
j_watch_add_child(JWatch *watch, JWatch *child)
{
    JNetIO *net_io;

    net_io = (JNetIO*)watch->priv_data;
    if (net_io)
    {
        j_net_io_add_child_watch(net_io, child);
    }
    else
    {
    	BUG();
	}
}


static __inline__ void
j_watch_on_establish(JWatch *watch)
{
    JNetIO *net_io;

    net_io = (JNetIO*)watch->priv_data;
    if (net_io)
    {
        j_net_io_establish(net_io);
    }
    else
    {
    	BUG();
	}
}


static __inline__ void
j_watch_on_clear(JWatch *watch, int err)
{
    JNetIO *net_io;

    net_io = (JNetIO*)watch->priv_data;
    if (net_io)
    {
        j_mutex_unlock(watch->lock);    /* drop the lock */
        j_net_io_async_kill(net_io, err);
        j_mutex_lock(watch->lock);
    }
}


static __inline__ void
__j_watch_close(JWatch *watch, int async)
{
    JWatchFuncs *funcs;

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
        j_watch_on_clear(watch, 0);
    }
}


static __inline__ void
__j_watch_error(JWatch *watch, int rw, int err)
{
    JWatchFuncs *funcs;

    if (watch->killed++)
        return;

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (funcs->error)
    {
        (*funcs->error)(watch, rw, err);
    }

    j_watch_on_clear(watch, err);
}


static __inline__ void
j_watch_destroy_private(void *priv_data)
{
    JNetIO *net_io;

    net_io = (JNetIO*)priv_data;
    if (net_io)
    {
        j_net_io_unref(net_io);
    }	
}


static __inline__ JBool
__j_listen_watch_dispatch(JWatch *watch, int revents, void *user_data)
{
	JConnection *conn;
	JWatch *child_watch;
	JWatchFuncs *funcs;
	int err;
	socklen_t len = sizeof(err);

	funcs = watch->funcs;
	BUG_ON(!funcs);

    if (revents & EV_ERROR)
    {
        getsockopt(j_event_fd(watch), SOL_SOCKET, SO_ERROR, &err, &len);
        err = -err;
        goto listen_io_error;
    }

	if (revents & EV_READ)
	{
        conn = j_connection_accept(watch->conn, &err);
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
                j_mutex_unlock(watch->lock);    /* drop the lock */
                j_watch_add_child(watch, child_watch);
                j_mutex_lock(watch->lock);
            }
        }
        else
        {
            j_connection_close(conn);
        }
	}

    return TRUE;

listen_io_error:
    __j_watch_error(watch, 0, err);
    return FALSE;
}


static JBool
j_listen_watch_dispatch(JEvent *ev, int revents, void *user_data)
{
	JWatch *watch;
	JBool ret;
	J_ASSERT(ev != NULL);

	watch = (JWatch*)ev;

	j_mutex_lock(watch->lock);
	ret = __j_listen_watch_dispatch(watch, revents, user_data);
	j_mutex_unlock(watch->lock);

	return ret;
}


static __inline__ JBool
__j_watch_rw_dispatch(JWatch *watch, int revents, void *user_data)
{
    JWatchFuncs *funcs;
    int err = 0;
    socklen_t len = sizeof(err);
    int buf[PAGE_SIZE/sizeof(int)]; // 4 ×Ö½Ú¶ÔÆë

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (revents & EV_ERROR)
    {
        getsockopt(j_event_fd(watch), SOL_SOCKET, SO_ERROR, &err, &len);
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

            err = j_connection_read(watch->conn, (char*)buf, PAGE_SIZE);
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
                	j_event_remove_events((JEvent*)watch, EV_READ);     /* no reader */
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
		if (J_UNLIKELY(j_connection_is_ingrogress(watch->conn, 1)))
		{
			if (getsockopt(j_event_fd(watch), SOL_SOCKET, SO_ERROR, &err, &len))
			{
				j_warning("getsockopt() after connect() failed\n");
				err = -errno;
				goto write_error;
			}

			if (err)
			{
				err = -err;
				goto write_error;
			}

			j_event_remove_events((JEvent*)watch, EV_WRITE); 
			j_event_add_events((JEvent*)watch, EV_READ);

			j_mutex_unlock(watch->lock);	/* drop the lock*/
			j_watch_on_establish(watch);
			j_mutex_lock(watch->lock);
		}
		else
		{
	        err = j_net_buf_flush(watch->buffer, watch);
	        if (!err)   /* all data flushed */
	        {
				j_event_remove_events((JEvent*)watch, EV_WRITE);
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
    __j_watch_close(watch, 1);
    return FALSE;

read_error:
    __j_watch_error(watch, 0, err);
    return FALSE;

write_error:
    __j_watch_error(watch, 1, err);
    return FALSE;
}


static JBool
j_watch_rw_dispatch(JEvent *ev, int revents, void *user_data)
{
	JBool ret;
	JWatch *watch;
	J_ASSERT(ev != NULL);

	watch = (JWatch *)ev;

	j_mutex_lock(watch->lock);
	ret = __j_watch_rw_dispatch(watch, revents, user_data);
	j_mutex_unlock(watch->lock);

	return ret;
}


static void
j_watch_finalize(JEvent *ev)
{
	JWatch *watch;
	J_ASSERT(ev != NULL);

	watch = (JWatch*)ev;

/*    jpf_debug(
        "Net watch '%p' finalized. total %d left.",
        source, g_atomic_int_get(&total_watch_count)
    );
*/
    j_watch_destroy_private(watch->priv_data);
    watch->priv_data = NULL;

    if (watch->conn)    /* we need to flush the rest of data!!! */
    {
        j_connection_close(watch->conn);
    }

    if (watch->buffer)
    {
        j_net_buf_free(watch->buffer);
    }

    j_mutex_free(watch->lock);
	//TODO;
}


JWatch *
j_watch_create(JConnection *conn, JWatchFuncs *funcs, int size)
{
	JWatch *watch;
	int events;
	J_ASSERT(conn != NULL && funcs != NULL && 
		size >= sizeof(JWatch));

	events = j_connection_is_ingrogress(conn, 0) ? EV_WRITE : EV_READ;

	watch = (JWatch*)j_event_new(size,
		j_connection_get_fd(conn), events);
	if (J_UNLIKELY(!watch))
		return NULL;

	watch->buffer = j_net_buf_alloc(BUFFER_BLOCKS,
		j_watch_write_out);
	if (J_UNLIKELY(!watch->buffer))
	{
		j_warning("j_watch_create()->j_net_buf_alloc() failed.\n");
		j_event_unref((JEvent*)watch);
		return NULL;
	}

	watch->lock = j_mutex_new();
	watch->conn = conn;
	watch->funcs = funcs;
	/* watch->next_timeout */
	watch->w_pending = 0;
	watch->killed = 0;
	watch->priv_data = NULL;

	j_event_set_callback((JEvent*)watch, j_watch_rw_dispatch,
		NULL, j_watch_finalize);

	return watch;
}


JWatch *
j_listen_watch_create(JConnection *conn,
    JWatchFuncs *funcs, int size)
{
	JWatch *watch;
	J_ASSERT(conn != NULL && funcs != NULL &&
		size >= sizeof(JWatch));

	watch = (JWatch*)j_event_new(size,
		j_connection_get_fd(conn), EV_READ);
	if (J_UNLIKELY(!watch))
		return NULL;

	watch->buffer = NULL;

	watch->lock = j_mutex_new();
	watch->conn = conn;
	watch->funcs = funcs;
	/* watch->next_timeout */
	watch->w_pending = 0;
	watch->killed = 0;
	watch->priv_data = NULL;

	j_event_set_callback((JEvent*)watch, j_listen_watch_dispatch,
		NULL, j_watch_finalize);

	return watch;
}


void j_watch_attach(JWatch *watch, JEventLoop *loop)
{
	J_ASSERT(watch != NULL && loop != NULL);

	j_event_loop_attach(loop, (JEvent*)watch);
}


static __inline__ int
j_watch_deliver_message(void *to, void *msg)
{
    JNetIO *net_io = (JNetIO*)to;

    return j_net_io_read_message(net_io, msg);
}


int j_watch_recv_message(JWatch *watch, void *msg)
{
    int rc;
    J_ASSERT(watch != NULL && msg != NULL);

    if (!watch->priv_data)
        return -E_WATCHDIE;

    j_mutex_unlock(watch->lock);    /* drop the lock, watch->priv_data is destroyed in watch finalize() */
    rc = j_watch_deliver_message(watch->priv_data, msg);
    j_mutex_lock(watch->lock);

    return rc;
}


static __inline__ int
__j_watch_write_message(JWatch *watch, void *msg)
{
    int packet[MAX_IO_BUFFER_SIZE/sizeof(int)];
    int ret = 0, pending = 0;
    JWatchFuncs *funcs;

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (funcs->format)      /* if !funcs->format, return 0 */
    {
        ret = (*funcs->format)(watch, msg, (char*)packet,
            MAX_IO_BUFFER_SIZE);
        if (ret > 0)
        {
            ret = j_net_buf_write(watch->buffer, (char*)packet, ret, 
                watch, &pending);
            if (!ret)
            {
				j_print(
					"Net watch '%p' send buffer full, drop 1 packet.\n",
					watch
				);

#if 0			/* Just report, without killing action */
            	j_event_remove((JEvent*)watch);
            	__j_watch_error(watch, 1, -E_SNDBUFFULL);
#endif
            }
            else if (ret < 0)
            {
                j_print(
                    "Net watch '%p' send failed.\n", watch
                );

                j_event_remove((JEvent*)watch);
                __j_watch_error(watch, 1, ret);
            }
            else
            {
                if (pending && !watch->w_pending)
                {
                    j_event_add_events((JEvent*)watch, EV_WRITE);
                    watch->w_pending = 1;
                }
            }
        }
        else
        {
            j_print(
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
int j_watch_write_message(JWatch *watch, void *msg)
{
    int ret = -E_WATCHDIE;
    J_ASSERT(watch != NULL);

    j_mutex_lock(watch->lock);

    /**
     *  if writes failed, __j_watch_error() has been invoked,
     *  and the watch may be destroyed (because __finalize() can
     *  be called by this execute flow, just here. so, we must
     *  increase the watch ref-count before writing.
    */

    if (!watch->killed)
    {
        ret = __j_watch_write_message(watch, msg);
    }
    j_mutex_unlock(watch->lock);

    return ret;
}


void j_watch_kill(JWatch *watch)
{
	J_ASSERT(watch != NULL);

    /* remove it from loop-conext */
    j_event_remove((JEvent*)watch);

    j_mutex_lock(watch->lock);
    __j_watch_close(watch, 0);
    j_mutex_unlock(watch->lock);	
}


void j_watch_ref(JWatch *watch)
{
	J_ASSERT(watch != NULL);

	j_event_ref((JEvent*)watch);
}


void j_watch_unref(JWatch *watch)
{
	J_ASSERT(watch != NULL);

	j_event_unref((JEvent*)watch);
}


void j_watch_set_private(JWatch *watch, void *priv_data)
{
	J_ASSERT(watch != NULL);

	j_mutex_lock(watch->lock);
	BUG_ON(watch->priv_data);
    watch->priv_data = priv_data;
	j_mutex_unlock(watch->lock);
}


JBool j_watch_set_conn_ttd(JWatch *watch, int milli_sec)
{
	return TRUE;
}


static __inline__ char *
__j_watch_get_host(JWatch *watch, char *ip)
{
	if (watch->conn)
	{
		return j_connection_get_host(watch->conn, ip);
	}

	return NULL;
}


__export char *
j_watch_get_host(JWatch *watch, char *ip)
{
	char *addr = NULL;
	j_mutex_lock(watch->lock);
	if (!watch->killed)
	{
		addr = __j_watch_get_host(watch, ip);
	}
	j_mutex_unlock(watch->lock);

	return addr;
}


static __inline__ char *
__j_watch_get_peer(JWatch *watch, char *ip)
{
	if (watch->conn)
	{
		return j_connection_get_peer(watch->conn, ip);
	}

	return NULL;
}


__export char *
j_watch_get_peer(JWatch *watch, char *ip)
{
	char *addr = NULL;
	j_mutex_lock(watch->lock);
	if (!watch->killed)
	{
		addr = __j_watch_get_peer(watch, ip);
	}
	j_mutex_unlock(watch->lock);

	return addr;
}


//:~ End
