
#include <stdio.h>
#include <stdlib.h>

#include "xml-tree.h"
#include "j_packet.h"

#include "config.h"

#if 1

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\|
#define PARSE_PU_REGISTER_XML					"parse/REGISTER_REQUEST.xml"
#define PARSE_REGISTER_RESPONSE_XML				"parse/REGISTER_RESPONSE.xml"
#define PARSE_PU_HEART_XML						"parse/HEART_BEAT_REQUEST.xml"
#define PARSE_HEART_RESPONSE_XML				"parse/HEART_BEAT_RESPONSE.xml"
#define PARSE_CHANGE_DISPATCH_REQUEST_XML		"parse/CHANGE_DISPATCH_REQUEST.xml"
#define PARSE_CHANGE_DISPATCH_RESULT_XML		"parse/CHANGE_DISPATCH_RESULT.xml"
#define PARSE_GET_DEVICE_INFO_XML				"parse/GET_DEVICE_INFO.xml"
#define PARSE_DEVICE_INFO_RESPONSE_XML			"parse/DEVICE_INFO_RESPONSE.xml"
#define PARSE_GET_DEVICE_NTP_INFO_XML			"parse/GET_DEVICE_NTP_INFO.xml"
#define PARSE_DEVICE_NTP_INFO_RESPONSE_XML		"parse/DEVICE_NTP_INFO_RESPONSE.xml"
#define PARSE_SET_DEVICE_NTP_INFO_XML			"parse/SET_DEVICE_NTP_INFO.xml"
#define PARSE_DEVICE_NTP_INFO_RESULT_XML		"parse/DEVICE_NTP_INFO_RESULT.xml"
#define PARSE_SET_DEVICE_TIME_XML				"parse/SET_DEVICE_TIME.xml"
#define PARSE_DEVICE_TIME_RESULT_XML			"parse/DEVICE_TIME_RESULT.xml"
#define PARSE_GET_PLATFORM_INFO_XML				"parse/GET_PLATFORM_INFO.xml"
#define PARSE_PLATFORM_INFO_RESPONSE_XML		"parse/PLATFORM_INFO_RESPONSE.xml"
#define PARSE_SET_PLATFORM_INFO_XML				"parse/SET_PLATFORM_INFO.xml"
#define PARSE_PLATFORM_INFO_RESULT_XML			"parse/PLATFORM_INFO_RESULT.xml"
#define PARSE_GET_NETWORK_INFO_XML				"parse/GET_NETWORK_INFO.xml"
#define PARSE_NETWORK_INFO_RESPONSE_XML			"parse/NETWORK_INFO_RESPONSE.xml"
#define PARSE_SET_NETWORK_INFO_XML				"parse/SET_NETWORK_INFO.xml"
#define PARSE_NETWORK_INFO_RESULT_XML			"parse/NETWORK_INFO_RESULT.xml"
#define PARSE_GET_PPPOE_INFO_XML				"parse/GET_PPPOE_INFO.xml"
#define PARSE_PPPOE_INFO_RESPONSE_XML			"parse/PPPOE_INFO_RESPONSE.xml"
#define PARSE_SET_PPPOE_INFO_XML				"parse/SET_PPPOE_INFO.xml"
#define PARSE_PPPOE_INFO_RESULT_XML				"parse/PPPOE_INFO_RESULT.xml"
#define PARSE_GET_ENCODE_PARAMETER_XML			"parse/GET_ENCODE_PARAMETER.xml"
#define PARSE_ENCODE_PARAMETER_RESPONSE_XML		"parse/ENCODE_PARAMETER_RESPONSE.xml"
#define PARSE_SET_ENCODE_PARAMETER_XML			"parse/SET_ENCODE_PARAMETER.xml"
#define PARSE_ENCODE_PARAMETER_RESULT_XML		"parse/ENCODE_PARAMETER_RESULT.xml"
#define PARSE_GET_DISPLAY_PARAMETER_XML			"parse/GET_DISPLAY_PARAMETER.xml"
#define PARSE_DISPLAY_PARAMETER_RESPONSE_XML	"parse/DISPLAY_PARAMETER_RESPONSE.xml"
#define PARSE_SET_DISPLAY_PARAMETER_XML			"parse/SET_DISPLAY_PARAMETER.xml"
#define PARSE_DISPLAY_PARAMETER_RESULT_XML		"parse/DISPLAY_PARAMETER_RESULT.xml"
#define PARSE_GET_RECORD_PARAMETER_XML			"parse/GET_RECORD_PARAMETER.xml"
#define PARSE_RECORD_PARAMETER_RESPONSE_XML		"parse/RECORD_PARAMETER_RESPONSE.xml"
#define PARSE_SET_RECORD_PARAMETER_XML			"parse/SET_RECORD_PARAMETER.xml"
#define PARSE_RECORD_PARAMETER_RESULT_XML		"parse/RECORD_PARAMETER_RESULT.xml"
#define PARSE_GET_HIDE_PARAMETER_XML			"parse/GET_HIDE_PARAMETER.xml"
#define PARSE_HIDE_PARAMETER_RESPONSE_XML		"parse/HIDE_PARAMETER_RESPONSE.xml"
#define PARSE_SET_HIDE_PARAMETER_XML			"parse/SET_HIDE_PARAMETER.xml"
#define PARSE_HIDE_PARAMETER_RESULT_XML			"parse/HIDE_PARAMETER_RESULT.xml"
#define PARSE_GET_SERIAL_PARAMETER_XML			"parse/GET_SERIAL_PARAMETER.xml"
#define PARSE_SERIAL_PARAMETER_RESPONSE_XML		"parse/SERIAL_PARAMETER_RESPONSE.xml"
#define PARSE_SET_SERIAL_PARAMETER_XML			"parse/SET_SERIAL_PARAMETER.xml"
#define PARSE_SERIAL_PARAMETER_RESULT_XML		"parse/SERIAL_PARAMETER_RESULT.xml"
#define PARSE_GET_OSD_PARAMETER_XML				"parse/GET_OSD_PARAMETER.xml"
#define PARSE_OSD_PARAMETER_RESPONSE_XML		"parse/OSD_PARAMETER_RESPONSE.xml"
#define PARSE_SET_OSD_PARAMETER_XML				"parse/SET_OSD_PARAMETER.xml"
#define PARSE_OSD_PARAMETER_RESULT_XML			"parse/OSD_PARAMETER_RESULT.xml"
#define PARSE_GET_PTZ_PARAMETER_XML				"parse/GET_PTZ_PARAMETER.xml"
#define PARSE_PTZ_PARAMETER_RESPONSE_XML		"parse/PTZ_PARAMETER_RESPONSE.xml"
#define PARSE_SET_PTZ_PARAMETER_XML				"parse/SET_PTZ_PARAMETER.xml"
#define PARSE_PTZ_PARAMETER_RESULT_XML			"parse/PTZ_PARAMETER_RESULT.xml"
#define PARSE_GET_FTP_PARAMETER_XML				"parse/GET_FTP_PARAMETER.xml"
#define PARSE_FTP_PARAMETER_RESPONSE_XML		"parse/FTP_PARAMETER_RESPONSE.xml"
#define PARSE_SET_FTP_PARAMETER_XML				"parse/SET_FTP_PARAMETER.xml"
#define PARSE_FTP_PARAMETER_RESULT_XML			"parse/FTP_PARAMETER_RESULT.xml"
#define PARSE_GET_SMTP_PARAMETER_XML			"parse/GET_SMTP_PARAMETER.xml"
#define PARSE_SMTP_PARAMETER_RESPONSE_XML		"parse/SMTP_PARAMETER_RESPONSE.xml"
#define PARSE_SET_SMTP_PARAMETER_XML			"parse/SET_SMTP_PARAMETER.xml"
#define PARSE_SMTP_PARAMETER_RESULT_XML			"parse/SMTP_PARAMETER_RESULT.xml"
#define PARSE_GET_UPNP_PARAMETER_XML			"parse/GET_UPNP_PARAMETER.xml"
#define PARSE_UPNP_PARAMETER_RESPONSE_XML		"parse/UPNP_PARAMETER_RESPONSE.xml"
#define PARSE_SET_UPNP_PARAMETER_XML			"parse/SET_UPNP_PARAMETER.xml"
#define PARSE_UPNP_PARAMETER_RESULT_XML			"parse/UPNP_PARAMETER_RESULT.xml"
#define PARSE_GET_DISK_INFO_XML					"parse/GET_DISK_INFO.xml"
#define PARSE_DISK_INFO_RESPONSE_XML			"parse/DISK_INFO_RESPONSE.xml"
#define PARSE_FORMAT_DISK_REQUEST_XML			"parse/FORMAT_DISK_REQUEST.xml"
#define PARSE_FORMAT_DISK_RESULT_XML			"parse/FORMAT_DISK_RESULT.xml"
#define PARSE_GET_FORMAT_PROGRESS_XML			"parse/GET_FORMAT_PROGRESS.xml"
#define PARSE_FORMAT_PROGRESS_RESPONSE_XML		"parse/FORMAT_PROGRESS_RESPONSE.xml"
#define PARSE_GET_MOVE_ALARM_INFO_XML			"parse/GET_MOVE_ALARM_INFO.xml"
#define PARSE_MOVE_ALARM_INFO_RESPONSE_XML		"parse/MOVE_ALARM_INFO_RESPONSE.xml"
#define PARSE_SET_MOVE_ALARM_INFO_XML			"parse/SET_MOVE_ALARM_INFO.xml"
#define PARSE_MOVE_ALARM_INFO_RESULT_XML		"parse/MOVE_ALARM_INFO_RESULT.xml"
#define PARSE_GET_LOST_ALARM_INFO_XML			"parse/GET_LOST_ALARM_INFO.xml"
#define PARSE_LOST_ALARM_INFO_RESPONSE_XML		"parse/LOST_ALARM_INFO_RESPONSE.xml"
#define PARSE_SET_LOST_ALARM_INFO_XML			"parse/SET_LOST_ALARM_INFO.xml"
#define PARSE_LOST_ALARM_INFO_RESULT_XML		"parse/LOST_ALARM_INFO_RESULT.xml"
#define PARSE_GET_HIDE_ALARM_INFO_XML			"parse/GET_HIDE_ALARM_INFO.xml"
#define PARSE_HIDE_ALARM_INFO_RESPONSE_XML		"parse/HIDE_ALARM_INFO_RESPONSE.xml"
#define PARSE_SET_HIDE_ALARM_INFO_XML			"parse/SET_HIDE_ALARM_INFO.xml"
#define PARSE_HIDE_ALARM_INFO_RESULT_XML		"parse/HIDE_ALARM_INFO_RESULT.xml"
#define PARSE_GET_IO_ALARM_INFO_XML				"parse/GET_IO_ALARM_INFO.xml"
#define PARSE_IO_ALARM_INFO_RESPONSE_XML		"parse/IO_ALARM_INFO_RESPONSE.xml"
#define PARSE_SET_IO_ALARM_INFO_XML				"parse/SET_IO_ALARM_INFO.xml"
#define PARSE_IO_ALARM_INFO_RESULT_XML			"parse/IO_ALARM_INFO_RESULT.xml"
#define PARSE_SUBMIT_ALARM_REQUEST_XML			"parse/SUBMIT_ALARM_REQUEST.xml"
#define PARSE_SUBMIT_ALARM_RESULT_XML			"parse/SUBMIT_ALARM_RESULT.xml"
//////////////////////////////////////////////////////////////////////////////
#define MERGE_PU_REGISTER_XML					"merge/REGISTER_REQUEST.xml"
#define MERGE_REGISTER_RESPONSE_XML				"merge/REGISTER_RESPONSE.xml"
#define MERGE_PU_HEART_XML						"merge/HEART_BEAT_REQUEST.xml"
#define MERGE_HEART_RESPONSE_XML				"merge/HEART_BEAT_RESPONSE.xml"
#define MERGE_CHANGE_DISPATCH_REQUEST_XML		"merge/CHANGE_DISPATCH_REQUEST.xml"
#define MERGE_CHANGE_DISPATCH_RESULT_XML		"merge/CHANGE_DISPATCH_RESULT.xml"
#define MERGE_GET_DEVICE_INFO_XML				"merge/GET_DEVICE_INFO.xml"
#define MERGE_DEVICE_INFO_RESPONSE_XML			"merge/DEVICE_INFO_RESPONSE.xml"
#define MERGE_GET_DEVICE_NTP_INFO_XML			"merge/GET_DEVICE_NTP_INFO.xml"
#define MERGE_DEVICE_NTP_INFO_RESPONSE_XML		"merge/DEVICE_NTP_INFO_RESPONSE.xml"
#define MERGE_SET_DEVICE_NTP_INFO_XML			"merge/SET_DEVICE_NTP_INFO.xml"
#define MERGE_DEVICE_NTP_INFO_RESULT_XML		"merge/DEVICE_NTP_INFO_RESULT.xml"
#define MERGE_SET_DEVICE_TIME_XML				"merge/SET_DEVICE_TIME.xml"
#define MERGE_DEVICE_TIME_RESULT_XML			"merge/DEVICE_TIME_RESULT.xml"
#define MERGE_GET_PLATFORM_INFO_XML				"merge/GET_PLATFORM_INFO.xml"
#define MERGE_PLATFORM_INFO_RESPONSE_XML		"merge/PLATFORM_INFO_RESPONSE.xml"
#define MERGE_SET_PLATFORM_INFO_XML				"merge/SET_PLATFORM_INFO.xml"
#define MERGE_PLATFORM_INFO_RESULT_XML			"merge/PLATFORM_INFO_RESULT.xml"
#define MERGE_GET_NETWORK_INFO_XML				"merge/GET_NETWORK_INFO.xml"
#define MERGE_NETWORK_INFO_RESPONSE_XML			"merge/NETWORK_INFO_RESPONSE.xml"
#define MERGE_SET_NETWORK_INFO_XML				"merge/SET_NETWORK_INFO.xml"
#define MERGE_NETWORK_INFO_RESULT_XML			"merge/NETWORK_INFO_RESULT.xml"
#define MERGE_GET_PPPOE_INFO_XML				"merge/GET_PPPOE_INFO.xml"
#define MERGE_PPPOE_INFO_RESPONSE_XML			"merge/PPPOE_INFO_RESPONSE.xml"
#define MERGE_SET_PPPOE_INFO_XML				"merge/SET_PPPOE_INFO.xml"
#define MERGE_PPPOE_INFO_RESULT_XML				"merge/PPPOE_INFO_RESULT.xml"
#define MERGE_GET_ENCODE_PARAMETER_XML			"merge/GET_ENCODE_PARAMETER.xml"
#define MERGE_ENCODE_PARAMETER_RESPONSE_XML		"merge/ENCODE_PARAMETER_RESPONSE.xml"
#define MERGE_SET_ENCODE_PARAMETER_XML			"merge/SET_ENCODE_PARAMETER.xml"
#define MERGE_ENCODE_PARAMETER_RESULT_XML		"merge/ENCODE_PARAMETER_RESULT.xml"
#define MERGE_GET_DISPLAY_PARAMETER_XML			"merge/GET_DISPLAY_PARAMETER.xml"
#define MERGE_DISPLAY_PARAMETER_RESPONSE_XML	"merge/DISPLAY_PARAMETER_RESPONSE.xml"
#define MERGE_SET_DISPLAY_PARAMETER_XML			"merge/SET_DISPLAY_PARAMETER.xml"
#define MERGE_DISPLAY_PARAMETER_RESULT_XML		"merge/DISPLAY_PARAMETER_RESULT.xml"
#define MERGE_GET_RECORD_PARAMETER_XML			"merge/GET_RECORD_PARAMETER.xml"
#define MERGE_RECORD_PARAMETER_RESPONSE_XML		"merge/RECORD_PARAMETER_RESPONSE.xml"
#define MERGE_SET_RECORD_PARAMETER_XML			"merge/SET_RECORD_PARAMETER.xml"
#define MERGE_RECORD_PARAMETER_RESULT_XML		"merge/RECORD_PARAMETER_RESULT.xml"
#define MERGE_GET_HIDE_PARAMETER_XML			"merge/GET_HIDE_PARAMETER.xml"
#define MERGE_HIDE_PARAMETER_RESPONSE_XML		"merge/HIDE_PARAMETER_RESPONSE.xml"
#define MERGE_SET_HIDE_PARAMETER_XML			"merge/SET_HIDE_PARAMETER.xml"
#define MERGE_HIDE_PARAMETER_RESULT_XML			"merge/HIDE_PARAMETER_RESULT.xml"
#define MERGE_GET_SERIAL_PARAMETER_XML			"merge/GET_SERIAL_PARAMETER.xml"
#define MERGE_SERIAL_PARAMETER_RESPONSE_XML		"merge/SERIAL_PARAMETER_RESPONSE.xml"
#define MERGE_SET_SERIAL_PARAMETER_XML			"merge/SET_SERIAL_PARAMETER.xml"
#define MERGE_SERIAL_PARAMETER_RESULT_XML		"merge/SERIAL_PARAMETER_RESULT.xml"
#define MERGE_GET_OSD_PARAMETER_XML				"merge/GET_OSD_PARAMETER.xml"
#define MERGE_OSD_PARAMETER_RESPONSE_XML		"merge/OSD_PARAMETER_RESPONSE.xml"
#define MERGE_SET_OSD_PARAMETER_XML				"merge/SET_OSD_PARAMETER.xml"
#define MERGE_OSD_PARAMETER_RESULT_XML			"merge/OSD_PARAMETER_RESULT.xml"
#define MERGE_GET_PTZ_PARAMETER_XML				"merge/GET_PTZ_PARAMETER.xml"
#define MERGE_PTZ_PARAMETER_RESPONSE_XML		"merge/PTZ_PARAMETER_RESPONSE.xml"
#define MERGE_SET_PTZ_PARAMETER_XML				"merge/SET_PTZ_PARAMETER.xml"
#define MERGE_PTZ_PARAMETER_RESULT_XML			"merge/PTZ_PARAMETER_RESULT.xml"
#define MERGE_GET_FTP_PARAMETER_XML				"merge/GET_FTP_PARAMETER.xml"
#define MERGE_FTP_PARAMETER_RESPONSE_XML		"merge/FTP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_FTP_PARAMETER_XML				"merge/SET_FTP_PARAMETER.xml"
#define MERGE_FTP_PARAMETER_RESULT_XML			"merge/FTP_PARAMETER_RESULT.xml"
#define MERGE_GET_SMTP_PARAMETER_XML			"merge/GET_SMTP_PARAMETER.xml"
#define MERGE_SMTP_PARAMETER_RESPONSE_XML		"merge/SMTP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_SMTP_PARAMETER_XML			"merge/SET_SMTP_PARAMETER.xml"
#define MERGE_SMTP_PARAMETER_RESULT_XML			"merge/SMTP_PARAMETER_RESULT.xml"
#define MERGE_GET_UPNP_PARAMETER_XML			"merge/GET_UPNP_PARAMETER.xml"
#define MERGE_UPNP_PARAMETER_RESPONSE_XML		"merge/UPNP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_UPNP_PARAMETER_XML			"merge/SET_UPNP_PARAMETER.xml"
#define MERGE_UPNP_PARAMETER_RESULT_XML			"merge/UPNP_PARAMETER_RESULT.xml"
#define MERGE_GET_DISK_INFO_XML					"merge/GET_DISK_INFO.xml"
#define MERGE_DISK_INFO_RESPONSE_XML			"merge/DISK_INFO_RESPONSE.xml"
#define MERGE_FORMAT_DISK_REQUEST_XML			"merge/FORMAT_DISK_REQUEST.xml"
#define MERGE_FORMAT_DISK_RESULT_XML			"merge/FORMAT_DISK_RESULT.xml"
#define MERGE_GET_FORMAT_PROGRESS_XML			"merge/GET_FORMAT_PROGRESS.xml"
#define MERGE_FORMAT_PROGRESS_RESPONSE_XML		"merge/FORMAT_PROGRESS_RESPONSE.xml"
#define MERGE_GET_MOVE_ALARM_INFO_XML			"merge/GET_MOVE_ALARM_INFO.xml"
#define MERGE_MOVE_ALARM_INFO_RESPONSE_XML		"merge/MOVE_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_MOVE_ALARM_INFO_XML			"merge/SET_MOVE_ALARM_INFO.xml"
#define MERGE_MOVE_ALARM_INFO_RESULT_XML		"merge/MOVE_ALARM_INFO_RESULT.xml"
#define MERGE_GET_LOST_ALARM_INFO_XML			"merge/GET_LOST_ALARM_INFO.xml"
#define MERGE_LOST_ALARM_INFO_RESPONSE_XML		"merge/LOST_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_LOST_ALARM_INFO_XML			"merge/SET_LOST_ALARM_INFO.xml"
#define MERGE_LOST_ALARM_INFO_RESULT_XML		"merge/LOST_ALARM_INFO_RESULT.xml"
#define MERGE_GET_HIDE_ALARM_INFO_XML			"merge/GET_HIDE_ALARM_INFO.xml"
#define MERGE_HIDE_ALARM_INFO_RESPONSE_XML		"merge/HIDE_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_HIDE_ALARM_INFO_XML			"merge/SET_HIDE_ALARM_INFO.xml"
#define MERGE_HIDE_ALARM_INFO_RESULT_XML		"merge/HIDE_ALARM_INFO_RESULT.xml"
#define MERGE_GET_IO_ALARM_INFO_XML				"merge/GET_IO_ALARM_INFO.xml"
#define MERGE_IO_ALARM_INFO_RESPONSE_XML		"merge/IO_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_IO_ALARM_INFO_XML				"merge/SET_IO_ALARM_INFO.xml"
#define MERGE_IO_ALARM_INFO_RESULT_XML			"merge/IO_ALARM_INFO_RESULT.xml"
#define MERGE_SUBMIT_ALARM_REQUEST_XML			"merge/SUBMIT_ALARM_REQUEST.xml"
#define MERGE_SUBMIT_ALARM_RESULT_XML			"merge/SUBMIT_ALARM_RESULT.xml"

