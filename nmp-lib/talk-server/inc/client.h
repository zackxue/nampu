
#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "macros.h"
#include "fd_set.h"
#include "talk_api.h"
#include "loop.h"

enum
{
    RECV_HDR,
    RECV_DATA,
};

#define MAX_BACKLOG_SIZE (160*100)
#define MAX_RESEND_FRAME_SIZE (160*20) /* 重传帧的大小 */

typedef struct _client
{    
    fd_node_t base;
    atomic_t ref_count; 
    int fd;            /* client'fd */
    int timeout;    
    int stat;    
    int recv_stat;
    talk_ops_t parm;
    void *user_data;   /* user data */ 

    media_info_t info;
    int response;      /* response to client */
    
    //char frm_buffer[MAX_BACKLOG_SIZE]; /* frm buffer for dev */
    
    char backlog[MAX_BACKLOG_SIZE];
    int backlog_size; 

    LOCK_T lock;
        
    POINTER_BACK_TO(loop_t);
    void *client_node;   /* point to client node */
}client_t;

client_t *client_new(int fd, talk_ops_t *ops, loop_t *loop);
void client_free(client_t *cli);

client_t *client_set_node(client_t *c, void *cn);
void *client_get_node(client_t *c);

client_t *client_ref(client_t *cli);
void client_unref(client_t *cli);

// client_node_t 
typedef struct _client_node
{
    struct list_head list;
    client_t *c;
    atomic_t ref_count;

    POINTER_BACK_TO(media_node_t);
}client_node_t;

client_node_t *client_node_new(int fd, talk_ops_t *ops, loop_t *loop);
void client_node_del(client_node_t * cn);

client_node_t *client_node_ref(client_node_t *cn);
void client_node_unref(client_node_t *cn);

#endif
