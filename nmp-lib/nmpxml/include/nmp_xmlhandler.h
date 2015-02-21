#ifndef __J_XML_HANDLER_H__
#define __J_XML_HANDLER_H__

#include <stdio.h>

#include "config.h"
#include "nmp_packet.h"
#include "nmp_xmlmsg.h"


#define XML_DATA_ID(msg)	((msg)->id)
#define XML_DATA(msg)		((msg)->data)
#define XML_DATA_SIZE(msg)	((msg)->size - sizeof(XmlData))



typedef int (*JpfCreateXml)(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
typedef JpfXmlMsg *(*JpfParseXml)(char buf[], size_t size, int *err, unsigned int flags);

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
	JpfCreateXml func_c;
	JpfParseXml  func_p;
}XmlEntry;

typedef struct __XmlTable
{
	int n_entries;
	int n_capacity;
	XmlEntry *entries;
}XmlTable;

extern XmlTable *get_xml_table();
extern int register_xml_table(XmlTable *table, xmlid_t id, char *command, 
		JpfCreateXml func_c, JpfParseXml func_p, unsigned int flags);
extern int xml_table_create(XmlTable *table, JpfXmlMsg *msg, 
		char buf[], size_t size, unsigned int flags);
extern void *xml_table_parse(XmlTable *table, xmlid_t id, 
		char buf[], size_t size, int *err, unsigned int flags);

extern int create_get_css_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_css_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_pu_register_css(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_register_css_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);

