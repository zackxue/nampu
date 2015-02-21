/*
 * nmp_wait.h
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __HM_WAIT_H__
#define __HM_WAIT_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _JpfWait JpfWait;

struct _JpfWait
{
    gboolean        waited;		/* condition */
    GMutex          *lock;
    GCond           *cond;
};


static __inline__ JpfWait *
jpf_wait_new( void )
{//TODO: ERR
    JpfWait *w = g_new0(JpfWait, 1);
    if (G_UNLIKELY(!w))		/* glib, unlikely */
        return NULL;

    w->waited = FALSE;
    w->lock = g_mutex_new();
    w->cond = g_cond_new();

    return w;
}


static __inline__ void
jpf_wait_free(JpfWait *w)
{
    if (G_UNLIKELY(!w))
        return;
    
    g_cond_free(w->cond);
    g_mutex_free(w->lock);
    g_free(w);
}


static __inline__ void
jpf_wait_begin(JpfWait *w)
{
    g_mutex_lock(w->lock);
}


static __inline__ void
jpf_wait_waiting(JpfWait *w)
{
    g_cond_wait(w->cond, w->lock);  
}


static __inline__ void
jpf_wait_timed_waiting(JpfWait *w, GTimeVal *abs_time)
{
    g_cond_timed_wait(w->cond, w->lock, abs_time);  
}


static __inline__ void
jpf_wait_end(JpfWait *w)
{
    g_mutex_unlock(w->lock);
}


static __inline__ void
jpf_wait_set_cond_true(JpfWait *w)
{
    w->waited = TRUE;
}


static __inline__ void
jpf_wait_set_cond_false(JpfWait *w)
{
    w->waited = FALSE;
}


static __inline__ gboolean
jpf_wait_is_cond_true(JpfWait *w)
{
    return w->waited;
}


static __inline__ void
jpf_wait_wakeup(JpfWait *w)
{
    g_cond_signal(w->cond);
}


static __inline__ void
jpf_wait_wakeup_all(JpfWait *w)
{
    g_cond_broadcast(w->cond);
}


G_END_DECLS

#endif  //__HM_WAIT_H__
