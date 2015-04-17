
#include "nmplib.h"
#include "nmp_proxy_cond.h" 


struct proxy_cond
{
    nmp_mutex_t *lock;
    nmp_cond_t  *cond;
};


int proxy_cond_new(proxy_cond_t **cond)
{
    *cond = (proxy_cond_t*)nmp_new0(proxy_cond_t, 1);

    (*cond)->lock  = nmp_mutex_new();
    (*cond)->cond  = nmp_cond_new();

    return 0;
}

int proxy_cond_wait(proxy_cond_t *cond)
{
    NMP_ASSERT(cond);

    nmp_mutex_lock(cond->lock);
    nmp_cond_wait(cond->cond, cond->lock);
    nmp_mutex_unlock(cond->lock);

    return 0;
}

int proxy_cond_timed_wait(proxy_cond_t *cond, int sec)
{
    nmp_timeval_t abs_time;

    NMP_ASSERT(cond && 0 < sec);

    abs_time.tv_sec  = time(NULL) + sec;
    abs_time.tv_usec = 0;

    nmp_mutex_lock(cond->lock);
    nmp_cond_timed_wait(cond->cond, cond->lock, &abs_time);
    nmp_mutex_unlock(cond->lock);

    return 0;
}

int proxy_cond_signal(proxy_cond_t *cond)
{
    NMP_ASSERT(cond);
    nmp_cond_signal(cond->cond);
    return 0;
}

int proxy_cond_free(proxy_cond_t *cond)
{
    NMP_ASSERT(cond);

    if (cond->lock)
        nmp_mutex_free(cond->lock);

    if (cond->cond)
        nmp_cond_free(cond->cond);

    nmp_del(cond, proxy_cond_t, 1);
    return 0;
}


