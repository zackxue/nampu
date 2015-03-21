#include "mb_cu.h"


static void *mb_recv_task(void *parm);
static void *mb_recv_timer(void *parm);



mb_handle_t*
    mb_hdl_new(char *addr
            , unsigned short port
            , seq_t  *seq
            , list_t *req_list)
{
    //generate src_id(mac+pid);
    mb_handle_t *hdl;
    int ret;
    hdl = (mb_handle_t *)malloc(sizeof(mb_handle_t));
    if(hdl == NULL)return NULL;
    memset(hdl, 0, sizeof(mb_handle_t));
    hdl->listen_fd = -1;
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
    printf("mb_lib src_id :%s\n", hdl->src_id);

    //open mb listen

    int i;
    int listen_ok = 0;
    for(i = 1; i<100; i++)
    {
        if((hdl->listen_fd = mb_listen_2(addr, port + i, 0)) >= 0)
        {
            listen_ok = 1;
            break;
        }
    }
    if(listen_ok == 1)
    {
        printf("cu_open listen on addr:%s, port:%d success ^_^ ^_^ ^_^\n\n\n", addr, port + i);
    }
    else
    {
        printf("cu_open listen on addr:%s, port:%d failed  T_T T_T T_T\n\n\n", addr, port + i);
        if(hdl)free(hdl);
        return NULL;
    }
    hdl->port = port+i;


    //start mb recv thread;
    hdl->req_list = list_ref(req_list);
    hdl->seq      = seq_ref(seq);
    hdl->tid_running = 1;
    if((pthread_create(&hdl->tid, NULL, mb_recv_task, hdl)) != 0)
    {
        printf("thread recv create failed\n");
        list_unref(req_list);
        seq_unref(seq);
        if(hdl->listen_fd >= 0)close(hdl->listen_fd);
        if(hdl)free(hdl);
        return NULL;
    }
    if((pthread_create(&hdl->timer_tid, NULL, mb_recv_timer, hdl)) != 0)
    {
        printf("thread timer create failed\n");
        list_unref(req_list);
        seq_unref(seq);
        if(hdl->listen_fd >= 0)close(hdl->listen_fd);
        if(hdl)free(hdl);
        return NULL;
    }
    //save group_ip and port for send
    inet_aton(addr, &hdl->addr.sin_addr);
    hdl->addr.sin_port = htons(port);
    hdl->addr.sin_family = AF_INET;
    return hdl;
    //
}
int mb_hdl_del(mb_handle_t *hdl)
{
    mb_req_t* req;
    mb_req_t* p_next;
    //stop mb recv thread;
    hdl->tid_running  = 0;
    pthread_join(hdl->tid, NULL);
    pthread_join(hdl->timer_tid, NULL);
    //un_ref
    pthread_mutex_lock(&hdl->req_list->mutex);

    list_for_each_entry_safe(req, p_next, &hdl->req_list->head, list)
    {
        list_del(&req->list);
        mb_req_unref(req);
    }
    pthread_mutex_unlock(&hdl->req_list->mutex);
    seq_unref(hdl->seq);
    list_unref(hdl->req_list);
    //close mb listen;
    if(hdl->listen_fd)close(hdl->listen_fd);
    //del hadle
    if(hdl)free(hdl);
    return 0;


}

int mb_hdl_add(mb_handle_t *hdl, mb_req_t *req)
{
    pthread_mutex_lock(&hdl->req_list->mutex);
    list_add(&req->list, &hdl->req_list->head);
    pthread_mutex_unlock(&hdl->req_list->mutex);
    return 0;
    //add req to req_list;
}
int mb_hdl_rm(mb_handle_t *hdl, mb_req_t *req)
{
    pthread_mutex_lock(&hdl->req_list->mutex);
    list_del(&req->list);
    pthread_mutex_unlock(&hdl->req_list->mutex);
    return 0;
    //del req from req_list;
}

mb_req_t*
     mb_req_new(int msg_id
             , unsigned int seq_no
             , unsigned int ttl
             , int req_type
             , mb_cu_notify_t *nfy)
{
    //generate seq_no;
    mb_req_t *req = (mb_req_t *)malloc(sizeof(mb_req_t));
    if(req == NULL)return NULL;
    memset(req, 0, sizeof(mb_req_t));
    req->ref_count = 1;
    req->ttl = ttl;

    req->event_true = 0;
    req->req_no = seq_no;
    req->req_type = req_type;
    req->msg_id = msg_id;

    pthread_mutex_init(&req->mutex, NULL);
    pthread_cond_init(&req->cond, NULL);
    if(nfy)
    {
    req->notify = *nfy;
    }
    printf("mb_req => new (seq_no:%d)\n", req->req_no);
    return req;

}

