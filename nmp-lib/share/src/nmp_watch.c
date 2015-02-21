/*
 * nmp_watch.c
 *
 * This file implements net watch.
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/


#include <unistd.h>
#include <fcntl.h>
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_watch.h"
#include "nmp_netio.h"

/* clock drifted by time setting */
#define DRIFTED_TIME    5
#define MONOTIME_ERR    0x7fffffff

#define READ_ERR    (G_IO_HUP|G_IO_ERR|G_IO_NVAL)
#define READ_COND   (G_IO_IN|READ_ERR)
#define WRITE_ERR   (G_IO_HUP|G_IO_ERR|G_IO_NVAL)
#define WRITE_COND  (G_IO_OUT|WRITE_ERR)

#define SMALL_BUFFER_BLOCKS     1
#define LARGE_BUFFER_BLOCKS     3

typedef gboolean (*HmWatchCallback)(HmWatch *watch, 
    gpointer user_data);
static void hm_watch_kill_unref(HmWatch *watch);
static gint total_watch_count = 0;


static __inline__ void
hm_watch_add_child(HmWatch *watch, HmWatch *child)
{
    HmNetIO *net_io;

    net_io = (HmNetIO*)watch->priv_data;
    if (net_io)
    {
        hm_net_io_add_child_watch(net_io, child);
    }
    else
    {
        hm_watch_kill_unref(child);
    }
}


static __inline__ void
hm_watch_on_establish(HmWatch *watch)
{
    HmNetIO *net_io;

    net_io = (HmNetIO*)watch->priv_data;
    if (net_io)
    {
        hm_net_io_establish(net_io);
    }
    else
    {
        BUG();
    }
}


static __inline__ gint
hm_watch_deliver_message(gpointer to, gpointer msg)
{
    HmNetIO *net_io = (HmNetIO*)to;

    return hm_net_io_read_message(net_io, msg);
}


static gint
hm_watch_write_out(gchar *buf, gsize size, gpointer w)
{
    HmWatch *watch = (HmWatch*)w;

    return hm_connection_write(watch->conn, buf, size);
}


static __inline__ void
hm_watch_destroy_private(gpointer priv_data)
{
    HmNetIO *net_io;

    net_io = (HmNetIO*)priv_data;
    if (net_io)
    {
        hm_net_io_unref(net_io);
    }
}


/*
 * Let the private data know we are dying. invoked when
 * #watch->lock held.
*/
static __inline__ void
hm_watch_on_clear(HmWatch *watch, gint err)
{
    HmNetIO *net_io;

    net_io = (HmNetIO*)watch->priv_data;
    if (net_io)
    {
        g_mutex_unlock(watch->lock);    /* drop the lock */
        hm_net_io_async_kill(net_io, err);
        g_mutex_lock(watch->lock);
    }
}


static __inline__ void
__hm_watch_close(HmWatch *watch, gint async)
{
    HmWatchFuncs *funcs;

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
        hm_watch_on_clear(watch, 0);
    }
}


static __inline__ void
__hm_watch_error(HmWatch *watch, gint rw, gint err)
{
    HmWatchFuncs *funcs;

    if (watch->killed++)
        return;

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (funcs->error)
    {
        (*funcs->error)(watch, rw, err);
    }

    hm_watch_on_clear(watch, err);
}


