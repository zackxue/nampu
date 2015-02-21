
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>

#include "macros.h"
#include "fd_set.h"
#include "loop.h"

#ifdef WIN32
DWORD WINAPI poll_task(LPVOID parm);
#else
static void *poll_task(void *parm);
#endif

#ifdef WIN32
DWORD WINAPI poll_task(LPVOID parm)
#else
static void *poll_task(void *parm)
#endif
{
    int ret;
    struct timeval tv;
    loop_t *loop = (loop_t *)parm;
    
    loop->set_tid = gettid();
    while(loop->runing)
    {
        tv.tv_sec  = 0;
        tv.tv_usec = 200*1000;
#ifdef WIN32
		EnterCriticalSection(&loop->set_mutex);
#else
		pthread_mutex_lock(&loop->set_mutex);
#endif
		// poll_talk 在执行hi_talk_cli_init后, 不停地刷新select中的集合.
		// 其实上, 执行体就是select本身.
        ret = fd_set_poll(loop->_set, &tv);
#ifdef WIN32
		LeaveCriticalSection(&loop->set_mutex);	
#else
        pthread_mutex_unlock(&loop->set_mutex);
#endif       
        //if( ret < 0)
        {

#ifdef WIN32
			Sleep(0);	
#else
			usleep(0);
#endif
        }
    }
#ifdef WIN32
	if (loop->exitEvent  != NULL)
    {   
		SetEvent(loop->exitEvent);
    }
#endif
    //loop_unref(loop);  // 放弃对loop的拥有权, 不需要关心在线程运行过程中,loop会被释放, 因为loop的释放是在sched_del完成的,一定是先等待线程退出,再free(loop)
    
    return NULL;
}

/*
static inline void
on_obj_fin(obj *p)
{
	//
}
*/
loop_t *loop_new()
{
	int dummySocketNum;
    loop_t *loop = (loop_t *)calloc(1, sizeof(loop_t));
    if(!loop) 
    {
        return NULL;
    }
    
#ifdef WIN32
    InitializeCriticalSection(&loop->set_mutex);
    loop->exitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
    pthread_mutex_init(&loop->set_mutex, NULL);
#endif

    loop->ref_count = 1;
    loop->_set = fd_set_new0();
    loop->runing = 1;
    loop->weight = 0;

	// For some unknown reason, select() in Windoze sometimes fails with WSAEINVAL if
	// it was called with no entries set in "readSet".  If this happens, ignore it:
	dummySocketNum = socket(AF_INET, SOCK_DGRAM, 0);
	FD_SET((unsigned)dummySocketNum, &loop->_set->r_set);

#ifdef WIN32
    if ((loop->th_pid = CreateThread(NULL, 0, poll_task, (void *)loop, 0,NULL)) == NULL)
    {
        goto _error;
    }

    while(!loop->set_tid) 
    {
       Sleep(5);
    }
#else
    loop_ref(loop); // add ref_count
    if(pthread_create(&loop->th_pid, NULL, poll_task, (void *)loop) < 0)
    {
        loop_unref(loop); // reduce ref_count
        goto _error;
    }

    while(!loop->set_tid) 
    {
        usleep(0);
    }
#endif

    return loop;
_error:
    if(loop->_set) 
    {
        fd_set_del(loop->_set);
    }
#ifdef WIN32
    DeleteCriticalSection(&loop->set_mutex);
    if (loop->exitEvent)
    {
        CloseHandle(loop->exitEvent);
        loop->exitEvent = NULL;
    }  
#else
    pthread_mutex_destroy(&loop->set_mutex);
#endif
    
    free(loop);
    loop = NULL;
    return NULL;
}

void loop_free(loop_t *loop)
{
    if(!loop)
    {
        return ;
    }
    loop->runing = 0;
#ifdef WIN32
	WaitForSingleObject(loop->exitEvent, 2000);
#else
    pthread_join(loop->th_pid, NULL);
#endif
    
    if(loop->_set)
    {
        fd_set_del(loop->_set);
    }
    
#ifdef WIN32
	DeleteCriticalSection(&loop->set_mutex);	
#else
    pthread_mutex_destroy(&loop->set_mutex);
#endif
    free(loop);
    loop = NULL;
    return ;      
}

void LOOP_finalize(loop_t *loop)
{//@{fixme}
    loop_free(loop);
}

loop_t *loop_ref(loop_t *loop)
{
    __OBJECT_REF(loop);    
}

void loop_unref(loop_t *loop)
{
    __OBJECT_UNREF(loop, LOOP);
}

