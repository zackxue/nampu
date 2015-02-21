/*
 * Clinet Network SDK Interface.
 * Copyright(c) by HiMickey, 2010~2014
 */
#include "mb_api.h"
#include "mb_cu.h"
#include "mb_pu.h"



static seq_t      *g_seq;
list_t     *g_mb_req_list;
static mb_handle_t  *g_mb_hdl;
static pu_mb_hdl_t  *g_pu_mb_hdl;


int mb_cu_init(char *addr, unsigned int port, unsigned int local_port)
{
    printf("\n\n"
        "= = = = = = = = = = = = = = = = = = =\n"
        "\n"
        "module\t:%s\n"
        "version\t:%s\n"
        "date\t:"__DATE__"  "__TIME__"\n"
        "\n"
        "= = = = = = = = = = = = = = = = = = =\n"
        "\n\n"
        , MB_NAME
        , MB_VERSION);
    
    
    g_mb_req_list = list_new();
    //printf("init g_mb_req_list:%p\n", g_mb_req_list);
    g_seq      = seq_new();

    g_mb_hdl = mb_hdl_new(addr
            , port
            , g_seq
            , g_mb_req_list);
    if(g_mb_hdl == NULL)
    {
        printf("mb cu init error!\n");
        return -1;
    }
    else
    {
        printf("mb cu init success!\n");
    }
    return 0;
}

int mb_cu_uinit(void)
{
    mb_hdl_del(g_mb_hdl);
    list_unref(g_mb_req_list);
    seq_unref(g_seq);
    
    printf("mb uinit.\n");
    return 0;
}


/* if use jcu_net_mb_query please use jcu_net_mb_release later */
mb_query_t*
    mb_query(int msg_id, mb_cu_notify_t *item_cb)
{
    mb_req_t *req;

    req = mb_req_new(msg_id
            , seq_generate(g_mb_hdl->seq)
            , abs_time_gen(TV_UNLIMITED)
            , ITEM_REQ
            , item_cb);

    mb_hdl_add(g_mb_hdl, req);
    mb_req_send(g_mb_hdl, req, general_dst, MB_MSG_GET, NULL, NULL, 0, NULL);

    return req;
}
int mb_release(mb_query_t *hdl)
{
    mb_req_t *req = (mb_req_t*)hdl;

    mb_hdl_rm(g_mb_hdl, req);
    
    mb_req_unref(req);
    
    return 0;
}
int mb_cfg_get(int msg_id
            ,char *dst_id
            , int  timeout
            , mb_cu_notify_t *complete_cb
            , int is_sync)
{
    mb_req_t *req = NULL;
    int ttl = timeout >= 0? timeout : 0;

 
    req = mb_req_new(msg_id
            , seq_generate(g_mb_hdl->seq)
            , abs_time_gen(ttl)
            , NORMAL_REQ
            , complete_cb);
    if(!req)
    {
        return -1;
    }
    if(is_sync)
    {
        mb_req_ref(req);
    }
    
    mb_hdl_add(g_mb_hdl, req);
    mb_req_send(g_mb_hdl, req, dst_id, MB_MSG_GET, NULL, NULL, 0, NULL);

    if(is_sync)
    {
        pthread_mutex_lock(&req->mutex);
        while (!req->event_true)
        {
            pthread_cond_wait(&req->cond, &req->mutex);
        }
        pthread_mutex_unlock(&req->mutex);
        mb_req_unref(req);
    
    }

    return 0;
}

int mb_cfg_set(int msg_id
        ,char *dst_id
        , char *user
        , char *pass
        , int  timeout
        , int struct_size
        , void *struct_buf
        , mb_cu_notify_t *complete_cb
        , int is_sync)

{
    mb_req_t *req = NULL;
    int ttl = timeout >= 0? timeout : 0;

    req = mb_req_new(msg_id
            , seq_generate(g_mb_hdl->seq)
            , abs_time_gen(ttl)
            , NORMAL_REQ
            , complete_cb);
    if(!req)
    {
        return -1;
    }
    
    if(is_sync)
    {
        mb_req_ref(req);
    }    
    mb_hdl_add(g_mb_hdl, req);
    mb_req_send(g_mb_hdl, req, dst_id, MB_MSG_SET, user, pass, struct_size, struct_buf);

    if(is_sync)
    {
        pthread_mutex_lock(&req->mutex);
        while (!req->event_true)
        {
            pthread_cond_wait(&req->cond, &req->mutex);
        }
        pthread_mutex_unlock(&req->mutex);
        mb_req_unref(req);
    
    }
    return 0;
}


int pu_mb_init(char *addr
            , unsigned int port
            , pu_process_cb_t *cb)
{
    printf("\n\n"
        "= = = = = = = = = = = = = = = = = = =\n"
        "\n"
        "module\t:%s\n"
        "version\t:%s\n"
        "date\t:"__DATE__"  "__TIME__"\n"
        "\n"
        "= = = = = = = = = = = = = = = = = = =\n"
        "\n\n"
        , MB_NAME
        , MB_VERSION);

    g_pu_mb_hdl = pu_mb_hdl_new(addr, port, cb);
    if(g_pu_mb_hdl == NULL)
    {
        printf("mb pu init error!\n");
        return -1;
    }
    printf("mb pu init success!\n");
    return 0;
}

int pu_mb_uinit(void)
{
    pu_mb_hdl_del(g_pu_mb_hdl);
    return 0;
}




