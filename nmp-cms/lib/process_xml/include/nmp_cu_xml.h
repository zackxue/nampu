/********************************************************************
 * jpf_cu_xml.h  - deal xml of cu, parse and create xml
 * Function£ºparse or create xml relate to cu.
 * Author:yangy
 * Description:users can add parse or create message of cu,define
 *             struct about cu information
 * History:
 * 2011.06.01 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 ********************************************************************/

#ifndef __NMP_CU_XML__
#define __NMP_CU_XML__

//#include "jpf_xml_data_type.h"

#define CU_LOGIN				"ClientUserLogin"
#define CU_LOGIN_RESP			"ClientUserLoginResponse"
#define CU_HEART				"CuHeart"
#define CU_HEART_RESP			"CuHeartResponse"
#define CMS_FORCE_USR_OFFLINE		"ForceUserOffline"
#define CU_GET_ALL_AREA				"GetAllArea"
#define CU_GET_ALL_AREA_RESP		"GetAllAreaResponse"
#define CU_GET_AREA_INFO			"GetAreaInfo"
#define CU_GET_AREA_INFO_RESP		"GetAreaInfoResponse"
#define CU_GET_DEVICE_LIST			"GetDeviceList"
#define CU_GET_DEVICE_LIST_RESP		"GetDeviceListResponse"
#define CU_GET_AREA_DEVICE			"GetAreaDevice"
#define CU_GET_AREA_DEVICE_RESP		"GetAreaDeviceResponse"
#define CU_REQUEST_MEDIA			"GetMediaURL"
#define CU_REQUEST_MEDIA_RESP		"GetMediaURLResponse"
#define CHANGE_PU_ONLINE_STATE		"ChangePuOnlineState"
#define NOTIFY_MODIFY_DOMAIN		"NotifyModifyDomain"

#define GET_PLATFORM_INFO			"GetPlatformInfo"
#define GET_PLATFORM_INFO_RESP		"GetPlatformInfoResponse"
#define SET_PLATFORM_INFO			"SetPlatformInfo"
#define SET_PLATFORM_INFO_RESP		"SetPlatformInfoResponse"
#define GET_DEVICE_INFO				"GetDeviceInfo"
#define GET_DEVICE_INFO_RESP		"GetDeviceInfoResponse"
#define GET_NETWORK_INFO			"GetNetworkInfo"
#define GET_NETWORK_INFO_RESP		"GetNetworkInfoResponse"
#define SET_NETWORK_INFO			"SetNetworkInfo"
#define SET_NETWORK_INFO_RESP		"SetNetworkInfoResponse"
#define GET_PPPOE_INFO				"GetPPPOEInfo"
#define GET_PPPOE_INFO_RESP			"GetPPPOEInfoResponse"
#define SET_PPPOE_INFO				"SetPPPOEInfo"
#define SET_PPPOE_INFO_RESP			"SetPPPOEInfoResponse"
#define GET_ENCODE_PARA				"GetEncodeParameter"
#define GET_ENCODE_PARA_RESP		"GetEncodeParameterResponse"
#define SET_ENCODE_PARA				"SetEncodeParameter"
#define SET_ENCODE_PARA_RESP		"SetEncodeParameterResponse"
#define GET_DISPLAY_PARA			"GetDisplayParameter"
#define GET_DISPLAY_PARA_RESP		"GetDisplayParameterResponse"
#define GET_DEF_DISPLAY_PARA			"GetDefDisplayParameter"
#define GET_DEF_DISPLAY_PARA_RESP		"GetDefDisplayParameterResponse"
#define SET_DISPLAY_PARA			"SetDisplayParameter"
#define SET_DISPLAY_PARA_RESP		"SetDisplayParameterResponse"
#define GET_OSD_PARA				"GetOSDParameter"
#define GET_OSD_PARA_RESP			"GetOSDParameterResponse"
#define SET_OSD_PARA				"SetOSDParameter"
#define SET_OSD_PARA_RESP			"SetOSDParameterResponse"

#define GET_MOVE_ALARM_PARA			"GetMoveAlarmParameter"
#define GET_MOVE_ALARM_PARA_RESP	"GetMoveAlarmParameterResponse"
#define SET_MOVE_ALARM_PARA			"SetMoveAlarmParameter"
#define SET_MOVE_ALARM_PARA_RESP	"SetMoveAlarmParameterResponse"

