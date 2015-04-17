#ifndef __J_JNY_SERVICE_H__
#define __J_JNY_SERVICE_H__

#include "NvdcDll.h"

#include "nmp_msg_impl.h"
#include "nmp_proxy_cond.h"
#include "nmp_stream_operation.h"

#include "nmp_proxy_server.h"
#include "nmp_proxy_device.h"

#include "nmp_jny_sdkinfo.h"



//typedef void (*exception_cb)(DWORD type, LONG user_id, LONG handle, void *user_data);
//typedef BOOL (*message_cb)(LONG cmd, char *dev_ip, char *buf, DWORD buf_len);
#define jny_print_error(err) \
		do{\
			char szErrorInfo[512] = {0}; \
			Remote_Nvd_formatMessage(err, szErrorInfo, 512); \
			show_debug("%s, %d\n", szErrorInfo, err);\
		}while(0)


typedef struct _jny_conn_info
{
    int conn_wait_time;
    int conn_try_times;
    int reconn_interval;
    int reconn_enable;
    //exception_cb ex_cb;
    //message_cb   ms_cb;
}jny_conn_info;

typedef struct _jny_service_basic
{
    service_template_t tm;
    remote_operate_t   ro;

    int        total_count;     /* 统计海康服务总数*/
    jny_conn_info conn_info;
}jny_service_basic;


typedef struct _jngy_real_strm_info
{
    stream_info_t strm_base;
    proxy_cond_t *cond;
}jngy_real_strm_info;


typedef struct _jny_real_strm_info
{
    stream_info_t strm_base;
    proxy_cond_t *cond;
}jny_real_strm_info;


typedef struct _jny_rec_strm_info
{
    stream_info_t strm_base;

    void         *pos_timer;
    //NET_DVR_TIME  start_time;
    //NET_DVR_TIME  stop_time;
}jny_rec_strm_info;



typedef struct _jny_parm
{
    nmp_mutex_t *lock;

    unsigned int user_id;
    unsigned int state;
    unsigned int state_timer;
    unsigned int deliver;
}jny_parm;


typedef struct _jny_rtsp
{
    //nmp_mutex_t *lock;

    stream_list_t real_strm_list;
    stream_list_t rec_strm_list;
}jny_rtsp;


typedef struct _jny_service
{
    struct service base;

    jny_parm     parm;
    jny_rtsp     rtsp;
    proxy_device_t*owner;			/* point back to device object */

    //NET_DVR_DEVICEINFO_V30 hik_dev_info;
}jny_service;

#ifdef __cplusplus
	extern "C" {
#endif

int jny_get_state(jny_parm *pm, int *state_timer = NULL);
int jny_set_state(jny_service *jny_srv, JNY_STATE_E state, int error);
int jny_get_user_id(jny_parm *pm);
void jny_cleanup_stream_info( jny_rtsp *rm);
void jny_stop_all_stream(jny_rtsp *rm);
int jny_set_user_id(jny_service *jny_srv, int user_id);


#ifdef __cplusplus
	}
#endif

#endif
