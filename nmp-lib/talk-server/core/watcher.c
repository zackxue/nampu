
#include "watcher.h"

#ifndef NULL
#define NULL (void *)0
#endif

#define DEFAULT_TIMEOUT (10*1000) /* close connection when recv no data in 10s */

watcher_t *watcher_new(int fd, void *ctx)
{
    watcher_t *w = (watcher_t *)calloc(1, sizeof(watcher_t));
    if (!w)
    {
        return NULL;
    }

    w->fd = fd;
    w->timeout = DEFAULT_TIMEOUT;
    w->ctx = ctx;

    return w;
}
int watcher_del(watcher_t *watcher)
{
    if(watcher)    
    {       
        if(watcher->fd > 0) 
        {
#ifdef WIN32
            closesocket(watcher->fd);  
#else
            close(watcher->fd);
#endif
            watcher->fd = -1;
        }           
        free(watcher);
        watcher = NULL;    
    }    
    
    return 0;    
}



