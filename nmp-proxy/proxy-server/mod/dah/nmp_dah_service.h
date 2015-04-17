#ifndef __DAH_SERVICE_H__
#define __DAH_SERVICE_H__

#include "dhnetsdk.h"
#include "dhconfigsdk.h"

#include "nmp_msg_impl.h"
#include "nmp_stream_operation.h"

#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"

#include "nmp_dah_sdkinfo.h"


typedef void (*dis_conn_cb)(long int user_id, char *dev_ip, int dev_port, long int user_data);
typedef void (*have_reconn_cb)(LONG user_id, char *dev_ip, LONG dev_port, DWORD user_data);


typedef struct dah_conn_info dah_conn_t;

struct dah_conn_info
{
    int conn_wait_time;         //SDK默认值为3000ms,对外网一般需要增加到5000ms
    int conn_try_times;         //连接次数,暂时为无效值,填NULL. 
    dis_conn_cb dis_conn;       //断线回调函数
    have_reconn_cb have_reconn; //断线重连成功的回调函数
};

struct dah_service_basic
{
    service_template_t tm;
    remote_operate_t   ro;

    int        total_count;
    dah_conn_t conn_info;
};

typedef struct dah_real_strm_info dah_real_strm_t;

struct dah_real_strm_info
{
    stream_info_t strm_base;
};

typedef struct dah_rec_strm_info dah_rec_strm_t;

struct dah_rec_strm_info
{
    stream_info_t strm_base;

    int enable;
    NET_TIME start_time;
    NET_TIME stop_time;
};

typedef struct dah_parm dah_parm_t;

struct dah_parm
{
    nmp_mutex_t *lock;

    unsigned int user_id;
    unsigned int state;
    unsigned int state_timer;
};

typedef struct dah_rtsp dah_rtsp_t;

struct dah_rtsp
{
    nmp_mutex_t *lock;

    stream_list_t real_strm_list;
    stream_list_t rec_strm_list;
};

typedef struct dah_service dah_service_t;

struct dah_service
{
    struct service base;

    dah_parm_t     parm;
    dah_rtsp_t     rtsp;
    proxy_device_t*owner;           /* point back to device object */

    NET_DEVICEINFO dah_info;
};


#ifdef __cplusplus
extern "C" {
#endif

int dah_get_state(dah_parm_t *pm, int *state_timer = NULL);
int dah_set_state(dah_service_t *dah_srv, DAH_STATE_E state, int error);

int dah_get_user_id(dah_parm_t *pm);
int dah_set_user_id(dah_service_t *dah_srv, int user_id);

void dah_stop_all_stream(dah_rtsp_t *rm);
void dah_cleanup_stream_info(dah_rtsp_t *rm);


#ifdef __cplusplus
        }
#endif


#endif  //__DAH_SERVICE_H__


