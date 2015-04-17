#ifndef __XMT_SERVICE_H__
#define __XMT_SERVICE_H__

#include "xmtnetsdk.h"

#include "nmp_msg_impl.h"
#include "nmp_stream_operation.h"

#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"

#include "nmp_xmt_sdkinfo.h"


typedef void (*xmt_disconn_cb)(long user_id, char *dev_ip, long dev_port, unsigned long user_data);
typedef void (*xmt_reconn_cb)(long user_id, char *dev_ip, long dev_port, unsigned long user_data);


typedef struct xmt_conn_info xmt_conn_t;

struct xmt_conn_info
{
    int conn_wait_time;         //SDK默认值为3000ms,对外网一般需要增加到5000ms
    int conn_try_times;         //连接次数,暂时为无效值,填NULL. 
    xmt_disconn_cb dis_conn;    //断线回调函数
    xmt_reconn_cb  have_reconn; //断线重连成功的回调函数
};

struct xmt_service_basic
{
    service_template_t tm;
    remote_operate_t   ro;

    int        total_count;
    xmt_conn_t conn_info;
};

typedef struct xmt_real_strm_info xmt_real_strm_t;

struct xmt_real_strm_info
{
    stream_info_t strm_base;
};

typedef struct xmt_rec_strm_info xmt_rec_strm_t;

struct xmt_rec_strm_info
{
    stream_info_t strm_base;

    int enable;
    H264_DVR_TIME start_time;
    H264_DVR_TIME stop_time;
};

typedef struct xmt_parm xmt_parm_t;

struct xmt_parm
{
    nmp_mutex_t *lock;

    unsigned int user_id;
    unsigned int state;
    unsigned int state_timer;
};

typedef struct xmt_rtsp xmt_rtsp_t;

struct xmt_rtsp
{
    nmp_mutex_t *lock;

    stream_list_t real_strm_list;
    stream_list_t rec_strm_list;
};

typedef struct xmt_service xmt_service_t;

struct xmt_service
{
    struct service base;

    xmt_parm_t       parm;
    xmt_rtsp_t       rtsp;
    proxy_device_t *owner;           /* point back to device object */

    H264_DVR_DEVICEINFO xmt_info;
};


#ifdef __cplusplus
extern "C" {
#endif

int xmt_get_state(xmt_parm_t *pm, int *state_timer = NULL);
int xmt_set_state(xmt_service_t *xmt_srv, XMT_STATE_E state, int error);

int xmt_get_user_id(xmt_parm_t *pm);
int xmt_set_user_id(xmt_service_t *xmt_srv, int user_id);

void xmt_stop_all_stream(xmt_rtsp_t *rm);
void xmt_cleanup_stream_info(xmt_rtsp_t *rm);


#ifdef __cplusplus
    }
#endif


#endif  //__XMT_SERVICE_H__