extern int create_register_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_register_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_heart_beat_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_heart_beat_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_mds_info_request(JpfXmlMsg *msg, char buf[],  size_t size, unsigned int flags);
extern int create_get_mds_info_response(JpfXmlMsg *msg, char buf[],  size_t size, unsigned int flags);
extern int create_change_dispatch_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_change_dispatch_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_device_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_device_ntp_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_ntp_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_device_ntp_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_ntp_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_device_time(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_time_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_device_time(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_time_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_platform_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_platform_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_platform_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_platform_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_network_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_network_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_network_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_network_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_pppoe_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_pppoe_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_pppoe_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_pppoe_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_encode_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_encode_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_encode_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_encode_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_display_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_display_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_display_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_display_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_record_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_record_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_record_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_record_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_hide_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_hide_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_hide_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_hide_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_serial_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_serial_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_serial_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_serial_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_osd_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_osd_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_osd_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_osd_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_ptz_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ptz_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_ptz_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ptz_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_ftp_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ftp_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_ftp_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ftp_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_smtp_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_smtp_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_smtp_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_smtp_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_upnp_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_upnp_parameter_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_upnp_parameter(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_upnp_parameter_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_device_disk_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_device_disk_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_format_disk_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_format_disk_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_submit_format_progress(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_move_alarm_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_move_alarm_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_move_alarm_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_move_alarm_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_lost_alarm_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_lost_alarm_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_lost_alarm_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_lost_alarm_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_hide_alarm_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_hide_alarm_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_hide_alarm_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_hide_alarm_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_io_alarm_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_io_alarm_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_io_alarm_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_io_alarm_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_joint_action_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_joint_action_info_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_set_joint_action_info(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_joint_action_info_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_control_ptz_cmd(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_ptz_cmd_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_submit_alarm_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_media_url_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_media_url_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_store_log_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_get_store_log_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_user_login_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_user_login_result(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_user_heart_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_user_heart_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_firmware_upgrade_request(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_firmware_upgrade_response(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);
extern int create_submit_upgrade_progress(JpfXmlMsg *msg, char buf[], size_t size, unsigned int flags);


extern JpfXmlMsg *parse_get_css_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_css_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_pu_register_css(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_register_css_response(char buf[], size_t size, int *err, unsigned int flags);

extern JpfXmlMsg *parse_register_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_register_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_heart_beat_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_heart_beat_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_mds_info_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_mds_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_change_dispatch_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_change_dispatch_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_device_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_device_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_device_ntp_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_device_ntp_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_device_ntp_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_device_ntp_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_device_time(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_device_time_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_device_time(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_device_time_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_platform_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_platform_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_platform_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_platform_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_network_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_network_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_network_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_network_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_pppoe_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_pppoe_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_pppoe_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_pppoe_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_encode_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_encode_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_encode_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_encode_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_display_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_display_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_display_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_display_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_record_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_record_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_record_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_record_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_hide_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_hide_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_hide_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_hide_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_serial_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_serial_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_serial_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_serial_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_osd_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_osd_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_osd_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_osd_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_ptz_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_ptz_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_ptz_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_ptz_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_ftp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_ftp_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_ftp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_ftp_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_smtp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_smtp_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_smtp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_smtp_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_upnp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_upnp_parameter_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_upnp_parameter(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_upnp_parameter_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_device_disk_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_device_disk_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_format_disk_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_format_disk_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_submit_format_progress(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_move_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_move_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_move_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_move_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_lost_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_lost_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_lost_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_lost_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_hide_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_hide_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_hide_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_hide_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_io_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_io_alarm_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_io_alarm_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_io_alarm_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_joint_action_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_joint_action_info_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_set_joint_action_info(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_joint_action_info_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_control_ptz_cmd(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_ptz_cmd_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_submit_alarm_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_media_url_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_media_url_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_store_log_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_get_store_log_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_user_login_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_user_login_result(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_user_heart_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_user_heart_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_firmware_upgrade_request(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_firmware_upgrade_response(char buf[], size_t size, int *err, unsigned int flags);
extern JpfXmlMsg *parse_submit_upgrade_progress(char buf[], size_t size, int *err, unsigned int flags);



#ifdef HAVE_PROXY_INFO
#include "proxy_info.h"

	
	extern JpfXmlMsg *parse_add_user_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_add_user_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_del_user_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_del_user_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	
	extern JpfXmlMsg *parse_user_list_info(char buf[], 
			size_t size, int *err, unsigned int flags);	
	extern JpfXmlMsg *parse_device_list_info(char buf[], 
			size_t size, int *err, unsigned int flags);	
	extern JpfXmlMsg *parse_factory_list_info(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_get_factory_info_request(char buf[], 
			size_t size, int *err, unsigned int flags);	
	extern JpfXmlMsg *parse_get_factory_info_response(char buf[], 
			size_t size, int *err, unsigned int flags);	

	extern JpfXmlMsg *parse_fuzzy_find_user_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_fuzzy_find_user_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_modify_password_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_modify_password_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	
	extern JpfXmlMsg *parse_add_device_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_add_device_result(char buf[], 
			size_t size, int *err, unsigned int flags);	
	extern JpfXmlMsg *parse_del_device_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_del_device_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	
	extern JpfXmlMsg *parse_get_device_info_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_get_device_info_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_set_device_info_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_set_device_info_result(char buf[], 
			size_t size, int *err, unsigned int flags);
	
	extern JpfXmlMsg *parse_get_all_device_id_request(char buf[], 
			size_t size, int *err, unsigned int flags);
	extern JpfXmlMsg *parse_get_all_device_id_result(char buf[], 
			size_t size, int *err, unsigned int flags);

	
//#########################################################
	
	extern int create_add_user_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_add_user_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_del_user_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_del_user_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	
	extern int create_user_list_info(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_device_list_info(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_factory_list_info(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_get_factory_info_response(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);

	extern int create_fuzzy_find_user_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_fuzzy_find_user_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_modify_password_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_modify_password_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);

	extern int create_add_device_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_add_device_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);	
	extern int create_del_device_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_del_device_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	
	extern int create_get_device_info_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_get_device_info_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_set_device_info_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_set_device_info_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	
	extern int create_get_all_device_id_request(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);
	extern int create_get_all_device_id_result(JpfXmlMsg *msg, 
			char buf[], size_t size, unsigned int flags);

	extern JpfXmlMsg *parse_proxy_page_user_request(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern int create_proxy_page_user_response(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
  
	extern int create_broadcast_add_user(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_del_user(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_add_device(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_del_device(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_modify_device(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_broadcast_device_status(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	
	extern JpfXmlMsg *parse_get_server_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern JpfXmlMsg *parse_get_server_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern JpfXmlMsg *parse_set_server_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern JpfXmlMsg *parse_set_server_config_result(char buf[], size_t size, 
			int *err, unsigned int flags);
	extern int create_get_server_config_request(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_get_server_config_response(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_set_server_config_request(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
	extern int create_set_server_config_result(JpfXmlMsg *msg, char buf[], 
			size_t size, unsigned int flags);
    
    extern JpfXmlMsg *parse_download_request(char buf[], size_t size, 
			int *err, unsigned int flags);
    extern JpfXmlMsg *parse_download_response(char buf[], size_t size, 
			int *err, unsigned int flags);
    extern JpfXmlMsg *parse_upload_request(char buf[], size_t size, 
			int *err, unsigned int flags);
    extern JpfXmlMsg *parse_upload_response(char buf[], size_t size, 
			int *err, unsigned int flags);
    extern int create_download_request(JpfXmlMsg *msg, char buf[], 
            size_t size, unsigned int flags);
    extern int create_download_response(JpfXmlMsg *msg, char buf[], 
            size_t size, unsigned int flags);
    extern int create_upload_request(JpfXmlMsg *msg, char buf[], 
            size_t size, unsigned int flags);
    extern int create_upload_response(JpfXmlMsg *msg, char buf[], 
            size_t size, unsigned int flags);

    JpfXmlMsg *parse_limit_broadcast_status(char buf[], size_t size, 
                int *err, unsigned int flags);


#endif

JpfXmlMsg *parse_get_channel_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_channel_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_channel_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_channel_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_picture_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
/*JpfXmlMsg *parse_get_hl_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_hl_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_hl_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_hl_picture_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);*/
JpfXmlMsg *parse_get_wifi_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_wifi_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_wifi_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_wifi_config_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_wifi_search_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_wifi_search_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_network_status_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_network_status_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_control_device_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_control_device_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_ddns_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_ddns_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_ddns_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_ddns_config_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_def_display_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_def_display_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_def_picture_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_def_picture_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_avd_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_avd_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_avd_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_avd_config_result(char buf[], size_t size, 
			int *err, unsigned int flags);


int create_get_channel_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_channel_info_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_channel_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_channel_info_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_picture_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_picture_info_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_picture_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_picture_info_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_wifi_config_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_wifi_config_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_wifi_config_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_wifi_config_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_wifi_search_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_wifi_search_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_network_status_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_network_status_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_control_device_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_control_device_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

int create_get_ddns_config_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_ddns_config_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_ddns_config_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_ddns_config_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_def_display_info_requst(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_def_display_info_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_def_picture_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_def_picture_info_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_avd_config_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_avd_config_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_avd_config_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_avd_config_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);



JpfXmlMsg *parse_get_transparent_param_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_transparent_param_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_transparent_param_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_transparent_param_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_transparent_notify_enevt(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_transparent_control_device(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_transparent_param_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_transparent_param_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_transparent_param_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_transparent_param_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_transparent_notify_enevt(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_transparent_control_device(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

//##########################################################################
# ifdef _USE_DECODER_PROTO_

JpfXmlMsg *parse_query_division_mode_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_query_division_mode_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_screen_state_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_screen_state_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_division_mode_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_division_mode_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_full_screen_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_full_screen_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_exit_full_screen_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_exit_full_screen_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_tv_wall_play_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_tv_wall_play_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_clear_division_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_clear_division_response(char buf[], size_t size, 
			int *err, unsigned int flags);

int create_query_division_mode_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_query_division_mode_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_screen_state_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_screen_state_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_division_mode_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_division_mode_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_full_screen_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_full_screen_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_exit_full_screen_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_exit_full_screen_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_tv_wall_play_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_tv_wall_play_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_clear_division_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_clear_division_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

# endif //_USE_DECODER_PROTO_

int create_get_operation_log_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_operation_log_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
JpfXmlMsg *parse_get_operation_log_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_operation_log_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_alarm_upload_config_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_alarm_upload_config_response(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_set_alarm_upload_config_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_alarm_upload_config_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

JpfXmlMsg *parse_get_preset_point_set_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_preset_point_set_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_preset_point_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_preset_point_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_cruise_way_set_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_cruise_way_set_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_cruise_way_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_add_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_add_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_modify_cruise_way_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_modify_cruise_way_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_preset_point_set_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_preset_point_set_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_preset_point_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_preset_point_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_cruise_way_set_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_cruise_way_set_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_cruise_way_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_cruise_way_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_cruise_way_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_cruise_way_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_add_cruise_way_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_add_cruise_way_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_modify_cruise_way_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_modify_cruise_way_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

JpfXmlMsg *parse_3d_control_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_3d_control_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_3d_goback_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_3d_goback_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_3d_control_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_3d_control_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_3d_goback_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_3d_goback_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

JpfXmlMsg *parse_alarm_link_io_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_alarm_link_io_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_alarm_link_preset_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_alarm_link_preset_result(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_resolution_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_resolution_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_resolution_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_resolution_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_alarm_link_io_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_alarm_link_io_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_alarm_link_preset_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_alarm_link_preset_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_resolution_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_resolution_info_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_resolution_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_resolution_info_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

JpfXmlMsg *parse_get_ircut_control_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_ircut_control_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_ircut_control_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_ircut_control_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_ircut_control_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_ircut_control_info_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_ircut_control_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_ircut_control_info_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

JpfXmlMsg *parse_get_extranet_port_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_extranet_port_response(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_extranet_port_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_extranet_port_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

JpfXmlMsg *parse_get_herd_analyse_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_herd_analyse_info_response(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_herd_analyse_info_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_set_herd_analyse_info_result(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_herd_analyse_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_herd_analyse_info_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_herd_analyse_info_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_set_herd_analyse_info_result(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

JpfXmlMsg *parse_get_grass_percent_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_grass_percent_response(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_grass_percent_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_grass_percent_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);

JpfXmlMsg *parse_get_p2p_id_request(char buf[], size_t size, 
			int *err, unsigned int flags);
JpfXmlMsg *parse_get_p2p_id_response(char buf[], size_t size, 
			int *err, unsigned int flags);
int create_get_p2p_id_request(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);
int create_get_p2p_id_response(JpfXmlMsg *msg, char buf[], 
		size_t size, unsigned int flags);


#endif //__J_XML_HANDLER_H__
