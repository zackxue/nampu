#include <string.h>
#include <unistd.h>

#include "nmp_proxy_device.h" 

#include "nmp_dah_handler.h"
#include "nmp_dah_service.h"
#include "nmp_dah_srv_impl.h"


handler_table_t dah_handler_table[MAX_SDK_CONFIG] = 
{
    { GET_DEVICE_CONFIG,     dah_get_device_info },
    { GET_SERIAL_CONFIG,     dah_get_serial_info },
    { SET_SERIAL_CONFIG,     dah_set_serial_info },
    { GET_DEVICE_TIME,       dah_get_device_time },
    { SET_DEVICE_TIME,       dah_set_device_time },
    { GET_NTP_CONFIG,        dah_get_ntp_info },
    { SET_NTP_CONFIG,        dah_set_ntp_info },
    { GET_NETWORK_CONFIG,    dah_get_network_info },
    { SET_NETWORK_CONFIG,    dah_set_network_info },
    { GET_PPPOE_CONFIG,      dah_get_pppoe_info },
    { SET_PPPOE_CONFIG,      dah_set_pppoe_info },
    { GET_FTP_CONFIG,        dah_get_ftp_info },
    { SET_FTP_CONFIG,        dah_set_ftp_info },
    { GET_SMTP_CONFIG,       dah_get_smtp_info },
    { SET_SMTP_CONFIG,       dah_set_smtp_info },
    { GET_DDNS_CONFIG,       dah_get_ddns_info },
    { SET_DDNS_CONFIG,       dah_set_ddns_info },
    { GET_UPNP_CONFIG,       dah_get_upnp_info },
    { SET_UPNP_CONFIG,       dah_set_upnp_info },
    { GET_DISK_LIST,         dah_get_disk_list},
    { SET_DISK_FORMAT,       dah_format_disk },
    { CONTROL_DEVICE_CMD,    dah_control_device },

    { GET_ENCODE_CONFIG,     dah_get_encode_info },
    { SET_ENCODE_CONFIG,     dah_set_encode_info },
    { GET_DISPLAY_CONFIG,    dah_get_display_info },
    { SET_DISPLAY_CONFIG,    dah_set_display_info },
    { GET_OSD_CONFIG,        dah_get_osd_info },
    { SET_OSD_CONFIG,        dah_set_osd_info },
    { GET_PTZ_CONFIG,        dah_get_ptz_info },
    { SET_PTZ_CONFIG,        dah_set_ptz_info },
    { GET_RECORD_CONFIG,     dah_get_record_info },
    { SET_RECORD_CONFIG,     dah_set_record_info },
    { GET_HIDE_CONFIG,       dah_get_hide_info },
    { SET_HIDE_CONFIG,       dah_set_hide_info },

    { GET_MOTION_CONFIG,     dah_get_move_alarm_info },
    { SET_MOTION_CONFIG,     dah_set_move_alarm_info },
    { GET_VIDEO_LOST_CONFIG, dah_get_video_lost_info },
    { SET_VIDEO_LOST_CONFIG, dah_set_video_lost_info },
    { GET_HIDE_ALARM_CONFIG, dah_get_hide_alarm_info },
    { SET_HIDE_ALARM_CONFIG, dah_set_hide_alarm_info },
    { GET_IO_ALARM_CONFIG,   dah_get_io_alarm_info },
    { SET_IO_ALARM_CONFIG,   dah_set_io_alarm_info },

    { GET_STORE_LOG,         dah_get_store_log },

    { CONTROL_PTZ_CMD,       NULL /*dah_control_ptz*/ },
    { SET_PRESET_CONFIG,     NULL /*dah_set_preset_point*/ },
    { GET_CRUISE_CONFIG,     NULL /*dah_get_cruise_way*/ },
    { SET_CRUISE_CONFIG,     NULL /*dah_set_cruise_way*/ },
    { ADD_CRUISE_CONFIG,     NULL /*dah_add_cruise_way*/ },
    { MDF_CRUISE_CONFIG,     NULL /*dah_modify_cruise_way*/ },

    { GET_CAPABILITY_SET,    dah_get_capability_set },
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

void have_reconnect_call_back(LONG user_id, char *dev_ip, LONG dev_port, DWORD user_data)
{
}

void dah_disconnet_call_back(long int user_id, char *dev_ip, int dev_port, long int user_data)
{
    proxy_device_t *dev;
    dah_service_t *dah_srv;

    NMP_ASSERT(DAH_LOGOUT != user_id);
    show_info("--------------> connect die [dah user id: %d] <--------------\n", (int)user_id);

    dev = proxy_find_device_by_user_id(user_id, g_sdk_items[SDK_DAH].sdk_name);
    if (dev)
    {
        proxy_ctrl_t *ctrl;
        proxy_task_t *task;

        dah_srv = (dah_service_t*)dev->sdk_srv;
        ctrl = proxy_new_ctrl((struct service*)dah_srv, CTRL_CMD_LOGOUT, NULL);
        if (ctrl)
        {
            task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                    proxy_free_ctrl, dah_srv->owner);
            if (task)
            {
                dah_set_state(dah_srv, DAH_LOGING, -1);
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

BOOL dah_message_call_back(LONG command, LLONG user_id, char *buf, 
        DWORD buf_len, char *dvr_ip, LONG port, LDWORD user)
{
    int i;
    msg_t *msg = NULL;
    proxy_device_t *dev;
    dah_service_t *dah_srv;

    SubmitAlarmPacket submit;
    NET_CLIENT_STATE *client_state;
    
    NMP_ASSERT(DAH_LOGOUT != user_id);

    memset(&submit, 0, sizeof(SubmitAlarmPacket));

    dev = proxy_find_device_by_user_id(user_id, g_sdk_items[SDK_DAH].sdk_name);
    if (!dev)
        return FALSE;
    else
        dah_srv = (dah_service_t*)dev->sdk_srv;

    switch (command)
    {
        case DH_COMM_ALARM:
            client_state = (NET_CLIENT_STATE*)buf;
            for (i=0; i<16; i++)
            {
                if (client_state->motiondection[i])
                {
                    submit.alarm_type  = J_SDK_MOVE_ALARM;
                    submit.channel     = i;
                    submit.action_type = 0;
                    strcpy(submit.data, "Alarm form dahua device [motion dection]");
                    get_local_time(&submit.alarm_time);

                    msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                            &submit, sizeof(SubmitAlarmPacket), 0);

                    dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                    free_msg(msg);
                }

                if (client_state->videolost[i])
                {
                    submit.alarm_type  = J_SDK_LOST_ALARM;
                    submit.channel     = i;
                    submit.action_type = 0;
                    strcpy(submit.data, "Alarm form dahua device [video lost]");
                    get_local_time(&submit.alarm_time);

                    msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                            &submit, sizeof(SubmitAlarmPacket), 0);

                    dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                    free_msg(msg);
                }

                if (client_state->alarm[i])
                {
                    submit.alarm_type  = J_SDK_IO_ALARM;
                    submit.channel     = i;
                    submit.action_type = 0;
                    strcpy(submit.data, "Alarm form dahua device [external alarm]");
                    get_local_time(&submit.alarm_time);

                    msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                            &submit, sizeof(SubmitAlarmPacket), 0);

                    dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                        dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                    free_msg(msg);
                }
            }
            break;
        case DH_SHELTER_ALARM://视频通道的遮挡报警
            for (i=0; i++<16; *buf++)
            {
                if (1 == *buf)
                {
                    submit.channel     = i;
                    submit.alarm_type  = J_SDK_HIDE_ALARM;
                    submit.action_type = 0;//0：开始告警，1：结束告警
                    strcpy(submit.data, "Alarm form dahua device [shelter alarm]");
                    get_local_time(&submit.alarm_time);

                    msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                            &submit, sizeof(SubmitAlarmPacket), 0);

                    dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                        dev->plt_srv, CTRL_CMD_SUBMIT, msg);
                }
            }
            break;

        default:
            break;
    }

    proxy_device_unref(dev);
    return TRUE;
}



