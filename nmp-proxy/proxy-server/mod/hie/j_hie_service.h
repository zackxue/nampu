#ifndef __HIE_SERVICE_H__
#define __HIE_SERVICE_H__

#include "HieClientUnit.h"
#include "HieClient_Common.h"
#include "HieClient_Configure.h"

#include "nmp_msg_impl.h"
#include "nmp_stream_operation.h"

#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"

#include "j_hie_sdkinfo.h"



struct hie_service_basic
{
    service_template_t tm;
    remote_operate_t   ro;

    int total_count;
};

typedef struct hie_strm_header hie_strm_header_t;

struct hie_strm_header
{
    char head[DEF_MAX_MEDIA_SIZE];
    size_t size;
};

typedef struct hie_real_strm_info hie_real_strm_t;

struct hie_real_strm_info
{
    stream_info_t strm_base;
};

typedef struct hie_rec_strm_info hie_rec_strm_t;

struct hie_rec_strm_info
{
    stream_info_t strm_base;

    int enable;
    TimeInfo start_time;
    TimeInfo stop_time;
};

typedef struct hie_parm hie_parm_t;

struct hie_parm
{
    nmp_mutex_t *lock;
    HUSER user_id;
    unsigned int state;
    unsigned int state_timer;
};

typedef struct hie_rtsp hie_rtsp_t;

struct hie_rtsp
{
    nmp_mutex_t *lock;

    hie_strm_header_t real_header;

    stream_list_t real_strm_list;
    stream_list_t rec_strm_list;
};

typedef struct hie_service hie_service_t;

struct hie_service
{
    struct service base;
    proxy_device_t *owner;           /* point back to device object */

    hie_parm_t parm;
    hie_rtsp_t rtsp;
};


#ifdef __cplusplus
extern "C" {
#endif

int hie_get_state(hie_parm_t *pm, int *state_timer = NULL);
int hie_set_state(hie_service_t *hie_srv, HIE_STATE_E state, int error);

HUSER hie_get_user_id(hie_parm_t *pm);
HUSER hie_set_user_id(hie_service_t *hie_srv, HUSER user_id);

void hie_stop_all_stream(hie_rtsp_t *rm);
void hie_cleanup_stream_info(hie_rtsp_t *rm);


#ifdef __cplusplus
        }
#endif


#endif  //__HIE_SERVICE_H__