#else

//\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\|
#define PARSE_PU_REGISTER_XML					"merge/REGISTER_REQUEST.xml"
#define PARSE_REGISTER_RESPONSE_XML				"merge/REGISTER_RESPONSE.xml"
#define PARSE_PU_HEART_XML						"merge/HEART_BEAT_REQUEST.xml"
#define PARSE_HEART_RESPONSE_XML				"merge/HEART_BEAT_RESPONSE.xml"
#define PARSE_CHANGE_DISPATCH_REQUEST_XML		"merge/CHANGE_DISPATCH_REQUEST.xml"
#define PARSE_CHANGE_DISPATCH_RESULT_XML		"merge/CHANGE_DISPATCH_RESULT.xml"
#define PARSE_GET_DEVICE_INFO_XML				"merge/GET_DEVICE_INFO.xml"
#define PARSE_DEVICE_INFO_RESPONSE_XML			"merge/DEVICE_INFO_RESPONSE.xml"
#define PARSE_GET_DEVICE_NTP_INFO_XML			"merge/GET_DEVICE_NTP_INFO.xml"
#define PARSE_DEVICE_NTP_INFO_RESPONSE_XML		"merge/DEVICE_NTP_INFO_RESPONSE.xml"
#define PARSE_SET_DEVICE_NTP_INFO_XML			"merge/SET_DEVICE_NTP_INFO.xml"
#define PARSE_DEVICE_NTP_INFO_RESULT_XML		"merge/DEVICE_NTP_INFO_RESULT.xml"
#define PARSE_SET_DEVICE_TIME_XML				"merge/SET_DEVICE_TIME.xml"
#define PARSE_DEVICE_TIME_RESULT_XML			"merge/DEVICE_TIME_RESULT.xml"
#define PARSE_GET_PLATFORM_INFO_XML				"merge/GET_PLATFORM_INFO.xml"
#define PARSE_PLATFORM_INFO_RESPONSE_XML		"merge/PLATFORM_INFO_RESPONSE.xml"
#define PARSE_SET_PLATFORM_INFO_XML				"merge/SET_PLATFORM_INFO.xml"
#define PARSE_PLATFORM_INFO_RESULT_XML			"merge/PLATFORM_INFO_RESULT.xml"
#define PARSE_GET_NETWORK_INFO_XML				"merge/GET_NETWORK_INFO.xml"
#define PARSE_NETWORK_INFO_RESPONSE_XML			"merge/NETWORK_INFO_RESPONSE.xml"
#define PARSE_SET_NETWORK_INFO_XML				"merge/SET_NETWORK_INFO.xml"
#define PARSE_NETWORK_INFO_RESULT_XML			"merge/NETWORK_INFO_RESULT.xml"
#define PARSE_GET_PPPOE_INFO_XML				"merge/GET_PPPOE_INFO.xml"
#define PARSE_PPPOE_INFO_RESPONSE_XML			"merge/PPPOE_INFO_RESPONSE.xml"
#define PARSE_SET_PPPOE_INFO_XML				"merge/SET_PPPOE_INFO.xml"
#define PARSE_PPPOE_INFO_RESULT_XML				"merge/PPPOE_INFO_RESULT.xml"
#define PARSE_GET_ENCODE_PARAMETER_XML			"merge/GET_ENCODE_PARAMETER.xml"
#define PARSE_ENCODE_PARAMETER_RESPONSE_XML		"merge/ENCODE_PARAMETER_RESPONSE.xml"
#define PARSE_SET_ENCODE_PARAMETER_XML			"merge/SET_ENCODE_PARAMETER	.xml"
#define PARSE_ENCODE_PARAMETER_RESULT_XML		"merge/ENCODE_PARAMETER_RESULT.xml"
#define PARSE_GET_DISPLAY_PARAMETER_XML			"merge/GET_DISPLAY_PARAMETER.xml"
#define PARSE_DISPLAY_PARAMETER_RESPONSE_XML	"merge/DISPLAY_PARAMETER_RESPONSE.xml"
#define PARSE_SET_DISPLAY_PARAMETER_XML			"merge/SET_DISPLAY_PARAMETER.xml"
#define PARSE_DISPLAY_PARAMETER_RESULT_XML		"merge/DISPLAY_PARAMETER_RESULT.xml"
#define PARSE_GET_RECORD_PARAMETER_XML			"merge/GET_RECORD_PARAMETER.xml"
#define PARSE_RECORD_PARAMETER_RESPONSE_XML		"merge/RECORD_PARAMETER_RESPONSE.xml"
#define PARSE_SET_RECORD_PARAMETER_XML			"merge/SET_RECORD_PARAMETER.xml"
#define PARSE_RECORD_PARAMETER_RESULT_XML		"merge/RECORD_PARAMETER_RESULT.xml"
#define PARSE_GET_HIDE_PARAMETER_XML			"merge/GET_HIDE_PARAMETER.xml"
#define PARSE_HIDE_PARAMETER_RESPONSE_XML		"merge/HIDE_PARAMETER_RESPONSE.xml"
#define PARSE_SET_HIDE_PARAMETER_XML			"merge/SET_HIDE_PARAMETER.xml"
#define PARSE_HIDE_PARAMETER_RESULT_XML			"merge/HIDE_PARAMETER_RESULT.xml"
#define PARSE_GET_SERIAL_PARAMETER_XML			"merge/GET_SERIAL_PARAMETER.xml"
#define PARSE_SERIAL_PARAMETER_RESPONSE_XML		"merge/SERIAL_PARAMETER_RESPONSE.xml"
#define PARSE_SET_SERIAL_PARAMETER_XML			"merge/SET_SERIAL_PARAMETER.xml"
#define PARSE_SERIAL_PARAMETER_RESULT_XML		"merge/SERIAL_PARAMETER_RESULT.xml"
#define PARSE_GET_OSD_PARAMETER_XML				"merge/GET_OSD_PARAMETER.xml"
#define PARSE_OSD_PARAMETER_RESPONSE_XML		"merge/OSD_PARAMETER_RESPONSE.xml"
#define PARSE_SET_OSD_PARAMETER_XML				"merge/SET_OSD_PARAMETER.xml"
#define PARSE_OSD_PARAMETER_RESULT_XML			"merge/OSD_PARAMETER_RESULT.xml"
#define PARSE_GET_PTZ_PARAMETER_XML				"merge/GET_PTZ_PARAMETER.xml"
#define PARSE_PTZ_PARAMETER_RESPONSE_XML		"merge/PTZ_PARAMETER_RESPONSE.xml"
#define PARSE_SET_PTZ_PARAMETER_XML				"merge/SET_PTZ_PARAMETER.xml"
#define PARSE_PTZ_PARAMETER_RESULT_XML			"merge/PTZ_PARAMETER_RESULT.xml"
#define PARSE_GET_FTP_PARAMETER_XML				"merge/GET_FTP_PARAMETER.xml"
#define PARSE_FTP_PARAMETER_RESPONSE_XML		"merge/FTP_PARAMETER_RESPONSE.xml"
#define PARSE_SET_FTP_PARAMETER_XML				"merge/SET_FTP_PARAMETER.xml"
#define PARSE_FTP_PARAMETER_RESULT_XML			"merge/FTP_PARAMETER_RESULT.xml"
#define PARSE_GET_SMTP_PARAMETER_XML			"merge/GET_SMTP_PARAMETER.xml"
#define PARSE_SMTP_PARAMETER_RESPONSE_XML		"merge/SMTP_PARAMETER_RESPONSE.xml"
#define PARSE_SET_SMTP_PARAMETER_XML			"merge/SET_SMTP_PARAMETER.xml"
#define PARSE_SMTP_PARAMETER_RESULT_XML			"merge/SMTP_PARAMETER_RESULT.xml"
#define PARSE_GET_UPNP_PARAMETER_XML			"merge/GET_UPNP_PARAMETER.xml"
#define PARSE_UPNP_PARAMETER_RESPONSE_XML		"merge/UPNP_PARAMETER_RESPONSE.xml"
#define PARSE_SET_UPNP_PARAMETER_XML			"merge/SET_UPNP_PARAMETER.xml"
#define PARSE_UPNP_PARAMETER_RESULT_XML			"merge/UPNP_PARAMETER_RESULT.xml"
#define PARSE_GET_DISK_INFO_XML					"merge/GET_DISK_INFO.xml"
#define PARSE_DISK_INFO_RESPONSE_XML			"merge/DISK_INFO_RESPONSE.xml"
#define PARSE_FORMAT_DISK_REQUEST_XML			"merge/FORMAT_DISK_REQUEST.xml"
#define PARSE_FORMAT_DISK_RESULT_XML			"merge/FORMAT_DISK_RESULT.xml"
#define PARSE_GET_FORMAT_PROGRESS_XML			"merge/GET_FORMAT_PROGRESS.xml"
#define PARSE_FORMAT_PROGRESS_RESPONSE_XML		"merge/FORMAT_PROGRESS_RESPONSE.xml"
#define PARSE_GET_MOVE_ALARM_INFO_XML			"merge/GET_MOVE_ALARM_INFO.xml"
#define PARSE_MOVE_ALARM_INFO_RESPONSE_XML		"merge/MOVE_ALARM_INFO_RESPONSE.xml"
#define PARSE_SET_MOVE_ALARM_INFO_XML			"merge/SET_MOVE_ALARM_INFO.xml"
#define PARSE_MOVE_ALARM_INFO_RESULT_XML		"merge/MOVE_ALARM_INFO_RESULT.xml"
#define PARSE_GET_LOST_ALARM_INFO_XML			"merge/GET_LOST_ALARM_INFO.xml"
#define PARSE_LOST_ALARM_INFO_RESPONSE_XML		"merge/LOST_ALARM_INFO_RESPONSE.xml"
#define PARSE_SET_LOST_ALARM_INFO_XML			"merge/SET_LOST_ALARM_INFO.xml"
#define PARSE_LOST_ALARM_INFO_RESULT_XML		"merge/LOST_ALARM_INFO_RESULT.xml"
#define PARSE_GET_HIDE_ALARM_INFO_XML			"merge/GET_HIDE_ALARM_INFO.xml"
#define PARSE_HIDE_ALARM_INFO_RESPONSE_XML		"merge/HIDE_ALARM_INFO_RESPONSE.xml"
#define PARSE_SET_HIDE_ALARM_INFO_XML			"merge/SET_HIDE_ALARM_INFO.xml"
#define PARSE_HIDE_ALARM_INFO_RESULT_XML		"merge/HIDE_ALARM_INFO_RESULT.xml"
#define PARSE_GET_IO_ALARM_INFO_XML				"merge/GET_IO_ALARM_INFO.xml"
#define PARSE_IO_ALARM_INFO_RESPONSE_XML		"merge/IO_ALARM_INFO_RESPONSE.xml"
#define PARSE_SET_IO_ALARM_INFO_XML				"merge/SET_IO_ALARM_INFO.xml"
#define PARSE_IO_ALARM_INFO_RESULT_XML			"merge/IO_ALARM_INFO_RESULT.xml"
#define PARSE_SUBMIT_ALARM_REQUEST_XML			"merge/SUBMIT_ALARM_REQUEST.xml"
#define PARSE_SUBMIT_ALARM_RESULT_XML			"merge/SUBMIT_ALARM_RESULT.xml"

