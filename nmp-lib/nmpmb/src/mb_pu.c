#include "mb_pu.h"

static void *pu_mb_recv_task(void *parm);


pu_mb_hdl_t*
    pu_mb_hdl_new(char *addr
            , unsigned short port
            , pu_process_cb_t *cb)
{
    pu_mb_hdl_t* hdl = (pu_mb_hdl_t*)malloc(sizeof(pu_mb_hdl_t));
    int ret;
    if(hdl == NULL)
    {
        return NULL;
    }
    memset(hdl, 0 ,sizeof(pu_mb_hdl_t));
    /*open mb listen */

    ret = mb_listen_3(addr, port, 1, &hdl->eth_addr[0], &hdl->eth_active_num);
    if(ret < 0)
    {
        printf("pu_open listen on addr:%s, port:%d failed T_T T_T T_T\n\n\n", addr, port);
        if(hdl)free(hdl);
        return NULL;
    }
    else
    {
        printf("pu_open listen on addr:%s, port:%d success ^_^ ^_^ ^_^\n\n\n", addr, port);
    }

    /* start thread */
    hdl->tid_running = 1;
    if(pthread_create(&hdl->tid, NULL, pu_mb_recv_task, hdl) != 0)
    {
        printf("pu_thread_create error!\n");
        int j;
        for(j = 0; j< hdl->eth_active_num; j++)
        {
            close(hdl->eth_addr[j].listen_fd);
        }
        return NULL;
    }
    /*save addr*/
    inet_aton(addr, &hdl->addr.sin_addr);
    hdl->addr.sin_port = htons(port);
    hdl->addr.sin_family = AF_INET;

    /* add cb */
    hdl->cb = *cb;

    return hdl;
}

int pu_mb_hdl_del(pu_mb_hdl_t *hdl)
{
    assert(hdl);
    int i;
    hdl->tid_running  = 0;
    pthread_join(hdl->tid, NULL);

    for(i=0; i<hdl->eth_active_num; i++)
    {
        close(hdl->eth_addr[i].listen_fd);
    }
  
    if(hdl)free(hdl);
    
    return 0;
    
}

static int pu_mb_recv_func(void *parm, int the_eth_no)
{
    pu_mb_hdl_t* hdl = (pu_mb_hdl_t *)parm;
    char msg_buf[MB_MSG_MAX_SIZE] = {0};
    mb_msg_t *msg = (mb_msg_t *)msg_buf;

    struct sockaddr_in src_addr;
    socklen_t addrlen ;
    addrlen = sizeof(struct sockaddr_in);
    int recv_len = 0;
    int i;

    mb_pu_parm_t process_parm;

    recv_len = recvfrom(hdl->eth_addr[the_eth_no].listen_fd
        , msg
        , MB_MSG_MAX_SIZE
        , 0 
        , (struct sockaddr *)&src_addr
        , &addrlen);

    msg->error = MB_NOTIFY_ERR_FAILED;
    if(msg->size != recv_len - sizeof(mb_msg_t))
    {
        //printf("bad len msg!\n");
        return 0;
    }
    if(strcmp(hdl->eth_addr[the_eth_no].src_id, msg->dst) != 0
        && strcmp(general_dst, msg->dst) != 0)
    {
        //printf("bad dstination!\n");
        return 0;
    }
    #if 1
    printf("recv len:%d, from addr:%s, port:%d, eth:%s\n"
            , recv_len
            , inet_ntoa(src_addr.sin_addr)
            , ntohs(src_addr.sin_port)
            , hdl->eth_addr[the_eth_no].eth_name);
    #endif
    process_parm.id = msg->msg_id;
    process_parm.args = msg->args;
    strncpy(process_parm.user, msg->user, sizeof(process_parm.user));
    strncpy(process_parm.pass, msg->pass, sizeof(process_parm.pass));
    strncpy(process_parm.eth_name, hdl->eth_addr[the_eth_no].eth_name, sizeof(process_parm.eth_name));
    if(msg->args == MB_MSG_GET)
    {
        msg->size = MB_MSG_MAX_SIZE - sizeof(mb_msg_t);
    }
    process_parm.size = &msg->size;
    process_parm.data = msg->data;
    if(hdl->cb.callback)
    {
        msg->error = hdl->cb.callback(&hdl->cb, &process_parm);
    }
    if(msg->args == MB_MSG_GET)
    {
        if(msg->size == 0)
        {
            return 0;
        }
        if(msg->error != MB_NOTIFY_ERR_0)
        {
            return 0;
        }
    }
    else if(msg->args == MB_MSG_SET)
    {
        msg->size = 0;
    }
    else
    {
        return 0;
    }


    
    memcpy(msg->dst, msg->src, sizeof(msg->dst));
    strncpy(msg->src, hdl->eth_addr[the_eth_no].src_id, sizeof(msg->src));
    
    src_addr.sin_addr = hdl->addr.sin_addr;
    src_addr.sin_port = htons(msg->port);
    src_addr.sin_family = AF_INET;
    int sended;


    struct ifreq ifr;
    strncpy(ifr.ifr_name, hdl->eth_addr[the_eth_no].eth_name, sizeof(ifr.ifr_name));
    ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
    if (ioctl(hdl->eth_addr[the_eth_no].listen_fd, SIOCGIFADDR, &ifr) == 0)
    {
        if(setsockopt(hdl->eth_addr[the_eth_no].listen_fd
                , IPPROTO_IP
                , IP_MULTICAST_IF
                , &(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)
                , sizeof(struct in_addr)) < 0 )
        {
            printf("error: listen_sock set mb send if:%s  failed, (%m)\n", hdl->eth_addr[the_eth_no].eth_name);
        }
        sended = sendto(hdl->eth_addr[the_eth_no].listen_fd
        , msg
        , sizeof(mb_msg_t)+msg->size
        , 0
        , (struct sockaddr *)&src_addr
        , addrlen);
    }
    return 0;
    
}
static void *pu_mb_recv_task(void *parm)
{
    pu_mb_hdl_t* hdl = (pu_mb_hdl_t *)parm;
    struct timeval tv;
    fd_set r_set,tmp_set;
    int max_fd = 0;
    int ret, i;
    for(i=0; i<hdl->eth_active_num; i++)
    {
        if(hdl->eth_addr[i].listen_fd)
        {
            FD_SET(hdl->eth_addr[i].listen_fd, &tmp_set);
            if(hdl->eth_addr[i].listen_fd + 1 > max_fd)
            {
                max_fd = hdl->eth_addr[i].listen_fd + 1;
            }
        }
    }
    while(hdl->tid_running)
    {
        tv.tv_sec  = 1;
        tv.tv_usec = 0;
        FD_ZERO(&r_set);
        r_set = tmp_set;
        if((ret = select(max_fd, &r_set, NULL, NULL, &tv)) > 0)
        {
            for(i=0; i<hdl->eth_active_num; i++)
            {
                if(FD_ISSET(hdl->eth_addr[i].listen_fd, &r_set))
                {
                    pu_mb_recv_func(hdl, i);
                }
            }
        }
        else if(ret < 0)
        {
            printf("mb_pu_select_error");
        }
    }

    return NULL;
}




