
#ifndef __NMP_XML_INFO_H__
#define __NMP_XML_INFO_H__

#define PU_GET_CSS_REQUEST_CMD				"PuGetCss"
#define PU_GET_CSS_RESPONSE_CMD				"PuGetCssResponse"

#define REGISTER_REQUEST_CMD				"PuRegister"
#define REGISTER_RESPONSE_CMD				"PuRegisterResponse"

#define HEART_BEAT_REQUEST_CMD				"PuHeart"
#define HEART_BEAT_RESPONSE_CMD				"PuHeartResponse"

#define GET_MDS_INFO_REQUEST_CMD			"GetMdsInfo"
#define GET_MDS_INFO_RESPONSE_CMD			"GetMdsInfoResponse"

#define CHANGE_DISPATCH_REQUEST_CMD			"ChangeDispatch"
#define CHANGE_DISPATCH_RESULT_CMD			"ChangeDispatchResponse"

//###########################################################
#define GET_DEVICE_INFO_CMD					"GetDeviceInfo"
#define DEVICE_INFO_RESPONSE_CMD			"GetDeviceInfoResponse"

#define GET_DEVICE_NTP_INFO_CMD				"GetDeviceNTPInfo"
#define DEVICE_NTP_INFO_RESPONSE_CMD		"GetDeviceNTPInfoResponse"
#define SET_DEVICE_NTP_INFO_CMD				"SetDeviceNTPInfo"
#define DEVICE_NTP_INFO_RESULT_CMD			"SetDeviceNTPInfoResponse"

#define GET_DEVICE_TIME_CMD					"GetDeviceTime"
#define DEVICE_TIME_RESPONSE_CMD			"GetDeviceTimeResponse"
#define SET_DEVICE_TIME_CMD					"SetDeviceTime"
#define DEVICE_TIME_RESULT_CMD				"SetDeviceTimeResponse"

#define GET_PLATFORM_INFO_CMD				"GetPlatformInfo"
#define PLATFORM_INFO_RESPONSE_CMD			"GetPlatformInfoResponse"
#define SET_PLATFORM_INFO_CMD				"SetPlatformInfo"
#define PLATFORM_INFO_RESULT_CMD			"SetPlatformInfoResponse"

#define GET_NETWORK_INFO_CMD				"GetNetworkInfo"
#define NETWORK_INFO_RESPONSE_CMD			"GetNetworkInfoResponse"
#define SET_NETWORK_INFO_CMD				"SetNetworkInfo"
#define NETWORK_INFO_RESULT_CMD				"SetNetworkInfoResponse"

#define GET_PPPOE_INFO_CMD					"GetPPPOEInfo"
#define PPPOE_INFO_RESPONSE_CMD				"GetPPPOEInfoResponse"
#define SET_PPPOE_INFO_CMD					"SetPPPOEInfo"
#define PPPOE_INFO_RESULT_CMD				"SetPPPOEInfoResponse"

#define GET_ENCODE_PARAMETER_CMD			"GetEncodeParameter"
#define ENCODE_PARAMETER_RESPONSE_CMD		"GetEncodeParameterResponse"
#define SET_ENCODE_PARAMETER_CMD			"SetEncodeParameter"
#define ENCODE_PARAMETER_RESULT_CMD			"SetEncodeParameterResponse"

#define GET_DISPLAY_PARAMETER_CMD			"GetDisplayParameter"
#define DISPLAY_PARAMETER_RESPONSE_CMD		"GetDisplayParameterResponse"
#define SET_DISPLAY_PARAMETER_CMD			"SetDisplayParameter"
#define DISPLAY_PARAMETER_RESULT_CMD		"SetDisplayParameterResponse"

#define GET_RECORD_PARAMETER_CMD			"GetRecordParameter"
#define RECORD_PARAMETER_RESPONSE_CMD		"GetRecordParameterResponse"
#define SET_RECORD_PARAMETER_CMD			"SetRecordParameter"
#define RECORD_PARAMETER_RESULT_CMD			"SetRecordParameterResponse"

#define GET_HIDE_PARAMETER_CMD				"GetHideParameter"
#define HIDE_PARAMETER_RESPONSE_CMD			"GetHideParameterResponse"
#define SET_HIDE_PARAMETER_CMD				"SetHideParameter"
#define HIDE_PARAMETER_RESULT_CMD			"SetHideParameterResponse"

#define GET_SERIAL_PARAMETER_CMD			"GetSerialParameter"
#define SERIAL_PARAMETER_RESPONSE_CMD		"GetSerialParameterResponse"
#define SET_SERIAL_PARAMETER_CMD			"SetSerialParameter"
#define SERIAL_PARAMETER_RESULT_CMD			"SetSerialParameterResponse"

#define GET_OSD_PARAMETER_CMD				"GetOSDParameter"
#define OSD_PARAMETER_RESPONSE_CMD			"GetOSDParameterResponse"
#define SET_OSD_PARAMETER_CMD				"SetOSDParameter"
#define OSD_PARAMETER_RESULT_CMD			"SetOSDParameterResponse"

#define GET_PTZ_PARAMETER_CMD				"GetPTZParameter"
#define PTZ_PARAMETER_RESPONSE_CMD			"GetPTZParameterResponse"
#define SET_PTZ_PARAMETER_CMD				"SetPTZParameter"
#define PTZ_PARAMETER_RESULT_CMD			"SetPTZParameterResponse"