//////////////////////////////////////////////////////////////////////////////
#define MERGE_PU_REGISTER_XML					"parse/REGISTER_REQUEST.xml"
#define MERGE_REGISTER_RESPONSE_XML				"parse/REGISTER_RESPONSE.xml"
#define MERGE_PU_HEART_XML						"parse/HEART_BEAT_REQUEST.xml"
#define MERGE_HEART_RESPONSE_XML				"parse/HEART_BEAT_RESPONSE.xml"
#define MERGE_CHANGE_DISPATCH_REQUEST_XML		"parse/CHANGE_DISPATCH_REQUEST.xml"
#define MERGE_CHANGE_DISPATCH_RESULT_XML		"parse/CHANGE_DISPATCH_RESULT.xml"
#define MERGE_GET_DEVICE_INFO_XML				"parse/GET_DEVICE_INFO.xml"
#define MERGE_DEVICE_INFO_RESPONSE_XML			"parse/DEVICE_INFO_RESPONSE.xml"
#define MERGE_GET_DEVICE_NTP_INFO_XML			"parse/GET_DEVICE_NTP_INFO.xml"
#define MERGE_DEVICE_NTP_INFO_RESPONSE_XML		"parse/DEVICE_NTP_INFO_RESPONSE.xml"
#define MERGE_SET_DEVICE_NTP_INFO_XML			"parse/SET_DEVICE_NTP_INFO.xml"
#define MERGE_DEVICE_NTP_INFO_RESULT_XML		"parse/DEVICE_NTP_INFO_RESULT.xml"
#define MERGE_SET_DEVICE_TIME_XML				"parse/SET_DEVICE_TIME.xml"
#define MERGE_DEVICE_TIME_RESULT_XML			"parse/DEVICE_TIME_RESULT.xml"
#define MERGE_GET_PLATFORM_INFO_XML				"parse/GET_PLATFORM_INFO.xml"
#define MERGE_PLATFORM_INFO_RESPONSE_XML		"parse/PLATFORM_INFO_RESPONSE.xml"
#define MERGE_SET_PLATFORM_INFO_XML				"parse/SET_PLATFORM_INFO.xml"
#define MERGE_PLATFORM_INFO_RESULT_XML			"parse/PLATFORM_INFO_RESULT.xml"
#define MERGE_GET_NETWORK_INFO_XML				"parse/GET_NETWORK_INFO.xml"
#define MERGE_NETWORK_INFO_RESPONSE_XML			"parse/NETWORK_INFO_RESPONSE.xml"
#define MERGE_SET_NETWORK_INFO_XML				"parse/SET_NETWORK_INFO.xml"
#define MERGE_NETWORK_INFO_RESULT_XML			"parse/NETWORK_INFO_RESULT.xml"
#define MERGE_GET_PPPOE_INFO_XML				"parse/GET_PPPOE_INFO.xml"
#define MERGE_PPPOE_INFO_RESPONSE_XML			"parse/PPPOE_INFO_RESPONSE.xml"
#define MERGE_SET_PPPOE_INFO_XML				"parse/SET_PPPOE_INFO.xml"
#define MERGE_PPPOE_INFO_RESULT_XML				"parse/PPPOE_INFO_RESULT.xml"
#define MERGE_GET_ENCODE_PARAMETER_XML			"parse/GET_ENCODE_PARAMETER.xml"
#define MERGE_ENCODE_PARAMETER_RESPONSE_XML		"parse/ENCODE_PARAMETER_RESPONSE.xml"
#define MERGE_SET_ENCODE_PARAMETER_XML			"parse/SET_ENCODE_PARAMETER.xml"
#define MERGE_ENCODE_PARAMETER_RESULT_XML		"parse/ENCODE_PARAMETER_RESULT.xml"
#define MERGE_GET_DISPLAY_PARAMETER_XML			"parse/GET_DISPLAY_PARAMETER.xml"
#define MERGE_DISPLAY_PARAMETER_RESPONSE_XML	"parse/DISPLAY_PARAMETER_RESPONSE.xml"
#define MERGE_SET_DISPLAY_PARAMETER_XML			"parse/SET_DISPLAY_PARAMETER.xml"
#define MERGE_DISPLAY_PARAMETER_RESULT_XML		"parse/DISPLAY_PARAMETER_RESULT.xml"
#define MERGE_GET_RECORD_PARAMETER_XML			"parse/GET_RECORD_PARAMETER.xml"
#define MERGE_RECORD_PARAMETER_RESPONSE_XML		"parse/RECORD_PARAMETER_RESPONSE.xml"
#define MERGE_SET_RECORD_PARAMETER_XML			"parse/SET_RECORD_PARAMETER.xml"
#define MERGE_RECORD_PARAMETER_RESULT_XML		"parse/RECORD_PARAMETER_RESULT.xml"
#define MERGE_GET_HIDE_PARAMETER_XML			"parse/GET_HIDE_PARAMETER.xml"
#define MERGE_HIDE_PARAMETER_RESPONSE_XML		"parse/HIDE_PARAMETER_RESPONSE.xml"
#define MERGE_SET_HIDE_PARAMETER_XML			"parse/SET_HIDE_PARAMETER.xml"
#define MERGE_HIDE_PARAMETER_RESULT_XML			"parse/HIDE_PARAMETER_RESULT.xml"
#define MERGE_GET_SERIAL_PARAMETER_XML			"parse/GET_SERIAL_PARAMETER.xml"
#define MERGE_SERIAL_PARAMETER_RESPONSE_XML		"parse/SERIAL_PARAMETER_RESPONSE.xml"
#define MERGE_SET_SERIAL_PARAMETER_XML			"parse/SET_SERIAL_PARAMETER.xml"
#define MERGE_SERIAL_PARAMETER_RESULT_XML		"parse/SERIAL_PARAMETER_RESULT.xml"
#define MERGE_GET_OSD_PARAMETER_XML				"parse/GET_OSD_PARAMETER.xml"
#define MERGE_OSD_PARAMETER_RESPONSE_XML		"parse/OSD_PARAMETER_RESPONSE.xml"
#define MERGE_SET_OSD_PARAMETER_XML				"parse/SET_OSD_PARAMETER.xml"
#define MERGE_OSD_PARAMETER_RESULT_XML			"parse/OSD_PARAMETER_RESULT.xml"
#define MERGE_GET_PTZ_PARAMETER_XML				"parse/GET_PTZ_PARAMETER.xml"
#define MERGE_PTZ_PARAMETER_RESPONSE_XML		"parse/PTZ_PARAMETER_RESPONSE.xml"
#define MERGE_SET_PTZ_PARAMETER_XML				"parse/SET_PTZ_PARAMETER.xml"
#define MERGE_PTZ_PARAMETER_RESULT_XML			"parse/PTZ_PARAMETER_RESULT.xml"
#define MERGE_GET_FTP_PARAMETER_XML				"parse/GET_FTP_PARAMETER.xml"
#define MERGE_FTP_PARAMETER_RESPONSE_XML		"parse/FTP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_FTP_PARAMETER_XML				"parse/SET_FTP_PARAMETER.xml"
#define MERGE_FTP_PARAMETER_RESULT_XML			"parse/FTP_PARAMETER_RESULT.xml"
#define MERGE_GET_SMTP_PARAMETER_XML			"parse/GET_SMTP_PARAMETER.xml"
#define MERGE_SMTP_PARAMETER_RESPONSE_XML		"parse/SMTP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_SMTP_PARAMETER_XML			"parse/SET_SMTP_PARAMETER.xml"
#define MERGE_SMTP_PARAMETER_RESULT_XML			"parse/SMTP_PARAMETER_RESULT.xml"
#define MERGE_GET_UPNP_PARAMETER_XML			"parse/GET_UPNP_PARAMETER.xml"
#define MERGE_UPNP_PARAMETER_RESPONSE_XML		"parse/UPNP_PARAMETER_RESPONSE.xml"
#define MERGE_SET_UPNP_PARAMETER_XML			"parse/SET_UPNP_PARAMETER.xml"
#define MERGE_UPNP_PARAMETER_RESULT_XML			"parse/UPNP_PARAMETER_RESULT.xml"
#define MERGE_GET_DISK_INFO_XML					"parse/GET_DISK_INFO.xml"
#define MERGE_DISK_INFO_RESPONSE_XML			"parse/DISK_INFO_RESPONSE.xml"
#define MERGE_FORMAT_DISK_REQUEST_XML			"parse/FORMAT_DISK_REQUEST.xml"
#define MERGE_FORMAT_DISK_RESULT_XML			"parse/FORMAT_DISK_RESULT.xml"
#define MERGE_GET_FORMAT_PROGRESS_XML			"parse/GET_FORMAT_PROGRESS.xml"
#define MERGE_FORMAT_PROGRESS_RESPONSE_XML		"parse/FORMAT_PROGRESS_RESPONSE.xml"
#define MERGE_GET_MOVE_ALARM_INFO_XML			"parse/GET_MOVE_ALARM_INFO.xml"
#define MERGE_MOVE_ALARM_INFO_RESPONSE_XML		"parse/MOVE_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_MOVE_ALARM_INFO_XML			"parse/SET_MOVE_ALARM_INFO.xml"
#define MERGE_MOVE_ALARM_INFO_RESULT_XML		"parse/MOVE_ALARM_INFO_RESULT.xml"
#define MERGE_GET_LOST_ALARM_INFO_XML			"parse/GET_LOST_ALARM_INFO.xml"
#define MERGE_LOST_ALARM_INFO_RESPONSE_XML		"parse/LOST_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_LOST_ALARM_INFO_XML			"parse/SET_LOST_ALARM_INFO.xml"
#define MERGE_LOST_ALARM_INFO_RESULT_XML		"parse/LOST_ALARM_INFO_RESULT.xml"
#define MERGE_GET_HIDE_ALARM_INFO_XML			"parse/GET_HIDE_ALARM_INFO.xml"
#define MERGE_HIDE_ALARM_INFO_RESPONSE_XML		"parse/HIDE_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_HIDE_ALARM_INFO_XML			"parse/SET_HIDE_ALARM_INFO.xml"
#define MERGE_HIDE_ALARM_INFO_RESULT_XML		"parse/HIDE_ALARM_INFO_RESULT.xml"
#define MERGE_GET_IO_ALARM_INFO_XML				"parse/GET_IO_ALARM_INFO.xml"
#define MERGE_IO_ALARM_INFO_RESPONSE_XML		"parse/IO_ALARM_INFO_RESPONSE.xml"
#define MERGE_SET_IO_ALARM_INFO_XML				"parse/SET_IO_ALARM_INFO.xml"
#define MERGE_IO_ALARM_INFO_RESULT_XML			"parse/IO_ALARM_INFO_RESULT.xml"
#define MERGE_SUBMIT_ALARM_REQUEST_XML			"parse/SUBMIT_ALARM_REQUEST.xml"
#define MERGE_SUBMIT_ALARM_RESULT_XML			"parse/SUBMIT_ALARM_RESULT.xml"

