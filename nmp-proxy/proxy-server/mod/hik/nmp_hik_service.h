#ifndef __HIK_SERVICE_H__
#define __HIK_SERVICE_H__

#include "HCNetSDK.h"

#include "nmp_msg_impl.h"
#include "nmp_stream_operation.h"

#include "nmp_proxy_cond.h"
#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"

#include "nmp_hik_sdkinfo.h"



typedef void (*exception_cb)(DWORD type, LONG user_id, LONG handle, void *user_data);
typedef BOOL (*message_cb)(LONG cmd, char *dev_ip, char *buf, DWORD buf_len);

typedef struct hik_conn_info hik_conn_t;

struct hik_conn_info
{
    int conn_wait_time;
    int conn_try_times;
    int reconn_interval;
    int reconn_enable;
    exception_cb ex_cb;
    message_cb   ms_cb;
};

struct hik_service_basic
{
    service_template_t tm;
    remote_operate_t   ro;

    int        total_count;     /* 统计海康服务总数*/
    hik_conn_t conn_info;
};

typedef struct hik_real_strm_info hik_real_strm_t;

struct hik_real_strm_info
{
    stream_info_t strm_base;
    proxy_cond_t *cond;
};

typedef struct hik_rec_strm_info hik_rec_strm_t;

struct hik_rec_strm_info
{
    stream_info_t strm_base;

    void        *pos_timer;
    NET_DVR_TIME start_time;
    NET_DVR_TIME stop_time;
};

typedef struct hik_parm hik_parm_t;

struct hik_parm
{
    nmp_mutex_t *lock;

    unsigned int user_id;
    unsigned int state;
    unsigned int state_timer;
};

typedef struct hik_rtsp hik_rtsp_t;

struct hik_rtsp
{
    nmp_mutex_t *lock;

    stream_list_t real_strm_list;
    stream_list_t rec_strm_list;
};

typedef struct hik_service hik_service_t;

struct hik_service
{
    struct service base;

    int            listen;
    hik_parm_t     parm;
    hik_rtsp_t     rtsp;
    proxy_device_t*owner;           /* point back to device object */
	void	*init_g722_encoder;
	void 	*init_g722_decoder;
    NET_DVR_DEVICEINFO_V30 hik_info;
};


#ifdef __cplusplus
extern "C" {
#endif

int hik_get_state(hik_parm_t *pm, int *state_timer = NULL);
int hik_set_state(hik_service_t *hik_srv, HIK_STATE_E state, int error);

int hik_get_user_id(hik_parm_t *pm);
int hik_set_user_id(hik_service_t *hik_srv, int user_id);

void hik_stop_all_stream(hik_rtsp_t *rm);
void hik_cleanup_stream_info(hik_rtsp_t *rm);


#ifdef __cplusplus
        }
#endif


#endif  //__HIK_SERVICE_H__