#define CONTROL_PTZ_COMMAND_CMD				"ControlPTZCmd"
#define PTZ_COMMAND_RESULT_CMD				"ControlPTZCmdResponse"

//////////////////////////////////////////////////////
#define GET_FTP_PARAMETER_CMD				"GetFTPParameter"
#define FTP_PARAMETER_RESPONSE_CMD			"GetFTPParameterResponse"
#define SET_FTP_PARAMETER_CMD				"SetFTPParameter"
#define FTP_PARAMETER_RESULT_CMD			"SetFTPParameterResponse"

#define GET_SMTP_PARAMETER_CMD				"GetSMTPParameter"
#define SMTP_PARAMETER_RESPONSE_CMD			"GetSMTPParameterResponse"
#define SET_SMTP_PARAMETER_CMD				"SetSMTPParameter"
#define SMTP_PARAMETER_RESULT_CMD			"SetSMTPParameterResponse"

#define GET_UPNP_PARAMETER_CMD				"GetUPNPParameter"
#define UPNP_PARAMETER_RESPONSE_CMD			"GetUPNPParameterResponse"
#define SET_UPNP_PARAMETER_CMD				"SetUPNPParameter"
#define UPNP_PARAMETER_RESULT_CMD			"SetUPNPParameterResponse"
//////////////////////////////////////////////////////

#define GET_DEVICE_DISK_INFO_CMD			"GetDiskInfo"
#define DEVICE_DISK_INFO_RESPONSE_CMD		"GetDiskInfoResponse"

#define FORMAT_DISK_REQUEST_CMD				"FormatDisk"
#define FORMAT_DISK_RESULT_CMD				"FormatDiskResponse"
#define SUBMIT_FORMAT_PROGRESS_CMD			"SubmitFormatProgress"

/////////////////////////////////////////////////////
#define GET_MOVE_ALARM_INFO_CMD				"GetMoveAlarmParameter"
#define MOVE_ALARM_INFO_RESPONSE_CMD		"GetMoveAlarmParameterResponse"
#define SET_MOVE_ALARM_INFO_CMD				"SetMoveAlarmParameter"
#define MOVE_ALARM_INFO_RESULT_CMD			"SetMoveAlarmParameterResponse"

#define GET_LOST_ALARM_INFO_CMD				"GetVideoLostAlarmParameter"
#define LOST_ALARM_INFO_RESPONSE_CMD		"GetVideoLostAlarmParameterResponse"
#define SET_LOST_ALARM_INFO_CMD				"SetVideoLostAlarmParameter"
#define LOST_ALARM_INFO_RESULT_CMD			"SetVideoLostAlarmParameterResponse"

#define GET_HIDE_ALARM_INFO_CMD				"GetHideAlarmParameter"
#define HIDE_ALARM_INFO_RESPONSE_CMD		"GetHideAlarmParameterResponse"
#define SET_HIDE_ALARM_INFO_CMD				"SetHideAlarmParameter"
#define HIDE_ALARM_INFO_RESULT_CMD			"SetHideAlarmParameterResponse"

#define GET_IO_ALARM_INFO_CMD				"GetIoAlarmParameter"
#define IO_ALARM_INFO_RESPONSE_CMD			"GetIoAlarmParameterResponse"
#define SET_IO_ALARM_INFO_CMD				"SetIoAlarmParameter"
#define IO_ALARM_INFO_RESULT_CMD			"SetIoAlarmParameterResponse"

#define GET_JOINT_ACTION_INFO_CMD			"GetJointParameter"
#define JOINT_ACTION_INFO_RESPONSE_CMD		"GetJointParameterResponse"
#define SET_JOINT_ACTION_INFO_CMD			"SetJointParameter"
#define JOINT_ACTION_INFO_RESULT_CMD		"SetJointParameterResponse"

#define SUBMIT_ALARM_REQUEST_CMD			"SubmitAlarm"

#define GET_MEDIA_URL_REQUEST_CMD			"GetMediaURL"
#define GET_MEDIA_URL_RESPONSE_CMD			"GetMediaURLResponse"


#define GET_STORE_LOG_REQUEST_CMD			"GetStoreLog"
#define GET_STORE_LOG_RESPONSE_CMD			"GetStoreLogResponse"


#define USER_LONGIN_REQUEST_CMD				"UserLogin"
#define USER_LONGIN_RESULT_CMD				"UserLoginResponse"

#define USER_HEART_REQUEST_CMD 				"UserHeart"
#define USER_HEART_RESPONSE_CMD 			"UserHeartResponse"

#define FIRMWARE_UPGRADE_REQUEST_CMD		"FirmwareUpgrade"
#define FIRMWARE_UPGRADE_RESPONSE_CMD		"FirmwareUpgradeResponse"
#define SUBMIT_UPGRADE_PROGRESS_CMD			"SubmitFirmwareUpgradeProgress"

#define GET_CHANNEL_INFO_REQUEST_CMD		"GetChannelInfo"
#define GET_CHANNEL_INFO_RESPONSE_CMD		"GetChannelInfoResponse"
#define SET_CHANNEL_INFO_REQUEST_CMD		"SetChannelInfo"
#define SET_CHANNEL_INFO_RESULT_CMD			"SetChannelInfoResponse"