#define GET_VIDEO_LOST_PARA			"GetVideoLostAlarmParameter"
#define GET_VIDEO_LOST_PARA_RESP	"GetVideoLostAlarmParameterResponse"
#define SET_VIDEO_LOST_PARA			"SetVideoLostAlarmParameter"
#define SET_VIDEO_LOST_PARA_RESP	"SetVideoLostAlarmParameterResponse"

#define GET_HIDE_ALARM_PARA			"GetHideAlarmParameter"
#define GET_HIDE_ALARM_PARA_RESP	"GetHideAlarmParameterResponse"
#define SET_HIDE_ALARM_PARA			"SetHideAlarmParameter"
#define SET_HIDE_ALARM_PARA_RESP	"SetHideAlarmParameterResponse"

#define GET_HIDE_PARA				"GetHideParameter"
#define GET_HIDE_PARA_RESP			"GetHideParameterResponse"
#define SET_HIDE_PARA				"SetHideParameter"
#define SET_HIDE_PARA_RESP			"SetHideParameterResponse"


#define GET_IO_ALARM_PARA			"GetIoAlarmParameter"
#define GET_IO_ALARM_PARA_RESP		"GetIoAlarmParameterResponse"
#define SET_IO_ALARM_PARA			"SetIoAlarmParameter"
#define SET_IO_ALARM_PARA_RESP		"SetIoAlarmParameterResponse"


#define GET_RECORD_PARA				"GetRecordParameter"
#define GET_RECORD_PARA_RESP		"GetRecordParameterResponse"
#define SET_RECORD_PARA				"SetRecordParameter"
#define SET_RECORD_PARA_RESP		"SetRecordParameterResponse"

#define GET_JOINT_PARA				"GetJointParameter"
#define GET_JOINT_PARA_RESP			"GetJointParameterResponse"
#define SET_JOINT_PARA				"SetJointParameter"
#define SET_JOINT_PARA_RESP			"SetJointParameterResponse"

#define GET_PTZ_PARA				"GetPTZParameter"
#define GET_PTZ_PARA_RESP			"GetPTZParameterResponse"
#define SET_PTZ_PARA				"SetPTZParameter"
#define SET_PTZ_PARA_RESP			"SetPTZParameterResponse"
#define CONTROL_PTZ					"ControlPTZCmd"
#define CONTROL_PTZ_RESP			"ControlPTZCmdResponse"

#define GET_PRESET_POINT			"GetPresetPointSet"
#define GET_PRESET_POINT_RESP		"GetPresetPointSetResponse"
#define SET_PRESET_POINT			"SetPresetPoint"
#define SET_PRESET_POINT_RESP		"SetPresetPointResponse"
#define GET_CRUISE_WAY_SET			"GetCruiseWaySet"
#define GET_CRUISE_WAY_SET_RESP		"GetCruiseWaySetResponse"
#define GET_CRUISE_WAY				"GetCruiseWay"
#define GET_CRUISE_WAY_RESP			"GetCruiseWayResponse"
#define ADD_CRUISE_WAY				"AddCruiseWay"
#define ADD_CRUISE_WAY_RESP			"AddCruiseWayResponse"
#define MODIFY_CRUISE_WAY			"ModifyCruiseWay"
#define MODIFY_CRUISE_WAY_RESP		"ModifyCruiseWayResponse"
#define SET_CRUISE_WAY				"SetCruiseWay"
#define SET_CRUISE_WAY_RESP			"SetCruiseWayResponse"
#define THREE_D_CONTROL				"3DControl"
#define THREE_D_CONTROL_RESP		"3DControlResponse"
#define THREE_D_GOBACK				"3DGoback"
#define THREE_D_GOBACK_RESP			"3DGobackResponse"

#define GET_SERIAL_PARA				"GetSerialParameter"
#define GET_SERIAL_PARA_RESP		"GetSerialParameterResponse"
#define SET_SERIAL_PARA				"SetSerialParameter"
#define SET_SERIAL_PARA_RESP		"SetSerialParameterResponse"

#define GET_DEVICE_TIME				"GetDeviceTime"
#define GET_DEVICE_TIME_RESP		"GetDeviceTimeResponse"
#define SET_DEVICE_TIME				"SetDeviceTime"
#define SET_DEVICE_TIME_RESP		"SetDeviceTimeResponse"

#define GET_NTP_INFO				"GetDeviceNTPInfo"
#define GET_NTP_INFO_RESP			"GetDeviceNTPInfoResponse"
#define SET_NTP_INFO				"SetDeviceNTPInfo"
#define SET_NTP_INFO_RESP			"SetDeviceNTPInfoResponse"