static inline int mb_req_del(mb_req_t *req)
{
    assert(req);
        pthread_mutex_destroy(&req->mutex);
        pthread_cond_destroy(&req->cond);

    printf("mb_req => del (seq_no:%d)\n", req->req_no);
    if(req)free(req);
    return 0;
}


mb_req_t*
     mb_req_ref(mb_req_t *req)
{
    //ref_count++;
    assert(req != NULL &&
        atomic_get(&req->ref_count) > 0);

    atomic_inc(&req->ref_count);

    return req;
}
void mb_req_unref(mb_req_t *req)
{
    //ref_count--;
    //free req;
    assert( req != NULL &&
        atomic_get(&req->ref_count) > 0);

    if (atomic_dec_and_test_zero(&req->ref_count))
    {
        mb_req_del(req);
    }

}

int  mb_req_send(mb_handle_t *hdl
                    , mb_req_t *req
                    , char *dst_id
                    , int args
                    , char*user
                    , char*pass
                    ,int size
                    , void *buf)
{
    int sended, i;
    char msg_buf[MB_MSG_MAX_SIZE] = {0};
    //fill buff and send_len;
    mb_msg_t *msg = (mb_msg_t *)msg_buf;
    strncpy(msg->src, hdl->src_id, sizeof(msg->src));
    if(dst_id)
    {
        strncpy(msg->dst, dst_id, sizeof(msg->dst));
    }
    msg->req_no = req->req_no;
    msg->msg_id = req->msg_id;
    msg->size = size;
    msg->args = args;
    msg->port = hdl->port;
    if(user)
    {
        strncpy(msg->user, user, sizeof(msg->user));
    }
    if(pass)
    {
        strncpy(msg->pass, pass, sizeof(msg->pass));
    }
    if(buf && (msg->size <= MB_MSG_MAX_SIZE - sizeof(mb_msg_t)))
    {
        memcpy(msg->data, buf, msg->size);
    }

    //send mb msg;
    socklen_t sock_len = sizeof(struct sockaddr_in);
    //printf(" sizeof(mb_msg_t):%d sended :%d, error:%s!\n", sizeof(mb_msg_t),sended, strerror(errno));

    struct ifreq buffer[10];
    memset(buffer, 0, sizeof(buffer));
    struct ifconf ifc;

    int intrface;
    ifc.ifc_len = 10 * sizeof(struct ifreq);
    ifc.ifc_buf = (caddr_t) buffer;
    if (ioctl (hdl->listen_fd, SIOCGIFCONF, (char *) &ifc))
    {
        printf("SIOCGIFCONF failed\n");
        return -1;
    }

     intrface = ifc.ifc_len / sizeof (struct ifreq);
     //printf("interface num is intrface=%d\n\n\n",intrface);
     for(i=0; i<intrface; i++)
     {
        //printf ("net device [%s] [%p]\n", buffer[i].ifr_name, buffer[i].ifr_name);
        if(strstr(buffer[i].ifr_name, "lo") != NULL || strstr(buffer[i].ifr_name, ":") != NULL)
        {
            //printf("skip this interface! name:%s\n", buffer[i].ifr_name);
            continue;
        }
        if (ioctl(hdl->listen_fd, SIOCGIFADDR, &buffer[i]) == 0)
        {
            if(setsockopt(hdl->listen_fd
                    , IPPROTO_IP
                    , IP_MULTICAST_IF
                    , &(((struct sockaddr_in *)&buffer[i].ifr_addr)->sin_addr)
                    , sizeof(struct in_addr)) < 0 )
            {
                printf("error: listen_sock set mb send if:%s  failed, (%m)\n", buffer[i].ifr_name);
            }
        }
        sended = sendto(hdl->listen_fd
        , msg
        , sizeof(mb_msg_t)+msg->size
        , 0
        , (struct sockaddr *)&hdl->addr
        , sock_len);
     }
    return 0;
}

