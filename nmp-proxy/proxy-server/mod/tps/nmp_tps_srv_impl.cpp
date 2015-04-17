#include <string.h>
#include <unistd.h>

#include "nmp_proxy_device.h" 

#include "nmp_tps_handler.h"
#include "nmp_tps_service.h"
#include "nmp_tps_srv_impl.h"


handler_table_t tps_handler_table[MAX_SDK_CONFIG] = 
{
    { GET_DEVICE_CONFIG,     tps_get_device_info },
    { GET_SERIAL_CONFIG,     tps_get_serial_info },
    { SET_SERIAL_CONFIG,     tps_set_serial_info },
    { GET_DEVICE_TIME,       tps_get_device_time },
    { SET_DEVICE_TIME,       tps_set_device_time },
    { GET_NTP_CONFIG,        tps_get_ntp_info },
    { SET_NTP_CONFIG,        tps_set_ntp_info },
    { GET_NETWORK_CONFIG,    tps_get_network_info },
    { SET_NETWORK_CONFIG,    tps_set_network_info },
    { GET_PPPOE_CONFIG,      tps_get_pppoe_info },
    { SET_PPPOE_CONFIG,      tps_set_pppoe_info },
    { GET_FTP_CONFIG,        tps_get_ftp_info },
    { SET_FTP_CONFIG,        tps_set_ftp_info },
    { GET_SMTP_CONFIG,       tps_get_smtp_info },
    { SET_SMTP_CONFIG,       tps_set_smtp_info },
    { GET_DDNS_CONFIG,       tps_get_ddns_info },
    { SET_DDNS_CONFIG,       tps_set_ddns_info },
    { GET_UPNP_CONFIG,       tps_get_upnp_info },
    { SET_UPNP_CONFIG,       tps_set_upnp_info },
    { GET_DISK_LIST,         tps_get_disk_list},
    { SET_DISK_FORMAT,       tps_format_disk },
    { CONTROL_DEVICE_CMD,    tps_control_device },

    { GET_ENCODE_CONFIG,     tps_get_encode_info },
    { SET_ENCODE_CONFIG,     tps_set_encode_info },
    { GET_DISPLAY_CONFIG,    tps_get_display_info },
    { SET_DISPLAY_CONFIG,    tps_set_display_info },
    { GET_OSD_CONFIG,        tps_get_osd_info },
    { SET_OSD_CONFIG,        tps_set_osd_info },
    { GET_PTZ_CONFIG,        tps_get_ptz_info },
    { SET_PTZ_CONFIG,        tps_set_ptz_info },
    { GET_RECORD_CONFIG,     tps_get_record_info },
    { SET_RECORD_CONFIG,     tps_set_record_info },
    { GET_HIDE_CONFIG,       tps_get_hide_info },
    { SET_HIDE_CONFIG,       tps_set_hide_info },

    { GET_MOTION_CONFIG,     tps_get_move_alarm_info },
    { SET_MOTION_CONFIG,     tps_set_move_alarm_info },
    { GET_VIDEO_LOST_CONFIG, tps_get_video_lost_info },
    { SET_VIDEO_LOST_CONFIG, tps_set_video_lost_info },
    { GET_HIDE_ALARM_CONFIG, tps_get_hide_alarm_info },
    { SET_HIDE_ALARM_CONFIG, tps_set_hide_alarm_info },
    { GET_IO_ALARM_CONFIG,   tps_get_io_alarm_info },
    { SET_IO_ALARM_CONFIG,   tps_set_io_alarm_info },

    { GET_STORE_LOG,         tps_get_store_log },

    { CONTROL_PTZ_CMD,       tps_control_ptz },
    { SET_PRESET_CONFIG,     tps_set_preset_point },
    { GET_CRUISE_CONFIG,     NULL /*tps_get_cruise_way*/ },
    { SET_CRUISE_CONFIG,     NULL /*tps_set_cruise_way*/ },
    { ADD_CRUISE_CONFIG,     NULL /*tps_add_cruise_way*/ },
    { MDF_CRUISE_CONFIG,     NULL /*tps_modify_cruise_way*/ },

    { GET_CAPABILITY_SET,    tps_get_capability_set },
};


void 
tps_reconnect_call_back(long user_id, char *dev_ip, 
    long dev_port, unsigned long user_data)
{
    return ;
}


void 
tps_disconnet_call_back(long user_id, char *dev_ip, 
    long dev_port, unsigned long user_data)
{
    proxy_device_t *dev;
    tps_service_t *tps_srv;

    NMP_ASSERT(TPS_LOGOUT != user_id);

    show_info("--------------> connect die [tps user id: %d] <--------------\n", 
        (int)user_id);

    dev = proxy_find_device_by_user_id(user_id, g_sdk_items[SDK_TPS].sdk_name);
    if (dev)
    {
        proxy_ctrl_t *ctrl;
        proxy_task_t *task;

        tps_srv = (tps_service_t*)dev->sdk_srv;
        ctrl = proxy_new_ctrl((struct service*)tps_srv, CTRL_CMD_LOGOUT, NULL);
        if (ctrl)
        {
            task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                    proxy_free_ctrl, tps_srv->owner);
            if (task)
            {
                tps_set_state(tps_srv, TPS_LOGING, -1);
                proxy_thread_pool_push(task);
            }
            else
            {
                proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
            }
        }

        proxy_device_unref(dev);
    }

    return ;
}