#define GET_FTP_PARA				"GetFTPParameter"
#define GET_FTP_PARA_RESP			"GetFTPParameterResponse"
#define SET_FTP_PARA				"SetFTPParameter"
#define SET_FTP_PARA_RESP			"SetFTPParameterResponse"

#define GET_SMTP_PARA				"GetSMTPParameter"
#define GET_SMTP_PARA_RESP			"GetSMTPParameterResponse"
#define SET_SMTP_PARA				"SetSMTPParameter"
#define SET_SMTP_PARA_RESP			"SetSMTPParameterResponse"

#define GET_UPNP_PARA				"GetUPNPParameter"
#define GET_UPNP_PARA_RESP			"GetUPNPParameterResponse"
#define SET_UPNP_PARA				"SetUPNPParameter"
#define SET_UPNP_PARA_RESP			"SetUPNPParameterResponse"
#define GET_TRANSPARENT_PARA		"GetTransparentParam"
#define GET_TRANSPARENT_PARA_RESP	"GetTransparentParamResponse"
#define SET_TRANSPARENT_PARA		"SetTransparentParam"
#define SET_TRANSPARENT_PARA_RESP	"SetTransparentParamResponse"

#define GET_DDNS_PARA				"GetDdnsConfig"
#define GET_DDNS_PARA_RESP			"GetDdnsConfigResponse"
#define SET_DDNS_PARA				"SetDdnsConfig"
#define SET_DDNS_PARA_RESP			"SetDdnsConfigResponse"
#define GET_DISK_INFO				"GetDiskInfo"
#define GET_DISK_INFO_RESP			"GetDiskInfoResponse"
#define GET_RESOLUTION_INFO			"GetResolutionInfo"
#define GET_RESOLUTION_INFO_RESP	"GetResolutionInfoResponse"
#define SET_RESOLUTION_INFO			"SetResolutionInfo"
#define SET_RESOLUTION_INFO_RESP	"SetResolutionInfoResponse"
#define GET_IRCUT_CONTROL_INFO		"GetIrcutControlInfo"
#define GET_IRCUT_CONTROL_INFO_RESP	"GetIrcutControlInfoResponse"
#define SET_IRCUT_CONTROL_INFO		"SetIrcutControlInfo"
#define SET_IRCUT_CONTROL_INFO_RESP	"SetIrcutControlInfoResponse"
#define FORMAT_DISK					"FormatDisk"
#define FORMAT_DISK_RESP			"FormatDiskResponse"
#define GET_ALARM					"GetAlarm"
#define GET_ALARM_RESP				"GetAlarmResponse"
#define GET_ALARM_STATE				"GetAlarmState"
#define GET_ALARM_STATE_RESP		"GetAlarmStateResponse"
#define DEAL_ALARM					"DealAlarm"
#define DEAL_ALARM_RESP				"DealAlarmResponse"
#define GET_STORE_LOG				"GetStoreLog"
#define GET_STORE_LOG_RESP			"GetStoreLogResponse"
#define GET_MSS_STORE_LOG			"GetMssStoreLog"
#define GET_MSS_STORE_LOG_RESP		"GetMssStoreLogResponse"
#define FIRMWARE_UPGRADE			"FirmwareUpgrade"
#define FIRMWARE_UPGRADE_RESP		"FirmwareUpgradeResponse"
#define CONTROL_DEVICE				"ControlDevice"
#define CONTROL_DEVICE_RESP			"ControlDeviceResponse"
#define GET_GU_MSS					"GetGuMss"
#define GET_GU_MSS_RESP				"GetGuMssResponse"
#define GET_DEFENCE_AREA			"GetDefenceArea"
#define GET_DEFENCE_AREA_RESP		"GetDefenceAreaResponse"
#define GET_DEFENCE_MAP				"GetDefenceMap"
#define GET_DEFENCE_MAP_RESP		"GetDefenceMapResponse"
#define GET_DEFENCE_GU				"GetDefenceGu"
#define GET_DEFENCE_GU_RESP			"GetDefenceGuResponse"
#define GET_MAP_HREF				"GetMapHref"
#define GET_MAP_HREF_RESP			"GetMapHrefResponse"
#define GET_GU_MAP_LOCATION			"GetGuMapLocation"
#define GET_GU_MAP_LOCATION_RESP	"GetGuMapLocationResponse"
#define GET_TW						"GetTw"
#define GET_TW_RESP					"GetTwResponse"
#define GET_SCREEN					"GetScreens"
#define GET_SCREEN_RESP				"GetScreensResponse"
#define GET_SCR_DIV					"GetScreenDivision"
#define GET_SCR_DIV_RESP			"GetScreenDivsionResponse"
#define BROADCAST_GENERAL_MSG		"NotifyMessage"
#define GET_SCREEN_STATE			"GetScreenState"
#define GET_SCREEN_STATE_RESP		"GetScreenStateResponse"
#define CHANGE_DIV_MODE				"ChangeDivisionMode"
#define CHANGE_DIV_MODE_RESP		"ChangeDivisionModeResponse"
#define RUN_STEP					"TwRunStep"
#define RUN_STEP_RESP				"TwRunStepResponse"
#define FULL_SCREEN					"FullScreen"
#define FULL_SCREEN_RESP			"FullScreenResponse"
#define EXIT_FULL_SCREEN			"ExitFullScreen"
#define EXIT_FULL_SCREEN_RESP		"ExitFullScreenResponse"
#define TW_PLAY_NOTIFY				"TwPlayNotify"
#define TW_CLEAR_DIVISION			"ClearDivision"
#define TW_CLEAR_DIVISION_RESP		"ClearDivisionResponse"
#define TW_OPERATE_NOTIFY			"TwOperateNotify"
#define DEC_ONLINE_STATE_NOTIFY		"decoderOnlineStateNotify"
#define GET_TOUR					"GetTour"
#define GET_TOUR_RESP				"GetTourResponse"
#define GET_TOUR_STEP				"GetTourStep"
#define GET_TOUR_STEP_RESP			"GetTourStepResponse"
#define RUN_TOUR					"TwRunTour"
#define RUN_TOUR_RESP				"TwRunTourResponse"
#define STOP_TOUR					"TwStopTour"
#define STOP_TOUR_RESP				"TwStopTourResponse"
#define GET_GROUP					"GetGroup"
#define GET_GROUP_RESP				"GetGroupResponse"
#define RUN_GROUP					"TwRunGroup"
#define RUN_GROUP_RESP				"TwRunGroupResponse"
#define STOP_GROUP					"TwStopGroup"
#define STOP_GROUP_RESP				"TwStopGroupResponse"
#define GET_LICENSE_INFO			"GetLicenseInfo"
#define GET_LICENSE_INFO_RESP		"GetLicenseInfoResponse"
#define GET_TW_LICENSE_INFO			"GetTwLicenseInfo"
#define GET_TW_LICENSE_INFO_RESP	"GetTwLicenseInfoResponse"
#define ALARM_LINK_IO				"AlarmLinkIO"
#define ALARM_LINK_IO_RESP			"AlarmLinkIOResponse"
#define ALARM_LINK_MAP				"AlarmLinkMap"
#define CU_MODIFY_USER				"CuModifyUserPwd"
#define CU_MODIFY_USER_RES			"CuModifyUserPwdResponse"
#define CU_QUERY_GUID						"QueryGuid"
#define CU_QUERY_GUID_RESP					"QueryGuidResponse"
#define CU_QUERY_SCREEN_ID					"QueryScreenID"
#define CU_QUERY_SCREEN_ID_RESP			"QueryScreenIDResponse"
#define CU_QUERY_USER_GUIDS				"QueryUserGuids"
#define CU_QUERY_USER_GUIDS_RESP			"QueryUserGuidsResponse"
#define CU_SET_USER_GUIDS					"SetUserGuids"
#define CU_SET_USER_GUIDS_RESP				"SetUserGuidsResponse"
#define CU_SET_SCREEN_NUM					"SetScreenNum"
#define CU_SET_SCREEN_NUM_RESP			"SetScreenNumResponse"
#define CU_QUERY_TOUR_ID					"QueryTourID"
#define CU_QUERY_TOUR_ID_RESP				"QueryTourIDResponse"
#define CU_SET_TOUR_NUM					"SetTourNum"
#define CU_SET_TOUR_NUM_RESP				"SetTourNumResponse"
#define CU_QUERY_GROUP_ID					"QueryGroupID"
#define CU_QUERY_GROUP_ID_RESP			"QueryGroupIDResponse"
#define CU_SET_GROUP_NUM					"SetGroupNum"
#define CU_SET_GROUP_NUM_RESP				"SetGroupNumResponse"


