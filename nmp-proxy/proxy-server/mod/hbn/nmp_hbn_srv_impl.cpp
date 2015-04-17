#include <string.h>
#include <unistd.h>

#include "nmp_proxy_device.h" 

#include "nmp_hbn_handler.h"
#include "nmp_hbn_service.h"
#include "nmp_hbn_srv_impl.h"


handler_table_t hbn_handler_table[MAX_SDK_CONFIG] = 
{
    { GET_DEVICE_CONFIG,     hbn_get_device_info },
    { GET_SERIAL_CONFIG,     hbn_get_serial_info },
    { SET_SERIAL_CONFIG,     hbn_set_serial_info },
    { GET_DEVICE_TIME,       hbn_get_device_time },
    { SET_DEVICE_TIME,       hbn_set_device_time },
    { GET_NTP_CONFIG,        hbn_get_ntp_info },
    { SET_NTP_CONFIG,        hbn_set_ntp_info },
    { GET_NETWORK_CONFIG,    hbn_get_network_info },
    { SET_NETWORK_CONFIG,    hbn_set_network_info },
    { GET_PPPOE_CONFIG,      hbn_get_pppoe_info },
    { SET_PPPOE_CONFIG,      hbn_set_pppoe_info },
    { GET_FTP_CONFIG,        hbn_get_ftp_info },
    { SET_FTP_CONFIG,        hbn_set_ftp_info },
    { GET_SMTP_CONFIG,       hbn_get_smtp_info },
    { SET_SMTP_CONFIG,       hbn_set_smtp_info },
    { GET_DDNS_CONFIG,       hbn_get_ddns_info },
    { SET_DDNS_CONFIG,       hbn_set_ddns_info },
    { GET_UPNP_CONFIG,       hbn_get_upnp_info },
    { SET_UPNP_CONFIG,       hbn_set_upnp_info },
    { GET_DISK_LIST,         hbn_get_disk_list},
    { SET_DISK_FORMAT,       hbn_format_disk },
    { CONTROL_DEVICE_CMD,    hbn_control_device },

    { GET_ENCODE_CONFIG,     hbn_get_encode_info },
    { SET_ENCODE_CONFIG,     hbn_set_encode_info },
    { GET_DISPLAY_CONFIG,    hbn_get_display_info },
    { SET_DISPLAY_CONFIG,    hbn_set_display_info },
    { GET_OSD_CONFIG,        hbn_get_osd_info },
    { SET_OSD_CONFIG,        hbn_set_osd_info },
    { GET_PTZ_CONFIG,        hbn_get_ptz_info },
    { SET_PTZ_CONFIG,        hbn_set_ptz_info },
    { GET_RECORD_CONFIG,     hbn_get_record_info },
    { SET_RECORD_CONFIG,     hbn_set_record_info },
    { GET_HIDE_CONFIG,       hbn_get_hide_info },
    { SET_HIDE_CONFIG,       hbn_set_hide_info },

    { GET_MOTION_CONFIG,     hbn_get_move_alarm_info },
    { SET_MOTION_CONFIG,     hbn_set_move_alarm_info },
    { GET_VIDEO_LOST_CONFIG, hbn_get_video_lost_info },
    { SET_VIDEO_LOST_CONFIG, hbn_set_video_lost_info },
    { GET_HIDE_ALARM_CONFIG, hbn_get_hide_alarm_info },
    { SET_HIDE_ALARM_CONFIG, hbn_set_hide_alarm_info },
    { GET_IO_ALARM_CONFIG,   hbn_get_io_alarm_info },
    { SET_IO_ALARM_CONFIG,   hbn_set_io_alarm_info },

    { GET_STORE_LOG,         hbn_get_store_log },

    { CONTROL_PTZ_CMD,       hbn_ptz_control },
    { SET_PRESET_CONFIG,     hbn_set_preset_point },
    { GET_CRUISE_CONFIG,     hbn_get_cruise_way },
    { SET_CRUISE_CONFIG,     hbn_set_cruise_way },
    { ADD_CRUISE_CONFIG,     hbn_add_cruise_way },
    { MDF_CRUISE_CONFIG,     hbn_modify_cruise_way },

    { GET_CAPABILITY_SET,    hbn_get_capability_set },
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

void hbn_process_alarm_info(long command, HB_NET_ALARM *alarm_info)
{
    LONG user_id;
    msg_t *msg = NULL;
    proxy_device_t *dev;
    hbn_service_t *hbn_srv;

    SubmitAlarmPacket submit;
    HB_NET_ALARMOUT *alarm_out;

    NMP_ASSERT(alarm_info);

    alarm_out = (HB_NET_ALARMOUT*)alarm_info->pBuffer;
    user_id = alarm_info->lUserID;

    dev = proxy_find_device_by_user_id(user_id, g_sdk_items[SDK_HBN].sdk_name);
    if (!dev)
        return ;
    else
        hbn_srv = (hbn_service_t*)dev->sdk_srv;

    if (COMM_ALARM == command)
    {
        int i;
        for (i=0; i<HB_MAX_CHANNUM; i++)
        {
            if (alarm_out->byMotion[i])
            {
                submit.alarm_type  = J_SDK_MOVE_ALARM;
                submit.channel     = i;
                submit.action_type = 0;
                strcpy(submit.data, "Alarm form hbgk device [motion dection]");
                get_local_time(&submit.alarm_time);

                msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                        &submit, sizeof(SubmitAlarmPacket), 0);

                dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                    dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                free_msg(msg);
            }

            if (alarm_out->byVlost[i])
            {
                submit.alarm_type  = J_SDK_LOST_ALARM;
                submit.channel     = i;
                submit.action_type = 0;
                strcpy(submit.data, "Alarm form hbgk device [video lost]");
                get_local_time(&submit.alarm_time);

                msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                        &submit, sizeof(SubmitAlarmPacket), 0);

                dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                    dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                free_msg(msg);
            }

            if (alarm_out->byHide[i])
            {
                submit.alarm_type  = J_SDK_HIDE_ALARM;
                submit.channel     = i;
                submit.action_type = 0;
                strcpy(submit.data, "Alarm form hbgk device [hide alarm]");
                get_local_time(&submit.alarm_time);

                msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                        &submit, sizeof(SubmitAlarmPacket), 0);

                dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                    dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                free_msg(msg);
            }

            if (alarm_out->byAlarm[i])
            {
                submit.alarm_type  = J_SDK_IO_ALARM;
                submit.channel     = i;
                submit.action_type = 0;
                strcpy(submit.data, "Alarm form hbgk device [external alarm]");
                get_local_time(&submit.alarm_time);

                msg = alloc_new_msg(SUBMIT_ALARM_REQUEST_ID, 
                        &submit, sizeof(SubmitAlarmPacket), 0);

                dev->plt_srv->tm->control_service(dev->plt_srv->tm, 
                    dev->plt_srv, CTRL_CMD_SUBMIT, msg);

                free_msg(msg);
            }
        }
    }
    else if (COMM_CONNECT == command)
    {
        show_info("--------------> connect die [hb user id: %d] <--------------\n", (int)user_id);
        
        proxy_ctrl_t *ctrl;
        proxy_task_t *task;

        ctrl = proxy_new_ctrl((struct service*)hbn_srv, CTRL_CMD_LOGOUT, NULL);
        if (ctrl)
        {
            task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                    proxy_free_ctrl, hbn_srv->owner);
            if (task)
            {
                hbn_set_state(hbn_srv, HBN_LOGING, -1);
                proxy_thread_pool_push(task);
            }
            else
            {
                proxy_free_ctrl((void*)ctrl, sizeof(ctrl));
            }
        }
    }

    proxy_device_unref(dev);
    return ;
}