#define GET_PICTURE_INFO_REQUEST_CMD		"GetPictureInfo"
#define GET_PICTURE_INFO_RESPONSE_CMD		"GetPictureInfoResponse"
#define SET_PICTURE_INFO_REQUEST_CMD		"SetPictureInfo"
#define SET_PICTURE_INFO_RESULT_CMD			"SetPictureInfoResponse"

#define GET_HL_PICTURE_INFO_REQUEST_CMD		"GetHLPictureInfo"
#define GET_HL_PICTURE_INFO_RESPONSE_CMD	"GetHLPictureInfoResponse"
#define SET_HL_PICTURE_INFO_REQUEST_CMD		"SetHLPictureInfo"
#define SET_HL_PICTURE_INFO_RESULT_CMD		"SetHLPictureInfoResponse"

#define GET_WIFI_CONFIG_REQUEST_CMD			"GetWifiConfig"
#define GET_WIFI_CONFIG_RESPONSE_CMD		"GetWifiConfigResponse"
#define SET_WIFI_CONFIG_REQUEST_CMD			"SetWifiConfig"
#define SET_WIFI_CONFIG_RESULT_CMD			"SetWifiConfigResponse"

#define WIFI_SEARCH_REQUEST_CMD				"WifiSearch"
#define WIFI_SEARCH_RESPONSE_CMD			"WifiSearchResponse"

#define GET_NETWORK_STATUS_REQUEST_CMD		"GetNetworkStatus"
#define GET_NETWORK_STATUS_RESPONSE_CMD		"GetNetworkStatusResponse"

#define CONTROL_DEVICE_REQUEST_CMD			"ControlDevice"
#define CONTROL_DEVICE_RESULT_CMD			"ControlDeviceResponse"

#define GET_DDNS_CONFIG_REQUEST_CMD			"GetDdnsConfig"
#define GET_DDNS_CONFIG_RESPONSE_CMD		"GetDdnsConfigResponse"
#define SET_DDNS_CONFIG_REQUEST_CMD			"SetDdnsConfig"
#define SET_DDNS_CONFIG_RESULT_CMD			"SetDdnsConfigResponse"

#define GET_DEF_DISPLAY_INFO_REQUEST_CMD	"GetDefDisplayParameter"
#define GET_DEF_DISPLAY_INFO_RESPONSE_CMD	"GetDefDisplayParameterResponse"
#define GET_DEF_PICTURE_INFO_REQUEST_CMD	"GetDefPictureInfo"
#define GET_DEF_PICTURE_INFO_RESPONSE_CMD	"GetDefPictureInfoResponse"

#define GET_AVD_CONFIG_REQUEST_CMD			"GetAvdConfig"
#define GET_AVD_CONFIG_RESPONSE_CMD			"GetAvdConfigResponse"
#define SET_AVD_CONFIG_REQUEST_CMD			"SetAvdConfig"
#define SET_AVD_CONFIG_RESULT_CMD			"SetAvdConfigResponse"

#define GET_TRANSPARENTPARAM_REQUEST_CMD	"GetTransparentParam"
#define GET_TRANSPARENTPARAM_RESPONSE_CMD	"GetTransparentParamResponse"
#define SET_TRANSPARENTPARAM_REQUEST_CMD	"SetTransparentParam"
#define SET_TRANSPARENTPARAM_RESPONSE_CMD	"SetTransparentParamResponse"

#define TRANSPARENTPARAM_NOTIFYEVENT_CMD	"NotifyEvent"
#define TRANSPARENTPARAM_CONTROLDEVICE_CMD	"ControlDevice"

#define GET_OPERATION_LOG_REQUEST_CMD		"GetOperationLog"
#define GET_OPERATION_LOG_RESPONSE_CMD		"GetOperationLogResponse"

#define SET_ALARM_UPLOAD_CONFIG_REQUEST_CMD	"SetAlarmUploadConfig"
#define SET_ALARM_UPLOAD_CONFIG_RESULT_CMD	"SetAlarmUploadConfigResponse"

#define GET_PRESET_POINT_SET_REQUEST_CMD	"GetPresetPointSet"
#define GET_PRESET_POINT_SET_RESPONSE_CMD	"GetPresetPointSetResponse"
#define SET_PRESET_POINT_REQUEST_CMD		"SetPresetPoint"
#define SET_PRESET_POINT_RESULT_CMD			"SetPresetPointResponse"

#define GET_CRUISE_WAY_SET_REQUEST_CMD		"GetCruiseWaySet"
#define GET_CRUISE_WAY_SET_RESPONSE_CMD		"GetCruiseWaySetResponse"
#define GET_CRUISE_WAY_REQUEST_CMD			"GetCruiseWay"
#define GET_CRUISE_WAY_RESPONSE_CMD			"GetCruiseWayResponse"
#define ADD_CRUISE_WAY_REQUEST_CMD			"AddCruiseWay"
#define ADD_CRUISE_WAY_RESULT_CMD			"AddCruiseWayResponse"
#define MODIFY_CRUISE_WAY_REQUEST_CMD		"ModifyCruiseWay"
#define MODIFY_CRUISE_WAY_RESULT_CMD		"ModifyCruiseWayResponse"
#define SET_CRUISE_WAY_REQUEST_CMD			"SetCruiseWay"
#define SET_CRUISE_WAY_RESULT_CMD			"SetCruiseWayResponse"
#define GET_CRUISE_CAP_SET_REQUEST_CMD		"GetCruiseCapabilitySet"
#define GET_CRUISE_CAP_SET_RESPONSE_CMD		"GetCruiseCapabilitySetResponse"