static gboolean
hm_watch_rw_dispatch(HmWatch *watch, gpointer user_data)
{
    HmWatchFuncs *funcs;
    gint err = 0, timeout = 1;
    socklen_t len = sizeof(err);
    gchar buf[MAX_IO_BUFFER_SIZE];

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (watch->r_fd.revents & READ_COND)
    {
        timeout = 0;

        if (watch->r_fd.revents & READ_ERR)
        {
            getsockopt(watch->r_fd.fd, SOL_SOCKET, SO_ERROR, &err, &len);
            err = -err;
            goto read_error;
        }

        for (;;)
        {
            if (watch->killed)
                return FALSE;   /* end the loop if killed, funcs->recv() may drop lock. */

            err = hm_connection_read(watch->conn, buf, MAX_IO_BUFFER_SIZE);
            if (err == 0)
            {
                goto conn_reset;
            }
            else if (err > 0)
            {
                if (funcs->recv)
                {
                    if ((err = (*funcs->recv)(watch, buf, err)))
                    {
                        goto read_error;
                    }
                }
                else
                {
                    watch->r_fd.revents = 0;    /* no reader */
                    g_source_remove_poll((GSource*)watch, &watch->r_fd);
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

    if (watch->w_fd.revents & WRITE_COND)
    {
        timeout = 0;

        if (G_UNLIKELY(hm_connection_is_ingrogress(watch->conn, 1)))
        {
            if (getsockopt(watch->w_fd.fd, SOL_SOCKET, SO_ERROR, &err, &len))
            {
                hm_warning("getsockopt() after connect() failed");
                err = -errno;
                goto write_error;
            }

            if (err)
            {
                err = -err;
                goto write_error;
            }

            watch->w_fd.revents = 0;
            g_source_remove_poll((GSource*)watch, &watch->w_fd);
            g_source_add_poll((GSource*)watch, &watch->r_fd);

            g_mutex_unlock(watch->lock);    /* drop the lock*/
            hm_watch_on_establish(watch);
            g_mutex_lock(watch->lock);          
        }
        else
        {
            if (watch->w_fd.revents & WRITE_ERR)
            {
                len = sizeof(err);
                getsockopt(watch->w_fd.fd, SOL_SOCKET, SO_ERROR, &err, &len);
                err = -err;
                goto write_error;
            }
    
            err = hm_net_buf_flush(watch->buffer, watch);
            if (!err)   /* all data flushed */
            {
                watch->w_fd.revents = 0;
                g_source_remove_poll((GSource*)watch, &watch->w_fd);
                watch->w_pending = 0;
            }
            else
            {
                if (err < 0)
                    goto write_error;
            }
        }
    }

    if (timeout)
    {
        hm_print(
            "Net IO '%p' timeout.", NET_IO(watch)
        );
        __hm_watch_error(watch, 0, -E_CONNTIMEOUT);
        return FALSE;
    }

    return TRUE;

conn_reset:
    __hm_watch_close(watch, 1);
    return FALSE;

read_error:
    __hm_watch_error(watch, 0, err);
    return FALSE;

write_error:
    __hm_watch_error(watch, 1, err);
    return FALSE;       
}


static gboolean
hm_watch_listen_dispatch(HmWatch *watch, gpointer user_data)
{
    HmWatchFuncs *funcs;
    gint err;
    socklen_t len = sizeof(err);
    HmConnection *conn;
    HmWatch *child_watch;

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (watch->r_fd.revents & READ_COND)
    {
        if (watch->r_fd.revents & READ_ERR)
        {
            getsockopt(watch->r_fd.fd, SOL_SOCKET, SO_ERROR, &err, &len);
            err = -err;
            goto listen_io_error;
        }

        for (;;)
        {
            if (watch->killed)
                return FALSE;   /* end the loop if killed, lock dropped below. */

            conn = hm_connection_accept(watch->conn, &err);
            if (!conn)
            {
                if (err == -EAGAIN)
                    break;
                goto listen_io_error;
            }

            if (funcs->create)
            {
                if (watch->heavy_io_load)
                {
                    hm_connection_set_heavy(conn);
                }

                hm_connection_set_buffer_size(conn, watch->block_size);

                child_watch = (*funcs->create)(watch, conn);
                if (child_watch)
                {
                    g_mutex_unlock(watch->lock);    /* drop the lock */
                    hm_watch_add_child(watch, child_watch);
                    g_mutex_lock(watch->lock);
                }
            }
            else
            {
                hm_connection_close(conn);
            }
        }
    }

    return TRUE;

listen_io_error:
    __hm_watch_error(watch, 0, err);
    return FALSE;
}


static __inline__ glong
hm_watch_time_val_diff(const GTimeVal *compare, const GTimeVal *now)  
{  
    return (compare->tv_sec - now->tv_sec) * 1000 +     /* millisecond */
        (compare->tv_usec - now->tv_usec) / 1000;  
} 


static __inline__ void
hm_watch_update_time(HmWatch *watch, const GTimeVal *now)
{
    if (!now)
    {
        g_source_get_current_time((GSource*)watch,
            &watch->next_timeout);
    }
    else
    {
        memcpy(&watch->next_timeout, now, sizeof(*now));    
    }

    g_time_val_add(&watch->next_timeout,
        hm_connection_get_timeout(watch->conn) * 1000);
}


static __inline__ gint
hm_watch_clock_timeout(HmWatch *watch, const GTimeVal *now)
{
    glong ms;   /* millisecond */

    ms = hm_watch_time_val_diff(&watch->next_timeout, now);
    if (ms <= 0 || ms > hm_connection_get_timeout(watch->conn) + 1)
        return 1;
    return 0;
}


#ifdef USE_MONOTONIC_CLOCK

static __inline__ gint
hm_watch_get_monotonic_sec(HmWatch *watch)
{
    /* g_get_monotonic_time() isn't implemented yet */
    struct timespec ts;

    memset(&ts, 0, sizeof(ts));
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (gint)ts.tv_sec;
}


static __inline__ gint
hm_watch_time_drifted(HmWatch *watch, const GTimeVal *now)
{
    gint delta, diff;

    if (watch->delta_time == MONOTIME_ERR)
        return 0;

    delta = now->tv_sec - hm_watch_get_monotonic_sec(watch);
    diff = watch->delta_time - delta;
    if (diff >= -DRIFTED_TIME && diff <= DRIFTED_TIME)
        return 0;

    watch->delta_time = delta;
    return 1;
}

#endif


/*
 * Event source function - get timeout time of the source.
 *  -1 means never timeout.
*/
static gboolean
hm_watch_prepare(GSource *source, gint *timeout)
{
    GTimeVal now;
    glong diff;
    HmWatch *watch;

    watch = (HmWatch*)source;
    if (!watch->buffer)
    {
        *timeout = -1;
        return FALSE;
    }

    g_source_get_current_time(source, &now);
    diff = hm_watch_time_val_diff(&watch->next_timeout, &now);
    if (diff <= 0)
    {
        *timeout = 0;
        return TRUE;
    }
    else
    {
        if (diff <= hm_connection_get_timeout(watch->conn))
        {
            *timeout = diff;
            return FALSE;
        }
        *timeout = 0;
        return TRUE;
    }
}


/*
 * Event source function - check whether the condtion is
 *  satisfied.
*/
static gboolean
hm_watch_check(GSource *source)
{
    HmWatch *watch;
    GTimeVal now;
    G_ASSERT(source != NULL);

    watch = (HmWatch*)source;

    if (watch->r_fd.revents & READ_COND)
    {
        if (watch->buffer)
        {
            hm_watch_update_time(watch, NULL);
        }
        return TRUE;
    }

    if (watch->w_fd.revents & WRITE_COND)
        return TRUE;

    if (watch->buffer)
    {
        g_source_get_current_time(source, &now);
        if (hm_watch_clock_timeout(watch, &now))
        {
            hm_watch_update_time(watch, &now);
#ifdef USE_MONOTONIC_CLOCK
            if (hm_watch_time_drifted(watch, &now))
                return FALSE;
#endif
            return TRUE;
        }
    }

    return FALSE;
}


/*
 * Event source function - dispatch event, this is called
 * automatically by glib main loop when condition is satisfied. 
*/
gboolean
hm_watch_dispatch(GSource *source, GSourceFunc callback,
    gpointer user_data)
{
    HmWatchCallback dispath;
    HmWatch *watch;
    gboolean ret = FALSE;

    watch = (HmWatch*)source;
    dispath = (HmWatchCallback)callback;

    /* avoid race against hm_watch_kill() and _write()*/
    g_mutex_lock(watch->lock);
    if (!watch->killed)
    {
        ret = (*dispath)(watch, user_data);
    }
    g_mutex_unlock(watch->lock);

    return ret;
}


/*
 * Event source function - destructor of the source, called 
 *  before the source object destroyed. we decrease the reference
 *  count of ioc channel since the source object owes the count
 *  of it.
*/
static void
hm_watch_finalize(GSource *source)
{
    HmWatch *watch;
    G_ASSERT(source != NULL);

    g_atomic_int_add(&total_watch_count, -1);

    watch = (HmWatch*)source;

    hm_debug(
        "Net IO '%p' finalized. total %d left.",
        NET_IO(watch), g_atomic_int_get(&total_watch_count)
    );

    if (watch->funcs && watch->funcs->finalize)
    {
        (*watch->funcs->finalize)(watch);
    }

    hm_watch_destroy_private(watch->priv_data);
    watch->priv_data = NULL;

    if (watch->conn)    /* we need to flush the rest of data!!! */
    {
        hm_connection_close(watch->conn);
    }

    if (watch->buffer)
    {
        hm_net_buf_free(watch->buffer);
    }

    g_mutex_free(watch->lock);
}


static GSourceFuncs hm_watch_funcs =
{
    .prepare        = hm_watch_prepare,
    .check          = hm_watch_check,
    .dispatch       = hm_watch_dispatch,
    .finalize       = hm_watch_finalize
};


/*
 * Set GSource dispatch callback function.
*/
static __inline__ void
hm_watch_set_callback(HmWatch *watch, HmWatchCallback callback,
    gpointer user_data)
{
    G_ASSERT(watch != NULL && callback != NULL);

    g_source_set_callback((GSource*)watch,
        (GSourceFunc)callback, user_data, NULL);
}


/*
 * Monotonic time value caching havn't been implemented yet in this
 * glib version, so we need to do more work for these purposes below:
 * (1) Avoid invoking system call for getting monotonic time frequently.
 * (2) Action of timeout can't be affected by user time setting.
*/
static __inline__ void
hm_watch_init_time(HmWatch *watch)
{
    BUG_ON(!watch || !watch->conn);

    /* source may have not been attached to loop yet, so get it directly */
    g_get_current_time(&watch->next_timeout);

#ifdef USE_MONOTONIC_CLOCK
    gint mono_sec = hm_watch_get_monotonic_sec(watch);
    if (mono_sec)
    {
        watch->delta_time = watch->next_timeout.tv_sec - mono_sec;
    }
    else
    {
        watch->delta_time = MONOTIME_ERR;
    }
#endif

    g_time_val_add(&watch->next_timeout, 
        hm_connection_get_timeout(watch->conn) * 1000);
}


/*
 * Create a event source object for connection. The object 
 * will be added to "Event Context" for polling. 
*/ 
__export HmWatch *
hm_watch_create(HmConnection *conn, HmWatchFuncs *funcs, gint size)
{
    GSource *source;
    HmWatch *watch;
    G_ASSERT(conn != NULL && funcs != NULL && size >= sizeof(HmWatch));

    source = g_source_new(&hm_watch_funcs, size);
    watch = (HmWatch*)source;

    watch->buffer = hm_net_buf_alloc(
        hm_connection_is_heavy(conn) ? LARGE_BUFFER_BLOCKS : SMALL_BUFFER_BLOCKS,
        hm_connection_get_buffer_size(conn),
        hm_watch_write_out);
    if (!watch->buffer)
    {
        g_source_unref(source);
        hm_warning(
            "Net create watch, alloc buffer failed."
        );
        return NULL;
    }

    watch->lock = g_mutex_new();
    watch->conn = conn;
    watch->funcs = funcs;

    watch->r_fd.fd = hm_connection_get_fd(conn);
    watch->w_fd.fd = hm_connection_get_fd(conn);
    watch->r_fd.events = READ_COND;
    watch->w_fd.events = WRITE_COND;
    watch->r_fd.revents = 0;
    watch->w_fd.revents = 0;

    hm_watch_init_time(watch);

    watch->w_pending = 0;
    watch->killed = 0;
    watch->heavy_io_load = hm_connection_is_heavy(conn);
    watch->block_size = hm_connection_get_buffer_size(conn);

    hm_watch_set_callback(watch, hm_watch_rw_dispatch, NULL);

    if (hm_connection_is_ingrogress(conn, 0))
        g_source_add_poll(source, &watch->w_fd);
    else
        g_source_add_poll(source, &watch->r_fd);
    g_atomic_int_add(&total_watch_count, 1);

    return watch;
}


/*
 * Create a event source object for connection. The object 
 * will be added to "Event Context" for polling. 
*/ 
__export HmWatch *
hm_listen_watch_create(HmConnection *conn, HmWatchFuncs *funcs, gint size)
{
    GSource *source;
    HmWatch *watch;
    G_ASSERT(conn != NULL && funcs != NULL && size >= sizeof(HmWatch));

    source = g_source_new(&hm_watch_funcs, size);
    watch = (HmWatch*)source;

    watch->buffer = NULL;

    watch->lock = g_mutex_new();
    watch->conn = conn;
    watch->funcs = funcs;

    watch->r_fd.fd = hm_connection_get_fd(conn);
    watch->r_fd.events = READ_COND;
    watch->r_fd.revents = 0;

    hm_watch_init_time(watch);

    watch->w_pending = 0;
    watch->killed = 0;
    watch->heavy_io_load = hm_connection_is_heavy(conn);
    watch->block_size = hm_connection_get_buffer_size(conn);

    hm_watch_set_callback(watch, hm_watch_listen_dispatch, NULL);
    g_source_add_poll(source, &watch->r_fd);
    g_atomic_int_add(&total_watch_count, 1);

    return watch;
}


__export void
hm_watch_attach(HmWatch *watch, GMainContext *context)
{
    G_ASSERT(watch != NULL);

    g_source_attach((GSource*)watch, context);
}


__export void
hm_watch_ref(HmWatch *watch)
{
    G_ASSERT(watch != NULL);

    g_source_ref((GSource*)watch);
}


__export void
hm_watch_unref(HmWatch *watch)
{
    G_ASSERT(watch != NULL);

    g_source_unref((GSource*)watch);
}


/*
 * kill a watch. this leads to the 2 following thiings:
 *    1, remove the watch from loop-context.
 *    2, call destructor, destroy the object form derived
 *       to basic layer.
 * this function provides the 'sync' semantics, callback fun
 * registered by net-io user will not be called.
*/
__export void
hm_watch_kill(HmWatch *watch)
{
    G_ASSERT(watch != NULL);

    /* remove it from loop-conext */
    g_source_destroy((GSource*)watch);

    g_mutex_lock(watch->lock);
    __hm_watch_close(watch, 0);
    g_mutex_unlock(watch->lock);
}


static void
hm_watch_kill_unref(HmWatch *watch)
{
    G_ASSERT(watch != NULL);

    hm_watch_kill(watch);
    hm_watch_unref(watch);
}


__export void
hm_watch_set_private(HmWatch *watch, gpointer priv_data)
{
    G_ASSERT(watch != NULL);

    g_mutex_lock(watch->lock);
    BUG_ON(watch->priv_data);
    watch->priv_data = priv_data;
    g_mutex_unlock(watch->lock);
}


__export gboolean
hm_watch_set_conn_ttd(HmWatch *watch, gint milli_sec)
{
    gboolean set_ok = FALSE;

    g_mutex_lock(watch->lock);
    if (!watch->killed)
    {
        set_ok = TRUE;
        hm_connection_set_timeout(watch->conn, milli_sec);
        hm_watch_init_time(watch);
    }
    g_mutex_unlock(watch->lock);

    return set_ok;
}


/*
 * Recv a complete internal msg, deliver it to the owner
 * of watch. This function is call in GSource _dispatch()
 * indirectly, lock has already been held.
*/
__export gint
hm_watch_recv_message(HmWatch *watch, gpointer msg)
{
    gint rc;
    G_ASSERT(watch != NULL && msg != NULL);

    if (!watch->priv_data)
        return -E_WATCHDIE;

    g_mutex_unlock(watch->lock);    /* drop the lock */

    rc = hm_watch_deliver_message(watch->priv_data, msg);

    g_mutex_lock(watch->lock);

    return rc;
}


static __inline__ gint
__hm_watch_write_message(HmWatch *watch, gpointer msg)
{
    gchar packet[MAX_IO_BUFFER_SIZE];
    gint ret = 0, pending = 0;
    HmWatchFuncs *funcs;

    funcs = watch->funcs;
    BUG_ON(!funcs);

    if (funcs->format)      /* if !funcs->format, return 0 */
    {
        ret = (*funcs->format)(watch, msg, packet,
            MAX_IO_BUFFER_SIZE);
        if (ret > 0)
        {
            ret = hm_net_buf_write(watch->buffer, packet, ret, 
                watch, &pending);
            if (!ret)
            {
#ifdef __HANDLE_SENDING_FAIL_EVENT
              hm_print(
                  "Net watch '%p' snd buffer full, drop 1 packet.",
                  watch
              );

              g_source_destroy((GSource*)watch);
              __hm_watch_error(watch, 1, -E_SNDBUFFULL);
#endif
            }
            else if (ret < 0)
            {
                hm_print(
                    "Net IO '%p' send failed.", NET_IO(watch)
                );

                g_source_destroy((GSource*)watch);
                __hm_watch_error(watch, 1, ret);
            }
            else
            {
                if (pending && !watch->w_pending)
                {
                    watch->w_fd.revents = 0;
                    g_source_add_poll((GSource*)watch, &watch->w_fd);
                    watch->w_pending = 1;
                }
            }
        }
        else
        {
            hm_print(
                "Net format packet failed while sending, drop."
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
__export gint
hm_watch_write_message(HmWatch *watch, gpointer msg)
{
    gint ret = -E_WATCHDIE;
    G_ASSERT(watch != NULL);

    g_mutex_lock(watch->lock);

    /**
     *  if writes failed, __hm_watch_error() has been invoked,
     *  and the watch may be destroyed (because __finalize() can
     *  be called by this execute flow, just here. so, we must
     *  increase the watch ref-count before writing.
    */

    if (!watch->killed)
    {
        ret = __hm_watch_write_message(watch, msg);
    }
    g_mutex_unlock(watch->lock);

    return ret;
}


static __inline__ gchar *
__hm_watch_get_peer(HmWatch *watch)
{
    if (watch->conn)
    {
        return hm_connection_get_peer(watch->conn);
    }

    return NULL;
}


__export gchar *
hm_watch_get_peer(HmWatch *watch)
{
    gchar *ip = NULL;

    g_mutex_lock(watch->lock);
    if (!watch->killed)
    {
        ip = __hm_watch_get_peer(watch);
    }
    g_mutex_unlock(watch->lock);

    return ip;
}


__export void
hm_watch_set_heavy_load(HmWatch *watch)
{
    g_mutex_lock(watch->lock);
    if (!watch->killed)
    {
        watch->heavy_io_load = 1;
    }
    g_mutex_unlock(watch->lock);
}


__export void
hm_watch_set_block_size(HmWatch *watch, gint block_size)
{
	if (block_size < 0)
		return;

    g_mutex_lock(watch->lock);
    if (!watch->killed)
    {
        watch->block_size = block_size;
    }
    g_mutex_unlock(watch->lock);
}

//:~ End
