#ifndef __J_XML_HANDLER_H__
#define __J_XML_HANDLER_H__

#include <stdio.h>

#include "config.h"
#include "nmp_packet.h"
#include "nmp_xmlmsg.h"


#define XML_DATA_ID(msg)	((msg)->id)
#define XML_DATA(msg)		((msg)->data)
#define XML_DATA_SIZE(msg)	((msg)->size - sizeof(XmlData))



typedef int (*NmpCreateXml)(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
typedef NmpXmlMsg *(*NmpParseXml)(char buf[], size_t size, int *err, unsigned int flags);

typedef struct __XmlData
{
	xmlid_t	id;					//
	size_t	size;				//整个XmlData结构的大小
	char	data[0];			//xml 数据，注意: data 不是指针，
								//而是结构体的成员的开始地址，大小可变
}XmlData;

typedef struct __XmlEntry
{
	xmlid_t id;
	char command[J_SDK_MAX_COMMAND_LEN];
	unsigned int flags;
	NmpCreateXml func_c;
	NmpParseXml  func_p;
}XmlEntry;

typedef struct __XmlTable
{
	int n_entries;
	int n_capacity;
	XmlEntry *entries;
}XmlTable;

extern XmlTable *get_xml_table();
extern int register_xml_table(XmlTable *table, xmlid_t id, char *command, 
		NmpCreateXml func_c, NmpParseXml func_p, unsigned int flags);
extern int xml_table_create(XmlTable *table, NmpXmlMsg *msg, 
		char buf[], size_t size, unsigned int flags);
extern void *xml_table_parse(XmlTable *table, xmlid_t id, 
		char buf[], size_t size, int *err, unsigned int flags);

extern int create_get_css_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_css_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_pu_register_css(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_register_css_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);

extern int create_register_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_register_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_heart_beat_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_heart_beat_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_mds_info_request(NmpXmlMsg *msg, char buf[],  size_t size, unsigned int flags);
extern int create_get_mds_info_response(NmpXmlMsg *msg, char buf[],  size_t size, unsigned int flags);
extern int create_change_dispatch_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_change_dispatch_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_device_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_device_ntp_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_ntp_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_device_ntp_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_ntp_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_device_time(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_time_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_device_time(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_time_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_platform_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_platform_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_platform_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_platform_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_network_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_network_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_network_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_network_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_pppoe_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_pppoe_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_pppoe_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_pppoe_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_encode_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_encode_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_encode_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_encode_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_display_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_display_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_display_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_display_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_record_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_record_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_record_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_record_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_hide_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_hide_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_hide_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_hide_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_serial_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_serial_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_serial_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_serial_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_osd_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_osd_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_osd_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_osd_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_ptz_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ptz_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_ptz_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ptz_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_ftp_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ftp_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_ftp_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ftp_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_smtp_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_smtp_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_smtp_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_smtp_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_upnp_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_upnp_parameter_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_upnp_parameter(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_upnp_parameter_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_device_disk_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_disk_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_format_disk_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_format_disk_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_submit_format_progress(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_move_alarm_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_move_alarm_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_move_alarm_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_move_alarm_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_lost_alarm_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_lost_alarm_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_lost_alarm_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_lost_alarm_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_hide_alarm_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_hide_alarm_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_hide_alarm_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_hide_alarm_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_io_alarm_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_io_alarm_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_io_alarm_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_io_alarm_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_joint_action_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_joint_action_info_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_joint_action_info(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_joint_action_info_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_control_ptz_cmd(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ptz_cmd_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_submit_alarm_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_media_url_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_media_url_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_store_log_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_store_log_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_user_login_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_user_login_result(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_user_heart_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_user_heart_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_firmware_upgrade_request(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_firmware_upgrade_response(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_submit_upgrade_progress(NmpXmlMsg *msg, char buf[], size_t size, unsigned int flags);


extern NmpXmlMsg *parse_get_css_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_css_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_pu_register_css(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_register_css_response(char buf[], size_t size, int *err, unsigned int flags);

extern NmpXmlMsg *parse_register_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_register_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_heart_beat_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_heart_beat_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_mds_info_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_mds_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_change_dispatch_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_change_dispatch_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_device_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_device_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_device_ntp_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_device_ntp_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_device_ntp_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_device_ntp_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_device_time(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_device_time_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_device_time(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_device_time_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_platform_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_platform_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_platform_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_platform_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_network_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_network_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_network_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_network_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_pppoe_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_pppoe_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_pppoe_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_pppoe_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_encode_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_encode_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_encode_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_encode_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_display_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_display_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_display_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_display_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_record_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_record_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_record_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_record_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_hide_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_hide_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_hide_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_hide_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_serial_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_serial_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_serial_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_serial_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_osd_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_osd_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_osd_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_osd_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_ptz_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_ptz_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_ptz_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_ptz_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_ftp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_ftp_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_ftp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_ftp_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_smtp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_smtp_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_smtp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_smtp_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_upnp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_upnp_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_upnp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_upnp_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_device_disk_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_device_disk_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_format_disk_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_format_disk_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_submit_format_progress(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_move_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_move_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_move_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_move_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_lost_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_lost_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_lost_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_lost_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_hide_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_hide_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_hide_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_hide_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_io_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_io_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_io_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_io_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_joint_action_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_joint_action_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_set_joint_action_info(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_joint_action_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_control_ptz_cmd(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_ptz_cmd_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_submit_alarm_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_media_url_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_media_url_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_store_log_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_get_store_log_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_user_login_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_user_login_result(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_user_heart_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_user_heart_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_firmware_upgrade_request(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_firmware_upgrade_response(char buf[], size_t size, int *err, unsigned int flags);
extern NmpXmlMsg *parse_submit_upgrade_progress(char buf[], size_t size, int *err, unsigned int flags);



#ifdef HAVE_PROXY_INFO
#include "proxy_info.h"

	
	extern NmpXmlMsg *parse_add_user_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_add_user_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_del_user_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_del_user_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	
	extern NmpXmlMsg *parse_user_list_info(char buf[], 
			size_t size, int *err, unsigned int flags);	
	extern NmpXmlMsg *parse_device_list_info(char buf[], 
			size_t size, int *err, unsigned int flags);	
	extern NmpXmlMsg *parse_factory_list_info(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_get_factory_info_request(char buf[], 
			size_t size, int *err, unsigned int flags);	
	extern NmpXmlMsg *parse_get_factory_info_response(char buf[], 
			size_t size, int *err, unsigned int flags);	

	extern NmpXmlMsg *parse_fuzzy_find_user_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_fuzzy_find_user_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_modify_password_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_modify_password_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	
	extern NmpXmlMsg *parse_add_device_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_add_device_result(char buf[], 
			size_t size, int *err, unsigned int flags);	
	extern NmpXmlMsg *parse_del_device_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_del_device_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	
	extern NmpXmlMsg *parse_get_device_info_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_get_device_info_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_set_device_info_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_set_device_info_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	
	extern NmpXmlMsg *parse_get_all_device_id_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern NmpXmlMsg *parse_get_all_device_id_result(char buf[], 
			size_t size, int *err, unsigned int flags);

	
//#########################################################
	
	extern int create_add_user_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_add_user_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_del_user_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_del_user_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	
	extern int create_user_list_info(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_device_list_info(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_factory_list_info(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_get_factory_info_response(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);

	extern int create_fuzzy_find_user_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_fuzzy_find_user_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_modify_password_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_modify_password_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);

	extern int create_add_device_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_add_device_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);	
	extern int create_del_device_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_del_device_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	
	extern int create_get_device_info_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_get_device_info_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_set_device_info_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_set_device_info_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	
	extern int create_get_all_device_id_request(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_get_all_device_id_result(NmpXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);

	extern NmpXmlMsg *parse_proxy_page_user_request(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern int create_proxy_page_user_response(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
  
	extern int create_broadcast_add_user(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_del_user(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_add_device(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_del_device(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_modify_device(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_device_status(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	
	extern NmpXmlMsg *parse_get_server_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern NmpXmlMsg *parse_get_server_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern NmpXmlMsg *parse_set_server_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern NmpXmlMsg *parse_set_server_config_result(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern int create_get_server_config_request(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_get_server_config_response(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_set_server_config_request(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_set_server_config_result(NmpXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
    
    extern NmpXmlMsg *parse_download_request(char buf[], size_t size, 
			int *err, unsigned int flags);
    extern NmpXmlMsg *parse_download_response(char buf[], size_t size, 
			int *err, unsigned int flags);
    extern NmpXmlMsg *parse_upload_request(char buf[], size_t size, 
			int *err, unsigned int flags);
    extern NmpXmlMsg *parse_upload_response(char buf[], size_t size, 
			int *err, unsigned int flags);
    extern int create_download_request(NmpXmlMsg *msg, char buf[], 
            size_t size, unsigned int flags);
    extern int create_download_response(NmpXmlMsg *msg, char buf[], 
            size_t size, unsigned int flags);
    extern int create_upload_request(NmpXmlMsg *msg, char buf[], 
            size_t size, unsigned int flags);
    extern int create_upload_response(NmpXmlMsg *msg, char buf[], 
            size_t size, unsigned int flags);

    NmpXmlMsg *parse_limit_broadcast_status(char buf[], size_t size, 
                int *err, unsigned int flags);


#endif

NmpXmlMsg *parse_get_channel_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_channel_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_channel_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_channel_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_picture_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
/*NmpXmlMsg *parse_get_hl_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_hl_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_hl_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_hl_picture_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);*/
NmpXmlMsg *parse_get_wifi_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_wifi_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_wifi_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_wifi_config_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_wifi_search_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_wifi_search_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_network_status_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_network_status_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_control_device_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_control_device_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_ddns_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_ddns_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_ddns_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_ddns_config_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_def_display_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_def_display_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_def_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_def_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_avd_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_avd_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_avd_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_avd_config_result(char buf[], size_t size, 
			int *err, unsigned int flags);


int create_get_channel_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_channel_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_channel_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_channel_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_picture_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_picture_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_picture_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_picture_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_wifi_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_wifi_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_wifi_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_wifi_config_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_wifi_search_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_wifi_search_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_network_status_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_network_status_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_control_device_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_control_device_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

int create_get_ddns_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_ddns_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_ddns_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_ddns_config_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_def_display_info_requst(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_def_display_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_def_picture_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_def_picture_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_avd_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_avd_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_avd_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_avd_config_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);



NmpXmlMsg *parse_get_transparent_param_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_transparent_param_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_transparent_param_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_transparent_param_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_transparent_notify_enevt(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_transparent_control_device(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_transparent_param_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_transparent_param_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_transparent_param_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_transparent_param_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_transparent_notify_enevt(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_transparent_control_device(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

//##########################################################################
# ifdef _USE_DECODER_PROTO_

NmpXmlMsg *parse_query_division_mode_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_query_division_mode_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_screen_state_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_screen_state_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_division_mode_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_division_mode_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_full_screen_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_full_screen_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_exit_full_screen_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_exit_full_screen_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_tv_wall_play_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_tv_wall_play_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_clear_division_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_clear_division_response(char buf[], size_t size, 
			int *err, unsigned int flags);

int create_query_division_mode_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_query_division_mode_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_screen_state_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_screen_state_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_division_mode_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_division_mode_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_full_screen_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_full_screen_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_exit_full_screen_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_exit_full_screen_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_tv_wall_play_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_tv_wall_play_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_clear_division_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_clear_division_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

# endif //_USE_DECODER_PROTO_

int create_get_operation_log_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_operation_log_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
NmpXmlMsg *parse_get_operation_log_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_operation_log_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_alarm_upload_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_alarm_upload_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_set_alarm_upload_config_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_alarm_upload_config_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

NmpXmlMsg *parse_get_preset_point_set_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_preset_point_set_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_preset_point_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_preset_point_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_cruise_way_set_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_cruise_way_set_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_cruise_way_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_add_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_add_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_modify_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_modify_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_preset_point_set_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_preset_point_set_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_preset_point_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_preset_point_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_cruise_way_set_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_cruise_way_set_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_cruise_way_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_cruise_way_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_cruise_way_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_cruise_way_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_add_cruise_way_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_add_cruise_way_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_modify_cruise_way_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_modify_cruise_way_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

NmpXmlMsg *parse_3d_control_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_3d_control_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_3d_goback_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_3d_goback_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_3d_control_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_3d_control_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_3d_goback_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_3d_goback_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

NmpXmlMsg *parse_alarm_link_io_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_alarm_link_io_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_alarm_link_preset_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_alarm_link_preset_result(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_resolution_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_resolution_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_resolution_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_resolution_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_alarm_link_io_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_alarm_link_io_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_alarm_link_preset_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_alarm_link_preset_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_resolution_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_resolution_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_resolution_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_resolution_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

NmpXmlMsg *parse_get_ircut_control_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_ircut_control_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_ircut_control_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_ircut_control_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_ircut_control_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_ircut_control_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_ircut_control_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_ircut_control_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

NmpXmlMsg *parse_get_extranet_port_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_extranet_port_response(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_extranet_port_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_extranet_port_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

NmpXmlMsg *parse_get_herd_analyse_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_herd_analyse_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_herd_analyse_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_set_herd_analyse_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_herd_analyse_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_herd_analyse_info_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_herd_analyse_info_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_herd_analyse_info_result(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

NmpXmlMsg *parse_get_grass_percent_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_grass_percent_response(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_grass_percent_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_grass_percent_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

NmpXmlMsg *parse_get_p2p_id_request(char buf[], size_t size, 
			int *err, unsigned int flags);
NmpXmlMsg *parse_get_p2p_id_response(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_p2p_id_request(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_p2p_id_response(NmpXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);


#endif //__J_XML_HANDLER_H__
