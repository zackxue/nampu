#ifndef __FD_SET_H__
#define __FD_SET_H__

#include "list.h"

#ifdef WIN32
#include <winsock.h>

#endif
/*
 * FD描述符集合定义
 */

typedef  unsigned char uint8_t;


typedef struct fd_node_s {
	struct  list_head list;
    int     fd;
    int     timeout;
    int     ttl;
    int     flags;
    int     revent;
    uint8_t in_buf[64*1024];
    int     in_buf_len;
    int     (*read_cb)(struct fd_node_s *_this);
    void    *context;
}fd_node_t;
typedef int(fd_node_fun_t)(struct fd_node_s *_this);

typedef struct fd_set_s {
    fd_set          r_set;
    fd_set          w_set;
    fd_set          e_set;
    int             max_fd;
    int             fd_count;
    fd_node_t       node_list;
}fd_set_t;

fd_node_t *fd_node_new(int size, int fd, fd_node_fun_t* func, void *context); //fd_node_t对应的是watch, 每执行fd_node_new多一个, 是一个链表
int fd_node_del(fd_node_t *node);

fd_set_t *fd_set_new0(void); //fd_set_t 是每执行hi_talk_cli_init产生一个的
int fd_set_del(fd_set_t *set);

typedef enum _FD_FLAGS {
      FD_R_FLAGS    = 0x01
    , FD_W_FLAGS    = 0x02
    , FD_E_FLAGS    = 0x04
    , FD_T_FLAGS    = 0x08  //超时标志
}FD_FLAGS_E;

int fd_set_add_node(fd_set_t *set, fd_node_t *node);
int fd_set_del_node(fd_set_t *set, fd_node_t *node);

int fd_set_add_node2(fd_set_t *set, fd_node_t *node, int flags);

int fd_set_mod_node2(fd_set_t *set, fd_node_t *node, int flags);
int fd_set_del_node2(fd_set_t *set, fd_node_t *node);

int fd_set_add_node3(fd_set_t *set, fd_node_t *node, int flags, int tv_msec);

int fd_set_poll(fd_set_t *set, struct timeval *timeout);

#endif //__fd_select__

