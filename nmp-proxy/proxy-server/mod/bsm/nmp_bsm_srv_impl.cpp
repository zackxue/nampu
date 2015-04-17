#include <string.h>
#include <unistd.h>

#include "nmp_proxy_device.h" 

#include "nmp_bsm_handler.h"
#include "nmp_bsm_service.h"
#include "nmp_bsm_srv_impl.h"


handler_table_t bsm_handler_table[MAX_SDK_CONFIG] = 
{
    { GET_DEVICE_CONFIG,     bsm_get_device_info },
    { GET_SERIAL_CONFIG,     NULL },
    { SET_SERIAL_CONFIG,     NULL },
    { GET_DEVICE_TIME,       NULL },
    { SET_DEVICE_TIME,       NULL },
    { GET_NTP_CONFIG,        NULL },
    { SET_NTP_CONFIG,        NULL },
    { GET_NETWORK_CONFIG,    bsm_get_network_info },
    { SET_NETWORK_CONFIG,    bsm_set_network_info },
    { GET_PPPOE_CONFIG,      NULL },
    { SET_PPPOE_CONFIG,      NULL },
    { GET_FTP_CONFIG,        bsm_get_ftp_info },
    { SET_FTP_CONFIG,        bsm_set_ftp_info },
    { GET_SMTP_CONFIG,       bsm_get_smtp_info },
    { SET_SMTP_CONFIG,       bsm_set_smtp_info },
    { GET_DDNS_CONFIG,       bsm_get_ddns_info },
    { SET_DDNS_CONFIG,       bsm_set_ddns_info },
    { GET_UPNP_CONFIG,       NULL },
    { SET_UPNP_CONFIG,       NULL },
    { GET_DISK_LIST,         bsm_get_disk_list },
    { SET_DISK_FORMAT,       bsm_format_disk },
    { CONTROL_DEVICE_CMD,    bsm_control_device },

    { GET_ENCODE_CONFIG,     bsm_get_encode_info },
    { SET_ENCODE_CONFIG,     bsm_set_encode_info },
    { GET_DISPLAY_CONFIG,    bsm_get_display_info },
    { SET_DISPLAY_CONFIG,    bsm_set_display_info },
    { GET_OSD_CONFIG,        bsm_get_osd_info },
    { SET_OSD_CONFIG,        bsm_set_osd_info },
    { GET_PTZ_CONFIG,        bsm_get_ptz_info },
    { SET_PTZ_CONFIG,        bsm_set_ptz_info },
    { GET_RECORD_CONFIG,     NULL },
    { SET_RECORD_CONFIG,     NULL },
    { GET_HIDE_CONFIG,       NULL },
    { SET_HIDE_CONFIG,       NULL },

    { GET_MOTION_CONFIG,     bsm_get_move_alarm_info },
    { SET_MOTION_CONFIG,     bsm_set_move_alarm_info },
    { GET_VIDEO_LOST_CONFIG, NULL },
    { SET_VIDEO_LOST_CONFIG, NULL },
    { GET_HIDE_ALARM_CONFIG, NULL },
    { SET_HIDE_ALARM_CONFIG, NULL },
    { GET_IO_ALARM_CONFIG,   NULL },
    { SET_IO_ALARM_CONFIG,   NULL },

    { GET_STORE_LOG,         NULL },

    { CONTROL_PTZ_CMD,       bsm_control_ptz },
    { SET_PRESET_CONFIG,     NULL },
    { GET_CRUISE_CONFIG,     NULL },
    { SET_CRUISE_CONFIG,     NULL },
    { ADD_CRUISE_CONFIG,     NULL },
    { MDF_CRUISE_CONFIG,     NULL },

    { GET_CAPABILITY_SET,    bsm_get_capability_set },
};



//The End!
