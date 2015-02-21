#ifndef __jcu_mb__
#define __jcu_mb__

#include "mb_basic.h"
#include "mb_api.h"

#define TV_UNLIMITED   (60*60*24*365)


typedef struct mb_req_s {
    struct list_head list;
    unsigned int    ref_count;
    unsigned int    ttl;        //abs_time (s);
    unsigned int    req_no;
    int             req_type;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    unsigned int    event_true;
    unsigned int    msg_id;
    mb_cu_notify_t  notify;
}mb_req_t;

typedef struct mb_handle_s {
    list_t      *req_list;
    seq_t       *seq;               //ref   
    int         listen_fd;
    char        src_id[32];         //(mac+pid)
    pthread_t   tid;
    pthread_t   timer_tid;
    int         tid_running;
    int         port;
    struct sockaddr_in     addr;
}mb_handle_t;

typedef enum _MB_REQ_TYPE {
      ITEM_REQ = 0x0001   
    , NORMAL_REQ             

}MB_REQ_TYPE_E;




mb_handle_t*
    mb_hdl_new(char *addr
            , unsigned short port
            , seq_t  *seq
            , list_t *req_list);
int mb_hdl_del(mb_handle_t *hdl);

int mb_hdl_add(mb_handle_t *hdl, mb_req_t *req);
int mb_hdl_rm(mb_handle_t *hdl, mb_req_t *req);

mb_req_t*
     mb_req_new(int msg_id
             , unsigned int seq_no
             , unsigned int ttl
             , int req_type
             , mb_cu_notify_t *nfy);

mb_req_t*
     mb_req_ref(mb_req_t *req);
void mb_req_unref(mb_req_t *req);

int  mb_req_send(mb_handle_t *hdl
                    , mb_req_t *req
                    , char *dst_id
                    , int args
                    , char*user
                    , char*pass
                    ,int size
                    , void *buf);

int on_mb_timer(void *_parm);


#endif //__jcu_mb__
