#include <string.h>
#include <unistd.h>

#include "nmp_proxy_device.h" 

#include "nmp_xmt_handler.h"
#include "nmp_xmt_service.h"
#include "nmp_xmt_srv_impl.h"


handler_table_t xmt_handler_table[MAX_SDK_CONFIG] = 
{
    { GET_DEVICE_CONFIG,     xmt_get_device_info },
    { GET_SERIAL_CONFIG,     xmt_get_serial_info },
    { SET_SERIAL_CONFIG,     xmt_set_serial_info },
    { GET_DEVICE_TIME,       xmt_get_device_time },
    { SET_DEVICE_TIME,       xmt_set_device_time },
    { GET_NTP_CONFIG,        xmt_get_ntp_info },
    { SET_NTP_CONFIG,        xmt_set_ntp_info },
    { GET_NETWORK_CONFIG,    xmt_get_network_info },
    { SET_NETWORK_CONFIG,    xmt_set_network_info },
    { GET_PPPOE_CONFIG,      xmt_get_pppoe_info },
    { SET_PPPOE_CONFIG,      xmt_set_pppoe_info },
    { GET_FTP_CONFIG,        xmt_get_ftp_info },
    { SET_FTP_CONFIG,        xmt_set_ftp_info },
    { GET_SMTP_CONFIG,       xmt_get_smtp_info },
    { SET_SMTP_CONFIG,       xmt_set_smtp_info },
    { GET_DDNS_CONFIG,       xmt_get_ddns_info },
    { SET_DDNS_CONFIG,       xmt_set_ddns_info },
    { GET_UPNP_CONFIG,       xmt_get_upnp_info },
    { SET_UPNP_CONFIG,       xmt_set_upnp_info },
    { GET_DISK_LIST,         xmt_get_disk_list},
    { SET_DISK_FORMAT,       xmt_format_disk },
    { CONTROL_DEVICE_CMD,    xmt_control_device },

    { GET_ENCODE_CONFIG,     xmt_get_encode_info },
    { SET_ENCODE_CONFIG,     xmt_set_encode_info },
    { GET_DISPLAY_CONFIG,    xmt_get_display_info },
    { SET_DISPLAY_CONFIG,    xmt_set_display_info },
    { GET_OSD_CONFIG,        xmt_get_osd_info },
    { SET_OSD_CONFIG,        xmt_set_osd_info },
    { GET_PTZ_CONFIG,        xmt_get_ptz_info },
    { SET_PTZ_CONFIG,        xmt_set_ptz_info },
    { GET_RECORD_CONFIG,     xmt_get_record_info },
    { SET_RECORD_CONFIG,     xmt_set_record_info },
    { GET_HIDE_CONFIG,       xmt_get_hide_info },
    { SET_HIDE_CONFIG,       xmt_set_hide_info },

    { GET_MOTION_CONFIG,     xmt_get_move_alarm_info },
    { SET_MOTION_CONFIG,     xmt_set_move_alarm_info },
    { GET_VIDEO_LOST_CONFIG, xmt_get_video_lost_info },
    { SET_VIDEO_LOST_CONFIG, xmt_set_video_lost_info },
    { GET_HIDE_ALARM_CONFIG, xmt_get_hide_alarm_info },
    { SET_HIDE_ALARM_CONFIG, xmt_set_hide_alarm_info },
    { GET_IO_ALARM_CONFIG,   xmt_get_io_alarm_info },
    { SET_IO_ALARM_CONFIG,   xmt_set_io_alarm_info },

    { GET_STORE_LOG,         xmt_get_store_log },

    { CONTROL_PTZ_CMD,       xmt_control_ptz },
    { SET_PRESET_CONFIG,     xmt_set_preset_point },
    { GET_CRUISE_CONFIG,     NULL /*xmt_get_cruise_way*/ },
    { SET_CRUISE_CONFIG,     NULL /*xmt_set_cruise_way*/ },
    { ADD_CRUISE_CONFIG,     NULL /*xmt_add_cruise_way*/ },
    { MDF_CRUISE_CONFIG,     NULL /*xmt_modify_cruise_way*/ },

    { GET_CAPABILITY_SET,    xmt_get_capability_set },
};


void 
xmt_reconnect_call_back(long user_id, char *dev_ip, 
    long dev_port, unsigned long user_data)
{
    return ;
}


void 
xmt_disconnet_call_back(long user_id, char *dev_ip, 
    long dev_port, unsigned long user_data)
{
    proxy_device_t *dev;
    xmt_service_t *xmt_srv;

    NMP_ASSERT(XMT_LOGOUT != user_id);

    show_info("--------------> connect die [xmt user id: %d] <--------------\n", 
        (int)user_id);

    dev = proxy_find_device_by_user_id(user_id, g_sdk_items[SDK_XMT].sdk_name);
    if (dev)
    {
        proxy_ctrl_t *ctrl;
        proxy_task_t *task;

        xmt_srv = (xmt_service_t*)dev->sdk_srv;
        ctrl = proxy_new_ctrl((struct service*)xmt_srv, CTRL_CMD_LOGOUT, NULL);
        if (ctrl)
        {
            task = proxy_new_task(CONTROL_DEVICE, ctrl, sizeof(ctrl), 
                    proxy_free_ctrl, xmt_srv->owner);
            if (task)
            {
                xmt_set_state(xmt_srv, XMT_LOGING, -1);
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
xmt_message_call_back(long user_id, char *buf, 
    unsigned long buf_len, long user)
{
    return FALSE;
}