#endif



#define MAX_BUFFER_SIZE							4096



extern void show_time_struct(Time *time_struct);
extern void show_time_part(Time *time_start, Time *time_end);


extern void show_time_struct(Time *time_struct);


static int get_xml_info_from_file(const char *path, char *buffer, int buf_size)
{
	FILE *fpr = NULL;
	int file_len = 0;
	
	if (NULL == path)
	{
		printf("path NULL.\n");
		return -1;
	}
	if (0 >= buf_size)
	{
		printf("buf_size invalid.\n");
		return -1;
	}
	
	if (NULL == (fpr = fopen(path, "r")))
	{
		perror("fopen error");
		printf("[%s]\n", path);
		return -1;
	}
	else
	{
		fseek(fpr, 0, SEEK_END);
		file_len = ftell(fpr);
		fseek(fpr, 0, SEEK_SET);

		if (file_len >= buf_size)
		{
			printf("file_len >= buf_size");
			return -1;
		}
		else
		{
			fread(buffer, file_len, 1, fpr);
			
			if (0 != fclose(fpr))
				perror("close(fpr) error.\n");

			return 0;
		}
	}
}

static int create_xml_file_from_buffer(const char *buffer, int buf_len, const char *path)
{
	FILE *fpw = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (0 >= buf_len)
	{
		printf("buf_len invalid.\n");
		return -1;
	}
	if (NULL == path)
	{
		printf("path NULL.\n");
		return -1;
	}
	
	if (NULL == (fpw = fopen(path, "w")))
	{
		perror("fopen error.\n");
		return -1;
	}
	else
	{
		fwrite(buffer, buf_len, 1, fpw);

		if (0 != fclose(fpw))
			perror("close(fpw) error.\n");
		
		printf("create xml file succ...\n");
		return 0;
	}
}


/******************************************************
 *flags 值控制程序
 *-1: 直接生成节点文件
 * 0: 先分析文件再生成文件
 ******************************************************/
static void test_request_info(ContactType type, const char *parse_path, 
									const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	Request *request = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	request = j_xml_alloc(sizeof(Request));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	/*if (-1 == flags || 0 == CmdTable[type].parse_xml(request, buffer))
	{
		if (-1 == flags)
		{
			snprintf(request->session_id, sizeof(request->session_id), "%s", "JXJ-SESSION-ID-001");
			snprintf(request->domain_id, sizeof(request->domain_id), "%s", "JXJ-DOMAIN-ID-001");
			snprintf(request->pu_id, sizeof(request->pu_id), "%s", "JXJ-DVS-00000001");
		}
		
		memset(buffer, 0, sizeof(buffer));
		CmdTable[type].merge_xml(request, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}*/
	j_xml_dealloc(request, sizeof(Request));
}