#define _3D_CONTROL_REQUEST_CMD				"3DControl"
#define _3D_CONTROL_RESULT_CMD				"3DControlResponse"
#define _3D_GOBACK_REQUEST_CMD				"3DGoback"
#define _3D_GOBACK_RESULT_CMD				"3DGobackResponse"

#define GET_IRCUT_CFG_REQUEST_CMD		    "GetIrcutCfg"
#define GET_IRCUT_CFG_RESPONSE_CMD			"GetIrcutCfgResponse"
#define SET_IRCUT_CFG_REQUEST_CMD		    "SetIrcutCfg"
#define SET_IRCUT_CFG_RESULT_CMD			"SetIrcutCfgResponse"

#define ALARM_LINK_IO_REQUEST_CMD			"AlarmLinkIO"
#define ALARM_LINK_IO_RESULT_CMD			"AlarmLinkIOResponse"
#define ALARM_LINK_PRESET_REQUEST_CMD		"AlarmLinkPreset"
#define ALARM_LINK_PRESET_RESULT_CMD		"AlarmLinkPresetResponse"

#define GET_RESOLUTION_INFO_REQUEST_CMD		"GetResolutionInfo"
#define GET_RESOLUTION_INFO_RESPONSE_CMD	"GetResolutionInfoResponse"
#define SET_RESOLUTION_INFO_REQUEST_CMD		"SetResolutionInfo"
#define SET_RESOLUTION_INFO_RESULT_CMD	    "SetResolutionInfoResponse"

#define GET_IRCUT_CONTROL_REQUEST_CMD		"GetIrcutControlInfo"
#define GET_IRCUT_CONTROL_RESPONSE_CMD	    "GetIrcutControlInfoResponse"
#define SET_IRCUT_CONTROL_REQUEST_CMD		"SetIrcutControlInfo"
#define SET_IRCUT_CONTROL_RESULT_CMD	    "SetIrcutControlInfoResponse"

#define GET_EXTRANET_PORT_REQUEST_CMD       "CssGetExtranetPort"
#define GET_EXTRANET_PORT_RESPONSE_CMD      "CssGetExtranetPortResponse"

#define GET_HERD_ANALYSE_REQUEST_CMD        "GetHerdAnalyseParameter"
#define GET_HERD_ANALYSE_RESPONSE_CMD       "GetHerdAnalyseParameterResponse"
#define SET_HERD_ANALYSE_REQUEST_CMD        "SetHerdAnalyseParameter"
#define SET_HERD_ANALYSE_RESULT_CMD         "SetHerdAnalyseParameterResponse"

#define GET_GRASS_PERCENT_REQUEST_CMD       "GetGrassPercentInfo"
#define GET_GRASS_PERCENT_RESPONSE_CMD      "GetGrassPercentInfoResponse"

#define GET_P2P_ID_REQUEST_CMD              "PuGetP2PId"
#define GET_P2P_ID_RESPONSE_CMD             "PuGetP2PIdResponse"






//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
#define MESSAGE_STR							"message"
#define MESSAGE_TYPE_STR					"type"

#define DEVICE_CODE_STR                     "deviceCode"
#define PU_VERSION_STR                      "puVersion"
#define CSS_IP_STR                          "cssIP"
#define CSS_PORT_STR                        "cssPort"

#define PU_ID_STR							"puId"
#define PU_TYPE_STR							"puType"
#define DEVICE_IP_STR						"deviceIp"

#define CMS_IP_STR							"cmsIp"
#define CMS_PORT_STR						"cmsPort"
#define MDS_IP_STR							"mdsIp"
#define MDS_PORT_STR						"mdsPort"

#define CU_IP_STR							"cuIp"

#define RESULT_CODE_STR						"resultCode"
#define KEEP_ALIVE_STR						"keepAliveTime"
#define SERVER_TIME_STR						"serverTime"
#define PLT_TYPE_STR							"type"


#define MEDIA_TYPE_STR						"mediaType"
#define CHANNEL_STR 						"channel"
#define LEVEL_STR							"level"

#define SESSION_ID_STR						"sessionId"
#define DOMAIN_ID_STR						"domainId"

#define GU_ID_STR							"guId"
#define GU_TYPE_STR							"guType"

#define PROTOCOL_STR						"protocol"
#define IS_CON_CMS_STR						"isConCms"

#define MANU_INFO_STR						"manuInfo"
#define RELEASE_DATE_STR					"releaseDate"
#define DEV_VERSION_STR						"devVersion"
#define HW_VERSION_STR						"hwVersion"
#define PU_SUB_TYPE_STR						"puSubType"
#define DI_NUM_STR							"diNum"
#define DO_NUM_STR							"doNum"
#define CHANNEL_NUM_STR						"channelNum"
#define RS485_NUM_STR						"RS485Num"
#define RS232_NUM_STR						"RS232Num"

