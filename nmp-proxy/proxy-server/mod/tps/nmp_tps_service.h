#ifndef __TPS_SERVICE_H__
#define __TPS_SERVICE_H__

#include "NetSDKDLL.h"

#include "nmp_msg_impl.h"
#include "nmp_stream_operation.h"

#include "nmp_proxy_cond.h"
#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"

#include "nmp_tps_sdkinfo.h"


typedef void (*tps_disconn_cb)(long user_id, char *dev_ip, long dev_port, unsigned long user_data);
typedef void (*tps_reconn_cb)(long user_id, char *dev_ip, long dev_port, unsigned long user_data);


typedef struct tps_conn_info tps_conn_t;

struct tps_conn_info
{
    int conn_wait_time;         //SDK默认值为3000ms,对外网一般需要增加到5000ms
    int conn_try_times;         //连接次数,暂时为无效值,填NULL. 
    tps_disconn_cb disconn;     //断线回调函数
    tps_reconn_cb  reconn;      //断线重连成功的回调函数
};

struct tps_service_basic
{
    service_template_t tm;
    remote_operate_t   ro;

    int        total_count;
    tps_conn_t conn_info;
};

typedef struct tps_real_strm_info tps_real_strm_t;

struct tps_real_strm_info
{
    stream_info_t strm_base;
    proxy_cond_t *cond;
};

typedef struct tps_rec_strm_info tps_rec_strm_t;

struct tps_rec_strm_info
{
    stream_info_t strm_base;

    int enable;
    //H264_DVR_TIME start_time;
    //H264_DVR_TIME stop_time;
};

typedef struct tps_parm tps_parm_t;

struct tps_parm
{
    nmp_mutex_t *lock;

    unsigned int user_id;
    unsigned int state;
    unsigned int state_timer;
};

typedef struct tps_rtsp tps_rtsp_t;

struct tps_rtsp
{
    nmp_mutex_t *lock;

    stream_list_t real_strm_list;
    stream_list_t rec_strm_list;
};

typedef struct tps_service tps_service_t;

struct tps_service
{
    struct service base;

    tps_parm_t       parm;
    tps_rtsp_t       rtsp;
    proxy_device_t *owner;           /* point back to device object */

    IP_NET_DVR_DEVICEINFO tps_info;
};


#ifdef __cplusplus
extern "C" {
#endif

int tps_get_state(tps_parm_t *pm, int *state_timer = NULL);
int tps_set_state(tps_service_t *tps_srv, TPS_STATE_E state, int error);

int tps_get_user_id(tps_parm_t *pm);
int tps_set_user_id(tps_service_t *tps_srv, int user_id);

void tps_stop_all_stream(tps_rtsp_t *rm);
void tps_cleanup_stream_info(tps_rtsp_t *rm);


#ifdef __cplusplus
    }
#endif


#endif  //__TPS_SERVICE_H__