static void test_result_info(ContactType type, const char *parse_path, 
								const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	Result *result = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	result = j_xml_alloc(sizeof(Result));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	/*if (-1 == flags || 0 == CmdTable[type].parse_xml(result, buffer))
	{
		if (-1 == flags)
		{
			snprintf(result->session_id, sizeof(result->session_id), "%s", "JXJ-SESSION-ID-001");
			snprintf(result->domain_id, sizeof(result->domain_id), "%s", "JXJ-DOMAIN-ID-001");
			snprintf(result->pu_id, sizeof(result->pu_id), "%s", "JXJ-DVS-00000001");
			result->result.code = 1;
		}
		
		memset(buffer, 0, sizeof(buffer));
		CmdTable[type].merge_xml(result, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}*/
	j_xml_dealloc(result, sizeof(Result));
}
//################################################################################
static void register_request_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	RegisterRequestPacket *reg_request = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	reg_request = j_xml_alloc(sizeof(RegisterRequestPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_register_request_xml(reg_request, buffer))
	{
		/*printf("reg_request->pu_id   : %s\n", reg_request->pu_id);
		printf("reg_request->cms_ip  : %s\n", reg_request->cms_ip);
		printf("reg_request->pu_type : %d\n", reg_request->pu_type);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_register_request_xml(reg_request, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(reg_request, sizeof(RegisterRequestPacket));
}
static void register_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	RegisterResponsePacket *reg_response = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	reg_response = j_xml_alloc(sizeof(RegisterResponsePacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_register_response_xml(reg_response, buffer))
	{
		/*printf("reg_response->code      : %d\n", reg_response->result.code);
		printf("reg_response->pu_id     : %s\n", reg_response->pu_id);
		printf("reg_response->keep_alive: %d\n", reg_response->keep_alive);
		printf("reg_response->mds_ip    : %s\n", reg_response->mds_ip);
		printf("reg_response->mds_port  : %d\n", reg_response->mds_port);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_register_response_xml(reg_response, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(reg_response, sizeof(RegisterResponsePacket));
}
static void heart_beat_request_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];	
	HeartBeatRequestPacket *heart_beat = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	heart_beat = j_xml_alloc(sizeof(HeartBeatRequestPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_heart_beat_request_xml(heart_beat, buffer))
	{
		//printf("heart_beat->pu_id : %s\n", heart_beat->pu_id);
		
		memset(buffer, 0, sizeof(buffer));
		merge_heart_beat_request_xml(heart_beat, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(heart_beat, sizeof(HeartBeatRequestPacket));
}
static void heart_beat_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	HeartBeatResponsePacket *heart_beat_resp = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	heart_beat_resp = j_xml_alloc(sizeof(HeartBeatResponsePacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_heart_beat_response_xml(heart_beat_resp, buffer))
	{
		/*printf("heart_beat_resp->code: %d\n", heart_beat_resp->result.code);
		show_time_struct(&(heart_beat_resp->server_time));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_heart_beat_response_xml(heart_beat_resp, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(heart_beat_resp, sizeof(HeartBeatResponsePacket));
}
static void change_dispatch_request_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	ChangeDispatchPacket *change_disp = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	change_disp = j_xml_alloc(sizeof(ChangeDispatchPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_change_dispatch_request_xml(change_disp, buffer))
	{
		/*printf("change_disp->pu_id   : %s\n", change_disp->pu_id);
		printf("change_disp->mds_ip  : %s\n", change_disp->mds_ip);
		printf("change_disp->mds_port: %d\n", change_disp->mds_port);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_change_dispatch_request_xml(change_disp, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(change_disp, sizeof(ChangeDispatchPacket));
}
static void change_dispatch_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(CHANGE_DISPATCH_RESULT_ID, parse_path, merge_path, flags);
}
static void get_device_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_DEVICE_INFO_ID, parse_path, merge_path, flags);
}
static void device_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	DeviceInfoPacket *dev_info = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	dev_info = j_xml_alloc(sizeof(DeviceInfoPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_device_info_response_xml(dev_info, buffer))
	{
		/*printf("dev_info->result.code : %d\n", dev_info->result.code);
		printf("dev_info->session_id  : %s\n", dev_info->session_id);
		printf("dev_info->domain_id   : %s\n", dev_info->domain_id);
		printf("dev_info->pu_id       : %s\n", dev_info->pu_id);
		printf("dev_info->pu_type     : %d\n", dev_info->pu_type);
		printf("dev_info->manu_info   : %s\n", dev_info->manu_info);
		printf("dev_info->release_date: %s\n", dev_info->release_date);
		printf("dev_info->dev_version : %s\n", dev_info->dev_version);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_device_info_response_xml(dev_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(dev_info, sizeof(DeviceInfoPacket));
}
static void get_device_ntp_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_DEVICE_NTP_INFO_ID, parse_path, merge_path, flags);
}
static void device_ntp_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	DeviceNTPInfoPacket *ntp_info = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	ntp_info = j_xml_alloc(sizeof(DeviceNTPInfoPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_device_ntp_info_response_xml(ntp_info, buffer))
	{
		/*printf("ntp_info->result.code  : %d\n", ntp_info->result.code);
		printf("ntp_info->session_id   : %s\n", ntp_info->session_id);
		printf("ntp_info->domain_id    : %s\n", ntp_info->domain_id);
		printf("ntp_info->pu_id        : %s\n", ntp_info->pu_id);
		printf("ntp_info->ntp_server_ip: %s\n", ntp_info->ntp_server_ip);
		printf("ntp_info->time_zone    : %d\n", ntp_info->time_zone);
		printf("ntp_info->time_interval: %d\n", ntp_info->time_interval);
		printf("ntp_info->ntp_enable   : %d\n", ntp_info->ntp_enable);
		printf("ntp_info->dst_enable   : %d\n", ntp_info->dst_enable);
		printf("ntp_info->reserve      : %d\n", ntp_info->reserve);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_device_ntp_info_response_xml(ntp_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(ntp_info, sizeof(DeviceNTPInfoPacket));
}
static void set_device_ntp_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	DeviceNTPInfoPacket *ntp_info = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	ntp_info = j_xml_alloc(sizeof(DeviceNTPInfoPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_device_ntp_info_xml(ntp_info, buffer))
	{
		/*printf("ntp_info->result.code  : %d\n", ntp_info->result.code);
		printf("ntp_info->session_id   : %s\n", ntp_info->session_id);
		printf("ntp_info->domain_id    : %s\n", ntp_info->domain_id);
		printf("ntp_info->pu_id        : %s\n", ntp_info->pu_id);
		printf("ntp_info->ntp_server_ip: %s\n", ntp_info->ntp_server_ip);
		printf("ntp_info->time_zone    : %d\n", ntp_info->time_zone);
		printf("ntp_info->time_interval: %d\n", ntp_info->time_interval);
		printf("ntp_info->ntp_enable   : %d\n", ntp_info->ntp_enable);
		printf("ntp_info->dst_enable   : %d\n", ntp_info->dst_enable);
		printf("ntp_info->reserve      : %d\n", ntp_info->reserve);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_device_ntp_info_xml(ntp_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(ntp_info, sizeof(DeviceNTPInfoPacket));
}
static void device_ntp_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(DEVICE_NTP_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void set_device_time_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	DeviceTimePacket *dev_time = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	dev_time = j_xml_alloc(sizeof(DeviceTimePacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_device_time_xml(dev_time, buffer))
	{
		/*printf("dev_time->session_id   : %s\n", dev_time->session_id);
		printf("dev_time->domain_id    : %s\n", dev_time->domain_id);
		printf("dev_time->pu_id        : %s\n", dev_time->pu_id);
		printf("dev_time->time_zone    : %d\n", dev_time->time_zone);
		printf("dev_time->sync_enable  : %d\n", dev_time->sync_enable);
		show_time_struct(&(dev_time->server_time));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_device_time_xml(dev_time, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(dev_time, sizeof(DeviceTimePacket));
}
static void device_time_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(DEVICE_TIME_RESULT_ID, parse_path, merge_path, flags);
}
static void get_platform_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_PLATFORM_INFO_ID, parse_path, merge_path, flags);
}
static void platform_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	PlatformInfoPacket *pltf_info = NULL;

	memset(buffer, 0, sizeof(buffer));	
	pltf_info = j_xml_alloc(sizeof(PlatformInfoPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_platform_info_response_xml(pltf_info, buffer))
	{
		/*printf("pltf_info->result.code: %d\n", pltf_info->result.code);
		printf("pltf_info->session_id : %s\n", pltf_info->session_id);
		printf("pltf_info->domain_id  : %s\n", pltf_info->domain_id);
		printf("pltf_info->pu_id      : %s\n", pltf_info->pu_id);
		printf("pltf_info->cms_ip     : %s\n", pltf_info->cms_ip);
		printf("pltf_info->cms_port   : %d\n", pltf_info->cms_port);
		printf("pltf_info->mds_ip     : %s\n", pltf_info->mds_ip);
		printf("pltf_info->mds_port   : %d\n", pltf_info->mds_port);
		printf("pltf_info->protocol   : %d\n", pltf_info->protocol);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_platform_info_response_xml(pltf_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(pltf_info, sizeof(PlatformInfoPacket));
}
static void set_platform_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	PlatformInfoPacket *pltf_info = NULL;

	memset(buffer, 0, sizeof(buffer));	
	pltf_info = j_xml_alloc(sizeof(PlatformInfoPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_platform_info_xml(pltf_info, buffer))
	{
		/*printf("pltf_info->result.code: %d\n", pltf_info->result.code);
		printf("pltf_info->session_id : %s\n", pltf_info->session_id);
		printf("pltf_info->domain_id  : %s\n", pltf_info->domain_id);
		printf("pltf_info->pu_id      : %s\n", pltf_info->pu_id);
		printf("pltf_info->cms_ip     : %s\n", pltf_info->cms_ip);
		printf("pltf_info->cms_port   : %d\n", pltf_info->cms_port);
		printf("pltf_info->mds_ip     : %s\n", pltf_info->mds_ip);
		printf("pltf_info->mds_port   : %d\n", pltf_info->mds_port);
		printf("pltf_info->protocol   : %d\n", pltf_info->protocol);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_platform_info_xml(pltf_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(pltf_info, sizeof(PlatformInfoPacket));
}
static void platform_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(PLATFORM_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void get_network_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_NETWORK_INFO_ID, parse_path, merge_path, flags);
}
static void network_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	NetworkInfoPacket *net_info = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	net_info = j_xml_alloc(sizeof(NetworkInfoPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_network_info_response_xml(net_info, buffer))
	{
		/*printf("net_info->result.code : %d\n", net_info->result.code);
		printf("net_info->session_id  : %s\n", net_info->session_id);
		printf("net_info->domain_id   : %s\n", net_info->domain_id);
		printf("net_info->pu_id       : %s\n", net_info->pu_id);
		printf("net_info->lan_ip      : %s\n", net_info->network[J_SDK_ETH0].ip);
		printf("net_info->netmask     : %s\n", net_info->network[J_SDK_ETH0].netmask);
		printf("net_info->gateway     : %s\n", net_info->network[J_SDK_ETH0].gateway);
		printf("net_info->wifi_ip     : %s\n", net_info->network[J_SDK_WIFI].ip);
		printf("net_info->wifi_netmask: %s\n", net_info->network[J_SDK_WIFI].netmask);
		printf("net_info->wifi_gateway: %s\n", net_info->network[J_SDK_WIFI].gateway);
		printf("net_info->_3g_ip      : %s\n", net_info->network[J_SDK_3G].ip);
		printf("net_info->_3g_netmask : %s\n", net_info->network[J_SDK_3G].netmask);
		printf("net_info->_3g_gateway : %s\n", net_info->network[J_SDK_3G].gateway);
		printf("net_info->dns         : %s\n", net_info->dns);
		printf("net_info->dhcp_enable : %d\n", net_info->dhcp_enable);
		printf("net_info->auto_dns_   : %d\n", net_info->auto_dns_enable);
		printf("net_info->server_port : %d\n", net_info->server_port);
		printf("net_info->web_port    : %d\n", net_info->web_port);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_network_info_response_xml(net_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(net_info, sizeof(NetworkInfoPacket));
}
static void set_network_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	NetworkInfoPacket *net_info = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	net_info = j_xml_alloc(sizeof(NetworkInfoPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_network_info_xml(net_info, buffer))
	{
		/*printf("net_info->result.code : %d\n", net_info->result.code);
		printf("net_info->session_id  : %s\n", net_info->session_id);
		printf("net_info->domain_id   : %s\n", net_info->domain_id);
		printf("net_info->pu_id       : %s\n", net_info->pu_id);
		printf("net_info->lan_ip      : %s\n", net_info->network[J_SDK_ETH0].ip);
		printf("net_info->netmask     : %s\n", net_info->network[J_SDK_ETH0].netmask);
		printf("net_info->gateway     : %s\n", net_info->network[J_SDK_ETH0].gateway);
		printf("net_info->wifi_ip     : %s\n", net_info->network[J_SDK_WIFI].ip);
		printf("net_info->wifi_netmask: %s\n", net_info->network[J_SDK_WIFI].netmask);
		printf("net_info->wifi_gateway: %s\n", net_info->network[J_SDK_WIFI].gateway);
		printf("net_info->_3g_ip      : %s\n", net_info->network[J_SDK_3G].ip);
		printf("net_info->_3g_netmask : %s\n", net_info->network[J_SDK_3G].netmask);
		printf("net_info->_3g_gateway : %s\n", net_info->network[J_SDK_3G].gateway);
		printf("net_info->dns         : %s\n", net_info->dns);
		printf("net_info->dhcp_enable : %d\n", net_info->dhcp_enable);
		printf("net_info->auto_dns_   : %d\n", net_info->auto_dns_enable);
		printf("net_info->server_port : %d\n", net_info->server_port);
		printf("net_info->web_port    : %d\n", net_info->web_port);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_network_info_xml(net_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(net_info, sizeof(NetworkInfoPacket));
}
static void network_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(NETWORK_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void get_pppoe_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_PPPOE_INFO_ID, parse_path, merge_path, flags);
}
static void pppoe_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	PPPOEInfoPacket *pppoe_info = NULL;
	
	memset(buffer, 0, sizeof(buffer));	
	pppoe_info = j_xml_alloc(sizeof(PPPOEInfoPacket));

	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_pppoe_info_response_xml(pppoe_info, buffer))
	{
		/*printf("net_info->result.code : %d\n", pppoe_info->result.code);
		printf("net_info->session_id  : %s\n", pppoe_info->session_id);
		printf("net_info->domain_id   : %s\n", pppoe_info->domain_id);
		printf("net_info->pu_id       : %s\n", pppoe_info->pu_id);
		printf("net_info->account     : %s\n", pppoe_info->account);
		printf("net_info->passwd      : %s\n", pppoe_info->passwd);
		printf("net_info->type        : %d\n", pppoe_info->type);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_pppoe_info_response_xml(pppoe_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(pppoe_info, sizeof(PPPOEInfoPacket));
}
static void set_pppoe_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	PPPOEInfoPacket *pppoe_info = NULL;
	
	memset(buffer, 0, sizeof(buffer));	
	pppoe_info = j_xml_alloc(sizeof(PPPOEInfoPacket));

	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_pppoe_info_xml(pppoe_info, buffer))
	{
		/*printf("net_info->result.code : %d\n", pppoe_info->result.code);
		printf("net_info->session_id  : %s\n", pppoe_info->session_id);
		printf("net_info->domain_id   : %s\n", pppoe_info->domain_id);
		printf("net_info->pu_id       : %s\n", pppoe_info->pu_id);
		printf("net_info->account     : %s\n", pppoe_info->account);
		printf("net_info->passwd      : %s\n", pppoe_info->passwd);
		printf("net_info->type        : %d\n", pppoe_info->type);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_pppoe_info_xml(pppoe_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(pppoe_info, sizeof(PPPOEInfoPacket));
}
static void pppoe_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(PPPOE_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void get_encode_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_ENCODE_PARAMETER_ID, parse_path, merge_path, flags);
}
static void encode_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	EncodeParameterPacket *encode_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	encode_para = j_xml_alloc(sizeof(EncodeParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_encode_parameter_response_xml(encode_para, buffer))
	{
		/*printf("encode_para->result.code     : %d\n", encode_para->result.code);
		printf("encode_para->session_id      : %s\n", encode_para->session_id);
		printf("encode_para->domain_id       : %s\n", encode_para->domain_id);
		printf("encode_para->pu_id           : %s\n", encode_para->pu_id);
		printf("encode_para->channel         : %d\n", encode_para->channel);
		printf("encode_para->level           : %d\n", encode_para->level);
		printf("encode_para->frame_rate      : %d\n", encode_para->frame_rate);
		printf("encode_para->i_frame_interval: %d\n", encode_para->i_frame_interval);
		printf("encode_para->video_type      : %d\n", encode_para->video_type);
		printf("encode_para->audio_type      : %d\n", encode_para->audio_type);
		printf("encode_para->audio_enble     : %d\n", encode_para->audio_enble);
		printf("encode_para->resolution      : %d\n", encode_para->resolution);
		printf("encode_para->qp_value        : %d\n", encode_para->qp_value);
		printf("encode_para->code_rate       : %d\n", encode_para->code_rate);
		printf("encode_para->frame_priority  : %d\n", encode_para->frame_priority);
		printf("encode_para->format          : %d\n", encode_para->format);
		printf("encode_para->bit_rate        : %d\n", encode_para->bit_rate);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_encode_parameter_response_xml(encode_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(encode_para, sizeof(EncodeParameterPacket));
}
static void set_encode_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	EncodeParameterPacket *encode_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	encode_para = j_xml_alloc(sizeof(EncodeParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_encode_parameter_xml(encode_para, buffer))
	{
		/*printf("encode_para->result.code     : %d\n", encode_para->result.code);
		printf("encode_para->session_id      : %s\n", encode_para->session_id);
		printf("encode_para->domain_id       : %s\n", encode_para->domain_id);
		printf("encode_para->pu_id           : %s\n", encode_para->pu_id);
		printf("encode_para->channel         : %d\n", encode_para->channel);
		printf("encode_para->level           : %d\n", encode_para->level);
		printf("encode_para->frame_rate      : %d\n", encode_para->frame_rate);
		printf("encode_para->i_frame_interval: %d\n", encode_para->i_frame_interval);
		printf("encode_para->video_type      : %d\n", encode_para->video_type);
		printf("encode_para->audio_type      : %d\n", encode_para->audio_type);
		printf("encode_para->audio_enble     : %d\n", encode_para->audio_enble);
		printf("encode_para->resolution      : %d\n", encode_para->resolution);
		printf("encode_para->qp_value        : %d\n", encode_para->qp_value);
		printf("encode_para->code_rate       : %d\n", encode_para->code_rate);
		printf("encode_para->frame_priority  : %d\n", encode_para->frame_priority);
		printf("encode_para->format          : %d\n", encode_para->format);
		printf("encode_para->bit_rate        : %d\n", encode_para->bit_rate);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_encode_parameter_xml(encode_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(encode_para, sizeof(EncodeParameterPacket));
}
static void encode_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(ENCODE_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_display_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_DISPLAY_PARAMETER_ID, parse_path, merge_path, flags);
}
static void display_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	DisplayParameterPacket *display_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	display_para = j_xml_alloc(sizeof(DisplayParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_display_parameter_response_xml(display_para, buffer))
	{
		/*printf("display_para->result.code : %d\n", display_para->result.code);
		printf("display_para->session_id  : %s\n", display_para->session_id);
		printf("display_para->domain_id   : %s\n", display_para->domain_id);
		printf("display_para->pu_id       : %s\n", display_para->pu_id);
		printf("display_para->channel     : %d\n", display_para->channel);
		printf("display_para->contrast    : %d\n", display_para->contrast);
		printf("display_para->bright      : %d\n", display_para->bright);
		printf("display_para->hue         : %d\n", display_para->hue);
		printf("display_para->saturation  : %d\n", display_para->saturation);
		printf("display_para->sharpness   : %d\n", display_para->sharpness);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_display_parameter_response_xml(display_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(display_para, sizeof(DisplayParameterPacket));
}
static void set_display_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	DisplayParameterPacket *display_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	display_para = j_xml_alloc(sizeof(DisplayParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_display_parameter_xml(display_para, buffer))
	{
		/*printf("display_para->result.code : %d\n", display_para->result.code);
		printf("display_para->session_id  : %s\n", display_para->session_id);
		printf("display_para->domain_id   : %s\n", display_para->domain_id);
		printf("display_para->pu_id       : %s\n", display_para->pu_id);
		printf("display_para->channel     : %d\n", display_para->channel);
		printf("display_para->contrast    : %d\n", display_para->contrast);
		printf("display_para->bright      : %d\n", display_para->bright);
		printf("display_para->hue         : %d\n", display_para->hue);
		printf("display_para->saturation  : %d\n", display_para->saturation);
		printf("display_para->sharpness   : %d\n", display_para->sharpness);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_display_parameter_xml(display_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(display_para, sizeof(DisplayParameterPacket));
}
static void display_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(DISPLAY_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_record_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_RECORD_PARAMETER_ID, parse_path, merge_path, flags);
}
static void record_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	RecordParameterPacket *record_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	record_para = j_xml_alloc(sizeof(RecordParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_record_parameter_response_xml(record_para, buffer))
	{
		printf("record_para->result.code : %d\n", record_para->result.code);
		printf("record_para->session_id  : %s\n", record_para->session_id);
		printf("record_para->domain_id   : %s\n", record_para->domain_id);
		printf("record_para->pu_id       : %s\n", record_para->pu_id);
		printf("record_para->channel     : %d\n", record_para->channel);
		printf("record_para->level       : %d\n", record_para->level);
		printf("record_para->pre_record  : %d\n", record_para->pre_record);
		printf("record_para->auto_cover  : %d\n", record_para->auto_cover);
		printf("record_para->week_day    : %d\n", record_para->time_start.weekday);
		printf("record_para->week_day    : %d\n", record_para->time_end.weekday);
		show_time_part(&(record_para->time_start), &(record_para->time_end));
		
		memset(buffer, 0, sizeof(buffer));
		merge_record_parameter_response_xml(record_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(record_para, sizeof(RecordParameterPacket));
}
static void set_record_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	RecordParameterPacket *record_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	record_para = j_xml_alloc(sizeof(RecordParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_record_parameter_xml(record_para, buffer))
	{
		/*printf("record_para->result.code : %d\n", record_para->result.code);
		printf("record_para->session_id  : %s\n", record_para->session_id);
		printf("record_para->domain_id   : %s\n", record_para->domain_id);
		printf("record_para->pu_id       : %s\n", record_para->pu_id);
		printf("record_para->channel     : %d\n", record_para->channel);
		printf("record_para->level       : %d\n", record_para->level);
		printf("record_para->pre_record  : %d\n", record_para->pre_record);
		printf("record_para->auto_cover  : %d\n", record_para->auto_cover);
		printf("record_para->week_day    : %d\n", record_para->time_start.weekday);
		printf("record_para->week_day    : %d\n", record_para->time_end.weekday);
		show_time_part(&(record_para->time_start), &(record_para->time_end));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_record_parameter_xml(record_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(record_para, sizeof(RecordParameterPacket));
}
static void record_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(RECORD_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_hide_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_HIDE_PARAMETER_ID, parse_path, merge_path, flags);
}
static void hide_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	HideParameterPacket *hide_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	hide_para = j_xml_alloc(sizeof(HideParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_hide_parameter_response_xml(hide_para, buffer))
	{
		/*printf("hide_para->result.code     : %d\n", hide_para->result.code);
		printf("hide_para->session_id      : %s\n", hide_para->session_id);
		printf("hide_para->domain_id       : %s\n", hide_para->domain_id);
		printf("hide_para->pu_id           : %s\n", hide_para->pu_id);
		printf("hide_para->channel         : %d\n", hide_para->channel);
		printf("hide_para->hide_num        : %d\n", hide_para->hide_num);
		printf("hide_para->hide_area       : %d\n", hide_para->hide_area);
		printf("hide_para->hide_color      : %d\n", hide_para->hide_color);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_hide_parameter_response_xml(hide_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(hide_para, sizeof(HideParameterPacket));
}
static void set_hide_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	HideParameterPacket *hide_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	hide_para = j_xml_alloc(sizeof(HideParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_hide_parameter_xml(hide_para, buffer))
	{
		/*printf("hide_para->result.code     : %d\n", hide_para->result.code);
		printf("hide_para->session_id      : %s\n", hide_para->session_id);
		printf("hide_para->domain_id       : %s\n", hide_para->domain_id);
		printf("hide_para->pu_id           : %s\n", hide_para->pu_id);
		printf("hide_para->channel         : %d\n", hide_para->channel);
		printf("hide_para->hide_num        : %d\n", hide_para->hide_num);
		printf("hide_para->hide_area       : %d\n", hide_para->hide_area);
		printf("hide_para->hide_color      : %d\n", hide_para->hide_color);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_hide_parameter_xml(hide_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(hide_para, sizeof(HideParameterPacket));
}
static void hide_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(HIDE_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_serial_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_SERIAL_PARAMETER_ID, parse_path, merge_path, flags);
}
static void serial_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	SerialParameterPacket *serial_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	serial_para = j_xml_alloc(sizeof(SerialParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_serial_parameter_response_xml(serial_para, buffer))
	{
		/*printf("serial_para->result.code : %d\n", serial_para->result.code);
		printf("serial_para->session_id  : %s\n", serial_para->session_id);
		printf("serial_para->domain_id   : %s\n", serial_para->domain_id);
		printf("serial_para->pu_id       : %s\n", serial_para->pu_id);
		printf("serial_para->serial_id   : %s\n", serial_para->serial_id);
		printf("serial_para->baud_rate   : %d\n", serial_para->baud_rate);
		printf("serial_para->data_bit    : %d\n", serial_para->data_bit);
		printf("serial_para->stop_bit    : %d\n", serial_para->stop_bit);
		printf("serial_para->verify      : %d\n", serial_para->verify);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_serial_parameter_response_xml(serial_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(serial_para, sizeof(SerialParameterPacket));
}
static void set_serial_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	SerialParameterPacket *serial_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	serial_para = j_xml_alloc(sizeof(SerialParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_serial_parameter_xml(serial_para, buffer))
	{
		/*printf("serial_para->result.code : %d\n", serial_para->result.code);
		printf("serial_para->session_id  : %s\n", serial_para->session_id);
		printf("serial_para->domain_id   : %s\n", serial_para->domain_id);
		printf("serial_para->pu_id       : %s\n", serial_para->pu_id);
		printf("serial_para->serial_id   : %s\n", serial_para->serial_id);
		printf("serial_para->baud_rate   : %d\n", serial_para->baud_rate);
		printf("serial_para->data_bit    : %d\n", serial_para->data_bit);
		printf("serial_para->stop_bit    : %d\n", serial_para->stop_bit);
		printf("serial_para->verify      : %d\n", serial_para->verify);*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_serial_parameter_xml(serial_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(serial_para, sizeof(SerialParameterPacket));
}
static void serial_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(SERIAL_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_osd_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_OSD_PARAMETER_ID, parse_path, merge_path, flags);
}
static void osd_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	OSDParameterPacket *osd_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	osd_para = j_xml_alloc(sizeof(OSDParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_osd_parameter_response_xml(osd_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_osd_parameter_response_xml(osd_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(osd_para, sizeof(OSDParameterPacket));
}
static void set_osd_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	OSDParameterPacket *osd_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	osd_para = j_xml_alloc(sizeof(OSDParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_osd_parameter_xml(osd_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_set_osd_parameter_xml(osd_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(osd_para, sizeof(OSDParameterPacket));
}
static void osd_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(OSD_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_ptz_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_PTZ_PARAMETER_ID, parse_path, merge_path, flags);
}
//****************************************************************************************
//****************************************************************************************************
static void ptz_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	PTZParameterPacket *ptz_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	ptz_para = j_xml_alloc(sizeof(PTZParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_ptz_parameter_response_xml(ptz_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_ptz_parameter_response_xml(ptz_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(ptz_para, sizeof(PTZParameterPacket));
}
static void set_ptz_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	PTZParameterPacket *ptz_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	ptz_para = j_xml_alloc(sizeof(PTZParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_ptz_parameter_xml(ptz_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_set_ptz_parameter_xml(ptz_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(ptz_para, sizeof(PTZParameterPacket));
}
static void ptz_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(PTZ_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_ftp_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_FTP_PARAMETER_ID, parse_path, merge_path, flags);
}
static void ftp_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	FTPParameterPacket *ftp_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	ftp_para = j_xml_alloc(sizeof(FTPParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_ftp_parameter_response_xml(ftp_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_ftp_parameter_response_xml(ftp_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(ftp_para, sizeof(FTPParameterPacket));
}
static void set_ftp_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	FTPParameterPacket *ftp_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	ftp_para = j_xml_alloc(sizeof(FTPParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_ftp_parameter_xml(ftp_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_set_ftp_parameter_xml(ftp_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(ftp_para, sizeof(FTPParameterPacket));
}
static void ftp_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(FTP_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_smtp_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_SMTP_PARAMETER_ID, parse_path, merge_path, flags);
}
static void smtp_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	SMTPParameterPacket *smtp_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	smtp_para = j_xml_alloc(sizeof(SMTPParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_smtp_parameter_response_xml(smtp_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_smtp_parameter_response_xml(smtp_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(smtp_para, sizeof(SMTPParameterPacket));
}
static void set_smtp_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	SMTPParameterPacket *smtp_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	smtp_para = j_xml_alloc(sizeof(SMTPParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_smtp_parameter_xml(smtp_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_set_smtp_parameter_xml(smtp_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(smtp_para, sizeof(SMTPParameterPacket));
}
static void smtp_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(SMTP_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_upnp_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_UPNP_PARAMETER_ID, parse_path, merge_path, flags);
}
static void upnp_parameter_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	UPNPParameterPacket *upnp_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	upnp_para = j_xml_alloc(sizeof(UPNPParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_upnp_parameter_response_xml(upnp_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_upnp_parameter_response_xml(upnp_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(upnp_para, sizeof(UPNPParameterPacket));
}
static void set_upnp_parameter_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	UPNPParameterPacket *upnp_para = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	upnp_para = j_xml_alloc(sizeof(UPNPParameterPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_upnp_parameter_xml(upnp_para, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_set_upnp_parameter_xml(upnp_para, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(upnp_para, sizeof(UPNPParameterPacket));
}
static void upnp_parameter_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(UPNP_PARAMETER_RESULT_ID, parse_path, merge_path, flags);
}
static void get_disk_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_DEVICE_DISK_INFO_ID, parse_path, merge_path, flags);
}
static void disk_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	DeviceDiskInfoPacket *disk_info = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	disk_info = j_xml_alloc(sizeof(DeviceDiskInfoPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_device_disk_info_response_xml(disk_info, buffer))
	{printf("+++++++++++++++++\n");
		memset(buffer, 0, sizeof(buffer));
		merge_device_disk_info_response_xml(disk_info, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(disk_info, sizeof(DeviceDiskInfoPacket));
}
static void format_disk_request_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	FormatDiskPacket *format = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	format = j_xml_alloc(sizeof(FormatDiskPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_format_disk_request_xml(format, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_format_disk_request_xml(format, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(format, sizeof(FormatDiskPacket));
}
static void format_disk_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(FORMAT_DISK_RESULT_ID, parse_path, merge_path, flags);
}
static void get_format_progress_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_FORMAT_PROGRESS_ID, parse_path, merge_path, flags);
}
static void format_progress_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	FormatDiskPacket *format = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	format = j_xml_alloc(sizeof(FormatDiskPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_format_progress_response_xml(format, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_format_progress_response_xml(format, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(format, sizeof(FormatDiskPacket));
}
static void get_move_alarm_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_MOVE_ALARM_INFO_ID, parse_path, merge_path, flags);
}
static void move_alarm_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	MoveAlarmPacket *move_alarm = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	move_alarm = j_xml_alloc(sizeof(MoveAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_move_alarm_info_response_xml(move_alarm, buffer))
	{
		/*printf("move_alarm->result.code    : %d\n", move_alarm->result.code);
		printf("move_alarm->session_id     : %s\n", move_alarm->session_id);
		printf("move_alarm->domain_id      : %s\n", move_alarm->domain_id);
		printf("move_alarm->pu_id          : %s\n", move_alarm->pu_id);
		printf("move_alarm->channel        : %d\n", move_alarm->channel);
		printf("move_alarm->move_enable    : %d\n", move_alarm->move_enable);
		printf("move_alarm->sensitive_level: %d\n", move_alarm->sensitive_level);
		printf("move_alarm->detect_interval: %d\n", move_alarm->detect_interval);
		printf("move_alarm->detect_area    : %d\n", move_alarm->detect_area);
		printf("move_alarm->week_day       : %d\n", move_alarm->time_start.weekday);
		printf("move_alarm->week_day       : %d\n", move_alarm->time_end.weekday);
		show_time_part(&(move_alarm->time_start), &(move_alarm->time_endPacket));
		
		show_joint_info(&(move_alarm->jointPacket));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_move_alarm_info_response_xml(move_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(move_alarm, sizeof(MoveAlarmPacket));
}
static void set_move_alarm_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	MoveAlarmPacket *move_alarm = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	move_alarm = j_xml_alloc(sizeof(MoveAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_move_alarm_info_xml(move_alarm, buffer))
	{
		/*printf("move_alarm->result.code    : %d\n", move_alarm->result.code);
		printf("move_alarm->session_id     : %s\n", move_alarm->session_id);
		printf("move_alarm->domain_id      : %s\n", move_alarm->domain_id);
		printf("move_alarm->pu_id          : %s\n", move_alarm->pu_id);
		printf("move_alarm->channel        : %d\n", move_alarm->channel);
		printf("move_alarm->move_enable    : %d\n", move_alarm->move_enable);
		printf("move_alarm->sensitive_level: %d\n", move_alarm->sensitive_level);
		printf("move_alarm->detect_interval: %d\n", move_alarm->detect_interval);
		printf("move_alarm->detect_area    : %d\n", move_alarm->detect_area);
		printf("move_alarm->week_day       : %d\n", move_alarm->time_start.weekday);
		printf("move_alarm->week_day       : %d\n", move_alarm->time_end.weekday);
		show_time_part(&(move_alarm->time_start), &(move_alarm->time_end));
		
		show_joint_info(&(move_alarm->joint));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_move_alarm_info_xml(move_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(move_alarm, sizeof(MoveAlarmPacket));
}
static void move_alarm_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(MOVE_ALARM_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void get_lost_alarm_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_LOST_ALARM_INFO_ID, parse_path, merge_path, flags);
}
static void lost_alarm_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	LostAlarmPacket *lost_alarm = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	lost_alarm = j_xml_alloc(sizeof(LostAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_lost_alarm_info_response_xml(lost_alarm, buffer))
	{
		/*printf("lost_alarm->result.code    : %d\n", lost_alarm->result.code);
		printf("lost_alarm->session_id     : %s\n", lost_alarm->session_id);
		printf("lost_alarm->domain_id      : %s\n", lost_alarm->domain_id);
		printf("lost_alarm->pu_id          : %s\n", lost_alarm->pu_id);
		printf("lost_alarm->channel        : %d\n", lost_alarm->channel);
		printf("lost_alarm->gu_type        : %d\n", lost_alarm->gu_type);
		printf("lost_alarm->lost_enable    : %d\n", lost_alarm->lost_enable);
		printf("lost_alarm->detect_interval: %d\n", lost_alarm->detect_interval);
		printf("lost_alarm->week_day       : %d\n", lost_alarm->time_start.weekday);
		printf("lost_alarm->week_day       : %d\n", lost_alarm->time_end.weekday);
		show_time_part(&(lost_alarm->time_start), &(lost_alarm->time_end));
		
		show_joint_info(&(lost_alarm->joint));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_lost_alarm_info_response_xml(lost_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(lost_alarm, sizeof(LostAlarmPacket));
}
static void set_lost_alarm_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	LostAlarmPacket *lost_alarm = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	lost_alarm = j_xml_alloc(sizeof(LostAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_lost_alarm_info_xml(lost_alarm, buffer))
	{
		/*printf("lost_alarm->result.code    : %d\n", lost_alarm->result.code);
		printf("lost_alarm->session_id     : %s\n", lost_alarm->session_id);
		printf("lost_alarm->domain_id      : %s\n", lost_alarm->domain_id);
		printf("lost_alarm->pu_id          : %s\n", lost_alarm->pu_id);
		printf("lost_alarm->channel        : %d\n", lost_alarm->channel);
		printf("lost_alarm->gu_type        : %d\n", lost_alarm->gu_type);
		printf("lost_alarm->lost_enable    : %d\n", lost_alarm->lost_enable);
		printf("lost_alarm->detect_interval: %d\n", lost_alarm->detect_interval);
		printf("lost_alarm->week_day       : %d\n", lost_alarm->time_start.weekday);
		printf("lost_alarm->week_day       : %d\n", lost_alarm->time_end.weekday);
		show_time_part(&(lost_alarm->time_start), &(lost_alarm->time_end));
		
		show_joint_info(&(lost_alarm->joint));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_lost_alarm_info_xml(lost_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(lost_alarm, sizeof(LostAlarmPacket));
}
static void lost_alarm_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(LOST_ALARM_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void get_hide_alarm_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_HIDE_ALARM_INFO_ID, parse_path, merge_path, flags);
}
static void hide_alarm_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	HideAlarmPacket *hide_alarm = NULL;

	memset(buffer, 0, sizeof(buffer));
	hide_alarm = j_xml_alloc(sizeof(HideAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_hide_alarm_info_response_xml(hide_alarm, buffer))
	{
		/*printf("hide_alarm->result.code: %d\n", hide_alarm->result.code);
		printf("hide_alarm->session_id : %s\n", hide_alarm->session_id);
		printf("hide_alarm->domain_id  : %s\n", hide_alarm->domain_id);
		printf("hide_alarm->pu_id      : %s\n", hide_alarm->pu_id);
		printf("hide_alarm->channel    : %d\n", hide_alarm->channel);
		printf("hide_alarm->hide_enable: %d\n", hide_alarm->hide_enable);
		printf("hide_alarm->hide_x     : %d\n", hide_alarm->hide_x);
		printf("hide_alarm->hide_y     : %d\n", hide_alarm->hide_y);
		printf("hide_alarm->hide_w     : %d\n", hide_alarm->hide_w);
		printf("hide_alarm->hide_h     : %d\n", hide_alarm->hide_h);
		show_time_part(&(hide_alarm->time_start), &(hide_alarm->time_end));
		
		show_joint_info(&(hide_alarm->joint));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_hide_alarm_info_response_xml(hide_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(hide_alarm, sizeof(HideAlarmPacket));
}
static void set_hide_alarm_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	HideAlarmPacket *hide_alarm = NULL;

	memset(buffer, 0, sizeof(buffer));
	hide_alarm = j_xml_alloc(sizeof(HideAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_hide_alarm_info_xml(hide_alarm, buffer))
	{
		/*printf("hide_alarm->result.code: %d\n", hide_alarm->result.code);
		printf("hide_alarm->session_id : %s\n", hide_alarm->session_id);
		printf("hide_alarm->domain_id  : %s\n", hide_alarm->domain_id);
		printf("hide_alarm->pu_id      : %s\n", hide_alarm->pu_id);
		printf("hide_alarm->channel    : %d\n", hide_alarm->channel);
		printf("hide_alarm->hide_enable: %d\n", hide_alarm->hide_enable);
		printf("hide_alarm->hide_x     : %d\n", hide_alarm->hide_x);
		printf("hide_alarm->hide_y     : %d\n", hide_alarm->hide_y);
		printf("hide_alarm->hide_w     : %d\n", hide_alarm->hide_w);
		printf("hide_alarm->hide_h     : %d\n", hide_alarm->hide_h);
		show_time_part(&(hide_alarm->time_start), &(hide_alarm->time_end));
		
		show_joint_info(&(hide_alarm->joint));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_set_hide_alarm_info_xml(hide_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(hide_alarm, sizeof(HideAlarmPacket));
}
static void hide_alarm_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(HIDE_ALARM_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void get_io_alarm_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_IO_ALARM_INFO_ID, parse_path, merge_path, flags);
}
static void io_alarm_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	IoAlarmPacket *io_alarm = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	io_alarm = j_xml_alloc(sizeof(IoAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_io_alarm_info_response_xml(io_alarm, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_io_alarm_info_response_xml(io_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(io_alarm, sizeof(IoAlarmPacket));
}
static void set_io_alarm_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	IoAlarmPacket *io_alarm = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	io_alarm = j_xml_alloc(sizeof(IoAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_io_alarm_info_xml(io_alarm, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_set_io_alarm_info_xml(io_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(io_alarm, sizeof(IoAlarmPacket));
}
static void io_alarm_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(IO_ALARM_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void get_joint_action_info_test(const char *parse_path, const char *merge_path, int flags)
{
	test_request_info(GET_JOINT_ACTION_INFO_ID, parse_path, merge_path, flags);
}
static void joint_action_info_response_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	JointActionPacket *joint_action = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	joint_action = j_xml_alloc(sizeof(JointActionPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_joint_action_info_response_xml(joint_action, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_joint_action_info_response_xml(joint_action, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(joint_action, sizeof(JointActionPacket));
}
static void set_joint_action_info_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	JointActionPacket *joint_action = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	joint_action = j_xml_alloc(sizeof(JointActionPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_set_joint_action_info_xml(joint_action, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		merge_set_joint_action_info_xml(joint_action, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(joint_action, sizeof(JointActionPacket));
}
static void joint_action_info_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(JOINT_ACTION_INFO_RESULT_ID, parse_path, merge_path, flags);
}
static void submit_alarm_request_test(const char *parse_path, const char *merge_path, int flags)
{
	char buffer[MAX_BUFFER_SIZE];
	SubmitAlarmPacket *sub_alarm = NULL;
	
	memset(buffer, 0, sizeof(buffer));
	sub_alarm = j_xml_alloc(sizeof(SubmitAlarmPacket));
	
	get_xml_info_from_file(parse_path, buffer, sizeof(buffer));
	if (-1 == flags || 0 == parse_submit_alarm_request_xml(sub_alarm, buffer))
	{
		/*printf("sub_alarm->pu_id       : %s\n", sub_alarm->pu_id);
		printf("sub_alarm->data        : %s\n", sub_alarm->data);
		printf("sub_alarm->alarm       : %d\n", sub_alarm->alarm);
		printf("sub_alarm->channel     : %d\n", sub_alarm->channel);
		show_time_struct(&(sub_alarm->alarm_time));*/
		
		memset(buffer, 0, sizeof(buffer));
		merge_submit_alarm_request_xml(sub_alarm, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), merge_path);
		printf("\n");
	}
	j_xml_dealloc(sub_alarm, sizeof(SubmitAlarmPacket));
}
static void submit_alarm_result_test(const char *parse_path, const char *merge_path, int flags)
{
	test_result_info(SUBMIT_ALARM_RESULT_ID, parse_path, merge_path, flags);
}

