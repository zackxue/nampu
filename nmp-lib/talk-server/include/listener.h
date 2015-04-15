
#ifndef __LISTEN_H__
#define __LISTEN_H__

#include "macros.h"
#include "fd_set.h"

enum  
{      
    STAT_ACCEPT       = 0x01,   // accept cient's connction request, only server use
    STAT_REQUEST,
    STAT_RESPONSE,              // response to client, only server use 
    STAT_CONNECTING,            // connect to server, only client use
    STAT_WAIT,                  // wait for response, only client use 
    STAT_RW,                    //0x06               
    STAT_FIN,                   //0x07
    STAT_DEAD                   //0x08
};

typedef struct _listener 
{
    fd_node_t base;             // listen
    int  fd;
    int  timeout;
    int  stat;
    int  recv_stat;
    void *ctx;
    void *_owner;               // loop

    POINTER_BACK_TO(loop_t);
}listener_t;

listener_t *listener_new(int port, void *ctx);
int listener_del(listener_t *listener);

#endif
