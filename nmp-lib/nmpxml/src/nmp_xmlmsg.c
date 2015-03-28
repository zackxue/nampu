
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#include "nmp_xmlinfo.h"
#include "nmp_xmlhandler.h"
#include "nmp_xmlmsg.h"
#include "nmp_xmlmem.h"
#include "nmp_packet.h"



extern g_alloc_t g_alloc;
extern g_dealloc_t g_dealloc;

extern char *parse_xml_cmd_by_mxml(const char *xml_buf, 
				char cmd_buf[], size_t size);


static __inline__ void jpf_xml_msg_init(NmpXmlMsg *self)
{
	J_ASSERT(self);

	self->id = 0;
	self->flags = 0;
	self->priv_obj = NULL;
	self->destroy = NULL;
}

void set_mem_handler(void *alloc, void *dealloc)
{
	g_alloc   = (alloc_t)alloc;
	g_dealloc = (dealloc_t)dealloc;
}

void init_jpf_xml_msg()
{
	XmlTable *table = NULL;

	if (NULL != (table = get_xml_table()))
	{
		register_xml_table(table, PU_GET_CSS_REQUEST_ID, 
								  PU_GET_CSS_REQUEST_CMD, 
								  create_get_css_request, 
								  parse_get_css_request, 0);
		register_xml_table(table, PU_GET_CSS_RESPONSE_ID, 
								  PU_GET_CSS_RESPONSE_CMD, 
								  create_get_css_request, 
								  parse_get_css_response, 0);
		register_xml_table(table, REGISTER_CSS_REQUEST_ID, 
								  REGISTER_REQUEST_CMD, 
								  create_pu_register_css, 
								  parse_pu_register_css, 0);
		register_xml_table(table, REGISTER_CSS_RESPONSE_ID, 
								  REGISTER_RESPONSE_CMD, 
								  create_register_css_response, 
								  parse_register_css_response, 0);
        
		register_xml_table(table, REGISTER_REQUEST_ID, 
								  REGISTER_REQUEST_CMD, 
								  create_register_request, 
								  parse_register_request, 0);
		register_xml_table(table, REGISTER_RESPONSE_ID, 
								  REGISTER_RESPONSE_CMD, 
								  create_register_response, 
								  parse_register_response, 0);
		register_xml_table(table, HEART_BEAT_REQUEST_ID, 
								  HEART_BEAT_REQUEST_CMD, 
								  create_heart_beat_request, 
								  parse_heart_beat_request, 0);
		register_xml_table(table, HEART_BEAT_RESPONSE_ID, 
								  HEART_BEAT_RESPONSE_CMD, 
								  create_heart_beat_response, 
								  parse_heart_beat_response, 0);
		register_xml_table(table, GET_MDS_INFO_REQUEST_ID, 
								  GET_MDS_INFO_REQUEST_CMD, 
								  create_get_mds_info_request, 
								  parse_get_mds_info_request, 0);
		register_xml_table(table, GET_MDS_INFO_RESPONSE_ID, 
								  GET_MDS_INFO_RESPONSE_CMD, 
								  create_get_mds_info_response, 
								  parse_get_mds_info_response, 0);
		register_xml_table(table, CHANGE_DISPATCH_REQUEST_ID, 
								  CHANGE_DISPATCH_REQUEST_CMD, 
								  create_change_dispatch_request, 
								  parse_change_dispatch_request, 0);
		register_xml_table(table, CHANGE_DISPATCH_RESULT_ID, 
								  CHANGE_DISPATCH_RESULT_CMD, 
								  create_change_dispatch_result, 
								  parse_change_dispatch_result, 0);
		register_xml_table(table, GET_DEVICE_INFO_ID, 
								  GET_DEVICE_INFO_CMD, 
								  create_get_device_info, 
								  parse_get_device_info, 0);
		register_xml_table(table, DEVICE_INFO_RESPONSE_ID, 
								  DEVICE_INFO_RESPONSE_CMD, 
								  create_device_info_response, 
								  parse_device_info_response, 0);
		register_xml_table(table, GET_DEVICE_NTP_INFO_ID, 
								  GET_DEVICE_NTP_INFO_CMD, 
								  create_get_device_ntp_info, 
								  parse_get_device_ntp_info, 0);
		register_xml_table(table, DEVICE_NTP_INFO_RESPONSE_ID, 
								  DEVICE_NTP_INFO_RESPONSE_CMD, 
								  create_device_ntp_info_response, 
								  parse_device_ntp_info_response, 0);
		register_xml_table(table, SET_DEVICE_NTP_INFO_ID, 
								  SET_DEVICE_NTP_INFO_CMD, 
								  create_set_device_ntp_info, 
								  parse_set_device_ntp_info, 0);
		register_xml_table(table, DEVICE_NTP_INFO_RESULT_ID, 
								  DEVICE_NTP_INFO_RESULT_CMD, 
								  create_device_ntp_info_result, 
								  parse_device_ntp_info_result, 0);
		register_xml_table(table, GET_DEVICE_TIME_ID, 
								  GET_DEVICE_TIME_CMD, 
								  create_get_device_time, 
								  parse_get_device_time, 0);
		register_xml_table(table, DEVICE_TIME_RESPONSE_ID, 
								  DEVICE_TIME_RESPONSE_CMD, 
								  create_device_time_response, 
								  parse_device_time_response, 0);
		register_xml_table(table, SET_DEVICE_TIME_ID, 
								  SET_DEVICE_TIME_CMD, 
								  create_set_device_time, 
								  parse_set_device_time, 0);
		register_xml_table(table, DEVICE_TIME_RESULT_ID, 
								  DEVICE_TIME_RESULT_CMD, 
								  create_device_time_result, 
								  parse_device_time_result, 0);
		register_xml_table(table, GET_PLATFORM_INFO_ID, 
								  GET_PLATFORM_INFO_CMD, 
								  create_get_platform_info, 
								  parse_get_platform_info, 0);
		register_xml_table(table, PLATFORM_INFO_RESPONSE_ID, 
								  PLATFORM_INFO_RESPONSE_CMD, 
								  create_platform_info_response, 
								  parse_platform_info_response, 0);
		register_xml_table(table, SET_PLATFORM_INFO_ID, 
								  SET_PLATFORM_INFO_CMD, 
								  create_set_platform_info, 
								  parse_set_platform_info, 0);
		register_xml_table(table, PLATFORM_INFO_RESULT_ID, 
								  PLATFORM_INFO_RESULT_CMD, 
								  create_platform_info_result, 
								  parse_platform_info_result, 0);
		register_xml_table(table, GET_NETWORK_INFO_ID, 
								  GET_NETWORK_INFO_CMD, 
								  create_get_network_info, 
								  parse_get_network_info, 0);
		register_xml_table(table, NETWORK_INFO_RESPONSE_ID, 
								  NETWORK_INFO_RESPONSE_CMD, 
								  create_network_info_response, 
								  parse_network_info_response, 0);
		register_xml_table(table, SET_NETWORK_INFO_ID, 
								  SET_NETWORK_INFO_CMD, 
								  create_set_network_info, 
								  parse_set_network_info, 0);
		register_xml_table(table, NETWORK_INFO_RESULT_ID, 
								  NETWORK_INFO_RESULT_CMD, 
								  create_network_info_result, 
								  parse_network_info_result, 0);
		register_xml_table(table, GET_PPPOE_INFO_ID, 
								  GET_PPPOE_INFO_CMD, 
								  create_get_pppoe_info, 
								  parse_get_pppoe_info, 0);
		register_xml_table(table, PPPOE_INFO_RESPONSE_ID, 
								  PPPOE_INFO_RESPONSE_CMD, 
								  create_pppoe_info_response, 
								  parse_pppoe_info_response, 0);
		register_xml_table(table, SET_PPPOE_INFO_ID, 
								  SET_PPPOE_INFO_CMD, 
								  create_set_pppoe_info, 
								  parse_set_pppoe_info, 0);
		register_xml_table(table, PPPOE_INFO_RESULT_ID, 
								  PPPOE_INFO_RESULT_CMD, 
								  create_pppoe_info_result, 
								  parse_pppoe_info_result, 0);
		register_xml_table(table, GET_ENCODE_PARAMETER_ID, 
								  GET_ENCODE_PARAMETER_CMD, 
								  create_get_encode_parameter, 
								  parse_get_encode_parameter, 0);
		register_xml_table(table, ENCODE_PARAMETER_RESPONSE_ID, 
								  ENCODE_PARAMETER_RESPONSE_CMD, 
								  create_encode_parameter_response, 
								  parse_encode_parameter_response, 0);
		register_xml_table(table, SET_ENCODE_PARAMETER_ID, 
								  SET_ENCODE_PARAMETER_CMD, 
								  create_set_encode_parameter, 
								  parse_set_encode_parameter, 0);
		register_xml_table(table, ENCODE_PARAMETER_RESULT_ID, 
								  ENCODE_PARAMETER_RESULT_CMD, 
								  create_encode_parameter_result, 
								  parse_encode_parameter_result, 0);
		register_xml_table(table, GET_DISPLAY_PARAMETER_ID, 
								  GET_DISPLAY_PARAMETER_CMD, 
								  create_get_display_parameter, 
								  parse_get_display_parameter, 0);
		register_xml_table(table, DISPLAY_PARAMETER_RESPONSE_ID, 
								  DISPLAY_PARAMETER_RESPONSE_CMD, 
								  create_display_parameter_response, 
								  parse_display_parameter_response, 0);
		register_xml_table(table, SET_DISPLAY_PARAMETER_ID, 
								  SET_DISPLAY_PARAMETER_CMD, 
								  create_set_display_parameter, 
								  parse_set_display_parameter, 0);
		register_xml_table(table, DISPLAY_PARAMETER_RESULT_ID, 
								  DISPLAY_PARAMETER_RESULT_CMD, 
								  create_display_parameter_result, 
								  parse_display_parameter_result, 0);
		register_xml_table(table, GET_RECORD_PARAMETER_ID, 
								  GET_RECORD_PARAMETER_CMD, 
								  create_get_record_parameter, 
								  parse_get_record_parameter, 0);
		register_xml_table(table, RECORD_PARAMETER_RESPONSE_ID, 
								  RECORD_PARAMETER_RESPONSE_CMD, 
								  create_record_parameter_response, 
								  parse_record_parameter_response, 0);
		register_xml_table(table, SET_RECORD_PARAMETER_ID, 
								  SET_RECORD_PARAMETER_CMD, 
								  create_set_record_parameter, 
								  parse_set_record_parameter, 0);
		register_xml_table(table, RECORD_PARAMETER_RESULT_ID, 
								  RECORD_PARAMETER_RESULT_CMD, 
								  create_record_parameter_result, 
								  parse_record_parameter_result, 0);
		register_xml_table(table, GET_HIDE_PARAMETER_ID, 
								  GET_HIDE_PARAMETER_CMD, 
								  create_get_hide_parameter, 
								  parse_get_hide_parameter, 0);
		register_xml_table(table, HIDE_PARAMETER_RESPONSE_ID, 
								  HIDE_PARAMETER_RESPONSE_CMD, 
								  create_hide_parameter_response, 
								  parse_hide_parameter_response, 0);
		register_xml_table(table, SET_HIDE_PARAMETER_ID, 
								  SET_HIDE_PARAMETER_CMD, 
								  create_set_hide_parameter, 
								  parse_set_hide_parameter, 0);
		register_xml_table(table, HIDE_PARAMETER_RESULT_ID, 
								  HIDE_PARAMETER_RESULT_CMD, 
								  create_hide_parameter_result, 
								  parse_hide_parameter_result, 0);
		register_xml_table(table, GET_SERIAL_PARAMETER_ID, 
								  GET_SERIAL_PARAMETER_CMD, 
								  create_get_serial_parameter, 
								  parse_get_serial_parameter, 0);
		register_xml_table(table, SERIAL_PARAMETER_RESPONSE_ID, 
								  SERIAL_PARAMETER_RESPONSE_CMD, 
								  create_serial_parameter_response, 
								  parse_serial_parameter_response, 0);
		register_xml_table(table, SET_SERIAL_PARAMETER_ID, 
								  SET_SERIAL_PARAMETER_CMD, 
								  create_set_serial_parameter, 
								  parse_set_serial_parameter, 0);
		register_xml_table(table, SERIAL_PARAMETER_RESULT_ID, 
								  SERIAL_PARAMETER_RESULT_CMD, 
								  create_serial_parameter_result, 
								  parse_serial_parameter_result, 0);
		register_xml_table(table, GET_OSD_PARAMETER_ID, 
								  GET_OSD_PARAMETER_CMD, 
								  create_get_osd_parameter, 
								  parse_get_osd_parameter, 0);
		register_xml_table(table, OSD_PARAMETER_RESPONSE_ID, 
								  OSD_PARAMETER_RESPONSE_CMD, 
								  create_osd_parameter_response, 
								  parse_osd_parameter_response, 0);
		register_xml_table(table, SET_OSD_PARAMETER_ID, 
								  SET_OSD_PARAMETER_CMD, 
								  create_set_osd_parameter, 
								  parse_set_osd_parameter, 0);
		register_xml_table(table, OSD_PARAMETER_RESULT_ID, 
								  OSD_PARAMETER_RESULT_CMD, 
								  create_osd_parameter_result, 
								  parse_osd_parameter_result, 0);
		register_xml_table(table, GET_PTZ_PARAMETER_ID, 
								  GET_PTZ_PARAMETER_CMD, 
								  create_get_ptz_parameter, 
								  parse_get_ptz_parameter, 0);
		register_xml_table(table, PTZ_PARAMETER_RESPONSE_ID, 
								  PTZ_PARAMETER_RESPONSE_CMD, 
								  create_ptz_parameter_response, 
								  parse_ptz_parameter_response, 0);
		register_xml_table(table, SET_PTZ_PARAMETER_ID, 
								  SET_PTZ_PARAMETER_CMD, 
								  create_set_ptz_parameter, 
								  parse_set_ptz_parameter, 0);
		register_xml_table(table, PTZ_PARAMETER_RESULT_ID, 
								  PTZ_PARAMETER_RESULT_CMD, 
								  create_ptz_parameter_result, 
								  parse_ptz_parameter_result, 0);
		register_xml_table(table, GET_FTP_PARAMETER_ID, 
								  GET_FTP_PARAMETER_CMD, 
								  create_get_ftp_parameter, 
								  parse_get_ftp_parameter, 0);
		register_xml_table(table, FTP_PARAMETER_RESPONSE_ID, 
								  FTP_PARAMETER_RESPONSE_CMD, 
								  create_ftp_parameter_response, 
								  parse_ftp_parameter_response, 0);
		register_xml_table(table, SET_FTP_PARAMETER_ID, 
								  SET_FTP_PARAMETER_CMD, 
								  create_set_ftp_parameter, 
								  parse_set_ftp_parameter, 0);
		register_xml_table(table, FTP_PARAMETER_RESULT_ID, 
								  FTP_PARAMETER_RESULT_CMD, 
								  create_ftp_parameter_result, 
								  parse_ftp_parameter_result, 0);
		register_xml_table(table, GET_SMTP_PARAMETER_ID, 
								  GET_SMTP_PARAMETER_CMD, 
								  create_get_smtp_parameter, 
								  parse_get_smtp_parameter, 0);
		register_xml_table(table, SMTP_PARAMETER_RESPONSE_ID, 
								  SMTP_PARAMETER_RESPONSE_CMD, 
								  create_smtp_parameter_response, 
								  parse_smtp_parameter_response, 0);
		register_xml_table(table, SET_SMTP_PARAMETER_ID, 
								  SET_SMTP_PARAMETER_CMD, 
								  create_set_smtp_parameter, 
								  parse_set_smtp_parameter, 0);
		register_xml_table(table, SMTP_PARAMETER_RESULT_ID, 
								  SMTP_PARAMETER_RESULT_CMD, 
								  create_smtp_parameter_result, 
								  parse_smtp_parameter_result, 0);
		register_xml_table(table, GET_UPNP_PARAMETER_ID, 
								  GET_UPNP_PARAMETER_CMD, 
								  create_get_upnp_parameter, 
								  parse_get_upnp_parameter, 0);
		register_xml_table(table, UPNP_PARAMETER_RESPONSE_ID, 
								  UPNP_PARAMETER_RESPONSE_CMD, 
								  create_upnp_parameter_response, 
								  parse_upnp_parameter_response, 0);
		register_xml_table(table, SET_UPNP_PARAMETER_ID, 
								  SET_UPNP_PARAMETER_CMD, 
								  create_set_upnp_parameter, 
								  parse_set_upnp_parameter, 0);
		register_xml_table(table, UPNP_PARAMETER_RESULT_ID, 
								  UPNP_PARAMETER_RESULT_CMD, 
								  create_upnp_parameter_result, 
								  parse_upnp_parameter_result, 0);
		register_xml_table(table, GET_DEVICE_DISK_INFO_ID, 
								  GET_DEVICE_DISK_INFO_CMD, 
								  create_get_device_disk_info, 
								  parse_get_device_disk_info, 0);
		register_xml_table(table, DEVICE_DISK_INFO_RESPONSE_ID, 
								  DEVICE_DISK_INFO_RESPONSE_CMD, 
								  create_device_disk_info_response, 
								  parse_device_disk_info_response, 0);
		register_xml_table(table, FORMAT_DISK_REQUEST_ID, 
								  FORMAT_DISK_REQUEST_CMD, 
								  create_format_disk_request, 
								  parse_format_disk_request, 0);
		register_xml_table(table, FORMAT_DISK_RESULT_ID, 
								  FORMAT_DISK_RESULT_CMD, 
								  create_format_disk_result, 
								  parse_format_disk_result, 0);
		register_xml_table(table, SUBMIT_FORMAT_PROGRESS_ID, 
								  SUBMIT_FORMAT_PROGRESS_CMD, 
								  create_submit_format_progress, 
								  parse_submit_format_progress, 0);
		register_xml_table(table, GET_MOVE_ALARM_INFO_ID, 
								  GET_MOVE_ALARM_INFO_CMD, 
								  create_get_move_alarm_info, 
								  parse_get_move_alarm_info, 0);
		register_xml_table(table, MOVE_ALARM_INFO_RESPONSE_ID, 
								  MOVE_ALARM_INFO_RESPONSE_CMD, 
								  create_move_alarm_info_response, 
								  parse_move_alarm_info_response, 0);
		register_xml_table(table, SET_MOVE_ALARM_INFO_ID, 
								  SET_MOVE_ALARM_INFO_CMD, 
								  create_set_move_alarm_info, 
								  parse_set_move_alarm_info, 0);
		register_xml_table(table, MOVE_ALARM_INFO_RESULT_ID, 
								  MOVE_ALARM_INFO_RESULT_CMD, 
								  create_move_alarm_info_result, 
								  parse_move_alarm_info_result, 0);
		register_xml_table(table, GET_LOST_ALARM_INFO_ID, 
								  GET_LOST_ALARM_INFO_CMD, 
								  create_get_lost_alarm_info, 
								  parse_get_lost_alarm_info, 0);
		register_xml_table(table, LOST_ALARM_INFO_RESPONSE_ID, 
								  LOST_ALARM_INFO_RESPONSE_CMD, 
								  create_lost_alarm_info_response, 
								  parse_lost_alarm_info_response, 0);
		register_xml_table(table, SET_LOST_ALARM_INFO_ID, 
								  SET_LOST_ALARM_INFO_CMD, 
								  create_set_lost_alarm_info, 
								  parse_set_lost_alarm_info, 0);
		register_xml_table(table, LOST_ALARM_INFO_RESULT_ID, 
								  LOST_ALARM_INFO_RESULT_CMD, 
								  create_lost_alarm_info_result, 
								  parse_lost_alarm_info_result, 0);
		register_xml_table(table, GET_HIDE_ALARM_INFO_ID, 
								  GET_HIDE_ALARM_INFO_CMD, 
								  create_get_hide_alarm_info, 
								  parse_get_hide_alarm_info, 0);
		register_xml_table(table, HIDE_ALARM_INFO_RESPONSE_ID, 
								  HIDE_ALARM_INFO_RESPONSE_CMD, 
								  create_hide_alarm_info_response, 
								  parse_hide_alarm_info_response, 0);
		register_xml_table(table, SET_HIDE_ALARM_INFO_ID, 
								  SET_HIDE_ALARM_INFO_CMD, 
								  create_set_hide_alarm_info, 
								  parse_set_hide_alarm_info, 0);
		register_xml_table(table, HIDE_ALARM_INFO_RESULT_ID, 
								  HIDE_ALARM_INFO_RESULT_CMD, 
								  create_hide_alarm_info_result, 
								  parse_hide_alarm_info_result, 0);
		register_xml_table(table, GET_IO_ALARM_INFO_ID, 
								  GET_IO_ALARM_INFO_CMD, 
								  create_get_io_alarm_info, 
								  parse_get_io_alarm_info, 0);
		register_xml_table(table, IO_ALARM_INFO_RESPONSE_ID, 
								  IO_ALARM_INFO_RESPONSE_CMD, 
								  create_io_alarm_info_response, 
								  parse_io_alarm_info_response, 0);
		register_xml_table(table, SET_IO_ALARM_INFO_ID, 
								  SET_IO_ALARM_INFO_CMD, 
								  create_set_io_alarm_info, 
								  parse_set_io_alarm_info, 0);
		register_xml_table(table, IO_ALARM_INFO_RESULT_ID, 
								  IO_ALARM_INFO_RESULT_CMD, 
								  create_io_alarm_info_result, 
								  parse_io_alarm_info_result, 0);
		//////////////////////////////////////////////////////////
		register_xml_table(table, GET_JOINT_ACTION_INFO_ID, 
								  GET_JOINT_ACTION_INFO_CMD, 
								  create_get_joint_action_info, 
								  parse_get_joint_action_info, 0);
		register_xml_table(table, JOINT_ACTION_INFO_RESPONSE_ID, 
								  JOINT_ACTION_INFO_RESPONSE_CMD, 
								  create_joint_action_info_response, 
								  parse_joint_action_info_response, 0);
		register_xml_table(table, SET_JOINT_ACTION_INFO_ID, 
								  SET_JOINT_ACTION_INFO_CMD, 
								  create_set_joint_action_info, 
								  parse_set_joint_action_info, 0);
		register_xml_table(table, JOINT_ACTION_INFO_RESULT_ID, 
								  JOINT_ACTION_INFO_RESULT_CMD, 
								  create_joint_action_info_result, 
								  parse_joint_action_info_result, 0);
		//////////////////////////////////////////////////////////
		register_xml_table(table, CONTROL_PTZ_COMMAND_ID, 
								  CONTROL_PTZ_COMMAND_CMD, 
								  create_control_ptz_cmd, 
								  parse_control_ptz_cmd, 0);
		register_xml_table(table, PTZ_COMMAND_RESULT_ID, 
								  PTZ_COMMAND_RESULT_CMD, 
								  create_ptz_cmd_result, 
								  parse_ptz_cmd_result, 0);
		register_xml_table(table, SUBMIT_ALARM_REQUEST_ID, 
								  SUBMIT_ALARM_REQUEST_CMD, 
								  create_submit_alarm_request, 
								  parse_submit_alarm_request, 0);
		
		register_xml_table(table, GET_MEDIA_URL_REQUEST_ID, 
								  GET_MEDIA_URL_REQUEST_CMD, 
								  create_get_media_url_request, 
								  parse_get_media_url_request, 0);
		register_xml_table(table, GET_MEDIA_URL_RESPONSE_ID, 
								  GET_MEDIA_URL_RESPONSE_CMD, 
								  create_get_media_url_response, 
								  parse_get_media_url_response, 0);
   
		register_xml_table(table, GET_STORE_LOG_REQUEST_ID, 
	    						  GET_STORE_LOG_REQUEST_CMD, 
    							  create_get_store_log_request, 
    							  parse_get_store_log_request, 0);		
	    register_xml_table(table, GET_STORE_LOG_RESPONSE_ID, 
	    						  GET_STORE_LOG_RESPONSE_CMD, 
	    						  create_get_store_log_response, 
	    						  parse_get_store_log_response, 0);
		
		register_xml_table(table, USER_LONGI_REQUEST_ID, 
								  USER_LONGIN_REQUEST_CMD, 
								  create_user_login_request, 
								  parse_user_login_request, 0);
		register_xml_table(table, USER_LONGI_RESULT_ID, 
								  USER_LONGIN_RESULT_CMD, 
								  create_user_login_result, 
								  parse_user_login_result, 0);
		
		register_xml_table(table, USER_HEART_REQUEST_ID, 
								  USER_HEART_REQUEST_CMD, 
								  create_user_heart_request, 
								  parse_user_heart_request, 0);
		register_xml_table(table, USER_HEART_RESPONSE_ID, 
								  USER_HEART_RESPONSE_CMD, 
								  create_user_heart_response, 
								  parse_user_heart_response, 0);
		
		register_xml_table(table, FIRMWARE_UPGRADE_REQUEST_ID, 
								  FIRMWARE_UPGRADE_REQUEST_CMD, 
								  create_firmware_upgrade_request, 
								  parse_firmware_upgrade_request, 0);
		register_xml_table(table, FIRMWARE_UPGRADE_RESPONSE_ID, 
								  FIRMWARE_UPGRADE_RESPONSE_CMD, 
								  create_firmware_upgrade_response, 
								  parse_firmware_upgrade_response, 0);
		register_xml_table(table, SUBMIT_UPGRADE_PROGRESS_ID, 
								  SUBMIT_UPGRADE_PROGRESS_CMD, 
								  create_submit_upgrade_progress, 
								  parse_submit_upgrade_progress, 0);

        
# ifdef HAVE_PROXY_INFO
#include "proxy_info.h"
		
		register_xml_table(table, ADD_USER_REQUEST_ID, 
								  ADD_USER_REQUEST_CMD, 
								  create_add_user_request, 
								  parse_add_user_request, 0);
		register_xml_table(table, ADD_USER_RESULT_ID, 
								  ADD_USER_RESULT_CMD, 
								  create_add_user_result, 
								  parse_add_user_result, 0);
		register_xml_table(table, DEL_USER_REQUEST_ID, 
								  DEL_USER_REQUEST_CMD, 
								  create_del_user_request, 
								  parse_del_user_request, 0);
		register_xml_table(table, DEL_USER_RESULT_ID, 
								  DEL_USER_RESULT_CMD, 
								  create_del_user_result, 
								  parse_del_user_result, 0);
		
		register_xml_table(table, USER_LIST_INFO_ID, 
								  USER_LIST_INFO_CMD, 
								  create_user_list_info, 
								  parse_user_list_info, 0);
		register_xml_table(table, DEVICE_LIST_INFO_ID, 
								  DEVICE_LIST_INFO_CMD, 
								  create_device_list_info, 
								  parse_device_list_info, 0);
		register_xml_table(table, FACTORY_LIST_INFO_ID, 
								  FACTORY_LIST_INFO_CMD, 
								  create_factory_list_info, 
								  parse_factory_list_info, 0);
		
		register_xml_table(table, GET_FACTORY_REQUEST_ID, 
								  GET_FACTORY_REQUEST_CMD, 
								  NULL, 
								  parse_get_factory_info_request, 0);
		register_xml_table(table, GET_FACTORY_RESPONSE_ID, 
								  GET_FACTORY_RESPONSE_CMD, 
								  create_get_factory_info_response, 
								  parse_get_factory_info_response, 0);
		
		register_xml_table(table, FUZZY_FIND_USER_REQUEST_ID, 
								  FUZZY_FIND_USER_REQUEST_CMD, 
								  create_fuzzy_find_user_request, 
								  parse_fuzzy_find_user_request, 0);
		register_xml_table(table, FUZZY_FIND_USER_RESULT_ID, 
								  FUZZY_FIND_USER_RESULT_CMD, 
								  create_fuzzy_find_user_result, 
								  parse_fuzzy_find_user_result, 0);
		
		register_xml_table(table, MODIFY_PASSWORD_REQUEST_ID, 
								  MODIFY_PASSWORD_REQUEST_CMD, 
								  create_modify_password_request, 
								  parse_modify_password_request, 0);
		register_xml_table(table, MODIFY_PASSWORD_RESULT_ID, 
								  MODIFY_PASSWORD_RESULT_CMD, 
								  create_modify_password_result, 
								  parse_modify_password_result, 0);

		register_xml_table(table, ADD_DEVICE_REQUEST_ID, 
								  ADD_DEVICE_REQUEST_CMD, 
								  create_add_device_request, 
								  parse_add_device_request, 0);
		register_xml_table(table, ADD_DEVICE_RESULT_ID, 
								  ADD_DEVICE_RESULT_CMD, 
								  create_add_device_result, 
								  parse_add_device_result, 0);
		register_xml_table(table, DEL_DEVICE_REQUEST_ID, 
								  DEL_DEVICE_REQUEST_CMD, 
								  create_del_device_request, 
								  parse_del_device_request, 0);
		register_xml_table(table, DEL_DEVICE_RESULT_ID, 
								  DEL_DEVICE_RESULT_CMD, 
								  create_del_device_result, 
								  parse_del_device_result, 0);

		register_xml_table(table, GET_DEVICE_INFO_REQUEST_ID, 
								  GET_DEVICE_INFO_REQUEST_CMD, 
								  create_get_device_info_request, 
								  parse_get_device_info_request, 0);
		register_xml_table(table, GET_DEVICE_INFO_RESULT_ID, 
								  GET_DEVICE_INFO_RESULT_CMD, 
								  create_get_device_info_result, 
								  parse_get_device_info_result, 0);
		register_xml_table(table, SET_DEVICE_INFO_REQUEST_ID, 
								  SET_DEVICE_INFO_REQUEST_CMD, 
								  create_set_device_info_request, 
								  parse_set_device_info_request, 0);
		register_xml_table(table, SET_DEVICE_INFO_RESULT_ID, 
								  SET_DEVICE_INFO_RESULT_CMD, 
								  create_set_device_info_result, 
								  parse_set_device_info_result, 0);

		register_xml_table(table, GET_ALL_DEVICE_ID_REQUEST_ID, 
								  GET_ALL_DEVICE_ID_REQUEST_CMD, 
								  create_get_all_device_id_request, 
								  parse_get_all_device_id_request, 0);
		register_xml_table(table, GET_ALL_DEVICE_ID_RESULT_ID, 
								  GET_ALL_DEVICE_ID_RESULT_CMD, 
								  create_get_all_device_id_result, 
    							  parse_get_all_device_id_result, 0); 
		
		register_xml_table(table, GET_USER_INFO_REQUEST_ID, 
								  GET_USER_INFO_REQUEST_CMD, 
								  NULL, 
								  parse_proxy_page_user_request, 0);
		register_xml_table(table, GET_USER_INFO_RESPONSE_ID, 
								  GET_USER_INFO_RESPONSE_CMD, 
								  create_proxy_page_user_response, 
								  NULL, 0);
		
		register_xml_table(table, BROADCAST_ADD_USER_ID, 
								  BROADCAST_ADD_USER_CMD, 
								  create_broadcast_add_user, 
								  NULL, 0);
		register_xml_table(table, BROADCAST_DEL_USER_ID, 
								  BROADCAST_DEL_USER_CMD, 
								  create_broadcast_del_user, 
								  NULL, 0);
		register_xml_table(table, BROADCAST_ADD_DEVICE_ID, 
								  BROADCAST_ADD_DEVICE_CMD, 
								  create_broadcast_add_device, 
								  NULL, 0);
		register_xml_table(table, BROADCAST_DEL_DEVICE_ID, 
								  BROADCAST_DEL_DEVICE_CMD, 
								  create_broadcast_del_device, 
								  NULL, 0);
		register_xml_table(table, BROADCAST_MODIFY_DEVICE_ID, 
								  BROADCAST_MODIFY_DEVICE_CMD, 
								  create_broadcast_modify_device, 
								  NULL, 0);
		register_xml_table(table, BROADCAST_DEVICE_STATUS_ID, 
								  BROADCAST_DEVICE_STATUS_CMD, 
								  create_broadcast_device_status, 
								  NULL, 0);
		
		register_xml_table(table, GET_SERVER_CONFIG_REQUEST_ID, 
								  GET_SERVER_CONFIG_REQUEST_CMD, 
								  create_get_server_config_request, 
								  parse_get_server_config_request, 0);
		register_xml_table(table, GET_SERVER_CONFIG_RESPONSE_ID, 
								  GET_SERVER_CONFIG_RESPONSE_CMD, 
								  create_get_server_config_response, 
								  parse_get_server_config_response, 0);
		register_xml_table(table, SET_SERVER_CONFIG_REQUEST_ID, 
								  SET_SERVER_CONFIG_REQUEST_CMD, 
								  create_set_server_config_request, 
								  parse_set_server_config_request, 0);
		register_xml_table(table, SET_SERVER_CONFIG_RESULT_ID, 
								  SET_SERVER_CONFIG_RESULT_CMD, 
								  create_set_server_config_result, 
								  parse_set_server_config_result, 0);

		register_xml_table(table, DOWNLOAD_DATA_REQUEST_ID, 
								  DOWNLOAD_DATA_REQUEST_CMD, 
								  create_download_request, 
								  parse_download_request, 0);
		register_xml_table(table, DOWNLOAD_DATA_RESPONSE_ID, 
								  DOWNLOAD_DATA_RESPONSE_CMD, 
								  create_download_response, 
								  parse_download_response, 0);
		register_xml_table(table, UPLOAD_DATA_REQUEST_ID, 
								  UPLOAD_DATA_REQUEST_CMD, 
								  create_upload_request, 
								  parse_upload_request, 0);
		register_xml_table(table, UPLOAD_DATA_RESPONSE_ID, 
								  UPLOAD_DATA_RESPONSE_CMD, 
								  create_upload_response, 
								  parse_upload_response, 0);
        
		register_xml_table(table, LIMIT_BROADCASE_STATUE_ID, 
								  LIMIT_BROADCASE_STATUE_CMD, 
								  NULL, 
								  parse_limit_broadcast_status, 0);
# endif	//HAVE_PROXY_INFO

		//channel info
		register_xml_table(table, GET_CHANNEL_INFO_REQUEST_ID, 
								  GET_CHANNEL_INFO_REQUEST_CMD, 
								  create_get_channel_info_request, 
								  parse_get_channel_info_request, 0);
		register_xml_table(table, GET_CHANNEL_INFO_RESPONSE_ID, 
								  GET_CHANNEL_INFO_RESPONSE_CMD, 
								  create_get_channel_info_response, 
								  parse_get_channel_info_response, 0);
		register_xml_table(table, SET_CHANNEL_INFO_REQUEST_ID, 
								  SET_CHANNEL_INFO_REQUEST_CMD, 
								  create_set_channel_info_request, 
								  parse_set_channel_info_request, 0);
		register_xml_table(table, SET_CHANNEL_INFO_RESULT_ID, 
								  SET_CHANNEL_INFO_RESULT_CMD, 
								  create_set_channel_info_result, 
								  parse_set_channel_info_result, 0);

		//picture info
		register_xml_table(table, GET_PICTURE_INFO_REQUEST_ID, 
								  GET_PICTURE_INFO_REQUEST_CMD, 
								  create_get_picture_info_request, 
								  parse_get_picture_info_request, 0);
		register_xml_table(table, GET_PICTURE_INFO_RESPONSE_ID, 
								  GET_PICTURE_INFO_RESPONSE_CMD, 
								  create_get_picture_info_response, 
								  parse_get_picture_info_response, 0);
		register_xml_table(table, SET_PICTURE_INFO_REQUEST_ID, 
								  SET_PICTURE_INFO_REQUEST_CMD, 
								  create_set_picture_info_request, 
								  parse_set_picture_info_request, 0);
		register_xml_table(table, SET_PICTURE_INFO_RESULT_ID, 
								  SET_PICTURE_INFO_RESULT_CMD, 
								  create_set_picture_info_result, 
								  parse_set_picture_info_result, 0);

		//wifi config
		register_xml_table(table, GET_WIFI_CONFIG_REQUEST_ID, 
								  GET_WIFI_CONFIG_REQUEST_CMD, 
								  create_get_wifi_config_request, 
								  parse_get_wifi_config_request, 0);
		register_xml_table(table, GET_WIFI_CONFIG_RESPONSE_ID, 
								  GET_WIFI_CONFIG_RESPONSE_CMD, 
								  create_get_wifi_config_response, 
								  parse_get_wifi_config_response, 0);
		register_xml_table(table, SET_WIFI_CONFIG_REQUEST_ID, 
								  SET_WIFI_CONFIG_REQUEST_CMD, 
								  create_set_wifi_config_request, 
								  parse_set_wifi_config_request, 0);
		register_xml_table(table, SET_WIFI_CONFIG_RESULT_ID, 
								  SET_WIFI_CONFIG_RESULT_CMD, 
								  create_set_wifi_config_result, 
								  parse_set_wifi_config_result, 0);

		//wifi search
		register_xml_table(table, WIFI_SEARCH_REQUEST_ID, 
								  WIFI_SEARCH_REQUEST_CMD, 
								  create_wifi_search_request, 
								  parse_wifi_search_request, 0);
		register_xml_table(table, WIFI_SEARCH_RESPONSE_ID, 
								  WIFI_SEARCH_RESPONSE_CMD, 
								  create_wifi_search_response, 
								  parse_wifi_search_response, 0);

		//network status
		register_xml_table(table, GET_NETWORK_STATUS_REQUEST_ID, 
								  GET_NETWORK_STATUS_REQUEST_CMD, 
								  create_get_network_status_request, 
								  parse_get_network_status_request, 0);
		register_xml_table(table, GET_NETWORK_STATUS_RESPONSE_ID, 
								  GET_NETWORK_STATUS_RESPONSE_CMD, 
								  create_get_network_status_response, 
								  parse_get_network_status_response, 0);

		//control device
		register_xml_table(table, CONTROL_DEVICE_REQUEST_ID, 
								  CONTROL_DEVICE_REQUEST_CMD, 
								  create_control_device_request, 
								  parse_control_device_request, 0);
		register_xml_table(table, CONTROL_DEVICE_RESULT_ID, 
								  CONTROL_DEVICE_RESULT_CMD, 
								  create_control_device_result, 
								  parse_control_device_result, 0);

		//ddns config
		register_xml_table(table, GET_DDNS_CONFIG_REQUEST_ID, 
								  GET_DDNS_CONFIG_REQUEST_CMD, 
								  create_get_ddns_config_request, 
								  parse_get_ddns_config_request, 0);
		register_xml_table(table, GET_DDNS_CONFIG_RESPONSE_ID, 
								  GET_DDNS_CONFIG_RESPONSE_CMD, 
								  create_get_ddns_config_response, 
								  parse_get_ddns_config_response, 0);
		register_xml_table(table, SET_DDNS_CONFIG_REQUEST_ID, 
								  SET_DDNS_CONFIG_REQUEST_CMD, 
								  create_set_ddns_config_request, 
								  parse_set_ddns_config_request, 0);
		register_xml_table(table, SET_DDNS_CONFIG_RESULT_ID, 
								  SET_DDNS_CONFIG_RESULT_CMD, 
								  create_set_ddns_config_result, 
								  parse_set_ddns_config_result, 0);

		//default display info
		register_xml_table(table, GET_DEF_DISPLAY_INFO_REQUEST_ID, 
								  GET_DEF_DISPLAY_INFO_REQUEST_CMD, 
								  create_get_def_display_info_requst, 
								  parse_get_def_display_info_request, 0);
		register_xml_table(table, GET_DEF_DISPLAY_INFO_RESPONSE_ID, 
								  GET_DEF_DISPLAY_INFO_RESPONSE_CMD, 
								  create_get_def_display_info_response, 
								  parse_get_def_display_info_response, 0);

		//default picture info
		register_xml_table(table, GET_DEF_PICTURE_INFO_REQUEST_ID, 
								  GET_DEF_PICTURE_INFO_REQUEST_CMD, 
								  create_get_def_picture_info_request, 
								  parse_get_def_picture_info_request, 0);
		register_xml_table(table, GET_DEF_PICTURE_INFO_RESPONSE_ID, 
								  GET_DEF_PICTURE_INFO_RESPONSE_CMD, 
								  create_get_def_picture_info_response, 
								  parse_get_def_picture_info_response, 0);

		//avd config
		register_xml_table(table, GET_AVD_CONFIG_REQUEST_ID, 
								  GET_AVD_CONFIG_REQUEST_CMD, 
								  create_get_avd_config_request, 
								  parse_get_avd_config_request, 0);
		register_xml_table(table, GET_AVD_CONFIG_RESPONSE_ID, 
								  GET_AVD_CONFIG_RESPONSE_CMD, 
								  create_get_avd_config_response, 
								  parse_get_avd_config_response, 0);
		register_xml_table(table, SET_AVD_CONFIG_REQUEST_ID, 
								  SET_AVD_CONFIG_REQUEST_CMD, 
								  create_set_avd_config_request, 
								  parse_set_avd_config_request, 0);
		register_xml_table(table, SET_AVD_CONFIG_RESULT_ID, 
								  SET_AVD_CONFIG_RESULT_CMD, 
								  create_set_avd_config_result, 
								  parse_set_avd_config_result, 0);



		register_xml_table(table, GET_TRANSPARENTPARAM_REQUEST_ID, 
								  GET_TRANSPARENTPARAM_REQUEST_CMD, 
								  create_get_transparent_param_request, 
								  parse_get_transparent_param_request, 0);
		register_xml_table(table, GET_TRANSPARENTPARAM_RESPONSE_ID, 
								  GET_TRANSPARENTPARAM_RESPONSE_CMD, 
								  create_get_transparent_param_response, 
								  parse_get_transparent_param_response, 0);
		
		register_xml_table(table, SET_TRANSPARENTPARAM_REQUEST_ID, 
								  SET_TRANSPARENTPARAM_REQUEST_CMD, 
								  create_set_transparent_param_request, 
								  parse_set_transparent_param_request, 0);
		register_xml_table(table, SET_TRANSPARENTPARAM_RESPONSE_ID, 
								  SET_TRANSPARENTPARAM_RESPONSE_CMD, 
								  create_set_transparent_param_response, 
								  parse_set_transparent_param_response, 0);
		
		register_xml_table(table, TRANSPARENTPARAM_NOTIFYEVENT_ID, 
								  TRANSPARENTPARAM_NOTIFYEVENT_CMD, 
								  create_transparent_notify_enevt, 
								  parse_transparent_notify_enevt, 0);
		register_xml_table(table, TRANSPARENTPARAM_CONTROLDEVICE_ID, 
								  TRANSPARENTPARAM_CONTROLDEVICE_CMD, 
								  create_transparent_control_device, 
								  parse_transparent_control_device, 0);

		
# ifdef _USE_DECODER_PROTO_

		register_xml_table(table, QUERY_DIVISION_MODE_REQUEST_ID, 
								  QUERY_DIVISION_MODE_REQUEST_CMD, 
								  create_query_division_mode_request, 
								  parse_query_division_mode_request, 0);
		register_xml_table(table, QUERY_DIVISION_MODE_RESPONSE_ID, 
								  QUERY_DIVISION_MODE_RESPONSE_CMD, 
								  create_query_division_mode_response, 
								  parse_query_division_mode_response, 0);

		register_xml_table(table, GET_SCREEN_STATE_REQUEST_ID, 
								  GET_SCREEN_STATE_REQUEST_CMD, 
								  create_get_screen_state_request, 
								  parse_get_screen_state_request, 0);
		register_xml_table(table, GET_SCREEN_STATE_RESPONSE_ID, 
								  GET_SCREEN_STATE_RESPONSE_CMD, 
								  create_get_screen_state_response, 
								  parse_get_screen_state_response, 0);

		register_xml_table(table, SET_DIVISION_MODE_REQUEST_ID, 
								  SET_DIVISION_MODE_REQUEST_CMD, 
								  create_set_division_mode_request, 
								  parse_set_division_mode_request, 0);
		register_xml_table(table, SET_DIVISION_MODE_RESULT_ID, 
								  SET_DIVISION_MODE_RESULT_CMD, 
								  create_set_division_mode_response, 
								  parse_set_division_mode_response, 0);

		register_xml_table(table, SET_FULL_SCREEN_REQUEST_ID, 
								  SET_FULL_SCREEN_REQUEST_CMD, 
								  create_set_full_screen_request, 
								  parse_set_full_screen_request, 0);
		register_xml_table(table, SET_FULL_SCREEN_RESULT_ID, 
								  SET_FULL_SCREEN_RESULT_CMD, 
								  create_set_full_screen_response, 
								  parse_set_full_screen_response, 0);

		register_xml_table(table, EXIT_FULL_SCREEN_REQUEST_ID, 
								  EXIT_FULL_SCREEN_REQUEST_CMD, 
								  create_exit_full_screen_request, 
								  parse_exit_full_screen_request, 0);
		register_xml_table(table, EXIT_FULL_SCREEN_RESULT_ID, 
								  EXIT_FULL_SCREEN_RESULT_CMD, 
								  create_exit_full_screen_response, 
								  parse_exit_full_screen_response, 0);

		register_xml_table(table, TV_WALL_PLAY_REQUEST_ID, 
								  TV_WALL_PLAY_REQUEST_CMD, 
								  create_tv_wall_play_request, 
								  parse_tv_wall_play_request, 0);
		register_xml_table(table, TV_WALL_PLAY_RESULT_ID, 
								  TV_WALL_PLAY_RESULT_CMD, 
								  create_tv_wall_play_response, 
								  parse_tv_wall_play_response, 0);

		register_xml_table(table, CLEAR_DIVISION_REQUEST_ID, 
								  CLEAR_DIVISION_REQUEST_CMD, 
								  create_clear_division_request, 
								  parse_clear_division_request, 0);
		register_xml_table(table, CLEAR_DIVISION_RESULT_ID, 
								  CLEAR_DIVISION_RESULT_CMD, 
								  create_clear_division_response, 
								  parse_clear_division_response, 0);

# endif //_USE_DECODER_PROTO_
   
		register_xml_table(table, GET_OPERATION_LOG_REQUEST_ID, 
	    						  GET_OPERATION_LOG_REQUEST_CMD, 
    							  create_get_operation_log_request, 
    							  parse_get_operation_log_request, 0);		
	    register_xml_table(table, GET_OPERATION_LOG_RESPONSE_ID, 
	    						  GET_OPERATION_LOG_RESPONSE_CMD, 
	    						  create_get_operation_log_response, 
	    						  parse_get_operation_log_response, 0);

		register_xml_table(table, SET_ALARM_UPLOAD_CONFIG_REQUEST_ID, 
								  SET_ALARM_UPLOAD_CONFIG_REQUEST_CMD, 
								  create_set_alarm_upload_config_request, 
								  parse_set_alarm_upload_config_request, 0);
		register_xml_table(table, SET_ALARM_UPLOAD_CONFIG_RESULT_ID, 
								  SET_ALARM_UPLOAD_CONFIG_RESULT_CMD, 
								  create_set_alarm_upload_config_response, 
								  parse_set_alarm_upload_config_response, 0);

		register_xml_table(table, GET_PRESET_POINT_SET_REQUEST_ID, 
								  GET_PRESET_POINT_SET_REQUEST_CMD, 
								  create_get_preset_point_set_request, 
								  parse_get_preset_point_set_request, 0);
		register_xml_table(table, GET_PRESET_POINT_SET_RESPONSE_ID, 
								  GET_PRESET_POINT_SET_RESPONSE_CMD, 
								  create_get_preset_point_set_response, 
								  parse_get_preset_point_set_response, 0);

		register_xml_table(table, SET_PRESET_POINT_REQUEST_ID, 
								  SET_PRESET_POINT_REQUEST_CMD, 
								  create_set_preset_point_request, 
								  parse_set_preset_point_request, 0);
		register_xml_table(table, SET_PRESET_POINT_RESULT_ID, 
								  SET_PRESET_POINT_RESULT_CMD, 
								  create_set_preset_point_result, 
								  parse_set_preset_point_result, 0);

		register_xml_table(table, GET_CRUISE_WAY_SET_REQUEST_ID, 
								  GET_CRUISE_WAY_SET_REQUEST_CMD, 
								  create_get_cruise_way_set_request, 
								  parse_get_cruise_way_set_request, 0);
		register_xml_table(table, GET_CRUISE_WAY_SET_RESPONSE_ID, 
								  GET_CRUISE_WAY_SET_RESPONSE_CMD, 
								  create_get_cruise_way_set_response, 
								  parse_get_cruise_way_set_response, 0);

		register_xml_table(table, SET_CRUISE_WAY_REQUEST_ID, 
								  SET_CRUISE_WAY_REQUEST_CMD, 
								  create_set_cruise_way_request, 
								  parse_set_cruise_way_request, 0);
		register_xml_table(table, SET_CRUISE_WAY_RESULT_ID, 
								  SET_CRUISE_WAY_RESULT_CMD, 
								  create_set_cruise_way_result, 
								  parse_set_cruise_way_result, 0);

		register_xml_table(table, GET_CRUISE_WAY_REQUEST_ID, 
								  GET_CRUISE_WAY_REQUEST_CMD, 
								  create_get_cruise_way_request, 
								  parse_get_cruise_way_request, 0);
		register_xml_table(table, GET_CRUISE_WAY_RESPONSE_ID, 
								  GET_CRUISE_WAY_RESPONSE_CMD, 
								  create_get_cruise_way_response, 
								  parse_get_cruise_way_response, 0);

		register_xml_table(table, ADD_CRUISE_WAY_REQUEST_ID, 
								  ADD_CRUISE_WAY_REQUEST_CMD, 
								  create_add_cruise_way_request, 
								  parse_add_cruise_way_request, 0);
		register_xml_table(table, ADD_CRUISE_WAY_RESULT_ID, 
								  ADD_CRUISE_WAY_RESULT_CMD, 
								  create_add_cruise_way_result, 
								  parse_add_cruise_way_result, 0);

		register_xml_table(table, MODIFY_CRUISE_WAY_REQUEST_ID, 
								  MODIFY_CRUISE_WAY_REQUEST_CMD, 
								  create_modify_cruise_way_request, 
								  parse_modify_cruise_way_request, 0);
		register_xml_table(table, MODIFY_CRUISE_WAY_RESULT_ID, 
								  MODIFY_CRUISE_WAY_RESULT_CMD, 
								  create_modify_cruise_way_result, 
								  parse_modify_cruise_way_result, 0);

		register_xml_table(table, _3D_CONTROL_REQUEST_ID, 
								  _3D_CONTROL_REQUEST_CMD, 
								  create_3d_control_request, 
								  parse_3d_control_request, 0);
		register_xml_table(table, _3D_CONTROL_RESULT_ID, 
								  _3D_CONTROL_RESULT_CMD, 
								  create_3d_control_result, 
								  parse_3d_control_result, 0);

		register_xml_table(table, _3D_GOBACK_REQUEST_ID, 
								  _3D_GOBACK_REQUEST_CMD, 
								  create_3d_goback_request, 
								  parse_3d_goback_request, 0);
		register_xml_table(table, _3D_GOBACK_RESULT_ID, 
								  _3D_GOBACK_RESULT_CMD, 
								  create_3d_goback_result, 
								  parse_3d_goback_result, 0);

		register_xml_table(table, ALARM_LINK_IO_REQUEST_ID, 
								  ALARM_LINK_IO_REQUEST_CMD, 
								  create_alarm_link_io_request, 
								  parse_alarm_link_io_request, 0);
		register_xml_table(table, ALARM_LINK_IO_RESULT_ID, 
								  ALARM_LINK_IO_RESULT_CMD, 
								  create_alarm_link_io_result, 
								  parse_alarm_link_io_result, 0);

		register_xml_table(table, ALARM_LINK_PRESET_REQUEST_ID, 
								  ALARM_LINK_PRESET_REQUEST_CMD, 
								  create_alarm_link_preset_request, 
								  parse_alarm_link_preset_request, 0);
		register_xml_table(table, ALARM_LINK_PRESET_RESULT_ID, 
								  ALARM_LINK_PRESET_RESULT_CMD, 
								  create_alarm_link_preset_result, 
								  parse_alarm_link_preset_result, 0);

		register_xml_table(table, GET_RESOLUTION_INFO_REQUEST_ID, 
								  GET_RESOLUTION_INFO_REQUEST_CMD, 
								  create_get_resolution_info_request, 
								  parse_get_resolution_info_request, 0);
		register_xml_table(table, GET_RESOLUTION_INFO_RESPONSE_ID, 
								  GET_RESOLUTION_INFO_RESPONSE_CMD, 
								  create_get_resolution_info_response, 
								  parse_get_resolution_info_response, 0);

		register_xml_table(table, SET_RESOLUTION_INFO_REQUEST_ID, 
								  SET_RESOLUTION_INFO_REQUEST_CMD, 
								  create_set_resolution_info_request, 
								  parse_set_resolution_info_request, 0);
		register_xml_table(table, SET_RESOLUTION_INFO_RESULT_ID, 
								  SET_RESOLUTION_INFO_RESULT_CMD, 
								  create_set_resolution_info_result, 
								  parse_set_resolution_info_result, 0);

		register_xml_table(table, GET_IRCUT_CONTROL_REQUEST_ID, 
								  GET_IRCUT_CONTROL_REQUEST_CMD, 
								  create_get_ircut_control_info_request, 
								  parse_get_ircut_control_info_request, 0);
		register_xml_table(table, GET_IRCUT_CONTROL_RESPONSE_ID, 
								  GET_IRCUT_CONTROL_RESPONSE_CMD, 
								  create_get_ircut_control_info_response, 
								  parse_get_ircut_control_info_response, 0);

		register_xml_table(table, SET_IRCUT_CONTROL_REQUEST_ID, 
								  SET_IRCUT_CONTROL_REQUEST_CMD, 
								  create_set_ircut_control_info_request, 
								  parse_set_ircut_control_info_request, 0);
		register_xml_table(table, SET_IRCUT_CONTROL_RESULT_ID, 
								  SET_IRCUT_CONTROL_RESULT_CMD, 
								  create_set_ircut_control_info_result, 
								  parse_set_ircut_control_info_result, 0);

		register_xml_table(table, GET_EXTRANET_PORT_REQUEST_ID, 
								  GET_EXTRANET_PORT_REQUEST_CMD, 
								  create_get_extranet_port_request, 
								  parse_get_extranet_port_request, 0);
		register_xml_table(table, GET_EXTRANET_PORT_RESPONSE_ID, 
								  GET_EXTRANET_PORT_RESPONSE_CMD, 
								  create_get_extranet_port_response, 
								  parse_get_extranet_port_response, 0);

		register_xml_table(table, GET_HERD_ANALYSE_REQUEST_ID, 
								  GET_HERD_ANALYSE_REQUEST_CMD, 
								  create_get_herd_analyse_info_request, 
								  parse_get_herd_analyse_info_request, 0);
		register_xml_table(table, GET_HERD_ANALYSE_RESPONSE_ID, 
								  GET_HERD_ANALYSE_RESPONSE_CMD, 
								  create_get_herd_analyse_info_response, 
								  parse_get_herd_analyse_info_response, 0);

		register_xml_table(table, SET_HERD_ANALYSE_REQUEST_ID, 
								  SET_HERD_ANALYSE_REQUEST_CMD, 
								  create_set_herd_analyse_info_request, 
								  parse_set_herd_analyse_info_request, 0);
		register_xml_table(table, SET_HERD_ANALYSE_RESULT_ID, 
								  SET_HERD_ANALYSE_RESULT_CMD, 
								  create_set_herd_analyse_info_result, 
								  parse_set_herd_analyse_info_result, 0);

		register_xml_table(table, GET_GRASS_PERCENT_REQUEST_ID, 
								  GET_GRASS_PERCENT_REQUEST_CMD, 
								  create_get_grass_percent_request, 
								  parse_get_grass_percent_request, 0);
		register_xml_table(table, GET_GRASS_PERCENT_RESPONSE_ID, 
								  GET_GRASS_PERCENT_RESPONSE_CMD, 
								  create_get_grass_percent_response, 
								  parse_get_grass_percent_response, 0);

		register_xml_table(table, GET_P2P_ID_REQUEST_ID, 
								  GET_P2P_ID_REQUEST_CMD, 
								  create_get_p2p_id_request, 
								  parse_get_p2p_id_request, 0);
		register_xml_table(table, GET_P2P_ID_RESPONSE_ID, 
								  GET_P2P_ID_RESPONSE_CMD, 
								  create_get_p2p_id_response, 
								  parse_get_p2p_id_response, 0);

	}
}