int main()
{
	int flags = 0;
	char buffer[MAX_BUFFER_SIZE];
	XmlTreeNode *xml_tree = NULL;

	memset(buffer, 0, sizeof(buffer));

	record_parameter_response_test("ccc.xml", "ddd.xml", 0);

	//xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	//init_xml_tree_node(&xml_tree);

	/*get_xml_info_from_file("debug_data/DeviceInfoResponse.xml", buffer, sizeof(buffer));
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		xml_tree_merge_by_mxml(xml_tree, buffer, sizeof(buffer));
		create_xml_file_from_buffer(buffer, strlen(buffer), "DeviceInfoResponse.xml");
		xml_tree_delete(xml_tree);
	}
	else
		j_xml_dealloc(xml_tree, sizeof(XmlTreeNode));//*/
	
	/*xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	get_xml_info_from_file("debug_data/NetworkInfoResponse0.xml", buffer, sizeof(buffer));
	printf("buffer: %s\n", buffer);
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		xml_tree_merge_by_mxml(xml_tree, buffer, sizeof(buffer));
		printf("buffer: %s\n", buffer);
		create_xml_file_from_buffer(buffer, strlen(buffer), "NetworkInfoResponse0.xml");
		xml_tree_delete(xml_tree);
	}
	else
		j_xml_dealloc(xml_tree, sizeof(XmlTreeNode));//*/
	
	/*xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	get_xml_info_from_file("NetworkInfo.xml", buffer, sizeof(buffer));
	printf("file: %s\n", buffer);
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		memset(buffer, 0, sizeof(buffer));
		xml_tree_merge_by_mxml(xml_tree, buffer, sizeof(buffer));
		printf("buffer: %s\n", buffer);
		create_xml_file_from_buffer(buffer, strlen(buffer), "debug_data/NetworkInfo0.xml");
		xml_tree_delete(xml_tree);
	}
	else
		j_xml_dealloc(xml_tree, sizeof(XmlTreeNode));//*/

//////////////////////////////////////////////////////////////////////////////////////////

	/*flags = 0;
	register_request_test(PARSE_PU_REGISTER_XML, MERGE_PU_REGISTER_XML, flags);
	register_response_test(PARSE_REGISTER_RESPONSE_XML, MERGE_REGISTER_RESPONSE_XML, flags);
	heart_beat_request_test(PARSE_PU_HEART_XML, MERGE_PU_HEART_XML, flags);
	heart_beat_response_test(PARSE_HEART_RESPONSE_XML, MERGE_HEART_RESPONSE_XML, flags);
	change_dispatch_request_test(PARSE_CHANGE_DISPATCH_REQUEST_XML, MERGE_CHANGE_DISPATCH_REQUEST_XML, flags);
	change_dispatch_result_test(PARSE_CHANGE_DISPATCH_RESULT_XML, MERGE_CHANGE_DISPATCH_RESULT_XML, flags);
	get_device_info_test(PARSE_GET_DEVICE_INFO_XML, MERGE_GET_DEVICE_INFO_XML, flags);
	device_info_response_test(PARSE_DEVICE_INFO_RESPONSE_XML, MERGE_DEVICE_INFO_RESPONSE_XML, flags);
	get_device_ntp_info_test(PARSE_GET_DEVICE_NTP_INFO_XML, MERGE_GET_DEVICE_NTP_INFO_XML, flags);
	device_ntp_info_response_test(PARSE_DEVICE_NTP_INFO_RESPONSE_XML, MERGE_DEVICE_NTP_INFO_RESPONSE_XML, flags);
	set_device_ntp_info_test(PARSE_SET_DEVICE_NTP_INFO_XML, MERGE_SET_DEVICE_NTP_INFO_XML, flags);
	device_ntp_info_result_test(PARSE_DEVICE_NTP_INFO_RESULT_XML, MERGE_DEVICE_NTP_INFO_RESULT_XML, flags);
	set_device_time_test(PARSE_SET_DEVICE_TIME_XML, MERGE_SET_DEVICE_TIME_XML, flags);
	device_time_result_test(PARSE_DEVICE_TIME_RESULT_XML, MERGE_DEVICE_TIME_RESULT_XML, flags);
	get_platform_info_test(PARSE_GET_PLATFORM_INFO_XML, MERGE_GET_PLATFORM_INFO_XML, flags);
	platform_info_response_test(PARSE_PLATFORM_INFO_RESPONSE_XML, MERGE_PLATFORM_INFO_RESPONSE_XML, flags);
	set_platform_info_test(PARSE_SET_PLATFORM_INFO_XML, MERGE_SET_PLATFORM_INFO_XML, flags);
	platform_info_result_test(PARSE_PLATFORM_INFO_RESULT_XML, MERGE_PLATFORM_INFO_RESULT_XML, flags);
	get_network_info_test(PARSE_GET_NETWORK_INFO_XML, MERGE_GET_NETWORK_INFO_XML, flags);
	network_info_response_test(PARSE_NETWORK_INFO_RESPONSE_XML, MERGE_NETWORK_INFO_RESPONSE_XML, flags);
	set_network_info_test(PARSE_SET_NETWORK_INFO_XML, MERGE_SET_NETWORK_INFO_XML, flags);
	network_info_result_test(PARSE_NETWORK_INFO_RESULT_XML, MERGE_NETWORK_INFO_RESULT_XML, flags);
	get_pppoe_info_test(PARSE_GET_PPPOE_INFO_XML, MERGE_GET_PPPOE_INFO_XML, flags);
	pppoe_info_response_test(PARSE_PPPOE_INFO_RESPONSE_XML, MERGE_PPPOE_INFO_RESPONSE_XML, flags);
	set_pppoe_info_test(PARSE_SET_PPPOE_INFO_XML, MERGE_SET_PPPOE_INFO_XML, flags);
	pppoe_info_result_test(PARSE_PPPOE_INFO_RESULT_XML, MERGE_PPPOE_INFO_RESULT_XML, flags);
	get_encode_parameter_test(PARSE_GET_ENCODE_PARAMETER_XML, MERGE_GET_ENCODE_PARAMETER_XML, flags);
	encode_parameter_response_test(PARSE_ENCODE_PARAMETER_RESPONSE_XML, MERGE_ENCODE_PARAMETER_RESPONSE_XML, flags);
	set_encode_parameter_test(PARSE_SET_ENCODE_PARAMETER_XML, MERGE_SET_ENCODE_PARAMETER_XML, flags);
	encode_parameter_result_test(PARSE_ENCODE_PARAMETER_RESULT_XML, MERGE_ENCODE_PARAMETER_RESULT_XML, flags);
	get_display_parameter_test(PARSE_GET_DISPLAY_PARAMETER_XML, MERGE_GET_DISPLAY_PARAMETER_XML, flags);
	display_parameter_response_test(PARSE_DISPLAY_PARAMETER_RESPONSE_XML, MERGE_DISPLAY_PARAMETER_RESPONSE_XML, flags);
	set_display_parameter_test(PARSE_SET_DISPLAY_PARAMETER_XML, MERGE_SET_DISPLAY_PARAMETER_XML, flags);
	display_parameter_result_test(PARSE_DISPLAY_PARAMETER_RESULT_XML, MERGE_DISPLAY_PARAMETER_RESULT_XML, flags);
	get_record_parameter_test(PARSE_GET_RECORD_PARAMETER_XML, MERGE_GET_RECORD_PARAMETER_XML, flags);
	record_parameter_response_test("ccc.xml", "ddd.xml", 0);//PARSE_RECORD_PARAMETER_RESPONSE_XML, MERGE_RECORD_PARAMETER_RESPONSE_XML, flags);
	set_record_parameter_test(PARSE_SET_RECORD_PARAMETER_XML, MERGE_SET_RECORD_PARAMETER_XML, flags);
	record_parameter_result_test(PARSE_RECORD_PARAMETER_RESULT_XML, MERGE_RECORD_PARAMETER_RESULT_XML, flags);
	get_hide_parameter_test(PARSE_GET_HIDE_PARAMETER_XML, MERGE_GET_HIDE_PARAMETER_XML, flags);
	hide_parameter_response_test(PARSE_HIDE_PARAMETER_RESPONSE_XML, MERGE_HIDE_PARAMETER_RESPONSE_XML, flags);
	set_hide_parameter_test(PARSE_SET_HIDE_PARAMETER_XML, MERGE_SET_HIDE_PARAMETER_XML, flags);
	hide_parameter_result_test(PARSE_HIDE_PARAMETER_RESULT_XML, MERGE_HIDE_PARAMETER_RESULT_XML, flags);
	get_serial_parameter_test(PARSE_GET_SERIAL_PARAMETER_XML, MERGE_GET_SERIAL_PARAMETER_XML, flags);
	serial_parameter_response_test(PARSE_SERIAL_PARAMETER_RESPONSE_XML, MERGE_SERIAL_PARAMETER_RESPONSE_XML, flags);
	set_serial_parameter_test(PARSE_SET_SERIAL_PARAMETER_XML, MERGE_SET_SERIAL_PARAMETER_XML, flags);
	serial_parameter_result_test(PARSE_SERIAL_PARAMETER_RESULT_XML, MERGE_SERIAL_PARAMETER_RESULT_XML, flags);
	get_osd_parameter_test(PARSE_GET_OSD_PARAMETER_XML, MERGE_GET_OSD_PARAMETER_XML, flags);
	osd_parameter_response_test(PARSE_OSD_PARAMETER_RESPONSE_XML, MERGE_OSD_PARAMETER_RESPONSE_XML, flags);
	set_osd_parameter_test(PARSE_SET_OSD_PARAMETER_XML, MERGE_SET_OSD_PARAMETER_XML, flags);
	osd_parameter_result_test(PARSE_OSD_PARAMETER_RESULT_XML, MERGE_OSD_PARAMETER_RESULT_XML, flags);
	get_ptz_parameter_test(PARSE_GET_PTZ_PARAMETER_XML, MERGE_GET_PTZ_PARAMETER_XML, flags);
	ptz_parameter_response_test(PARSE_PTZ_PARAMETER_RESPONSE_XML, MERGE_PTZ_PARAMETER_RESPONSE_XML, flags);
	set_ptz_parameter_test(PARSE_SET_PTZ_PARAMETER_XML, MERGE_SET_PTZ_PARAMETER_XML, flags);
	ptz_parameter_result_test(PARSE_PTZ_PARAMETER_RESULT_XML, MERGE_PTZ_PARAMETER_RESULT_XML, flags);
	get_ftp_parameter_test(PARSE_GET_FTP_PARAMETER_XML, MERGE_GET_FTP_PARAMETER_XML, flags);
	ftp_parameter_response_test(PARSE_FTP_PARAMETER_RESPONSE_XML, MERGE_FTP_PARAMETER_RESPONSE_XML, flags);
	set_ftp_parameter_test(PARSE_SET_FTP_PARAMETER_XML, MERGE_SET_FTP_PARAMETER_XML, flags);
	ftp_parameter_result_test(PARSE_FTP_PARAMETER_RESULT_XML, MERGE_FTP_PARAMETER_RESULT_XML, flags);
	get_smtp_parameter_test(PARSE_GET_SMTP_PARAMETER_XML, MERGE_GET_SMTP_PARAMETER_XML, flags);
	smtp_parameter_response_test(PARSE_SMTP_PARAMETER_RESPONSE_XML, MERGE_SMTP_PARAMETER_RESPONSE_XML, flags);
	set_smtp_parameter_test(PARSE_SET_SMTP_PARAMETER_XML, MERGE_SET_SMTP_PARAMETER_XML, flags);
	smtp_parameter_result_test(PARSE_SMTP_PARAMETER_RESULT_XML, MERGE_SMTP_PARAMETER_RESULT_XML, flags);
	get_upnp_parameter_test(PARSE_GET_UPNP_PARAMETER_XML, MERGE_GET_UPNP_PARAMETER_XML, flags);
	upnp_parameter_response_test(PARSE_UPNP_PARAMETER_RESPONSE_XML, MERGE_UPNP_PARAMETER_RESPONSE_XML, flags);
	set_upnp_parameter_test(PARSE_SET_UPNP_PARAMETER_XML, MERGE_SET_UPNP_PARAMETER_XML, flags);
	upnp_parameter_result_test(PARSE_UPNP_PARAMETER_RESULT_XML, MERGE_UPNP_PARAMETER_RESULT_XML, flags);
	get_disk_info_test(PARSE_GET_DISK_INFO_XML, MERGE_GET_DISK_INFO_XML, flags);
	disk_info_response_test(PARSE_DISK_INFO_RESPONSE_XML, MERGE_DISK_INFO_RESPONSE_XML, flags);
	/*format_disk_request_test(PARSE_FORMAT_DISK_REQUEST_XML, MERGE_FORMAT_DISK_REQUEST_XML, flags);
	format_disk_result_test(PARSE_FORMAT_DISK_RESULT_XML, MERGE_FORMAT_DISK_RESULT_XML, flags);
	get_format_progress_test(PARSE_GET_FORMAT_PROGRESS_XML, MERGE_GET_FORMAT_PROGRESS_XML, flags);
	format_progress_response_test(PARSE_FORMAT_PROGRESS_RESPONSE_XML, MERGE_FORMAT_PROGRESS_RESPONSE_XML, flags);
	get_move_alarm_info_test(PARSE_GET_MOVE_ALARM_INFO_XML, MERGE_GET_MOVE_ALARM_INFO_XML, flags);
	move_alarm_info_response_test(PARSE_MOVE_ALARM_INFO_RESPONSE_XML, MERGE_MOVE_ALARM_INFO_RESPONSE_XML, flags);
	set_move_alarm_info_test(PARSE_SET_MOVE_ALARM_INFO_XML, MERGE_SET_MOVE_ALARM_INFO_XML, flags);
	move_alarm_info_result_test(PARSE_MOVE_ALARM_INFO_RESULT_XML, MERGE_MOVE_ALARM_INFO_RESULT_XML, flags);
	get_lost_alarm_info_test(PARSE_GET_LOST_ALARM_INFO_XML, MERGE_GET_LOST_ALARM_INFO_XML, flags);
	lost_alarm_info_response_test(PARSE_LOST_ALARM_INFO_RESPONSE_XML, MERGE_LOST_ALARM_INFO_RESPONSE_XML, flags);
	set_lost_alarm_info_test(PARSE_SET_LOST_ALARM_INFO_XML, MERGE_SET_LOST_ALARM_INFO_XML, flags);
	lost_alarm_info_result_test(PARSE_LOST_ALARM_INFO_RESULT_XML, MERGE_LOST_ALARM_INFO_RESULT_XML, flags);
	get_hide_alarm_info_test(PARSE_GET_HIDE_ALARM_INFO_XML, MERGE_GET_HIDE_ALARM_INFO_XML, flags);
	hide_alarm_info_response_test(PARSE_HIDE_ALARM_INFO_RESPONSE_XML, MERGE_HIDE_ALARM_INFO_RESPONSE_XML, flags);
	set_hide_alarm_info_test(PARSE_SET_HIDE_ALARM_INFO_XML, MERGE_SET_HIDE_ALARM_INFO_XML, flags);
	hide_alarm_info_result_test(PARSE_HIDE_ALARM_INFO_RESULT_XML, MERGE_HIDE_ALARM_INFO_RESULT_XML, flags);
	get_io_alarm_info_test(PARSE_GET_IO_ALARM_INFO_XML, MERGE_GET_IO_ALARM_INFO_XML, flags);
	io_alarm_info_response_test(PARSE_IO_ALARM_INFO_RESPONSE_XML, MERGE_IO_ALARM_INFO_RESPONSE_XML, flags);
	set_io_alarm_info_test(PARSE_SET_IO_ALARM_INFO_XML, MERGE_SET_IO_ALARM_INFO_XML, flags);
	io_alarm_info_result_test(PARSE_IO_ALARM_INFO_RESULT_XML, MERGE_IO_ALARM_INFO_RESULT_XML, flags);
	submit_alarm_request_test(PARSE_SUBMIT_ALARM_REQUEST_XML, MERGE_SUBMIT_ALARM_REQUEST_XML, flags);
	submit_alarm_result_test(PARSE_SUBMIT_ALARM_RESULT_XML, MERGE_SUBMIT_ALARM_RESULT_XML, flags);
	//*/

	//Time time;
	//parse_time_string("2012-01-06 12:12:12", &time);
	
	return 0;
}



