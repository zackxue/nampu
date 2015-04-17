#include <string.h>
#include <unistd.h>

#include "nmp_proxy_device.h" 

#include "nmp_hik_handler.h"
#include "nmp_hik_service.h"
#include "nmp_hik_srv_impl.h"


handler_table_t hik_handler_table[MAX_SDK_CONFIG] = 
{
    { GET_DEVICE_CONFIG,     hik_get_device_info },
    { GET_SERIAL_CONFIG,     hik_get_serial_info },
    { SET_SERIAL_CONFIG,     hik_set_serial_info },
    { GET_DEVICE_TIME,       hik_get_device_time },
    { SET_DEVICE_TIME,       hik_set_device_time },
    { GET_NTP_CONFIG,        hik_get_ntp_info },
    { SET_NTP_CONFIG,        hik_set_ntp_info },
    { GET_NETWORK_CONFIG,    hik_get_network_info },
    { SET_NETWORK_CONFIG,    hik_set_network_info },
    { GET_PPPOE_CONFIG,      hik_get_pppoe_info },
    { SET_PPPOE_CONFIG,      hik_set_pppoe_info },
    { GET_FTP_CONFIG,        hik_get_ftp_info },
    { SET_FTP_CONFIG,        hik_set_ftp_info },
    { GET_SMTP_CONFIG,       hik_get_smtp_info },
    { SET_SMTP_CONFIG,       hik_set_smtp_info },
    { GET_DDNS_CONFIG,       hik_get_ddns_info },
    { SET_DDNS_CONFIG,       hik_set_ddns_info },
    { GET_UPNP_CONFIG,       hik_get_upnp_info },
    { SET_UPNP_CONFIG,       hik_set_upnp_info },
    { GET_DISK_LIST,         hik_get_disk_list},
    { SET_DISK_FORMAT,       hik_format_disk },
    { CONTROL_DEVICE_CMD,    hik_control_device },

    { GET_ENCODE_CONFIG,     hik_get_encode_info },
    { SET_ENCODE_CONFIG,     hik_set_encode_info },
    { GET_DISPLAY_CONFIG,    hik_get_display_info },
    { SET_DISPLAY_CONFIG,    hik_set_display_info },
    { GET_OSD_CONFIG,        hik_get_osd_info },
    { SET_OSD_CONFIG,        hik_set_osd_info },
    { GET_PTZ_CONFIG,        hik_get_ptz_info },
    { SET_PTZ_CONFIG,        hik_set_ptz_info },
    { GET_RECORD_CONFIG,     hik_get_record_info },
    { SET_RECORD_CONFIG,     hik_set_record_info },
    { GET_HIDE_CONFIG,       hik_get_hide_info },
    { SET_HIDE_CONFIG,       hik_set_hide_info },

    { GET_MOTION_CONFIG,     hik_get_move_alarm_info },
    { SET_MOTION_CONFIG,     hik_set_move_alarm_info },
    { GET_VIDEO_LOST_CONFIG, hik_get_video_lost_info },
    { SET_VIDEO_LOST_CONFIG, hik_set_video_lost_info },
    { GET_HIDE_ALARM_CONFIG, hik_get_hide_alarm_info },
    { SET_HIDE_ALARM_CONFIG, hik_set_hide_alarm_info },
    { GET_IO_ALARM_CONFIG,   hik_get_io_alarm_info },
    { SET_IO_ALARM_CONFIG,   hik_set_io_alarm_info },

    { GET_STORE_LOG,         hik_get_store_log },

    { CONTROL_PTZ_CMD,       hik_control_ptz },
    { SET_PRESET_CONFIG,     hik_set_preset_point },
    { GET_CRUISE_CONFIG,     hik_get_cruise_way },
    { SET_CRUISE_CONFIG,     hik_set_cruise_way },
    { ADD_CRUISE_CONFIG,     hik_add_cruise_way },
    { MDF_CRUISE_CONFIG,     hik_modify_cruise_way },

    { GET_CAPABILITY_SET,    hik_get_capability_set },
};

static __inline__ JTime *
get_local_time(JTime *ts)
{
    time_t t;
    struct tm *local;

    t = time(NULL);
    local = localtime(&t);

    ts->year   = local->tm_year;
    ts->month  = local->tm_mon + 1;
    ts->date   = local->tm_mday;
    ts->hour   = local->tm_hour;
    ts->minute = local->tm_min;
    ts->second = local->tm_sec;

    return ts;
}