#define COUNT_STR							"count"
#define TOTAL_COUNT_STR						"totalCount"

#define TIME_STR							"time"
#define TIME_ZONE_STR						"timeZone"
#define SYNC_ENABLE_STR						"syncEnable"
#define SET_FLAG_STR						"setFlag"


#define NTP_SERVER_IP_STR					"ntpServerIp"
#define TIME_INTERVAL_STR					"timeInterval"
#define NTP_ENABLE_STR						"ntpEnable"
#define DST_ENABLE_STR						"dstEnable"
#define RESERVE_STR							"reserve"

#define NETWORK_STR							"network"
#define NETWORK_TYPE_STR					"type"
#define ETH0_STR							"eth0"
#define WIFI_STR							"wifi"
#define _3G_STR								"_3g"

#define IP_STR								"ip"
#define PORT_STR							"port"
#define NETMASK_STR							"netmask"
#define GATEWAY_STR							"gateway"

#define MAC_STR								"mac"
#define MAIN_DNS_STR						"mainDns"
#define BACKUP_DNS_STR						"backupDns"
#define DHCP_ENABLE_STR						"dhcpEnable"
#define AUTO_DNS_ENABLE_STR					"autoDnsEnable"
#define CMD_PORT_STR						"cmdPort"
#define DATA_PORT_STR						"dataPort"
#define WEB_PORT_STR						"webPort"
#define TALK_PORT_STR						"talkPort"


#define PPPOE_IP_STR						"pppoeIp"
#define PPPOE_ACCOUNT_STR					"pppoeAccount"
#define PPPOE_PASSED_STR					"pppoePasswd"
#define PPPOE_INTERFACE_STR					"pppoeInterface"
#define PPPOE_ENABLE_STR					"pppoeEnable"

#define FRAME_RATE_STR						"frameRate"
#define I_FRAME_INTERVAL_STR				"iFrameInterval"
#define VIDEO_TYPE_STR						"videoType"
#define AUDIO_TYPE_STR						"audioType"
#define AUDIO_INPUT_MODE_STR				"audioInputMode"
#define AUDIO_ENABLE_STR					"audioEnable"
#define RESOLUTION_STR						"resolution"
#define QPVALUE_STR							"QpValue"
#define CODE_RATE_STR						"codeRate"
#define FRAME_PRIORITY_STR					"framePriority"
#define VIDEO_FORMAT_STR					"videoFormat"
#define BIT_RATE_TYPE_STR					"bitRateType"
#define ENCODE_LEVEL_STR					"encodeLevel"

#define CONTRAST_STR						"contrast"
#define BRIGHT_STR							"bright"
#define HUE_STR								"hue"
#define SATURATION_STR						"saturation"
#define SHARPNESS_STR						"sharpness"

#define PRE_RECORD_STR						"preRecord"
#define AUTO_COVER_STR						"autoCover"
#define ALL_DAY_ENABLE_STR					"allDayEnable"

#define HIDE_NUM_STR						"hideNum"
#define HIDE_AREA_STR						"hideArea"
#define HIDE_COLOR_STR						"hideColor"

#define SERIAL_NO_STR						"serialNo"
#define BAUD_RATE_STR						"baudRate"
#define DATA_BIT_STR						"dataBit"
#define STOP_BIT_STR						"stopBit"
#define VERIFY_STR							"verify"

#define DISPLAY_TIME_ENABLE_STR				"displayTimeEnable"
#define TIME_DISPLAY_X_STR					"timeDisplayX"
#define TIME_DISPLAY_Y_STR					"timeDisplayY"
#define TIME_DISPLAY_COLOR_STR				"timeDisplayColor"
#define DISPLAY_TEXT_ENABLE_STR				"displayTextEnable"
#define TEXT_DISPLAY_X_STR					"textDisplayX"
#define TEXT_DISPLAY_Y_STR					"textDisplayY"
#define TEXT_DISPLAY_COLOR_STR				"textDisplayColor"
#define TEXT_DISPLAY_DATA_STR				"textDisplayData"
#define DISPLAY_STREAM_ENABLE_STR			"displayStreamEnable"

#define TIME_DISPLAY_W_STR					"timeDisplayW"
#define TIME_DISPLAY_H_STR					"timeDisplayH"
#define TEXT_DISPLAY_W_STR					"textDisplayW"
#define TEXT_DISPLAY_H_STR					"textDisplayH"

#define OSD_INVERT_COLOR_STR				"invertColor"

#define TEXT1_DISPLAY_ENABLE_STR			"text1DisplayEnable"
#define TEXT1_DISPLAY_DATA_STR				"text1DisplayData"
#define TEXT1_DISPLAY_X_STR					"text1DisplayX"
#define TEXT1_DISPLAY_Y_STR					"text1DisplayY"
#define TEXT1_DISPLAY_W_STR					"text1DisplayW"
#define TEXT1_DISPLAY_H_STR					"text1DisplayH"

