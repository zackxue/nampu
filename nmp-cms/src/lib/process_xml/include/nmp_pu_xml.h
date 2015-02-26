/********************************************************************
 * jpf_pu_xml.h  - deal xml of pu, parse and create xml
 * Function£ºparse or create xml relate to pu.
 * Author:yangy
 * Description:users can add parse or create message of pu,define
 *             struct about pu information
 * History:
 * 2011.06.01 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 * 2011.06.11 - Zhang Shiyong, optimize code
 ********************************************************************/

#ifndef __NMP_PU_XML__
#define __NMP_PU_XML__

//#include "jpf_xml_data_type.h"
#include "nmp_xml_fun.h"

#define MAX_GU_ENTRIES                  16

#define PU_REGISTER_CMS                      "PuRegister"
#define PU_REGISTER_CMS_RESP                 "PuRegisterResponse"
#define PU_CHANGE_DISPATCH                   "ChangeDispatch"
#define PU_HEART                             "PuHeart"
#define PU_HEART_RESP                        "PuHeartResponse"
#define SUBMIT_ALARM                         "SubmitAlarm"
#define FORMAR_DISK_PROGRESS                 "SubmitFormatProgress"
#define PU_GET_MDS_INFO                      "GetMdsInfo"
#define PU_GET_MDS_INFO_RESP                 "GetMdsInfoResponse"
#define TVWALL_PLAY						     "TwPlay"
#define TVWALL_PLAY_RESP				     "TwPlayResponse"
#define PU_GET_DIV_MODE				         "QueryDivisionMode"
#define PU_GET_DIV_MODE_RESP			     "QueryDivisionModeResponse"
#define ALARM_LINK_PRESET                    "AlarmLinkPreset"

JpfMsgInfo *
jpf_parse_pu_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_register_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_heart_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);
int
jpf_create_pu_change_dispatch(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_platform_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_platform_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_platform_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_platform_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_device_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_device_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_network_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_network_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_network_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_network_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_pppoe_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_pppoe_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_pppoe_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_pppoe_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_media_url_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_media_url_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_encode_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_encode_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_encode_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_encode_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_display_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_display_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_display_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_display_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_OSD_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_OSD_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_OSD_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_OSD_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_record_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_record_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_record_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_record_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_hide_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_hide_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_hide_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_hide_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_serial_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_serial_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_serial_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_serial_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_move_detection_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_move_detection(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_move_detection_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_move_detection(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_video_lost_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_video_lost(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_video_lost_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_video_lost(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_hide_alarm_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_hide_alarm(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_hide_alarm_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_hide_alarm(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_io_alarm_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_io_alarm(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_io_alarm_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_io_alarm(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_joint_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_joint_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_joint_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_joint_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_ptz_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_ptz_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_ptz_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_ptz_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_control_ptz_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_control_ptz(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_preset_point_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_preset_point(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_preset_point_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_preset_point(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_cruise_way_set_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_cruise_way_set(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_cruise_way_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_cruise_way(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_add_cruise_way_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_add_cruise_way(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_modify_cruise_way_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_modify_cruise_way(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_cruise_way_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_cruise_way(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_3D_control_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_3D_control(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_3D_goback_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_3D_goback(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_device_time_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_device_time(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_device_time_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_device_time(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_ntp_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_ntp_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_ntp_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_ntp_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_ftp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_ftp_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_ftp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_ftp_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_smtp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_smtp_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_smtp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_smtp_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_upnp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_upnp_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_upnp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_upnp_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_transparent_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_transparent_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_transparent_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_transparent_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_ddns_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_ddns_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_ddns_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_ddns_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_disk_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_disk_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_resolution_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_resolution_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_resolution_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_resolution_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_ircut_control_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_ircut_control_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_set_ircut_control_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_set_ircut_control_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_format_disk_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_format_disk(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_submit_format_pro(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

JpfMsgInfo *
jpf_parse_pu_submit_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

JpfMsgInfo *
jpf_parse_pu_get_store_log_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_store_log(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_firmware_upgrade_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_firmware_upgrade(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_control_device_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_control_device(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_mds_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_mds_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_div_mode(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_div_mode_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_tw_play_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_tw_play(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_scr_state_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_get_scr_state(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_change_div_mode_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_change_div_mode(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_full_screen_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_full_screen(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_exit_full_screen_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_exit_full_screen(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_clear_division_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_clear_division(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_alarm_link_io_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_pu_alarm_link_io(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_pu_alarm_link_preset(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_pu_get_def_display_para(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_pu_get_def_display_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);


#endif
