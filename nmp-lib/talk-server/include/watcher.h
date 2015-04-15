
#ifndef __WATCHER_H__
#define __WATCHER_H__

#include "macros.h"

typedef struct _watcher
{
    int  fd;
    int  timeout;
    int  stat;
    int  recv_stat;
    void *ctx;

    POINTER_BACK_TO(loop_t);
}watcher_t;

watcher_t *watcher_new(int fd, void *ctx);
int watcher_del(watcher_t *watcher);

#endif