#define TEXT2_DISPLAY_ENABLE_STR			"text2DisplayEnable"
#define TEXT2_DISPLAY_DATA_STR				"text2DisplayData"
#define TEXT2_DISPLAY_X_STR					"text2DisplayX"
#define TEXT2_DISPLAY_Y_STR					"text2DisplayY"
#define TEXT2_DISPLAY_W_STR					"text2DisplayW"
#define TEXT2_DISPLAY_H_STR					"text2DisplayH"


#define PTZ_SERIAL_NO						"serialNo"
#define PTZ_ADDR_STR						"ptzAddr"
#define PTZ_PROTOCOL_STR					"ptzProt"

#define PTZ_DIRECTION_STR					"direction"
#define PTZ_PARAM_STR						"param"

#define FTP_IP_STR							"ftpIp"
#define FTP_PORT_STR						"ftpPort"
#define FTP_USR_STR							"ftpUsr"
#define FTP_PWD_STR							"ftpPwd"
#define FTP_PATH_STR						"ftpPath"

#define MAIL_IP_STR							"mailIp"
#define MAIL_PORT_STR						"mailPort"
#define MAIL_ADDR_STR						"mailAddr"
#define MAIL_USR_STR						"mailUsr"
#define MAIL_PWD_STR						"mailPwd"
#define MAIL_RCTP1_STR						"mailRctp1"
#define MAIL_RCTP2_STR						"mailRctp2"
#define MAIL_RCTP3_STR						"mailRctp3"
#define SSL_ENABLE_STR						"SSLEnable"

#define UPNP_IP_STR							"upnpIp"
#define UPNP_ENABLE_STR						"upnpEnable"
#define UPNP_ETH_NO_STR						"upnpEthNo"
#define UPNP_MODEL_STR						"upnpModel"
#define UPNP_REFRESH_TIME_STR				"upnpRefreshTime"
#define UPNP_DATA_PORT_STR					"upnpDataPort"
#define UPNP_WEB_PORT_STR					"upnpWebPort"
#define UPNP_DATA_PORT_RESULT_STR			"upnpDataPortResult"
#define UPNP_WEB_PORT_RESULT_STR			"upnpWebPortResult"
#define UPNP_CMD_PORT_STR			        "upnpCmdPort"
#define UPNP_TALK_PORT_STR			        "upnpTalkPort"
#define UPNP_CMD_PORT_RESULT_STR			"upnpCmdPortResult"
#define UPNP_TALK_PORT_RESULT_STR			"upnpTalkPortResult"




#define DISK_INFO_STR						"diskInfo"
#define DISK_NUM_STR						"diskNum"
#define DISK_STR							"disk"
#define DISK_NO_STR							"diskNo"
#define TOTAL_SIZE_STR						"totalSize"
#define FREE_SIZE_STR						"freeSize"
#define IS_BACKUP_STR						"isBackup"
#define DISK_STATUS_STR						"diskStatus"
#define DISK_TYPE_STR						"diskType"
#define SYS_FILE_TYPE_STR					"sysFileType"

#define FORMAT_PROGRESS_STR					"fmtProgress"



#define MOVE_ENABLE_STR						"moveEnable"
#define SENSITIVE_LEVEL_STR					"sensitiveLevel"
#define DETECT_INTERVAL_STR					"detectInterval"
#define MAX_WIDTH_STR						"maxWidth"
#define MAX_HEIGHT_STR						"maxHeight"

#define DETECT_AREA_STR						"detectArea"
#define RECTANGLE_STR						"rect"
#define RECT_LEFT_STR						"left"
#define RECT_TOP_STR						"top"
#define RECT_RIGHT_STR						"right"
#define RECT_BOTTOM_STR						"bottom"

#define WEEK_DAY_S_STR						"weekDays"
#define WEEK_DAY_STR						"weekDay"
#define WEEK_DAY_ID_STR						"Id"
#define TIME_SEG_S_STR						"timeSegs"
#define TIME_SEG_STR						"timeSeg"
#define TIME_SEG_ENABLE_STR					"enable"

#define LOST_ENABLE_STR						"lostEnable"
#define HIDE_ENABLE_STR						"hideEnable"
#define IO_TYPE_STR							"ioType"
#define IO_ENABLE_STR						"ioEnable"


#define JOINT_ACTION_STR					"jointAction"

#define JOINT_RECORD_STR					"jointRecord"
#define JOINT_RECORD_ENABLE_CHANNEL_STR		"recordEnableChannel"
#define JOINT_RECORD_SECOND_STR				"recordSecond"
#define JOINT_IO_STR						"jointIO"
#define JOINT_BEEP_ENABLE_STR				"beepEnable"
#define JOINT_BEEP_SECOND_STR				"beepSecond"
#define JOINT_OUTPUT_TIMES_STR				"outputTimes"
#define JOINT_OUTPUT_ENABLE_CHANNEL_STR		"outputEnableChannel"
#define JOINT_SNAP_STR						"jointSnap"
#define JOINT_SNAP_ENABLE_CHANNEL_STR		"snapEnableChannel"
#define JOINT_SNAP_INTERVAL_STR				"snapInterval"
#define JOINT_SNAP_TIMES_STR				"snapTimes"
#define JOINT_EMAIL_STR						"jointEmail"
#define JOINT_EMAIL_ENABLE_STR				"emailEnable"

