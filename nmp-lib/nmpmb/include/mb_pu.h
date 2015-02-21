#ifndef __jpu_mb__
#define __jpu_mb__

#include "mb_basic.h"
#include "mb_api.h"

#define TV_UNLIMITED   (60*60*24*365)






typedef struct _pu_mb_hdl_s {
    int         eth_active_num;
    eth_addr_t  eth_addr[MB_MAX_ETH_NUM];
    pthread_t tid;
    int tid_running;
    pu_process_cb_t cb;
    struct sockaddr_in     addr;
}pu_mb_hdl_t;





pu_mb_hdl_t*
    pu_mb_hdl_new(char *addr
            , unsigned short port
            , pu_process_cb_t *cb);

int pu_mb_hdl_del(pu_mb_hdl_t *hdl);



#endif //__jpu_mb__