static __inline__ void jpf_priv_obj_destroy(void *priv, size_t size)
{
	J_ASSERT(priv && size);
	
	return j_xml_dealloc(priv, size);
}

static __inline__ NmpXmlMsg* 
__jpf_xml_msg_new(xmlid_t id, void *priv_obj, 
		size_t priv_size, NmpXmlMsgDes destroy)
{
	NmpXmlMsg *msg = NULL;
	
	J_ASSERT(priv_obj && priv_size && destroy);
	
	msg = j_xml_alloc(sizeof(NmpXmlMsg));
	jpf_xml_msg_init(msg);
	
	msg->id = id;
	msg->priv_obj = priv_obj;
	msg->priv_size = priv_size;
	msg->destroy = destroy;
	
	return msg;
}

NmpXmlMsg* jpf_xml_msg_new(xmlid_t id, void *priv_obj, size_t priv_size)
{
	void *copy_obj = NULL;
	NmpXmlMsgDes destroy = NULL;
	
	if (priv_obj && priv_size)
	{
		copy_obj = j_xml_alloc(priv_size);
		memcpy(copy_obj, priv_obj, priv_size);
		destroy = jpf_priv_obj_destroy;
	}
	
	return __jpf_xml_msg_new(id, copy_obj, priv_size, destroy);
}

