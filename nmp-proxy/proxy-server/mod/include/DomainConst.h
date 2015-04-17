#ifndef _DOMAINCONST_H_
#define _DOMAINCONST_H_

#ifndef CONST_MAXLENGTH_IP
#define CONST_MAXLENGTH_IP								48		//字符串IP长度
#endif

#define CONST_IPPROTO_V4								1		//IPV4协议
#define CONST_IPPROTO_V6								2		//IPV6协议
#define CONST_MAXLENGTH_DEVICEID						32		//设备id长度
#define CONST_MAXLENGTH_PTZDEVICEID						32		//
#define CONST_MAXLENGTH_ALARMINPUTNAME					32		//报警输入名字长度
#define CONST_MAXLENGTH_ALARMOUTNAME					32		//报警输出名字长度
#define CONST_MAXLENGTH_ALARMSOURCENAME					32		//报警源名字长度
#define CONST_MAXLENGTH_DEVICENAME						32		//设备名称长度
#define CONST_MAXLENGTH_USERNAME						32		//用户名长度
#define CONST_MAXLENGTH_USERPASSWORD					32		//用户密码长度
#define CONST_MAXLENGTH_USER_DESC						256		//用户描述长度
#define CONST_MAXLENGTH_GROUP_NAME						32		//组名长度
#define CONST_MAXLENGTH_GROUP_DESC						256		//组描述长度
#define CONST_MAXLENGTH_MANUFACTURERID					32		//设备型号长度
#define CONST_MAXLENGTH_MANUFACTURER_NAME				32		//设备名称长度
#define CONST_MAXLENGTH_DEVICEMODEL						64		//设备型号长度
#define CONST_MAXLENGTH_DEVICEDESCRIPTOR				32		//设备描述符长度
#define CONST_MAXLENGTH_MANUFACTURERNAME				32		//设备生产厂家长度
#define CONST_MAXLENGTH_DEVICEPARAM_RESERVE				32		//设备参数保留位

#define CONST_MAXLENGTH_PRODUCTMODEL					32		//产品模组
#define CONST_MAXLENGTH_PRODUCTDESCRIPTION				32		//产品描述
#define CONST_MAXLENGTH_HARDWAREMODEL					32		//硬件模组
#define CONST_MAXLENGTH_HARDWAREDESCRIPTION				64		//硬件描述
#define CONST_MAXLENGTH_MACADDRESS						20		//MAC地址
#define CONST_MAXLENGTH_BARCODE							16		//机器条形码
#define CONST_MAXLENGTH_PRODUCTIONTIME					32		//生产时间
#define CONST_MAXLENGTH_SUMMARYINFO_RESERVE				16		//概要信息保留长度



#define CONST_MAXLENGTH_PROTOCOL						16		//协议长度
#define CONST_MAXLENGTH_PPPOE_USERNAME					32		//PPPoE用户名长度
#define CONST_MAXLENGTH_PPPOE_PASSWORD					32		//PPPoE密码长度
#define CONST_MAXLENGTH_DDNS_DOMAINNAME					64		//DDNS域名长度
#define CONST_MAXLENGTH_DDNS_ACCOUNTS					32		//DDNS帐户长度
#define CONST_MAXLENGTH_DDNS_PASSWORD					32		//DDNS密码长度
#define CONST_MAXLENGTH_DDNS_PROVIDER					32		//DDNS提供者名称最大长度
#define CONST_MAXLENGTH_NAS_STORAGE_USERNAME			32		//NAS存储用户名
#define CONST_MAXLENGTH_NAS_STORAGE_PASSWORD			32		//NAS密码
#define CONST_MAXLENGTH_CAMERA_NAME						32		//摄像机名称长度
#define CONST_MAXLENGTH_CAMERAMODEL						64		//摄像机样式长度
#define CONST_MAXLENTH_PTZ_DEVICE_NAME					32		//云台设备的名字长度
#define CONST_MAXLENTH_PTZ_PROTOCOL_NAME				16		//云台协议的名字长度
#define CONST_MAXLENGTH_HARDWAREVERSION					16		//设备硬件版本长度
#define CONST_MAXLENGTH_SOFTWAREVERSION					16		//设备软件版本长度
#define CONST_MAXLENGTH_HARDWAREVERSION_EXPAND			48		//设备硬件版本长度
#define CONST_MAXLENGTH_SOFTWAREVERSION_EXPAND			48		//设备软件版本长度
#define CONST_MAXLENGTH_OSDINFO							256		//OSD信息长度