#define ALARM_TIME_STR						"alarmTime"
#define DATA_STR							"data"
#define ALARM_TYPE_STR						"alarmType"
#define ACTION_TYPE_STR						"actionType"

#define TIME_LEN_STR                        "timeLen"

#define MEDIA_STR							"media"
#define URL_STR								"url"

#define USERNAME_STR						"userName"
#define PASSWORD_STR						"password"

#define UPGREADE_DATA_STR					"data"
#define UPGREADE_DATA_LEN_STR				"dataLen"
#define FILE_LENGTH_LEN_STR					"fileLen"
#define UPGREADE_ADDR_STR					"addr"
#define UPGREADE_PROGRESS_STR				"percent"

#define TRANSPARENT_TYPE_STR				"type"
#define TRANSPARENT_CHANNEL_STR				"channel"
#define TRANSPARENT_LENGTH_STR				"length"
#define TRANSPARENT_DATA_STR				"data"


#define REC_INFO_STR						"recInfo"
#define REC_NODE_STR						"recNode"
#define REC_TYPE_STR						"recType"
#define BEG_TIME_STR						"begTime"
#define END_TIME_STR						"endTime"
#define FILE_SIZE_STR						"fileSize"
#define NODE_COUNT_STR						"count"
#define TOTAL_LOG_COUNT_STR					"logCount"
#define SESS_ID_STR							"sessId"
#define PROPERTY_STR						"property"

#define BEG_NODE_STR						"begNode"
#define END_NODE_STR						"endNode"

#define CH_INFO_STR                         "chInfo"
#define RTSP_PORT_STR                       "rtspPort"
#define LINK_NUM_STR                        "linkNum"
#define PROTOCOL_STR                        "protocol"

#define CHANNEL_NO_STR						"channelNo"
#define CHANNEL_TYPE_STR					"channelType"
#define CHANNEL_STATUS_STR					"channelStatus"
#define CHANNEL_NAME_STR					"channelName"

#define REMOTE_CHANNEL_INFO_STR				"remoteChannelInfo"
#define REMOTE_DEVICE_INFO_STR				"remoteDeviceInfo"
#define WINDOW_MODE_STR						"winMode"
#define WIN_MAX_STREAM_STR					"winMaxStream"
#define WIN_MIN_STREAM_STR					"winMinStream"
#define DEV_TYPE_STR						"deviceType"
#define CHANNEL_SUM_STR						"channelSum"
#define DNS_ENABLE_STR						"dnsEnable"
#define DNS_STR								"dns"

#define MIRROR_STR							"mirror"
#define FLIP_STR							"flip"
#define HZ_STR								"hz"

#define AWB_MODE_STR						"awbMode"
#define AWB_RED_STR							"awbRed"
#define AWB_BULE_STR						"awbBlue"
#define WDR_STR								"wdr"
#define IRIS_TYPE_STR						"irisType"
#define EXP_COMENSATION_STR					"expCompensation"
#define AE_MODE_STR							"aeMode"


#define WIFI_ENABLE_STR						"enable"
#define ESSID_STR							"essid"
#define ENCRYPT_TYPE_STR					"encryptType"
#define AUTH_MODE_STR						"authMode"
#define SECRET_KEY_TYPE_STR					"secretKeyType"

#define WIFI_AP_INFO_STR					"wifiAPInfo"
#define ACCESS_POINT_STR					"accessPoint"

#define SIGNAL_QUALITY_STR					"signalQuality"
#define BIT_RATE_STR						"bitRate"

#define ETH_STATUS_STR						"ethStatus"
#define WIFI_STATUS_STR						"wifiStatus"
#define PPPOE_STATUS_STR					"pppoeStatus"

#define COMMAND_STR							"command"

#define DDNS_ACCOUNT_STR					"account"
#define DDNS_OPNT_STR						"open"
#define DDNS_TYPE_STR						"type"
#define DDNS_PORT_STR						"port"
#define DDNS_TIMES_STR						"times"

#define AVD_ENABLE_STR						"enable"
#define AVD_RULE_STR						"avdRule"
#define RULE_STR							"rule"
#define RULE_TYPE_STR						"type"
#define RULE_ENABLE_STR						"enable"
#define ALARM_TIMES_STR						"alarmTimes"
#define AVD_SEGMENT_STR						"avdSegment"
#define SEGMENT_STR							"segment"
#define SEG_INDEX_STR						"index"
#define SEG_OPNT_STR						"open"
#define BEGIN_TIMES_STR						"begin"
#define END_TIMES_STR						"end"

#define NAME_STR						    "name"


# ifdef _USE_DECODER_PROTO_

#define QUERY_DIVISION_MODE_REQUEST_CMD			"QueryDivisionMode"
#define QUERY_DIVISION_MODE_RESPONSE_CMD		"QueryDivisionModeResponse"

#define GET_SCREEN_STATE_REQUEST_CMD			"GetScreenState"
#define GET_SCREEN_STATE_RESPONSE_CMD			"GetScreenStateResponse"
#define SET_DIVISION_MODE_REQUEST_CMD			"ChangeDivisionMode"
#define SET_DIVISION_MODE_RESULT_CMD			"ChangeDivisionModeResponse"

