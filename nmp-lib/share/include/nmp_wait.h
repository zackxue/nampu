/*
 * hm_wait.h
 *
 * Copyright(c) by HiMickey, 2010~2014
 * Author:
*/

#ifndef __HM_WAIT_H__
#define __HM_WAIT_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _HmWait HmWait;

struct _HmWait
{
    gboolean        waited;		/* condition */
    GMutex          *lock;
    GCond           *cond;
};


static __inline__ HmWait *
hm_wait_new( void )
{//TODO: ERR
    HmWait *w = g_new0(HmWait, 1);
    if (G_UNLIKELY(!w))		/* glib, unlikely */
        return NULL;

    w->waited = FALSE;
    w->lock = g_mutex_new();
    w->cond = g_cond_new();

    return w;
}


static __inline__ void
hm_wait_free(HmWait *w)
{
    if (G_UNLIKELY(!w))
        return;
    
    g_cond_free(w->cond);
    g_mutex_free(w->lock);
    g_free(w);
}


static __inline__ void
hm_wait_begin(HmWait *w)
{
    g_mutex_lock(w->lock);
}


static __inline__ void
hm_wait_waiting(HmWait *w)
{
    g_cond_wait(w->cond, w->lock);  
}


static __inline__ void
hm_wait_timed_waiting(HmWait *w, GTimeVal *abs_time)
{
    g_cond_timed_wait(w->cond, w->lock, abs_time);  
}


static __inline__ void
hm_wait_end(HmWait *w)
{
    g_mutex_unlock(w->lock);
}


static __inline__ void
hm_wait_set_cond_true(HmWait *w)
{
    w->waited = TRUE;
}


static __inline__ void
hm_wait_set_cond_false(HmWait *w)
{
    w->waited = FALSE;
}


static __inline__ gboolean
hm_wait_is_cond_true(HmWait *w)
{
    return w->waited;
}


static __inline__ void
hm_wait_wakeup(HmWait *w)
{
    g_cond_signal(w->cond);
}


static __inline__ void
hm_wait_wakeup_all(HmWait *w)
{
    g_cond_broadcast(w->cond);
}


G_END_DECLS

#endif  //__HM_WAIT_H__
