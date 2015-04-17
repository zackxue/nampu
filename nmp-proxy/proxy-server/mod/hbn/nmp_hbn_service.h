#ifndef __HBN_SERVICE_H__
#define __HBN_SERVICE_H__

#include "HBNetSDK.h"

#include "nmp_msg_impl.h"
#include "nmp_stream_operation.h"

#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"

#include "nmp_hbn_sdkinfo.h"



struct hbn_service_basic
{
    service_template_t tm;
    remote_operate_t   ro;

    int        total_count;
};

typedef struct hbn_strm_header hbn_strm_header_t;

struct hbn_strm_header
{
    char head[DEF_MAX_MEDIA_SIZE];
    size_t size;
};

typedef struct hbn_real_strm_info hbn_real_strm_t;

struct hbn_real_strm_info
{
    stream_info_t strm_base;
};

typedef struct hbn_rec_strm_info hbn_rec_strm_t;

struct hbn_rec_strm_info
{
    stream_info_t strm_base;

    int enable;
    HB_NET_TIME  start_time;
    HB_NET_TIME  stop_time;
};

typedef struct hbn_parm hbn_parm_t;

struct hbn_parm
{
    nmp_mutex_t *lock;

    unsigned int user_id;
    unsigned int state;
    unsigned int state_timer;
};

typedef struct hbn_rtsp hbn_rtsp_t;

struct hbn_rtsp
{
    nmp_mutex_t *lock;

    hbn_strm_header_t real_header;

    stream_list_t real_strm_list;
    stream_list_t rec_strm_list;
};

typedef struct hbn_service hbn_service_t;

struct hbn_service
{
    struct service base;

    hbn_parm_t     parm;
    hbn_rtsp_t     rtsp;
    proxy_device_t*owner;           /* point back to device object */

    HB_NET_DEVICEINFO hbn_info;
};


#ifdef __cplusplus
extern "C" {
#endif

int hbn_get_state(hbn_parm_t *pm, int *state_timer = NULL);
int hbn_set_state(hbn_service_t *hbn_srv, HBN_STATE_E state, int error);

int hbn_get_user_id(hbn_parm_t *pm);
int hbn_set_user_id(hbn_service_t *hbn_srv, int user_id);

void hbn_stop_all_stream(hbn_rtsp_t *rm);
void hbn_cleanup_stream_info(hbn_rtsp_t *rm);


#ifdef __cplusplus
        }
#endif


#endif  //__HBN_SERVICE_H__



