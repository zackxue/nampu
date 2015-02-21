#ifndef __LOOP_H__
#define __LOOP_H__

#include <pthread.h>
#include <sys/types.h>

#ifdef __WIN32__
#define atomic_t int
#else
#include <unistd.h>
#include "jlib.h"
#endif
#include "fd_set.h"

typedef struct loop_s 
{    
    //obj             obj_base;
    atomic_t	    ref_count;
    fd_set_t        *_set;    
    int             runing; 
#ifdef WIN32
	DWORD			set_tid;
	HANDLE			th_pid;
	HANDLE			exitEvent;		//线程退出事件
	CRITICAL_SECTION set_mutex;
#else
	pid_t           set_tid;    
    pthread_t       th_pid; 
	pthread_mutex_t set_mutex;
#endif    
    int             weight;
}loop_t;

#define LOOP_ADD_WEIGHT(x) \
    (x)->weight++;
#define LOOP_DEC_WEIGHT(x) \
    (x)->weight--;

loop_t * loop_new();
void loop_free(loop_t *loop);

loop_t *loop_ref(loop_t *loop);
void loop_unref(loop_t *loop);

#endif
