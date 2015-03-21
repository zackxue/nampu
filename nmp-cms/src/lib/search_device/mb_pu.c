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
    hdl->listen_fd = -1;

    /* generate source id */
    char mac[32] = {0};
    int rand_ID;
    srand(time(0));
    rand_ID = rand()%100;
    if((ret = mb_get_mac_addr("eth0", mac, sizeof(mac))) != 0)
    {
        if(hdl)free(hdl);
        return NULL;
    }
    snprintf(hdl->src_id, sizeof(hdl->src_id), "nmp#%s#%02d", mac, rand_ID);
    printf("mb_lib pu_id :%s\n\n\n", hdl->src_id);

    /*open mb listen */

    hdl->listen_fd = mb_listen_2(addr, port, 1);
    if(hdl->listen_fd < 0)
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
        if(hdl->listen_fd >= 0)close(hdl->listen_fd);
        if(hdl)free(hdl);
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

    hdl->tid_running  = 0;
    pthread_join(hdl->tid, NULL);

    if(hdl->listen_fd)close(hdl->listen_fd);

    if(hdl)free(hdl);

    return 0;

}

static int pu_mb_recv_func(void *parm)
{
    pu_mb_hdl_t* hdl = (pu_mb_hdl_t *)parm;
    char msg_buf[MB_MSG_MAX_SIZE] = {0};
    int i;
    mb_msg_t *msg = (mb_msg_t *)msg_buf;

    struct sockaddr_in src_addr;
    socklen_t addrlen ;
    addrlen = sizeof(struct sockaddr_in);
    int recv_len = 0;

    mb_pu_parm_t process_parm;

    recv_len = recvfrom(hdl->listen_fd
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
    if(strcmp(hdl->src_id, msg->dst) != 0
        && strcmp(general_dst, msg->dst) != 0)
    {
        //printf("bad dstination!\n");
        return 0;
    }
    #if 1
    printf("recv len:%d, from addr:%s, port:%d\n"
            , recv_len
            , inet_ntoa(src_addr.sin_addr)
            , ntohs(src_addr.sin_port));
    #endif
    process_parm.id = msg->msg_id;
    process_parm.args = msg->args;
    strncpy(process_parm.user, msg->user, sizeof(process_parm.user));
    strncpy(process_parm.pass, msg->pass, sizeof(process_parm.pass));
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
    strncpy(msg->src, hdl->src_id, sizeof(msg->src));

    src_addr.sin_addr = hdl->addr.sin_addr;
    src_addr.sin_port = htons(msg->port);
    src_addr.sin_family = AF_INET;
    int sended;
    struct ifreq buf[10];
    memset(buf, 0, sizeof(buf));
    struct ifconf ifc;

    int intrface;
    ifc.ifc_len = 10 * sizeof(struct ifreq);
    ifc.ifc_buf = (caddr_t) buf;
    if (ioctl (hdl->listen_fd, SIOCGIFCONF, (char *) &ifc))
    {
        printf("SIOCGIFCONF failed\n");
        return -1;
    }

     intrface = ifc.ifc_len / sizeof (struct ifreq);
     //printf("interface num is intrface=%d\n\n\n",intrface);
     for(i=0; i<intrface; i++)
     {
        //printf ("net device [%s] [%p]\n", buf[i].ifr_name, buf[i].ifr_name);
        if(strstr(buf[i].ifr_name, "lo") != NULL || strstr(buf[i].ifr_name, ":") != NULL)
        {
            //printf("skip this interface! name:%s\n", buf[i].ifr_name);
            continue;
        }
        if (ioctl(hdl->listen_fd, SIOCGIFADDR, &buf[i]) == 0)
        {
            if(setsockopt(hdl->listen_fd
                    , IPPROTO_IP
                    , IP_MULTICAST_IF
                    , &(((struct sockaddr_in *)&buf[i].ifr_addr)->sin_addr)
                    , sizeof(struct in_addr)) < 0 )
            {
                printf("error: listen_sock set mb send if:%s  failed, (%m)\n", buf[i].ifr_name);
            }
            sended = sendto(hdl->listen_fd
            , msg
            , sizeof(mb_msg_t)+msg->size
            , 0
            , (struct sockaddr *)&src_addr
            , addrlen);
        }

     }



    return 0;

}
static void *pu_mb_recv_task(void *parm)
{
    pu_mb_hdl_t* hdl = (pu_mb_hdl_t *)parm;
    struct timeval tv;
    fd_set r_set;
    int max_fd = hdl->listen_fd + 1;
    int ret;
    while(hdl->tid_running)
    {
        //printf("run!\n");
        //select();
        tv.tv_sec  = 1;
        tv.tv_usec = 0;
        FD_ZERO(&r_set);
        FD_SET(hdl->listen_fd, &r_set);
        if((ret = select(max_fd, &r_set, NULL, NULL, &tv)) > 0)
        {
            if(FD_ISSET(hdl->listen_fd, &r_set))
            {
                pu_mb_recv_func(hdl);
            }
        }
    }

    return NULL;
}




