
#include <stdio.h>
#include <stdlib.h>
#include "loop.h"
#include "log.h"
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
            loop_unref(sched->loops[nloop]);
        }
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
            loop_unref(sched->loops[nloop]);
        }
        free(sched);
    }
}

//loop_t *get_best_loop(sched_t *sched)
void *get_best_loop(sched_t *sched)
{
    loop_t *loop;
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
    
void *get_best_loop2(sched_t *sched)
{//@{no use total_weight}
    loop_t *loop;
    int w, best_w; // total_weight 
	int best_i = 0, idx = 0;

    printf("sched->count:%d\n", sched->count);
    best_w = ((loop_t *)(sched->loops[idx]))->weight;   // best_w equel 1st loop'weight
    best_i = idx;
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