#define SET_FULL_SCREEN_REQUEST_CMD				"FullScreen"
#define SET_FULL_SCREEN_RESULT_CMD				"FullScreenResponse"
#define EXIT_FULL_SCREEN_REQUEST_CMD			"ExitFullScreen"
#define EXIT_FULL_SCREEN_RESULT_CMD				"ExitFullScreenResponse"

#define TV_WALL_PLAY_REQUEST_CMD				"TwPlay"
#define TV_WALL_PLAY_RESULT_CMD					"TwPlayResponse"
#define CLEAR_DIVISION_REQUEST_CMD				"ClearDivision"
#define CLEAR_DIVISION_RESULT_CMD				"ClearDivisionResponse"


#define PAGE_SIZE_STR					"pageSize"
#define START_ROW_STR					"startRow"

#define DIVISION_INFO_STR				"divisionInfo"
#define DIVISION_MODE_STR				"divisionMode"
#define DIVISIONS_STR					"divisions"
#define DIVISION_STR					"division"

#define DIVISION_ID_STR					"divisionId"
#define DIVISION_NO_STR					"divisionNum"
#define DIVISION_NAME_STR				"divisionName"
#define DESCRIPTION_STR					"description"

#define DIS_CHANNEL_STR					"disChannel"
#define SCREEN_IFNO_STR					"screenInfo"
#define ENCODER_NAME_STR				"encoderName"
#define ENCODER_CHANNEL_STR				"encoderChannel"
#define ENCODER_URL_STR					"encoderUrl"
#define ACTION_TYPE_STR					"actionType"
#define ACTION_RESULT_STR				"actionResult"
#define CLEAR_FLAG_STR					"clearFlag"

#define FULL_SCREEN_STATE_STR			"fullScreenState"
#define FULL_SCREEN_MODE_STR			"mode"



#define STEP_NO_STR						"stepNum"
#define KEEP_OTHER_STR					"keepOther"

# endif //_USE_DECODER_PROTO_


#define COND_INFO_STR					"condInfo"
#define LOG_TYPE_STR					"logType"
#define IP_ADDR_STR						"addr"
#define LOG_INFO_STR					"logInfo"
#define LOG_ITEM_STR					"logItem"
#define LOG_TIMES_STR					"times"
#define MAJOR_STR						"major"
#define MINOR_STR						"minor"
#define ARGS_STR						"args"


#define ALARM_UPLOAD_ENABLE_STR			"enable"
#define ALARM_UPLOAD_HOST_STR			"host"
#define ALARM_UPLOAD_PROT_STR			"port"
#define ALARM_UPLOAD_TYPE_STR			"type"


#define PRESET_STR						"preset"
#define PRESET_POINT_STR				"point"
#define PRESET_ACTION_STR				"presetAction"
#define PRESET_NAME_STR					"presetName"
#define PRESET_NO_STR					"presetNo"

#define CRUISE_SET_STR					"cruiseSet"
#define CRUISE_ACTION_STR				"cruiseAction"
#define CRUISE_WAY_STR					"cruiseWay"
#define CRUISE_NAME_STR					"cruiseName"
#define CRUISE_NO_STR					"cruiseNo"
#define CRUISE_SPEED_STR				"speed"
#define DWELL_TIME_STR					"dwellTime"

#define SUPP_MASK_STR					"suppMask"
#define MAX_WAY_NUM_STR					"maxWayNum"
#define MAX_NAME_LEN_STR				"maxNameLen"
#define MAX_PRESET_NUM_STR				"maxPresetNum"
#define MIN_PRESET_NO_STR				"minPresetNo"
#define MAX_PRESET_NO_STR				"maxPresetNo"
#define MIN_SPEED_STR					"minSpeed"
#define MAX_SPEED_STR					"maxSpeed"
#define MIN_DWELL_TIME_STR				"minDwelltime"
#define MAX_DWELL_TIME_STR				"maxDwelltime"


#define X_OFFSET_STR					"xOffset"
#define Y_OFFSET_STR					"yOffset"
#define AMPLIFY_STR						"amplify"

#define IRCUT_CONTROL_STR               "ircutControl"
#define IRCUT_STR                       "ircut"
#define SWITCH_MODE_STR                 "switchMode"
#define AUTO_C2B_STR                    "autoC2B"
#define AUTO_SWITCH_STR                 "autoSwitch"
#define RTC_SWITCH_STR                  "rtcSwitch"
#define TIMER_SWITCH_STR                "timerSwitch"
#define SENSITIVE_STR                   "sensitive"
#define IRCUT_OPEN_STR                  "open"
#define RTC_STR                         "rtc"
#define DAY_STR                         "day"
#define ID_STR                          "id"

#define FODDER_EANBLE_STR               "fodderEnable"
#define REPORT_INTV_STR                 "reportIntv"

#define TROUGH_PARAMS_STR               "troughParams"
#define TROUGH_PARAM_STR                "troughParam"
#define TROUGH_ID_STR                   "Id"
#define QUADR_ANGLE_STR                 "quadrAngle"
#define POINT_STR                       "point"
#define POINT_X_STR                     "x"
#define POINT_Y_STR                     "y"

#define GRASS_PERCENT_STR               "grassPercent"

#define P2P_ID_STR                      "p2pId"

#define CHANNELCOUNT					"channelNum"







#endif //__NMP_XML_INFO_H__


