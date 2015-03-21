/*
 * nmp_wait.h
 *
 * Copyright(c) by Nampu, 2010~2014
 * Author:
*/

#ifndef __NMP_WAIT_H__
#define __NMP_WAIT_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _NmpWait NmpWait;

struct _NmpWait
{
    gboolean        waited;		/* condition */
    GMutex          *lock;
    GCond           *cond;
};


static __inline__ NmpWait *
nmp_wait_new( void )
{//TODO: ERR
    NmpWait *w = g_new0(NmpWait, 1);
    if (G_UNLIKELY(!w))		/* glib, unlikely */
        return NULL;

    w->waited = FALSE;
    w->lock = g_mutex_new();
    w->cond = g_cond_new();

    return w;
}


static __inline__ void
nmp_wait_free(NmpWait *w)
{
    if (G_UNLIKELY(!w))
        return;
    
    g_cond_free(w->cond);
    g_mutex_free(w->lock);
    g_free(w);
}


static __inline__ void
nmp_wait_begin(NmpWait *w)
{
    g_mutex_lock(w->lock);
}


static __inline__ void
nmp_wait_waiting(NmpWait *w)
{
    g_cond_wait(w->cond, w->lock);  
}


static __inline__ void
nmp_wait_timed_waiting(NmpWait *w, GTimeVal *abs_time)
{
    g_cond_timed_wait(w->cond, w->lock, abs_time);  
}


static __inline__ void
nmp_wait_end(NmpWait *w)
{
    g_mutex_unlock(w->lock);
}


static __inline__ void
nmp_wait_set_cond_true(NmpWait *w)
{
    w->waited = TRUE;
}


static __inline__ void
nmp_wait_set_cond_false(NmpWait *w)
{
    w->waited = FALSE;
}


static __inline__ gboolean
nmp_wait_is_cond_true(NmpWait *w)
{
    return w->waited;
}


static __inline__ void
nmp_wait_wakeup(NmpWait *w)
{
    g_cond_signal(w->cond);
}


static __inline__ void
nmp_wait_wakeup_all(NmpWait *w)
{
    g_cond_broadcast(w->cond);
}


G_END_DECLS

#endif  //__NMP_WAIT_H__