#if 0


//=================== Internal Msg =============

typedef struct _Msg Msg;
typedef void (*JMsgFinalize)(void *private_data, size_t size);
struct _Msg
{
	msg_t	msg_id;

	void *private_data;
	JMsgFinalize finalize;
};


Msg *jpf_new_msg(msg_t id, char *priv, size_t priv_size, JMsgFinalize fin);
Msg *jpf_new_msg_2(msg_t id, char *priv, size_t priv_size);



void fin(void *ptr, size_t size)
{
	j_xml_dealloc(ptr, size);
}

void a()
{
	struct DEVICE_INFO *di = malloc();

	jpf_new_msg(xx, di, sizeof(*di), fin);
}



void b()
{
	struct DEVICE_INFO di;

	jpf_new_msg_2(xx, &di, sizeof(*di));
}



///================




//===========================   XML Layer ==============


typedef int (*JCreateXML)(Msg *msg, char buf[], size_t size);
typedef void *(*JParseXML)(char buf[], size_t size, int *err, msg_t *msg_id);


typedef struct _XmlEntry XmlEntry;
struct _XmlEntry
{
	xml_id		cmd;
	JCreateXML  func_c;
	JParseXML   func_p;
	//
//	unsigned flags;
};


typedef void *(*alloc_t)(size_t);