JpfMsgInfo *
jpf_parse_cu_login(xmlDocPtr doc, xmlNodePtr cur, char *cmd);
int
jpf_create_cu_login(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_heart(xmlDocPtr doc, xmlNodePtr cur, char *cmd);

int
jpf_create_cu_heart_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_login_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);
int
jpf_create_cu_login_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_all_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_all_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);


JpfMsgInfo *
jpf_parse_get_area_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_area_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_device_list(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_device_list_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_area_device(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_area_device_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_media_url(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_media_url_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_change_pu_online_state(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_broadcast_general_msg(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_force_usr_offline(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_notify_modify_domain(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_platform_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_platform_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_platform_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_platform_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_device_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_device_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_network_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_network_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_network_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_network_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_pppoe_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_pppoe_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_pppoe_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_pppoe_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_encode_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_encode_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_encode_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_encode_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_display_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_display_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_display_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_display_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_OSD_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_OSD_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_OSD_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_OSD_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_record_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_record_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_record_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_record_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_hide_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_hide_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_hide_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_hide_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_serial_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_serial_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_serial_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_serial_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_move_detection(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_move_detection_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_move_detection(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_move_detection_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_video_lost(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_video_lost_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_video_lost(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_video_lost_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_hide_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_hide_alarm_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_hide_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_hide_alarm_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_io_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_io_alarm_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_io_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_io_alarm_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_joint_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_joint_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_joint_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_joint_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_ptz_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_ptz_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_ptz_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_ptz_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_control_ptz(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_control_ptz_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_preset_point(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_preset_point_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_preset_point(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_preset_point_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_cruise_way_set(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_cruise_way_set_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_cruise_way(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_cruise_way_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_cruise_way(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_cruise_way_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_add_cruise_way(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_add_cruise_way_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_modify_cruise_way(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_modify_cruise_way_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_cruise_way(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_cruise_way_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_3D_control(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_3D_control_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_3D_goback(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_3D_goback_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_device_time(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_device_time_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_device_time(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_device_time_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_ntp_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_ntp_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_ntp_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_ntp_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_ftp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_ftp_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_ftp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_ftp_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_smtp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_smtp_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_smtp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_smtp_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_upnp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_upnp_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_upnp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_upnp_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_transparent_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_transparent_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_transparent_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_transparent_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_ddns_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_ddns_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_ddns_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_ddns_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_disk_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_disk_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_resolution_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_resolution_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_resolution_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_resolution_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_ircut_control_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_ircut_control_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_ircut_control_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_ircut_control_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_format_disk(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_format_disk_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_cu_submit_format_pro(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_cu_submit_alarm(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_cu_get_alarm_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_alarm(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_alarm_state_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_alarm_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_deal_alarm_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_deal_alarm(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_store_log_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_store_log(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_mss_store_log_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_mss_store_log(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

JpfMsgInfo *
jpf_parse_cu_firmware_upgrade(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_firmware_upgrade_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_control_device(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_control_device_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_gu_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_gu_mss_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_defence_area_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_defence_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_defence_map_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_defence_gu_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_map_href_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_gu_map_location(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_gu_map_location_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_tw_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_screen_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_div_mode(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_div_mode_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_scr_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_scr_state_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_change_div_mode(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_change_div_mode_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_run_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_run_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_tw_play_notify(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_full_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_full_screen_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_exit_full_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_exit_full_screen_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_clear_division(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_clear_division_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_tw_operate_notify(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_tw_decoder_online_state_notify(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_tour_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_tour_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_tour_step_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_run_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_run_tour_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_stop_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_stop_tour_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_run_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_run_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_stop_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_stop_group_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_license_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_get_license_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_get_tw_auth_info(xmlDocPtr doc , xmlNodePtr cur, char *cmd);

int
jpf_create_get_tw_auth_info_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_alarm_link_io(xmlDocPtr doc ,	xmlNodePtr cur, char *cmd);

int
jpf_create_cu_alarm_link_io_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_cu_alarm_link_map(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_modify_user_pwd(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_modify_user_pwd_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_get_def_display_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_get_def_display_para_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_query_guid(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_query_guid_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_query_screen_id(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_query_screen_id_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_query_user_guids(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_query_user_guids_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_user_guids(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_user_guids_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_screen_num(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_screen_num_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_query_tour_id(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_query_tour_id_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_tour_num(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_tour_num_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_query_group_id(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_query_group_id_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_cu_set_group_num(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_cu_set_group_num_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);


#endif