bool 
tps_message_call_back(long user_id, char *buf, unsigned long buf_len, long user)
{
    return FALSE;
}


LONG 
tps_status_event_call_back(LONG user_id, LONG state, char *resp, void *user)
{
    int error = 0;
    tps_service_t *tps_srv;
    tps_service_basic_t *tps_basic = (tps_service_basic_t*)user;
    proxy_device_t *prx_dev;
    show_info("--------------> event_call_back[user id:%d, state:%d] <--------------\n", 
        (int)user_id, (int)state);

    prx_dev = proxy_find_device_by_user_id(user_id, 
                (const char*)tps_basic->tm.service_name);
    if (prx_dev)
        tps_srv = (tps_service_t*)prx_dev->sdk_srv;
    else
        return -1;

    switch (state)
    {
        case EVENT_CONNECTING://连接中
            break;
        case EVENT_CONNECTOK://连接成功
            break;
        case EVENT_LOGINOK://登录成功
            state = TPS_LOGIN;
            goto LOGIN_STATE;

        case EVENT_CONNECTFAILED://连接失败
        case EVENT_SOCKETERROR://sock失败
        case EVENT_LOGINFAILED://登录失败
        case EVENT_LOGIN_USERERROR://登录用户或密码错误
        case EVENT_LOGOUT_FINISH://登录线程已退出主循环
        case EVENT_LOGIN_RECONNECT://进行重新登录
        case EVENT_LOGIN_HEARTBEAT_LOST://心跳丢失
            state = TPS_OUTING;
            error = TPS_OUTING;
            goto LOGIN_STATE;

        case EVENT_STARTAUDIOOK://对讲启动成功
            break;
        case EVENT_STARTAUDIOFAILED://对讲启动失败
            break;
        case EVENT_STOPAUDIOOK://停止对讲成功
            break;
        case EVENT_STOPAUDIOFAILED://停止对讲失败
            break;
        case EVENT_SENDPTZOK://发送云台控制命令成功
            break;
        case EVENT_SENDPTZFAILED://发送云台控制命令失败
            break;
        case EVENT_SENDAUXOK://发送辅助通道成功
            break;
        case EVENT_SENDAUXFAILED://发送辅助通道失败
            break;
        case EVENT_UPLOADOK://上传文件成功
            break;
        case EVENT_UPLOADFAILED://上传文件失败
            break;
        case EVENT_DOWNLOADOK://下载成功
            break;
        case EVENT_DOWNLOADFAILED://下载失败
            break;
        case EVENT_REMOVEOK://删除文件成功
            break;
        case EVENT_REMOVEFAILED://删除文件失败
            break;
        case EVENT_SENDPTZERROR://云台操作失败
            break;
        case EVENT_PTZPRESETINFO://预置点信息
            break;
        case EVNET_PTZNOPRESETINFO://没有预置点
            break;
        case EVENT_PTZALARM://告警信息
            break;
        case EVENT_RECVVIDEOPARAM://视频参数返回
            break;
        case EVENT_RECVAUDIOPARAM://音频参数返回
            break;
        case EVENT_CONNECTRTSPERROR://请求实时视频失败
            break;
        case EVENT_CONNECTRTSPOK://读求媒体流成功
            break;
        case EVENT_RTSPTHREADEXIT://媒体流线程退出
            break;
        case EVENT_URLERROR://解释连接媒体流URL时失败
            break;
        case EVENT_RECVVIDEOAUDIOPARAM://收到音视频参数
            break;
        case EVENT_STARTAUDIO_ISBUSY://设备对讲被占用
            break;
        case EVENT_STARTAUDIO_PARAMERROR://对讲音频参数错误
            break;
        case EVENT_STARTAUDIO_AUDIODDISABLED://设备不支持对讲
            break;
        case EVENT_CONNECT_RTSPSERVER_ERROR://连接错误，可能是IP或端口错误
            break;
        case EVENT_CREATE_RTSPCLIENT_ERROR://创建rtsp客户端错误
            break;
        case EVENT_GET_RTSP_CMDOPTION_ERROR://连接rtsp服务器，返回错误
            break;
        case EVENT_RTSP_AUTHERROR://认证失败，可能是用户或密码错
            break;
    }

    proxy_device_unref(prx_dev);
    return -1;

LOGIN_STATE:
    tps_set_state(tps_srv, (TPS_STATE_E)state, error);
    proxy_device_unref(prx_dev);
    return 0;
}


