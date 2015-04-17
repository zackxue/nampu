#include <string.h>
#include <unistd.h>

#include "nmp_proxy_device.h" 

#include "nmp_jny_handler.h"
#include "nmp_jny_service.h"
#include "nmp_jny_srv_impl.h"



handler_table_t jny_handler_table[MAX_SDK_CONFIG] =
{
	{ GET_DEVICE_CONFIG,	jny_get_device_info },
	{ GET_SERIAL_CONFIG, 	jny_get_serial_info },
	{ SET_SERIAL_CONFIG,    jny_set_serial_info },
	{ GET_DEVICE_TIME,      jny_get_device_time },
	{ SET_DEVICE_TIME,      jny_set_device_time },
	{ GET_NTP_CONFIG,       jny_get_ntp_info },
	{ SET_NTP_CONFIG,       jny_set_ntp_info },
	{ GET_NETWORK_CONFIG,	jny_get_network_info },
	{ SET_NETWORK_CONFIG,   jny_set_network_info },
	{ GET_PPPOE_CONFIG,     jny_get_pppoe_info },
	{ SET_PPPOE_CONFIG,     jny_set_pppoe_info },
	{ GET_FTP_CONFIG,       jny_get_ftp_info },
	{ SET_FTP_CONFIG,       jny_set_ftp_info },
	{ GET_SMTP_CONFIG,      jny_get_smtp_info },
	{ SET_SMTP_CONFIG,      jny_set_smtp_info },
	{ GET_DDNS_CONFIG,      jny_get_ddns_info },
	{ SET_DDNS_CONFIG,      jny_set_ddns_info },
	{ GET_UPNP_CONFIG,      jny_get_upnp_info },
	{ SET_UPNP_CONFIG,		NULL },
	{ GET_DISK_LIST,        jny_get_disk_list },
	{ SET_DISK_FORMAT,      jny_format_disk },
	{ CONTROL_DEVICE_CMD,   jny_control_device },
	
	{ GET_ENCODE_CONFIG,    jny_get_encode_info },
	{ SET_ENCODE_CONFIG,    jny_set_encode_info },

	{ GET_DISPLAY_CONFIG,   NULL },
	{ SET_DISPLAY_CONFIG,   jny_set_display_info },
	{ GET_OSD_CONFIG,       jny_get_osd_info },
	{ SET_OSD_CONFIG,       jny_set_osd_info },
	{ GET_PTZ_CONFIG,       jny_get_ptz_info },
	{ SET_PTZ_CONFIG,       jny_set_ptz_info },
	{ GET_RECORD_CONFIG,    jny_get_record_info },
	{ SET_RECORD_CONFIG,    jny_set_record_info },
	{ GET_HIDE_CONFIG,      jny_get_hide_info },
	{ SET_HIDE_CONFIG,      jny_set_hide_info },
	{ GET_MOTION_CONFIG,    jny_get_move_alarm_info },
	{ SET_MOTION_CONFIG,    jny_set_move_alarm_info },

	{ GET_VIDEO_LOST_CONFIG, jny_get_video_lost_info },
	{ SET_VIDEO_LOST_CONFIG, jny_set_video_lost_info },
	{ GET_HIDE_ALARM_CONFIG, jny_get_hide_alarm_info },
	{ SET_HIDE_ALARM_CONFIG, jny_set_hide_alarm_info },
	{ GET_IO_ALARM_CONFIG,	 NULL },
	{ SET_IO_ALARM_CONFIG,	 NULL },
	
	{ GET_STORE_LOG,		 NULL },
	
	{ CONTROL_PTZ_CMD,		 NULL },
	{ SET_PRESET_CONFIG,	 NULL },
	{ GET_CRUISE_CONFIG,	 NULL },
	{ SET_CRUISE_CONFIG,	 NULL },
	{ ADD_CRUISE_CONFIG,	 NULL },
	{ MDF_CRUISE_CONFIG,     NULL },
	{ GET_CAPABILITY_SET, 	jny_get_capability_list },
};


