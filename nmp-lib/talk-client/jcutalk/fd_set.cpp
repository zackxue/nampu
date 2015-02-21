
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#ifdef __WIN32__
#include "stdint.h"
#else
#include <stdint.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include "macros.h"
#include "fd_set.h"
#include "list.h"

fd_node_t *fd_node_new(int size, int fd, fd_node_fun_t* func, void *context)
{
    fd_node_t *p_node = (fd_node_t *)calloc(1, size); // size is son's size
    if(p_node == NULL)return NULL;
    p_node->fd = fd;
    p_node->read_cb = func;
    p_node->context = context;
    return p_node;
}
int fd_node_del(fd_node_t *node)
{
    if(node)free(node);
    return 0;
}

fd_set_t *fd_set_new0(void)
{
    fd_set_t *p_set = (fd_set_t *)calloc(1, sizeof(fd_set_t));
    if(p_set == NULL)return NULL;
    INIT_LIST_HEAD(&p_set->node_list.list);
    FD_ZERO(&p_set->r_set);
    FD_ZERO(&p_set->w_set);
    FD_ZERO(&p_set->e_set);
    return p_set;
}
int fd_set_del(fd_set_t *set)
{
    if(set)free(set);
    return 0;
}

int fd_set_add_node(fd_set_t *set, fd_node_t *node)
{
    FD_SET(node->fd, &set->r_set);
    if(node->fd > set->max_fd) set->max_fd = node->fd;
    printf("add => fd_count:%d, node->fd:%d\n", ++set->fd_count, node->fd);
    list_add_tail(&node->list, &set->node_list.list);
    return 0;
}

int fd_set_del_node(fd_set_t *set, fd_node_t *node)
{
    FD_CLR(node->fd, &set->r_set);
    printf("del => fd_count:%d, node->fd:%d\n", --set->fd_count, node->fd);
    list_del(&node->list);
    return 0;
}

int fd_set_add_node2(fd_set_t *set, fd_node_t *node, int flags)
{
    node->flags = flags; 
    if(node->flags & FD_R_FLAGS)
    {
        FD_SET(node->fd, &set->r_set); //将node加入到set的集合
    }
    if(node->flags & FD_W_FLAGS)
    {
        FD_SET(node->fd, &set->w_set);
    }
    if(node->flags & FD_E_FLAGS)
    {
        FD_SET(node->fd, &set->e_set);
    }
    if(node->fd > set->max_fd) set->max_fd = node->fd;
    printf("add => fd_count:%d, node->fd:%d\n", ++set->fd_count, node->fd);
    list_add_tail(&node->list, &set->node_list.list);

	return 0;
}

int fd_set_add_node3(fd_set_t *set, fd_node_t *node, int flags, int tv_msec)
{
    node->ttl = node->timeout = tv_msec;
    fd_set_add_node2(set, node, flags);

	return 0;
}

int fd_set_mod_node2(fd_set_t *set, fd_node_t *node, int flags)
{
    node->ttl = node->timeout;
    if(node->flags & FD_R_FLAGS)
    {
        FD_CLR(node->fd, &set->r_set);
    }
    if(node->flags & FD_W_FLAGS)
    {
        FD_CLR(node->fd, &set->w_set);
    }
    if(node->flags & FD_E_FLAGS)
    {
        FD_CLR(node->fd, &set->e_set);
    }
    
    node->flags = flags; 
    
    if(node->flags & FD_R_FLAGS)
    {
        FD_SET(node->fd, &set->r_set);
    }
    if(node->flags & FD_W_FLAGS)
    {
        FD_SET(node->fd, &set->w_set);
    }
    if(node->flags & FD_E_FLAGS)
    {
        FD_SET(node->fd, &set->e_set);
    }
    return 0;
}

int fd_set_del_node2(fd_set_t *set, fd_node_t *node)
{
    if(node->flags & FD_R_FLAGS)
    {
        FD_CLR(node->fd, &set->r_set);
    }
    if(node->flags & FD_W_FLAGS)
    {
        FD_CLR(node->fd, &set->w_set);
    }
    if(node->flags & FD_E_FLAGS)
    {
        FD_CLR(node->fd, &set->e_set);
    }
    printf("del => fd_count:%d, node->fd:%d\n", --set->fd_count, node->fd);
    list_del(&node->list);
    return 0;
}
int fd_set_poll(fd_set_t *set, struct timeval *timeout)
{
    int ret = 0;

    struct timeval tv;
    fd_set r_set = set->r_set;
    fd_set w_set = set->w_set;
    fd_set e_set = set->e_set;

	tv.tv_sec  = timeout->tv_sec;
	tv.tv_usec = timeout->tv_usec;

    if ((ret = select(set->max_fd + 1, &r_set, &w_set, &e_set, &tv)) < 0)
    {
        if(errno == EINTR)
        {
            printf("select was interrupt by signal!\n");
            printf("tv.t_sec:%d\n",tv.tv_sec);
            ret = 0;
        }
        else
        {
			Sleep(20);
			printf("[ERR] pid:%d select failed:[%d]:%s, max_fd:%d\n", gettid(), errno, strerror(errno), set->max_fd);
        }
        //error;
    }
    else if(ret == 0)
    {
        //timeout;
    }
    else
    {
        fd_node_t *p_node = NULL;
        fd_node_t *p_next = NULL;
#ifdef WIN32
        list_for_each_entry_safe(p_node, fd_node_t, p_next, fd_node_t, &set->node_list.list, list)
#else
        list_for_each_entry_safe(p_node, p_next, &set->node_list.list, list)
#endif
        {   
            p_node->revent = 0;
            
            if(FD_ISSET(p_node->fd, &r_set)) p_node->revent |= FD_R_FLAGS;
            if(FD_ISSET(p_node->fd, &w_set)) p_node->revent |= FD_W_FLAGS;
            if(FD_ISSET(p_node->fd, &e_set)) p_node->revent |= FD_E_FLAGS;

            if(!p_node->revent)
            {
                continue;
            }
            
            p_node->ttl = p_node->timeout; // 当服务器可读可写时,重置超时时间, 一旦服务器很久不可读了,直接超时退出
            
            if(p_node->read_cb)
            {
                p_node->read_cb(p_node);
            }
        }
    }
    timeout->tv_sec  -= tv.tv_sec;
    timeout->tv_usec -= tv.tv_usec;
   
    {
        fd_node_t *p_node = NULL;
        fd_node_t *p_next = NULL;
#ifdef WIN32
        list_for_each_entry_safe(p_node, fd_node_t, p_next, fd_node_t, &set->node_list.list, list)
#else
        list_for_each_entry_safe(p_node, p_next, &set->node_list.list, list)
#endif
        {
            if(p_node->timeout)
            {
                p_node->ttl -= (timeout->tv_sec*1000+timeout->tv_usec/1000);
                if((p_node->ttl <= 0) && (p_node->read_cb != NULL))
                {
                    p_node->revent |= FD_T_FLAGS;
                    p_node->read_cb(p_node);
                }
            }
        }
    }
    
    return ret;
}

