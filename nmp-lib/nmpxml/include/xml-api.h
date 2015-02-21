
#ifndef __J_XML_API_H__
#define __J_XML_API_H__


#include "nmp_packet.h"
#include "nmp_xmlmsg.h"
#include "nmp_xmlmem.h"

/*
 * C++ support...
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int parse_get_css_request_xml(void *pvalue, const char *buffer);
    int parse_get_css_response_xml(void *pvalue, const char *buffer);
    int parse_pu_register_css_xml(void *pvalue, const char *buffer);
    
    int merge_get_css_request_xml(void *pvalue, const char *buffer, size_t size);
    int merge_get_css_response_xml(void *pvalue, const char *buffer, size_t size);
    int merge_pu_register_css_xml(void *pvalue, const char *buffer, size_t size);

	int parse_register_request_xml(void *pvalue, const char *buffer);
	int parse_register_response_xml(void *pvalue, const char *buffer);
	int parse_heart_beat_request_xml(void *pvalue, const char *buffer);
	int parse_heart_beat_response_xml(void *pvalue, const char *buffer);
	
	int parse_get_mds_info_request_xml(void *pvalue, const char *buffer);
	int parse_get_mds_info_response_xml(void *pvalue, const char *buffer);
	int parse_change_dispatch_request_xml(void *pvalue, const char *buffer);
	int parse_change_dispatch_result_xml(void *pvalue, const char *buffer);

	//dev_info
	int parse_get_device_info_xml(void *pvalue, const char *buffer);
	int parse_device_info_response_xml(void *pvalue, const char *buffer);

	//ntp_info
	int parse_get_device_ntp_info_xml(void *pvalue, const char *buffer);
	int parse_device_ntp_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_device_ntp_info_xml(void *pvalue, const char *buffer);
	int parse_device_ntp_info_result_xml(void *pvalue, const char *buffer);

	//dev_time
	int parse_get_device_time_xml(void *pvalue, const char *buffer);
	int parse_device_time_response_xml(void *pvalue, const char *buffer);
	int parse_set_device_time_xml(void *pvalue, const char *buffer);
	int parse_device_time_result_xml(void *pvalue, const char *buffer);

	//pltf_info
	int parse_get_platform_info_xml(void *pvalue, const char *buffer);
	int parse_platform_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_platform_info_xml(void *pvalue, const char *buffer);
	int parse_platform_info_result_xml(void *pvalue, const char *buffer);

	//net_info
	int parse_get_network_info_xml(void *pvalue, const char *buffer);
	int parse_network_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_network_info_xml(void *pvalue, const char *buffer);
	int parse_network_info_result_xml(void *pvalue, const char *buffer);

	//pppoe_info	
	int parse_get_pppoe_info_xml(void *pvalue, const char *buffer);
	int parse_pppoe_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_pppoe_info_xml(void *pvalue, const char *buffer);
	int parse_pppoe_info_result_xml(void *pvalue, const char *buffer);

	//encode_para
	int parse_get_encode_parameter_xml(void *pvalue, const char *buffer);
	int parse_encode_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_encode_parameter_xml(void *pvalue, const char *buffer);
	int parse_encode_parameter_result_xml(void *pvalue, const char *buffer);

	//display_para
	int parse_get_display_parameter_xml(void *pvalue, const char *buffer);
	int parse_display_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_display_parameter_xml(void *pvalue, const char *buffer);
	int parse_display_parameter_result_xml(void *pvalue, const char *buffer);
	//record_para
	int parse_get_record_parameter_xml(void *pvalue, const char *buffer);
	int parse_record_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_record_parameter_xml(void *pvalue, const char *buffer);
	int parse_record_parameter_result_xml(void *pvalue, const char *buffer);

	//hide_para
	int parse_get_hide_parameter_xml(void *pvalue, const char *buffer);
	int parse_hide_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_hide_parameter_xml(void *pvalue, const char *buffer);
	int parse_hide_parameter_result_xml(void *pvalue, const char *buffer);
	
	//serial_para
	int parse_get_serial_parameter_xml(void *pvalue, const char *buffer);
	int parse_serial_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_serial_parameter_xml(void *pvalue, const char *buffer);
	int parse_serial_parameter_result_xml(void *pvalue, const char *buffer);

	//osd_para
	int parse_get_osd_parameter_xml(void *pvalue, const char *buffer);
	int parse_osd_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_osd_parameter_xml(void *pvalue, const char *buffer);
	int parse_osd_parameter_result_xml(void *pvalue, const char *buffer);

	//ptz_para
	int parse_get_ptz_parameter_xml(void *pvalue, const char *buffer);
	int parse_ptz_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_ptz_parameter_xml(void *pvalue, const char *buffer);
	int parse_ptz_parameter_result_xml(void *pvalue, const char *buffer);

	//ftp_para
	int parse_get_ftp_parameter_xml(void *pvalue, const char *buffer);
	int parse_ftp_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_ftp_parameter_xml(void *pvalue, const char *buffer);
	int parse_ftp_parameter_result_xml(void *pvalue, const char *buffer);

	//smtp_para	
	int parse_get_smtp_parameter_xml(void *pvalue, const char *buffer);
	int parse_smtp_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_smtp_parameter_xml(void *pvalue, const char *buffer);
	int parse_smtp_parameter_result_xml(void *pvalue, const char *buffer);

	//upnp_para
	int parse_get_upnp_parameter_xml(void *pvalue, const char *buffer);
	int parse_upnp_parameter_response_xml(void *pvalue, const char *buffer);
	int parse_set_upnp_parameter_xml(void *pvalue, const char *buffer);
	int parse_upnp_parameter_result_xml(void *pvalue, const char *buffer);
	
	//disk_info
	int parse_get_device_disk_info_xml(void *pvalue, const char *buffer);
	int parse_device_disk_info_response_xml(void *pvalue, const char *buffer);

	//format_disk
	int parse_format_disk_request_xml(void *pvalue, const char *buffer);
	int parse_format_disk_result_xml(void *pvalue, const char *buffer);
	int parse_submit_format_progress_xml(void *pvalue, const char *buffer);

	//move_alarm
	int parse_get_move_alarm_info_xml(void *pvalue, const char *buffer);
	int parse_move_alarm_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_move_alarm_info_xml(void *pvalue, const char *buffer);
	int parse_move_alarm_info_result_xml(void *pvalue, const char *buffer);

	//lost_alarm
	int parse_get_lost_alarm_info_xml(void *pvalue, const char *buffer);
	int parse_lost_alarm_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_lost_alarm_info_xml(void *pvalue, const char *buffer);
	int parse_lost_alarm_info_result_xml(void *pvalue, const char *buffer);

	//hide_alarm
	int parse_get_hide_alarm_info_xml(void *pvalue, const char *buffer);
	int parse_hide_alarm_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_hide_alarm_info_xml(void *pvalue, const char *buffer);
	int parse_hide_alarm_info_result_xml(void *pvalue, const char *buffer);

	//io_alarm
	int parse_get_io_alarm_info_xml(void *pvalue, const char *buffer);
	int parse_io_alarm_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_io_alarm_info_xml(void *pvalue, const char *buffer);
	int parse_io_alarm_info_result_xml(void *pvalue, const char *buffer);
	
	int parse_get_joint_action_info_xml(void *pvalue, const char *buffer);
	int parse_joint_action_info_response_xml(void *pvalue, const char *buffer);
	int parse_set_joint_action_info_xml(void *pvalue, const char *buffer);
	int parse_joint_action_info_result_xml(void *pvalue, const char *buffer);

	int parse_control_ptz_cmd_xml(void *pvalue, const char *buffer);
	int parse_ptz_cmd_response_xml(void *pvalue, const char *buffer);
	
	int parse_submit_alarm_request_xml(void *pvalue, const char *buffer);
	
	int parse_get_media_url_request_xml(void *pvalue, const char *buffer);
	int parse_get_media_url_response_xml(void *pvalue, const char *buffer);

	int parse_get_store_log_request_xml(void *pvalue, const char *buffer);
	int parse_get_store_log_response_xml(void *pvalue, const char *buffer);

	int parse_user_login_request_xml(void *pvalue, const char *buffer);
	int parse_user_login_result_xml(void *pvalue, const char *buffer);
	
	int parse_user_heart_request_xml(void *pvalue, const char *buffer);
	int parse_user_heart_response_xml(void *pvalue, const char *buffer);
	
	int parse_firmware_upgrade_request_xml(void *pvalue, const char *buffer);
	int parse_firmware_upgrade_response_xml(void *pvalue, const char *buffer);
	int parse_submit_upgrade_progress_xml(void *pvalue, const char *buffer);
    

	//##########################################################################
	int merge_register_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_register_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_heart_beat_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_heart_beat_response_xml(void *pvalue, char *buffer, size_t size);
	
	int merge_get_mds_info_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_get_mds_info_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_change_dispatch_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_change_dispatch_result_xml(void *pvalue, char *buffer, size_t size);
	
	//device_info
	int merge_get_device_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_device_info_response_xml(void *pvalue, char *buffer, size_t size);
	
	//ntp_info
	int merge_get_device_ntp_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_device_ntp_info_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_device_ntp_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_device_ntp_info_result_xml(void *pvalue, char *buffer, size_t size);
	
	//device_time
	int merge_get_device_time_xml(void *pvalue, char *buffer, size_t size);
	int merge_device_time_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_device_time_xml(void *pvalue, char *buffer, size_t size);
	int merge_device_time_result_xml(void *pvalue, char *buffer, size_t size);
	
	//plathrom_info
	int merge_get_platform_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_platform_info_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_platform_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_platform_info_result_xml(void *pvalue, char *buffer, size_t size);
	
	//network_info
	int merge_get_network_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_network_info_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_network_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_network_info_result_xml(void *pvalue, char *buffer, size_t size);

	//pppoe_info	
	int merge_get_pppoe_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_pppoe_info_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_pppoe_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_pppoe_info_result_xml(void *pvalue, char *buffer, size_t size);

	//encode_para
	int merge_get_encode_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_encode_parameter_response_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_set_encode_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_encode_parameter_result_xml(void *pvalue, char *buffer, size_t size);

	//display_para
	int merge_get_display_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_display_parameter_response_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_set_display_parameter_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_display_parameter_result_xml(void *pvalue, 
																char *buffer, size_t size);
	
	//record_para
	int merge_get_record_parameter_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_record_parameter_response_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_set_record_parameter_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_record_parameter_result_xml(void *pvalue, 
																char *buffer, size_t size);

	//hide_para
	int merge_get_hide_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_hide_parameter_response_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_set_hide_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_hide_parameter_result_xml(void *pvalue, char *buffer, size_t size);
	
	//serial_para
	int merge_get_serial_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_serial_parameter_response_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_set_serial_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_serial_parameter_result_xml(void *pvalue, char *buffer, size_t size);

	//osd_para
	int merge_get_osd_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_osd_parameter_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_osd_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_osd_parameter_result_xml(void *pvalue, char *buffer, size_t size);

	//ptz_para
	int merge_get_ptz_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_ptz_parameter_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_ptz_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_ptz_parameter_result_xml(void *pvalue, char *buffer, size_t size);

	//ftp_para
	int merge_get_ftp_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_ftp_parameter_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_ftp_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_ftp_parameter_result_xml(void *pvalue, char *buffer, size_t size);

	//smtp_para	
	int merge_get_smtp_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_smtp_parameter_response_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_set_smtp_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_smtp_parameter_result_xml(void *pvalue, char *buffer, size_t size);

	//upnp_para
	int merge_get_upnp_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_upnp_parameter_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_upnp_parameter_xml(void *pvalue, char *buffer, size_t size);
	int merge_upnp_parameter_result_xml(void *pvalue, char *buffer, size_t size);
	
	//disk_info
	int merge_get_device_disk_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_device_disk_info_response_xml(void *pvalue, char *buffer, size_t size);

	//format_disk
	int merge_format_disk_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_format_disk_result_xml(void *pvalue, char *buffer, size_t size);
	int merge_submit_format_progress_xml(void *pvalue, char *buffer, size_t size);

	//move_alarm
	int merge_get_move_alarm_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_move_alarm_info_response_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_set_move_alarm_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_move_alarm_info_result_xml(void *pvalue, char *buffer, size_t size);

	//lost_alarm
	int merge_get_lost_alarm_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_lost_alarm_info_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_lost_alarm_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_lost_alarm_info_result_xml(void *pvalue, char *buffer, size_t size);

	//hide_alarm
	int merge_get_hide_alarm_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_hide_alarm_info_response_xml(void *pvalue, 
																char *buffer, size_t size);
	int merge_set_hide_alarm_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_hide_alarm_info_result_xml(void *pvalue, char *buffer, size_t size);

	//io_alarm
	int merge_get_io_alarm_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_io_alarm_info_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_io_alarm_info_xml(void *pvalue, char *buffer, size_t size);
	int merge_io_alarm_info_result_xml(void *pvalue, char *buffer, size_t size);
	
	//joint_action
	int merge_get_joint_action_info_xml(void *pvalue, char *buffer, size_t size);	
	int merge_joint_action_info_response_xml(void *pvalue, char *buffer, size_t size);	
	int merge_set_joint_action_info_xml(void *pvalue, char *buffer, size_t size);	
	int merge_joint_action_info_result_xml(void *pvalue, char *buffer, size_t size);

	int merge_control_ptz_cmd_xml(void *pvalue, char *buffer, size_t size);	
	int merge_ptz_cmd_result_xml(void *pvalue, char *buffer, size_t size);

	int merge_submit_alarm_request_xml(void *pvalue, char *buffer, size_t size);
	
	int merge_get_media_url_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_get_media_url_response_xml(void *pvalue, char *buffer, size_t size);
	
	int merge_get_store_log_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_get_store_log_response_xml(void *pvalue, char *buffer, size_t size);

	int merge_user_login_request_xml(void *pvalue, char *buffer, size_t size);	
	int merge_user_login_result_xml(void *pvalue, char *buffer, size_t size);	
	
	int merge_user_heart_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_user_heart_response_xml(void *pvalue, char *buffer, size_t size);
	
	int merge_firmware_upgrade_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_firmware_upgrade_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_submit_upgrade_progress_xml(void *pvalue, char *buffer, size_t size);


#ifdef HAVE_PROXY_INFO
#include "proxy_info.h"

	int parse_add_user_request_xml(void *pvalue, const char *buffer);	
	int parse_add_user_result_xml(void *pvalue, const char *buffer);	
	int parse_del_user_request_xml(void *pvalue, const char *buffer);	
	int parse_del_user_result_xml(void *pvalue, const char *buffer);

	int merge_add_user_request_xml(void *pvalue, 
					char *buffer, size_t size);
	int merge_add_user_result_xml(void *pvalue, 
					char *buffer, size_t size);
	int merge_del_user_request_xml(void *pvalue, 
					char *buffer, size_t size);
	int merge_del_user_result_xml(void *pvalue, 
					char *buffer, size_t size);
	
	int parse_proxy_user_list_xml(void *pvalue, 
				const char *buffer);
	int merge_proxy_user_list_xml(void *pvalue, 
				char *buffer, size_t size);	
	int parse_proxy_device_list_xml(void *pvalue, 
				const char *buffer);	
	int merge_proxy_device_list_xml(void *pvalue, 
				char *buffer, size_t size);
	
	int parse_fuzzy_find_user_request_xml(void *pvalue, 
				const char *buffer);
	int parse_fuzzy_find_user_result_xml(void *pvalue, 
				const char *buffer);	
	int merge_fuzzy_find_user_request_xml(void *pvalue, 
				char *buffer, size_t size);
	int merge_fuzzy_find_user_result_xml(void *pvalue, 
				char *buffer, size_t size);
	
	int parse_modify_password_request_xml(void *pvalue, 
				const char *buffer);
	int parse_modify_password_result_xml(void *pvalue, 
				const char *buffer);
	int merge_modify_password_request_xml(void *pvalue, 
				char *buffer, size_t size);
	int merge_modify_password_result_xml(void *pvalue, 
				char *buffer, size_t size);

	int parse_add_device_request_xml(void *pvalue, 
				const char *buffer);
	int parse_add_device_result_xml(void *pvalue, 
				const char *buffer);
	int merge_add_device_request_xml(void *pvalue, 
				char *buffer, size_t size);	
	int merge_add_device_result_xml(void *pvalue, 
				char *buffer, size_t size);

	int parse_del_device_request_xml(void *pvalue, 
				const char *buffer);
	int parse_del_device_result_xml(void *pvalue, 
				const char *buffer);
	int merge_del_device_request_xml(void *pvalue, 
				char *buffer, size_t size);
	int merge_del_device_result_xml(void *pvalue, 
				char *buffer, size_t size);
	
	int parse_get_device_info_request_xml(void *pvalue, 
				const char *buffer);
	int parse_get_device_info_result_xml(void *pvalue, 
				const char *buffer);
	int parse_set_device_info_request_xml(void *pvalue, 
				const char *buffer);
	int parse_set_device_info_result_xml(void *pvalue, 
				const char *buffer);
	int merge_get_device_info_request_xml(void *pvalue, 
				char *buffer, size_t size);
	int merge_get_device_info_result_xml(void *pvalue, 
				char *buffer, size_t size);
	int merge_set_device_info_request_xml(void *pvalue, 
				char *buffer, size_t size);
	int merge_set_device_info_result_xml(void *pvalue, 
				char *buffer, size_t size);


	int parse_get_all_device_id_request_xml(void *pvalue, 
				const char *buffer);
	int parse_get_all_device_id_result_xml(void *pvalue, 
				const char *buffer);	
	int merge_get_all_device_id_request_xml(void *pvalue, 
				char *buffer, size_t size);
	int merge_get_all_device_id_result_xml(void *pvalue, 
				char *buffer, size_t size);
	
	int parse_proxy_factory_list_xml(void *pvalue, 
				const char *buffer);
	int merge_proxy_factory_list_xml(void *pvalue, 
				char *buffer, size_t size);
	
	int parse_get_factory_info_request_xml(void *pvalue, 
				const char *buffer);
	int parse_get_factory_info_response_xml(void *pvalue, 
				const char *buffer);
	int merge_get_factory_info_response_xml(void *pvalue, 
				char *buffer, size_t size);
  
  	int parse_proxy_page_user_request_xml(void *pvalue, 
  				const char *buffer);
	int merge_proxy_page_user_response_xml(void *pvalue, 
				char *buffer, size_t size);
	
	int merge_broadcast_add_user_xml(void *pvalue, char *buffer, size_t size);
	int merge_broadcast_del_user_xml(void *pvalue, char *buffer, size_t size);
	int merge_broadcast_add_device_xml(void *pvalue, char *buffer, size_t size);
	int merge_broadcast_del_device_xml(void *pvalue, char *buffer, size_t size);
	int merge_broadcast_modify_device_xml(void *pvalue, char *buffer, size_t size);
	int merge_broadcast_device_status_xml(void *pvalue, char *buffer, size_t size);
	
	int parse_get_server_config_request_xml(void *pvalue, const char *buffer);
	int parse_get_server_config_response_xml(void *pvalue, const char *buffer);
	int parse_set_server_config_request_xml(void *pvalue, const char *buffer);
	int parse_set_server_config_result_xml(void *pvalue, const char *buffer);
	int merge_get_server_config_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_get_server_config_response_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_server_config_request_xml(void *pvalue, char *buffer, size_t size);
	int merge_set_server_config_result_xml(void *pvalue, char *buffer, size_t size);

    int parse_download_request_xml(void *pvalue, const char *buffer);
    int parse_download_response_xml(void *pvalue, const char *buffer);
    int parse_upload_request_xml(void *pvalue, const char *buffer);
    int parse_upload_response_xml(void *pvalue, const char *buffer);
    int merge_download_request_xml(void *pvalue, char *buffer, size_t size);
    int merge_download_response_xml(void *pvalue, char *buffer, size_t size);
    int merge_upload_request_xml(void *pvalue, char *buffer, size_t size);
    int merge_upload_response_xml(void *pvalue, char *buffer, size_t size);
    
    int parse_limit_broadcast_status_xml(void *pvalue, const char *buffer);


#endif


int parse_get_channel_info_request_xml(void *pvalue, const char *buffer);
int parse_get_channel_info_response_xml(void *pvalue, const char *buffer);
int parse_set_channel_info_request_xml(void *pvalue, const char *buffer);
int parse_set_channel_info_result_xml(void *pvalue, const char *buffer);
int parse_get_picture_info_request_xml(void *pvalue, const char *buffer);
int parse_get_picture_info_response_xml(void *pvalue, const char *buffer);
int parse_set_picture_info_request_xml(void *pvalue, const char *buffer);
int parse_set_picture_info_result_xml(void *pvalue, const char *buffer);
//int parse_get_hl_picture_info_request_xml(void *pvalue, const char *buffer);
//int parse_get_hl_picture_info_response_xml(void *pvalue, const char *buffer);
//int parse_set_hl_picture_info_request_xml(void *pvalue, const char *buffer);
//int parse_set_hl_picture_info_result_xml(void *pvalue, const char *buffer);
int parse_get_wifi_config_request_xml(void *pvalue, const char *buffer);
int parse_get_wifi_config_response_xml(void *pvalue, const char *buffer);
int parse_set_wifi_config_request_xml(void *pvalue, const char *buffer);
int parse_set_wifi_config_result_xml(void *pvalue, const char *buffer);
int parse_wifi_search_request_xml(void *pvalue, const char *buffer);
int parse_wifi_search_response_xml(void *pvalue, const char *buffer);
int parse_get_network_status_request_xml(void *pvalue, const char *buffer);
int parse_get_network_status_response_xml(void *pvalue, const char *buffer);
int parse_control_device_request_xml(void *pvalue, const char *buffer);
int parse_control_device_result_xml(void *pvalue, const char *buffer);
int parse_get_ddns_config_request_xml(void *pvalue, const char *buffer);
int parse_get_ddns_config_response_xml(void *pvalue, const char *buffer);
int parse_set_ddns_config_request_xml(void *pvalue, const char *buffer);
int parse_set_ddns_config_result_xml(void *pvalue, const char *buffer);
int parse_get_def_display_info_request_xml(void *pvalue, const char *buffer);
int parse_get_def_display_info_response_xml(void *pvalue, const char *buffer);
int parse_get_def_picture_info_request_xml(void *pvalue, const char *buffer);
int parse_get_def_picture_info_response_xml(void *pvalue, const char *buffer);
int parse_get_avd_config_request_xml(void *pvalue, const char *buffer);
int parse_get_avd_config_response_xml(void *pvalue, const char *buffer);
int parse_set_avd_config_request_xml(void *pvalue, const char *buffer);
int parse_set_avd_config_result_xml(void *pvalue, const char *buffer);

int merge_get_channel_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_channel_info_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_channel_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_channel_info_result_xml(void *pvalue, char *buffer, size_t size);
int merge_get_picture_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_picture_info_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_picture_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_picture_info_result_xml(void *pvalue, char *buffer, size_t size);
//int merge_get_hl_picture_info_request_xml(void *pvalue, char *buffer, size_t size);
//int merge_get_hl_picture_info_response_xml(void *pvalue, char *buffer, size_t size);
//int merge_set_hl_picture_info_request_xml(void *pvalue, char *buffer, size_t size);
//int merge_set_hl_picture_info_result_xml(void *pvalue, char *buffer, size_t size);
int merge_get_wifi_config_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_wifi_config_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_wifi_config_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_wifi_config_result_xml(void *pvalue, char *buffer, size_t size);
int merge_wifi_search_request_xml(void *pvalue, char *buffer, size_t size);
int merge_wifi_search_response_xml(void *pvalue, char *buffer, size_t size);
int merge_get_network_status_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_network_status_response_xml(void *pvalue, char *buffer, size_t size);
int merge_control_device_request_xml(void *pvalue, char *buffer, size_t size);
int merge_control_device_result_xml(void *pvalue, char *buffer, size_t size);
int merge_get_ddns_config_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_ddns_config_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_ddns_config_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_ddns_config_result_xml(void *pvalue, char *buffer, size_t size);
int merge_get_def_display_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_def_display_info_response_xml(void *pvalue, char *buffer, size_t size);
int merge_get_def_picture_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_def_picture_info_response_xml(void *pvalue, char *buffer, size_t size);
int merge_get_avd_config_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_avd_config_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_avd_config_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_avd_config_result_xml(void *pvalue, char *buffer, size_t size);






int parse_get_transparent_param_request_xml(void *pvalue, const char *buffer);
int parse_get_transparent_param_response_xml(void *pvalue, const char *buffer);
int parse_set_transparent_param_request_xml(void *pvalue, const char *buffer);
int parse_set_transparent_param_response_xml(void *pvalue, const char *buffer);
int parse_transparent_notify_enevt_xml(void * pvalue,const char * buffer);
int parse_transparent_control_device_xml(void * pvalue,const char * buffer);

int merge_get_transparent_param_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_transparent_param_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_transparent_param_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_transparent_param_response_xml(void *pvalue, char *buffer, size_t size);
int merge_transparent_notify_enevt_xml(void *pvalue, char *buffer, size_t size);
int merge_transparent_control_device_xml(void *pvalue, char *buffer, size_t size);


# ifdef _USE_DECODER_PROTO_

int parse_query_division_mode_request_xml(void *pvalue, const char *buffer);
int parse_query_division_mode_response_xml(void *pvalue, const char *buffer);
int parse_get_screen_state_request_xml(void *pvalue, const char *buffer);
int parse_get_screen_state_response_xml(void *pvalue, const char *buffer);
int parse_set_division_mode_request_xml(void *pvalue, const char *buffer);
int parse_set_division_mode_response_xml(void *pvalue, const char *buffer);
int parse_set_full_screen_request_xml(void *pvalue, const char *buffer);
int parse_set_full_screen_response_xml(void *pvalue, const char *buffer);
int parse_exit_full_screen_request_xml(void *pvalue, const char *buffer);
int parse_exit_full_screen_response_xml(void *pvalue, const char *buffer);
int parse_tv_wall_play_request_xml(void *pvalue, const char *buffer);
int parse_tv_wall_play_response_xml(void *pvalue, const char *buffer);
int parse_clear_division_request_xml(void *pvalue, const char *buffer);
int parse_clear_division_response_xml(void *pvalue, const char *buffer);

int merge_query_division_mode_request_xml(void *pvalue, char *buffer, size_t size);
int merge_query_division_mode_response_xml(void *pvalue, char *buffer, size_t size);
int merge_get_screen_state_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_screen_state_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_division_mode_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_division_mode_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_full_screen_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_full_screen_response_xml(void *pvalue, char *buffer, size_t size);
int merge_exit_full_screen_request_xml(void *pvalue, char *buffer, size_t size);
int merge_exit_full_screen_response_xml(void *pvalue, char *buffer, size_t size);
int merge_tv_wall_play_request_xml(void *pvalue, char *buffer, size_t size);
int merge_tv_wall_play_response_xml(void *pvalue, char *buffer, size_t size);
int merge_clear_division_request_xml(void *pvalue, char *buffer, size_t size);
int merge_clear_division_response_xml(void *pvalue, char *buffer, size_t size);

# endif //_USE_DECODER_PROTO_


int parse_get_operation_log_request_xml(void *pvalue, const char *buffer);
int parse_get_operation_log_response_xml(void *pvalue, const char *buffer);
int merge_get_operation_log_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_operation_log_response_xml(void *pvalue, char *buffer, size_t size);

int parse_set_alarm_upload_config_request_xml(void *pvalue, const char *buffer);
int parse_set_alarm_upload_config_response_xml(void *pvalue, const char *buffer);
int merge_set_alarm_upload_config_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_alarm_upload_config_response_xml(void *pvalue, char *buffer, size_t size);


int parse_get_preset_point_set_request_xml(void *pvalue, const char *buffer);
int parse_get_preset_point_set_response_xml(void *pvalue, const char *buffer);
int parse_set_preset_point_request_xml(void *pvalue, const char *buffer);
int parse_set_preset_point_result_xml(void *pvalue, const char *buffer);
int parse_get_cruise_way_set_request_xml(void *pvalue, const char *buffer);
int parse_get_cruise_way_set_response_xml(void *pvalue, const char *buffer);
int parse_set_cruise_way_request_xml(void *pvalue, const char *buffer);
int parse_set_cruise_way_result_xml(void *pvalue, const char *buffer);
int parse_get_cruise_way_request_xml(void *pvalue, const char *buffer);
int parse_get_cruise_way_response_xml(void *pvalue, const char *buffer);
int parse_add_cruise_way_request_xml(void *pvalue, const char *buffer);
int parse_add_cruise_way_result_xml(void *pvalue, const char *buffer);
int parse_modify_cruise_way_request_xml(void *pvalue, const char *buffer);
int parse_modify_cruise_way_result_xml(void *pvalue, const char *buffer);

int merge_get_preset_point_set_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_preset_point_set_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_preset_point_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_preset_point_result_xml(void *pvalue, char *buffer, size_t size);
int merge_get_cruise_way_set_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_cruise_way_set_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_cruise_way_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_cruise_way_result_xml(void *pvalue, char *buffer, size_t size);
int merge_get_cruise_way_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_cruise_way_response_xml(void *pvalue, char *buffer, size_t size);
int merge_add_cruise_way_request_xml(void *pvalue, char *buffer, size_t size);
int merge_add_cruise_way_result_xml(void *pvalue, char *buffer, size_t size);
int merge_modify_cruise_way_request_xml(void *pvalue, char *buffer, size_t size);
int merge_modify_cruise_way_result_xml(void *pvalue, char *buffer, size_t size);

int merge_get_cruise_capability_set_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_cruise_capability_set_response_xml(void *pvalue, char *buffer, size_t size);
int parse_get_cruise_capability_set_request_xml(void *pvalue, const char *buffer);
int parse_get_cruise_capability_set_response_xml(void *pvalue, const char *buffer);

int parse_3d_control_request_xml(void *pvalue, const char *buffer);
int parse_3d_control_result_xml(void *pvalue, const char *buffer);
int parse_3d_goback_request_xml(void *pvalue, const char *buffer);
int parse_3d_goback_result_xml(void *pvalue, const char *buffer);
int merge_3d_control_request_xml(void *pvalue, char *buffer, size_t size);
int merge_3d_control_result_xml(void *pvalue, char *buffer, size_t size);
int merge_3d_goback_request_xml(void *pvalue, char *buffer, size_t size);
int merge_3d_goback_result_xml(void *pvalue, char *buffer, size_t size);

int parse_alarm_link_io_request_xml(void *pvalue, const char *buffer);
int parse_alarm_link_io_result_xml(void *pvalue, const char *buffer);
int parse_alarm_link_preset_request_xml(void *pvalue, const char *buffer);
int parse_alarm_link_preset_result_xml(void *pvalue, const char *buffer);

int parse_get_resolution_info_request_xml(void *pvalue, const char *buffer);
int parse_get_resolution_info_response_xml(void *pvalue, const char *buffer);
int parse_set_resolution_info_request_xml(void *pvalue, const char *buffer);
int parse_set_resolution_info_result_xml(void *pvalue, const char *buffer);

int merge_alarm_link_io_request_xml(void *pvalue, char *buffer, size_t size);
int merge_alarm_link_io_result_xml(void *pvalue, char *buffer, size_t size);
int merge_alarm_link_preset_request_xml(void *pvalue, char *buffer, size_t size);
int merge_alarm_link_preset_result_xml(void *pvalue, char *buffer, size_t size);

int merge_get_resolution_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_resolution_info_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_resolution_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_resolution_info_result_xml(void *pvalue, char *buffer, size_t size);

int parse_get_ircut_control_info_request_xml(void *pvalue, const char *buffer);
int parse_get_ircut_control_info_response_xml(void *pvalue, const char *buffer);
int parse_set_ircut_control_info_request_xml(void *pvalue, const char *buffer);
int parse_set_ircut_control_info_result_xml(void *pvalue, const char *buffer);

int merge_get_ircut_control_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_ircut_control_info_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_ircut_control_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_ircut_control_info_result_xml(void *pvalue, char *buffer, size_t size);

int parse_get_extranet_port_request_xml(void *pvalue, const char *buffer);
int parse_get_extranet_port_response_xml(void *pvalue, const char *buffer);

int merge_get_extranet_port_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_extranet_port_response_xml(void *pvalue, char *buffer, size_t size);

int parse_get_herd_analyse_info_request_xml(void *pvalue, const char *buffer);
int parse_get_herd_analyse_info_response_xml(void *pvalue, const char *buffer);
int parse_set_herd_analyse_info_request_xml(void *pvalue, const char *buffer);
int parse_set_herd_analyse_info_result_xml(void *pvalue, const char *buffer);

int merge_get_herd_analyse_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_herd_analyse_info_response_xml(void *pvalue, char *buffer, size_t size);
int merge_set_herd_analyse_info_request_xml(void *pvalue, char *buffer, size_t size);
int merge_set_herd_analyse_info_result_xml(void *pvalue, char *buffer, size_t size);

int parse_get_grass_percent_request_xml(void *pvalue, const char *buffer);
int parse_get_grass_percent_response_xml(void *pvalue, const char *buffer);
int merge_get_grass_percent_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_grass_percent_response_xml(void *pvalue, char *buffer, size_t size);

int parse_get_p2p_id_request_xml(void *pvalue, const char *buffer);
int parse_get_p2p_id_response_xml(void *pvalue, const char *buffer);
int merge_get_p2p_id_request_xml(void *pvalue, char *buffer, size_t size);
int merge_get_p2p_id_response_xml(void *pvalue, char *buffer, size_t size);

/*
* C++ support...
*/
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //__J_XML_API_H__