void hik_exception_call_back(DWORD type, LONG user_id, LONG handle, void *user_data)
{
printf("\n>>>>>---------------------- 0x%x ----------------------<<<<<\n\n", type);
    proxy_device_t *prx_dev;
    hik_service_t *hik_srv;

    if (HIK_LOGOUT == user_id)
        return ;

    switch (type)
    {
        case EXCEPTION_EXCHANGE: //用户交互时异常（注册心跳超时，心跳间隔为2分钟）
            show_info("--------------> connect die [hik user id: %d] <--------------\n", user_id);

            prx_dev = proxy_find_device_by_user_id((int)user_id, 
                        g_sdk_items[SDK_HIK].sdk_name);
            if (prx_dev)
            {
                proxy_ctrl_t *ctrl;
                proxy_task_t *task;

                hik_srv = (hik_service_t*)prx_dev->sdk_srv;
                ctrl = proxy_new_ctrl((struct service*)hik_srv, CTRL_CMD_LOGOUT, NULL);
                if (ctrl)
                {
                    task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                            proxy_free_ctrl, hik_srv->owner);
                    if (task)
                    {
                        hik_set_state(hik_srv, HIK_LOGING, -1);
                        proxy_thread_pool_push(task);
                    }
                    else
                    {
                        proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
                    }
                }
                proxy_device_ref(prx_dev);
            }
            break;

        case EXCEPTION_AUDIOEXCHANGE:       //语音对讲异常
            break;

        case EXCEPTION_ALARM:               //报警异常
            break;

        case EXCEPTION_PREVIEW:             //网络预览异常
            break;

        case EXCEPTION_SERIAL:              //透明通道异常
            break;

        case EXCEPTION_RECONNECT:           //预览时重连
            break;

        case EXCEPTION_ALARMRECONNECT:      //报警时重连
            break;

        case EXCEPTION_SERIALRECONNECT:     //透明通道重连
            break;

        case SERIAL_RECONNECTSUCCESS:       //透明通道重连成功
            break;

        case EXCEPTION_PLAYBACK:            //回放异常
            break;
            
        case EXCEPTION_DISKFMT:             //硬盘格式化
            break;

        case EXCEPTION_PASSIVEDECODE:       //被动解码异常
            break;

        case EXCEPTION_EMAILTEST:           //邮件测试异常
            break;

        case EXCEPTION_BACKUP:              //备份异常
            break;

        case PREVIEW_RECONNECTSUCCESS:      //预览时重连成功
            break;

        case ALARM_RECONNECTSUCCESS:        //报警时重连成功
            break;

        case RESUME_EXCHANGE:               //用户交互恢复
            //Never
            printf("-------------- reconnect ok [user id: %d] --------------\n", user_id);
            break;
            
        default:
            break;
    }
}

BOOL hik_message_call_back(LONG cmd, char *dev_ip, char *buf, DWORD buf_len)
{
    int i;
    msg_t *msg = NULL;
    proxy_device_t *dev;
    hik_service_t *hik_srv;

    NET_DVR_ALARMINFO *alarm;
    SubmitAlarmPacket submit;

    NMP_ASSERT(dev_ip && buf);

    memset(&submit, 0, sizeof(SubmitAlarmPacket));

    dev = proxy_find_device_by_dev_ip(dev_ip);
    if (!dev)
        return FALSE;
    else
        hik_srv = (hik_service_t*)dev->sdk_srv;

    if (COMM_ALARM == cmd)
    {
        alarm = (NET_DVR_ALARMINFO*)buf;
        switch (alarm->dwAlarmType)
        {
            case 0://信号量报警
                break;
            case 1://硬盘满
                break;
            case 2://信号丢失
                for (i=0; i<MAX_CHANNUM; i++)
                {
                    if (alarm->dwChannel[i])
                    {
                        submit.channel     = i;
                        submit.alarm_type  = J_SDK_LOST_ALARM;
                        submit.action_type = 0;//0：开始告警，1：结束告警
                        strcpy(submit.data, "Alarm form hikvision device [video lost]");
                        get_local_time(&submit.alarm_time);

                        msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                                &submit, sizeof(SubmitAlarmPacket), 0);

                        dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                            dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                        free_msg(msg);
                    }
                }
                break;
            case 3://移动侦测
                for (i=0; i<MAX_CHANNUM; i++)
                {
                    if (alarm->dwChannel[i])
                    {
                        submit.channel     = i;
                        submit.alarm_type  = J_SDK_MOVE_ALARM;
                        submit.action_type = 0;//0：开始告警，1：结束告警
                        strcpy(submit.data, "Alarm form hikvision device [motion dection]");
                        get_local_time(&submit.alarm_time);

                        msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                                &submit, sizeof(SubmitAlarmPacket), 0);

                        dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                            dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                        free_msg(msg);
                    }
                }
                break;
            case 4://硬盘未格式化
                break;
            case 5://读写硬盘出错
                break;
            case 6://遮挡报警
                for (i=0; i<MAX_CHANNUM; i++)
                {
                    if (alarm->dwChannel[i])
                    {
                        submit.channel     = i;
                        submit.alarm_type  = J_SDK_HIDE_ALARM;
                        submit.action_type = 0;//0：开始告警，1：结束告警
                        strcpy(submit.data, "Alarm form hikvision device [hide alarm]");
                        get_local_time(&submit.alarm_time);

                        msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                                &submit, sizeof(SubmitAlarmPacket), 0);

                        dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                            dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                        free_msg(msg);
                    }
                }
                break;
            case 7://制式不匹配
                break;
            case 8://非法访问
                break;
            default:
                break;
        }
    }

    proxy_device_unref(dev);
    return TRUE;
}