NmpXmlMsg* jpf_xml_msg_new_2(xmlid_t id, void *priv_obj, 
		size_t priv_size, NmpXmlMsgDes destroy)
{
	return __jpf_xml_msg_new(id, priv_obj, priv_size, destroy);
}

void jpf_xml_msg_destroy(NmpXmlMsg *self)
{
	J_ASSERT(self);

	if (self->destroy)
		(*self->destroy)(XML_MSG_DATA(self), XML_MSG_DATA_SIZE(self));
	
	return j_xml_dealloc(self, sizeof(NmpXmlMsg));
}

void jpf_xml_msg_destroy_2(NmpXmlMsg *self)
{
	J_ASSERT(self);

	return j_xml_dealloc(self, sizeof(NmpXmlMsg));
}

int create_xml(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags)
{
	XmlTable *table = NULL;
	
	J_ASSERT(msg);
	J_ASSERT(buf && size);
	
	table = get_xml_table();
	return xml_table_create(table, msg, buf, size, flags);
}

static __inline__ int 
get_xml_buf(char **ptr, char page[], 
	const char *buf, size_t size)
{
	if (J_SDK_MAX_PAGE_SIZE <= size)
	{
		*ptr = j_xml_alloc(size+1);
		memcpy(*ptr, (const void*)buf, size);
		(*ptr)[size] = 0;

		return 1;
	}
	else
	{
		memcpy(page, (const void*)buf, size);
		page[size] = 0;
		*ptr = page;

		return 0;
	}
}