static int mb_recv_func(void *parm)
{
    mb_handle_t* hdl = (mb_handle_t*)parm;
    char msg_buf[MB_MSG_MAX_SIZE] = {0};
    mb_msg_t *msg = (mb_msg_t *)msg_buf;

    struct sockaddr_in src_addr;
    socklen_t addrlen;
    int recvd = 0;
    mb_cu_parm_t nfy_parm;
    mb_req_t *req = NULL;
    mb_req_t *p_next = NULL;
    int find_flag = 0;
    addrlen = sizeof(struct sockaddr_in);
    //recv mb msg;
    recvd = recvfrom(hdl->listen_fd
                    , msg
                    , MB_MSG_MAX_SIZE
                    , 0
                    , (struct sockaddr*)&src_addr
                    , &addrlen);
    //size
    if(msg->size != recvd - sizeof(mb_msg_t))
    {
        printf("bad len msg!\n");
        return 0;
    }
    //dst_id
    if(strcmp(hdl->src_id, msg->dst) != 0)
    {
        printf("bad dstination!\n");
        return 0;
    }

    //if(seq_no == req->seq_no;
    pthread_mutex_lock(&hdl->req_list->mutex);

    list_for_each_entry_safe(req, p_next, &hdl->req_list->head, list)
    {
        if(req->req_no == msg->req_no)
        {
            find_flag = 1;
            if(req->req_type != ITEM_REQ)
            {
                list_del(&req->list);
            }
            break;
        }
    }

    pthread_mutex_unlock(&hdl->req_list->mutex);


    //call item_cb;
    if(find_flag)
    {
        if(req->notify.callback)
        {
            nfy_parm.id  = msg->msg_id;
            nfy_parm.error= msg->error;
            nfy_parm.size= msg->size;
            nfy_parm.data= msg->data;
            strncpy(nfy_parm.dst_id, msg->src, sizeof(nfy_parm.dst_id));
            req->notify.callback(&req->notify, &nfy_parm);

        }
        if(req->req_type == NORMAL_REQ)
        {
            pthread_mutex_lock(&req->mutex);
            req->event_true = 1;
            pthread_cond_signal(&req->cond);
            pthread_mutex_unlock(&req->mutex);

        //free req;
            mb_req_unref(req);
        }
    }

    return 0;
}

static void *mb_recv_task(void *parm)
{
    mb_handle_t *hdl = (mb_handle_t *)parm;
    struct timeval tv;
    fd_set r_set;
    int max_fd = hdl->listen_fd + 1;
    int ret;
    while(hdl->tid_running)
    {
        //select();
        tv.tv_sec  = 1;
        tv.tv_usec = 0;
        FD_ZERO(&r_set);
        FD_SET(hdl->listen_fd, &r_set);
        if((ret = select(max_fd, &r_set, NULL, NULL, &tv)) > 0)
        {
            if(FD_ISSET(hdl->listen_fd, &r_set))
            {
                mb_recv_func(hdl);
            }
        }
        //printf("hdl->req_list:%p\n",hdl->req_list);
        //on_mb_timer(hdl->req_list);
    }

    return NULL;
}

static void *mb_recv_timer(void *parm)
{
    mb_handle_t *hdl = (mb_handle_t *)parm;
    while(hdl->tid_running)
    {
        sleep(1);
        on_mb_timer(hdl->req_list);
    }
    return NULL;
}


int on_mb_timer(void *_parm)
{

    assert(_parm);
    //printf("in mb timer!\n");
    mb_cu_parm_t nfy_parm;
    memset(&nfy_parm, 0 ,sizeof(mb_cu_parm_t));

    list_t *ls = (list_t*)_parm;

    mb_req_t *req = NULL;
    mb_req_t *p_next = NULL;

    pthread_mutex_lock(&ls->mutex);
    list_for_each_entry_safe(req, p_next, &ls->head, list)
    {
        if(curr_time_greater(req->ttl))
        {
            list_del(&req->list);
            if(req->notify.callback)
            {
                nfy_parm.id  = req->msg_id;
                nfy_parm.error= MB_NOTIFY_ERR_TIMEOUT;
                nfy_parm.size= 0;
                nfy_parm.data= NULL;
                req->notify.callback(&req->notify, &nfy_parm);
            }
            pthread_mutex_lock(&req->mutex);
            req->event_true = 1;
            pthread_cond_signal(&req->cond);
            pthread_mutex_unlock(&req->mutex);
            mb_req_unref(req);
        }
    }
    pthread_mutex_unlock(&ls->mutex);

    return 0;
}

