
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "sched.h"

#include "log.h"
#include "media_struct.h"
#include "talk_api.h"
#include "fd_set.h"
#include "loop.h"
#include "sched.h"
#include "media.h"
#include "listener.h"
#include "client.h"
#include "media_set.h"
#include "talk_server.h"

#define TALK_HOST "0.0.0.0"

typedef struct _talk_server
{
    char *host;
    char *port;

    listener_t *listener;

    media_set_t ms;             /* media set */
}talk_server_t;

void init_signal_facility()
{
    signal(SIGPIPE, SIG_IGN);   
}

static talk_ops_t *talk_ops;

int register_talk_ops(talk_ops_t *ops) /* 注册对讲的接口 */
{
    if (!ops)
    {
        return -1;
    }

    talk_ops = ops;
}

talk_ops_t *__get_talk_ops()
{
    return talk_ops;
}

void *__get_main_loop()
{
    static loop_t *g_loop = NULL;
    if (!g_loop)
    {
        g_loop = loop_new();
    }
    
    return g_loop;
}

#define MAX_LOOP_NUM (1)

void *__get_main_sched()
{
    static sched_t *g_sched = NULL;
    if (!g_sched)
    {
        g_sched = sched_new(MAX_LOOP_NUM);
    }

    return g_sched;
}

talk_hdl* talk_server_new()
{
    talk_server_t *server = NULL;

    server = (talk_server_t *)calloc(1, sizeof(talk_server_t));
    if (server)
    {   
        init_log_facility();
        init_signal_facility();
        server->host = strdup(TALK_HOST);
        server->port = strdup("TALK_PORT");
    }
    LOG_I("succeed to create server:%p", server);
    return (talk_hdl *)server;
}


int __server_bind_port(int port)
{
    loop_t *loop = NULL;
    listener_t *listener = NULL;
#if 0    
    loop_t *loop = __get_main_loop(); //@{get_best_loop}
#else
    sched_t* sched = __get_main_sched();
    if (!sched)
    {
        goto _error;
    }

    loop = get_best_loop2(sched);
    if (!loop)
    {//@{not need free loop, because loop is own to sched}
        goto _error;
    }
    LOG_I("get best loop:%p", loop);
#endif
    
    listener = listener_new(port, NULL);
    if (!listener)
    {
        goto _error;
    }
    
    SET_OWNER(listener, (void*)loop);       // set listen's loop
    LOOP_ADD_WEIGHT(loop);

    listener->stat = STAT_ACCEPT;
    
    if(gettid() == loop->set_tid) 
    {
        fd_set_add_node3(loop->_set, (fd_node_t *)listener, FD_R_FLAGS, listener->timeout);
    }
    else
    {

#ifdef WIN32
        EnterCriticalSection(&loop->set_mutex);
#else
		pthread_mutex_lock(&loop->set_mutex);
#endif
        
        fd_set_add_node3(loop->_set, (fd_node_t *)listener, FD_R_FLAGS, listener->timeout);
#ifdef WIN32
		LeaveCriticalSection(&loop->set_mutex);	
#else
        pthread_mutex_unlock(&loop->set_mutex);
#endif
    }
    
    return 0;
    
_error:
    if (sched)
    {   printf("sched:%p g_sched: %p\n", sched, __get_main_sched());
        sched_del(sched);
        printf("g_sched: %p\n", __get_main_sched());
    }
    
    if(listener)
    {
        listener_del(listener);
    }
 
    return -1;
}

int talk_server_start(talk_hdl *s, int port)
{
    if (!s)
    {
        return -1;
    }

    return __server_bind_port(port);
}

void talk_server_free(talk_hdl *s)
{
    sched_t *sched = NULL;
    talk_server_t *server = (talk_server_t *)s;
    if (!server)
    {
        return ;
    }
#if 0
    loop_unref(__get_main_loop());
#else
/*
    sched = __get_main_sched();
    if (sched)
    {
        sched_del(sched);
    }
*/    
#endif
    if (server->host)
    {
        free(server->host);
    }
    if (server->port)
    {
        free(server->port);
    }
    return;
}


