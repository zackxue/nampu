
#include <stdio.h>
#include "macros.h"
#include "loop.h"
//#include "log.h"
#include "sched.h"

#ifndef NULL
#define NULL (void *)0
#endif

sched_t* __sched_new(int nloop)
{
    sched_t *sched = NULL;
    if (!sched)
    {
        sched = (sched_t *)calloc(1, sizeof(sched_t));
        if (!sched)
        {
            goto _error;        
        }

        sched->count = nloop;
        sched->total_weight = 0;
        sched->loops = (void **)calloc(1, sizeof(loop_t*)*nloop); /*  */
#ifdef WIN32
		InitializeCriticalSection(&sched->set_mutex);
#else
		pthread_mutex_init(&sched->set_mutex, NULL);
#endif

        while (--nloop >= 0)
        {
            sched->loops[nloop] = loop_new();
            printf("[%p]:%p %p\n", (sched->loops + nloop), *(sched->loops + nloop), sched->loops[nloop]);
            if (!sched->loops[nloop])
            {
                goto _error;
            }
        }
    }
    
    return sched;

_error:
    if (sched)
    {
        while (sched->loops[--nloop] && nloop >= 0)
        {
            loop_unref((loop_t *)sched->loops[nloop]);
        }
#ifdef WIN32
		DeleteCriticalSection(&sched->set_mutex);
#else
		pthread_mutex_destroy(&sched->set_mutex);
#endif
        free(sched);
    }

    return NULL;
}

sched_t* sched_new(int nloop)
{
    if (nloop <= 0)
    {
        nloop = MIN_LOOP_NUM;
    }
    
    if (nloop > MAX_LOOP_NUM)
    {
        nloop = MAX_LOOP_NUM;
    }

    return __sched_new(nloop);
}

void sched_del(sched_t *sched)
{
    int nloop = 0;
    
    if (sched)
    {
        nloop = sched->count;
        while (sched->loops[--nloop] && nloop >= 0)
        {
            loop_unref((loop_t *)sched->loops[nloop]);
        }
#ifdef WIN32
		DeleteCriticalSection(&sched->set_mutex);
#else
		pthread_mutex_destroy(&sched->set_mutex);
#endif
		free(sched->loops);  // release loops

        free(sched);
    }
}

//loop_t *get_best_loop(sched_t *sched)
void *get_best_loop(sched_t *sched)
{
    int w, best_w = sched->total_weight; // total_weight 
	int best_i = 0, idx = 0;

    printf("sched->count:%d\n", sched->count);
	for (; idx < sched->count; ++idx)
	{
		w = ((loop_t *)(sched->loops[idx]))->weight;
        printf("loop[%d](%p)->w[%d]\n", idx, sched->loops[idx], w);
		if (w < best_w)
		{
			best_w = w;
			best_i = idx;
		}
	}

	return sched->loops[best_i];
}

/*
 * get_best_loop2
 *
 * @Describe 获取一个负载fd_node_t最少的loop_t
 * @疑问     对loop->weight只是进行只读操作,没有必要加锁, 在loop->weight变化时加锁就行了
 *  
 * @分析     当线程A运行到(1)时，线程B已经获取了自己需要的loop,并创建好client，还未执行
 *           ++weight的操作，此时线程A获取的weight是和线程B同一个loop,但是该loop在线程B
 *			 获取以后，已经不是最优的loop了
 * 
 * @结论     
 */
void *get_best_loop2(sched_t *sched)
{//@{no use total_weight}
    int w, best_w; // total_weight 
	int best_i = 0, idx = 0;

//@{在此处加锁是没用的, 因为只要已经获取loop的线程B没有++weight,则线程A总是获取的是和线程
//B一样的loop}

#if 0
#ifdef WIN32
	EnterCriticalSection(&sched->set_mutex);
#else
	pthread_mutex_lock(&sched->set_mutex);
#endif
#endif
    printf("sched->count:%d\n", sched->count);

    best_w = ((loop_t *)(sched->loops[idx]))->weight;   // best_w equel 1st loop'weight
    best_i = idx;
	for (; idx < sched->count; ++idx)
	{                                                 
		w = ((loop_t *)(sched->loops[idx]))->weight;    //(1)
        printf("loop[%d](%p)->w[%d]\n", idx, sched->loops[idx], w);
		if (w < best_w)
		{
			best_w = w;
			best_i = idx;
		}
	}
#if 0

#ifdef WIN32
	LeaveCriticalSection(&sched->set_mutex);
#else
	pthread_mutex_unlock(&sched->set_mutex);
#endif

#endif
	return sched->loops[best_i];
}