void set_mem_handler(alloc_t alloc, void (*dalloc)(void *, size_t))
{
	////
	g_alloc = alloc;
	g_dalloc = dalloc;
}


void *(*g_alloc)(size_t size) = defalut_alloc;
void (*g_dalloc)(void *ptr, size_t size) = default_dalloc;
void (*g_log)(const char *msg);

#define xml_alloc(size) (*g_alloc)(size)
#define xml_dealloc(ptr, size) (*g_dalloc)((ptr), (size))


typedef struct _XmlTable XmlTable;

struct _XmlTable
{
	int n_entries;
	int n_capacity;
	XmlEntry *entries;
};


void init_xml_table(XmlTable *table)
{
	//J_ASSERT(table != NULL);
	memset(table, 0, sizeof(*table));
}

int register_xml_entry(XmlTable *table, xml_id cmd, JCreateXML func_c, JParseXML func_p)
{
	XmlEntry *entries;
	int capacity;
	//J_ASSERT(table != NULL);

	if (table->n_entries >= table->n_capacity)
	{
		capacity = table->n_capacity ? 2 * table->n_capacity : 1;
		entries = j_xml_alloc(capacity * sizeof(XmlEntry));
		if (entries)
			return -E_NOMEM;
		memcpy(entries, table->entries, table->n_capacity * sizeof(XmlEntry));
		j_xml_dealloc(table->entries);
		table->n_capacity = capacity;
		table->entries = entries;
	}

	table->entries[table->n_entries].cmd = cmd;
	table->entries[table->n_entries].func_c = func_c;
	table->entries[table->n_entries].func_p = func_p;

	++table->n_entries;
	return 0;

}


//================= interfaces to upper layer ==============

XmlTable global_xml_table = {0};


#define E_XML_BASE		512
#define E_NOMEM		(E_XML_BASE + 1)
#define E_UNSUPP	(E_XML_BASE + 2)


int xml_table_create(XmlTable *table, Msg *msg, char buf[], size_t size)
{
	int ret = -E_UNSUPP, index = 0;

	for (;;)
	{
		if (table->entries[index].cmd == msg->msg_id)
			ret = (*table->entries[index].func_c)(msg, buf, size);
	}

	return ret;
}



int create_xml(Msg *msg, char buf[], size_t size)
{
	XmlTable *table = get_xml_table();

	return xml_table_create(table, msg, buf, size);
}


Msg *parse_xml(char buf[], size_t size, int *err)
{
	void *struct_obj = xml_table_parse(, &msg_id);
	Msg *msg = create_msg(struct_obj, msg_id);

	return xml_table_parse(table, msg, buf, size);
}



register_xml_entry();
register_xml_entry();
register_xml_entry();
register_xml_entry();
register_xml_entry();
register_xml_entry();
register_xml_entry();
register_xml_entry();
register_xml_entry();

	
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>



#endif