static __inline__ char *
get_msg_cmd(const char *xml_buf, size_t xml_size, 
	char cmd_buf[], size_t cmd_size)
{
	int ret = -1;
	char *new_buf = NULL;
	char page_buf[J_SDK_MAX_PAGE_SIZE];
	
	J_ASSERT(xml_buf && xml_size);
	
	ret = get_xml_buf(&new_buf, page_buf, xml_buf, xml_size);
	cmd_buf = parse_xml_cmd_by_mxml(new_buf, cmd_buf, cmd_size);
	
	if (1 == ret)
		j_xml_dealloc(new_buf, xml_size+1);

	return cmd_buf;
}

static __inline__ int 
get_msg_cmd_type(XmlTable *table, const char *command)
{
	int index = 0;
	
	J_ASSERT(table);

	for (; index<table->n_entries; index++)
	{
		if (!strcmp(command, table->entries[index].command))
		{
			return table->entries[index].id;
		}
	}

	return -1;
}

NmpXmlMsg *parse_xml(char buf[], size_t size, int *err, unsigned int flags)
{
	xmlid_t id = -1;
	XmlTable *table = NULL;
	char *ret = NULL;
	char command[J_SDK_MAX_COMMAND_LEN];
	
	J_ASSERT(buf && size);

	ret = get_msg_cmd(buf, size, command, sizeof(command));
	//printf("command: %s<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", command);
	
	if (ret)
	{
		table = get_xml_table();
		id = get_msg_cmd_type(table, ret);
		
		if (COMMAND_ID_NONE<(int)id && MAX_COMMAND_ID_SIZE>(int)id)
		{
			return (NmpXmlMsg*)xml_table_parse(table, id, buf, size, err, flags);
		}
        else
            *err = -E_COMMAND_INVALID;
	}
  
	return NULL;
}








