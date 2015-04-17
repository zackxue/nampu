#include <stdlib.h>
#include "unix_sock.h"
#include "client.h"
#include "loop.h"
#include "log.h"
#include "listener.h"
#include "talk_sched.h"

#define MAX_SNDBUF  (100*1024)

int __listen_read(struct fd_node_s *_this)
{
    int cli_fd;
    client_t *cli;
    client_node_t *cn;
    listener_t *listener = (listener_t*)_this;
    LOG_I("__listen_read:%d", _this->fd);
    switch(listener->stat)
    {
        case STAT_ACCEPT:
            if(_this->revent & FD_R_FLAGS)
            {
                // accept client's connection
                cli_fd = unix_sock_tcp_accept(listener->fd);
                if (cli_fd < 0)
                {
                    LOG_I("fail unix_sock_tcp_accept %d(%s)", cli_fd, strerror(errno));
                    goto _error;
                }
                
                if (socket_set_noblock(cli_fd) < 0)
                {
                    goto _error;
                }
#if 0
                if (socket_set_attr_send_buf(cli_fd, 16 * 1024) < 0)
                {
                    goto _error;
                }
#endif           
                LOG_I("accept client sock:%d", cli_fd);
#if 0
                cli = client_new(cli_fd, __get_talk_ops(), GET_OWNER(listener, loop_t));
#else           
                if (0)                
                {
                    //mn = __media_node_ref(mn);
                    // media_node_add_client(mn, client_node);
                }
    #if 0
                cn = client_node_new(cli_fd, (talk_ops_t *)__get_talk_ops(), get_best_loop2((sched_t *)__get_main_sched()));
                if (!cn)
                {
                    LOG_I("fail new client node");
                    goto _error;
                }
                LOG_I("new client node:%p fd:%d", cn, cli_fd);
    #else
                /* happen in same thread, no need to lock */
                cli = client_new(cli_fd, (talk_ops_t *)__get_talk_ops(), get_best_loop2((sched_t *)__get_main_sched()));                               
                if (!cli)
                {
                    LOG_I("fail client_new\n");
                    goto _error;
                }
                LOG_I("new client:0x%p fd:%d", cli, cli_fd);
    #endif
#endif
            }
            break;
    }
    return 0;
_error:
    if (cli_fd > 0)
    {
        close(cli_fd);
    }
    
    return -1;
}

listener_t *listener_new(int port, void *ctx)
{
    int err;
    listener_t *listener = NULL;
    
    int sock = unix_sock_bind(L4_TCP, 0, htons(port), FORCE_BIND);
    if (sock < 0)
    {
        LOG_I("unix_sock_bind err:%d", sock);
        goto _error; 
    }

    if (socket_set_noblock(sock) < 0)
    {    
        LOG_I("socket_set_noblock err:%d", errno);
        goto _error;
    }

    if (socket_set_linger(sock) < 0)
    {
        LOG_I("socket_set_linger err:%d", errno);
        goto _error;
    }

    listener = (listener_t *)fd_node_new(sizeof(listener_t), sock, __listen_read, listener);
    if(!listener)
    {
        LOG_I("failed to fd_node_new");
        goto _error;
    }
   
    err = unix_sock_tcp_listen(sock);
    if (err)
    {   
        LOG_I("failed to unix_sock_tcp_listen err:%d", err);
        goto _error;
    }
    
    listener->fd = sock;
    listener->timeout = 0;            // not need handle timeout
    listener->ctx = ctx;
    listener->_owner = NULL; 

    LOG_I("Succeed to create listener:%p sock:%d port:%d", listener, sock, port);

    return listener;

 _error:
    if (sock > 0)
    {
        close(sock);
        sock = -1;
    }
    if (listener)
    {
        free(listener);
        listener = NULL;
    }
    
    return NULL;
}

int listener_del(listener_t *listener)
{
    if(listener)    
    {       
        if(listener->fd > 0) 
        {
#ifdef WIN32
            closesocket(listener->fd);  
#else
            close(listener->fd);
#endif
            listener->fd = -1;
        }           
        free(listener);
        listener = NULL;    
    }    
    
    return 0;
}


