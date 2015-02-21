
// sched.h manage loop
#ifndef __SCHED_H__
#define __SCHED_H__

#include <Windows.h>
//#include "loop.h"

#define MAX_LOOP_NUM 8
#define MIN_LOOP_NUM 1

typedef struct _sched
{
    int count;        /* count of loop */
    int total_weight;
#ifdef WIN32    
	CRITICAL_SECTION set_mutex;
#else
	pthread_mutex_t set_mutex;
#endif
    //loop_t **loop;     /* loop array in heap */
    void **loops;
}sched_t;

sched_t* sched_new(int nloop);
void sched_del(sched_t *sched);

void *get_best_loop(sched_t *sched); /* */
void *get_best_loop2(sched_t *sched); /* */

#endif