#define CONST_MAXLENGTH_TIME							16		//字符串时间长度
#define CONST_MAXLENGTH_FILE_NAME						512		//录制文件名长度

#define CONST_MAXLENGTH_ENCODER_NAME					32		//编码器名称长度
#define CONST_MAXLENGTH_RESOLUTION_NAME					32		//分辨率名称长度
#define CONST_MAXLENGTH_IMAGEFORMAT_NAME				32		//图像格式名称长度
#define CONST_MAXLENGTH_DEVICETYPE_NAME					16		//设备类型名称长度
#define CONST_MAXLENGTH_BITRATETYPE_NAME				16		//比特率类型名称长度
#define CONST_MAXLENGTH_VIDEOSYSTEM_NAME				16		//视频制式名称长度

#define CONST_MAXLENGTH_AUDIO_IN_TYPE_NAME				32		//音频输入类型名称长度
#define CONST_MAXLENGTH_AUDIO_ENCODE_TYPE_NAME			32		//音频编码类型名称长度
#define CONST_MAXLENGTH_AUDIO_DECODE_TYPE_NAME			32		//音频解码类型名称长度


#define CONST_LENGTH_RS232_NAME							32		//RS232串口名称长度
#define CONST_LENGTH_COM_ID								32		//串口id长度

#define CONST_MAXLENGTH_NAS_PATH						256		//NAS路径长度

#define CONST_LENGTH_RS485_NAME							32		//RS485串口名称长度

#define CONST_MAXLENGTH_COMDATA							1024	//每次读写串口最大数据长度
#define CONST_MAXLENGTH_FILEDATA						1024	//每次读写远程文件最大数据长度

#define CONST_MAXLENGTH_ALARM_TYPE_NAME					64		//报警类型名称长度

#define CONST_MAXLENGTH_PTZ_PRESET_NAME					32		//云台预制点的名字长度
#define CONST_MAXLENGTH_PTZ_TRACK_NAME					32		//云台轨迹的名字长度
#define CONST_MAXLENGTH_PTZ_TOUR_NAME					32		//云台巡游的名字长度
#define CONST_MAXLENGTH_PTZ_SCAN_NAME					32		//云台扫描的名字长度
#define CONST_MAXLENGTH_PTZ_KEEPER_NAME					32		//云台看守位的名字长度

#define CONST_MAXLENGTH_VIDEO_OSD_TEXT					256		//视频OSD信息长度
#define CONST_MAXLENGTH_FILENAME_SUFFIX					16		//录制文件后缀名
#define CONST_MAXLENGTH_LOG_DATA						128		//日志信息最多长度

#define  CONST_MAXLENTH_PTZOPERATION_CMD				10		//云台操作命令最大长度

#define  CONST_MAXLENTH_COMMUNICATION_VERIFY_CODE		10		//通信校验码最大长度
#define  CONST_MAXLENTH_RESERVE							8		//保留位最大长度

#define  CONST_MAXLENTH_EMAIL_ADDRESS					32		//邮件最大长度

#define  CONST_MAXLENGTH_FTP_SERVERADDR					32		//FTP服务器地址最大长度
#define  CONST_MAXLENGTH_FTP_USERNAME					32		//FTP账户名最大长度
#define  CONST_MAXLENGTH_FTP_PASSWORD					32		//FTP密码最大长度
#define  CONST_MAXLENGTH_FTP_WORKPATH					64		//FTP工作路径最大长度

#define CONST_MAXLENGTH_SQL								1024	//SQL查询语句最大长度
#define CONST_MAXLENGTH_DBNAME							256		//数据库名称最大长度

#define CONST_ALARM_INPUT_EVENT_TYPE_ID					1		//输入报警事件类型
#define CONST_MOTION_DETECT_EVENT_TYPE_ID				2		//运动检测报警事件类型
#define CONST_CAMERA_MASK_EVENT_TYPE_ID					3		//摄像头遮挡报警事件类型

#define CONST_MAXLENGTH_USERID							32		//用户名ID最大长度
#define CONST_MAXLENGTH_PASSWORD						20		//密码口令最大长度

