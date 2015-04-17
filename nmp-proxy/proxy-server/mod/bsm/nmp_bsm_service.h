#ifndef __BSM_SERVICE_H__
#define __BSM_SERVICE_H__

#include "hi_net_dev_sdk.h"
#include "hi_net_dev_errors.h"

#include "nmp_msg_impl.h"
#include "nmp_proxy_cond.h"
#include "nmp_stream_operation.h"

#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"

#include "nmp_bsm_sdkinfo.h"



struct bsm_service_basic
{
    service_template_t tm;
    remote_operate_t   ro;

    int total_count;
};

typedef struct bsm_real_strm_info bsm_real_strm_t;

struct bsm_real_strm_info
{
    stream_info_t strm_base;
    proxy_cond_t *cond;
};

typedef struct bsm_rec_strm_info bsm_rec_strm_t;

struct bsm_rec_strm_info
{
    stream_info_t strm_base;

    int enable;
    int start_time;
    int stop_time;
};

typedef struct bsm_parm bsm_parm_t;

struct bsm_parm
{
    nmp_mutex_t *lock;

    unsigned int user_id;
    unsigned int state;
    unsigned int state_timer;
};

typedef struct bsm_rtsp bsm_rtsp_t;

struct bsm_rtsp
{
    nmp_mutex_t *lock;

    stream_list_t real_strm_list;
    stream_list_t rec_strm_list;
};

typedef struct bsm_service bsm_service_t;

struct bsm_service
{
    struct service base;

    bsm_parm_t     parm;
    bsm_rtsp_t     rtsp;
    proxy_device_t*owner;           /* point back to device object */
};


#ifdef __cplusplus
extern "C" {
#endif

int bsm_get_state(bsm_parm_t *pm, int *state_timer = NULL);
int bsm_set_state(bsm_service_t *bsm_srv, BSM_STATE_E state, int error);

int bsm_get_user_id(bsm_parm_t *pm);
int bsm_set_user_id(bsm_service_t *bsm_srv, int user_id);

void bsm_stop_all_stream(bsm_rtsp_t *rm);
void bsm_cleanup_stream_info(bsm_rtsp_t *rm);


#ifdef __cplusplus
        }
#endif


#endif  //__BSM_SERVICE_H__