#define CONST_MAXLENGTH_EMAIL_ADDRESS					128		//电子邮件地址最大长度

#define CONST_MAXLENGTH_RECORD_FILE_HEAD				128		//录像文件头最大长度

#define CONST_MAXLENGTH_GROUPNAME						32		//组名称长度
#define CONST_MAXLENGTH_PICTURENAME						32		//画面名称长度

#define CONST_MAXLENGTH_DEVICE_SERVICE					32		//设备服务内容长度

#define CONST_MAXLENGTH_AVSREAM_NAME					32		//流名称长度

#define CONST_MAXLENGTH_ALARM_SOURCE_ID					32		//报警源ID
#define CONST_MAXLENGTH_ALARM_DESCRIPTION				64		//报警描述
#define CONST_MAXLENTH_PTZ_CMD							32		//云台命令长度
#define CONST_MAXLENTH_AUDIO_DECODE_TYPE_NAME			32		//音频解码类型名称长度
#define CONST_MAXLENTH_AUDIO_ENCODE_TYPE_NAME			32		//音频编码类型名称长度
#define CONST_MAXLENTH_AUDIO_IN_TYPE_NAME				32		//音频输入类型名称长度
#define CONST_MAXLENTH_RESOLUTION_NAME					32		//分辨率名称长度
#define CONST_MAXLENTH_DISK_FILE_SYSTEM_NAME			32		//文件系统名长度
#define CONST_MAXLENTH_MONITOR_TYPE_NAME				32		//监视器输出类型名长度

#define CONST_MAXLENTH_PROTOCOL_NAME					64		//协议名字长度
#define CONST_MAXLENTH_PROTOCOL_VERSION					64		//协议版本长度
#define CONST_MAXLENTH_PROTOCOL_SOFTWARE_VERSION		64		//协议软件版本长度

#define	 CONST_MAXLENGTH_DEVICE_STATUS					256		//设备连接状态描述长度	
#define  CONST_MAXLENGTH_DEVICE_REGISTER_ID				128
#define  CONST_MAXLENGTH_CAMERA_REGISTER_ID				128
#define  CONST_MAXLENGTH_UUID							128		//UUID最大长度
#define  CONST_MAXLENGTH_DEVICE_ADDR					256		//设备地址（IP或域名或url的形式）
#define  CONST_MAXLENGTH_EXTERN_CONFIG					64		//允许的最大扩展配置长度

#define CONST_MAXLENGTH_LANGUAGE_NAME					32		//语言名称长度
#define CONST_OSD_TIMEFORMAT_DESCRIPTION				128		//OSD时间格式长度
#define CONST_WORKMODE_NAME								32		//工作模式名称

#define CONST_MAXLENGTH_PROTOCOL_TYPE                   64		//协议类型长度
#define CONST_MAXLENGTH_PROTOCOL_TYPE_VERSION			64		//协议类型版本长度

#define CONST_MAXLENGTH_ENCODER_LEVEL_NAME				32		//编码等级名称长度

#define CONST_MAXLENGTH_TIME_ZONE_NAME					128		//时区名称长度

#define CONST_MAXLENGTH_PROTOCOL_PARAM                  256		//协议参数长度

#define CONST_MAXLENGTH_SSID							64		//热点SSID长度
#define CONST_MAXLENGTH_ENCRYPTION_TYPE                 64		//加密类型长度
#define CONST_MAXLENGTH_PHONE_NO						32		//电话号码长度
#define CONST_MAXLENGTH_MESSAGE_CONTENT                 256		//消息内容长度长度
#define CONST_MAXLENGTH_MESSAGE_TIME					64		//消息时间长度
#define CONST_MAXLENGTH_ACCESS_POINT					64		//接入点长度
#define CONST_MAXLENGTH_IDENTIFICATION_TYPE				32		//认证方式长度
#define CONST_MAXLENGTH_3G_OPERATOR						32		//3G供应商长度
#define CONST_MAXLENGTH_NET_TYPE						32		//3G网络类型长度
#define CONST_MAXLENGTH_3G_DEVICE_NAME					32		//3G设备名长度
#define CONST_MAXLENGTH_WIFI_DEVICE_NAME				32		//wifi 设备名长度



#endif //_DOMAINCONST_H_
