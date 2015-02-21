#ifndef __PROXY_INFO_H__
#define __PROXY_INFO_H__


#include "j_sdk.h"
#include "j_xmlmsg.h"

#define GET_USER_INFO_REQUEST_CMD		"GetUserInfo"
#define GET_USER_INFO_RESPONSE_CMD		"GetUserInfoResponse"

#define ADD_USER_REQUEST_CMD			"AddUser"
#define ADD_USER_RESULT_CMD				"AddUserResponse"
#define DEL_USER_REQUEST_CMD			"DelUser"
#define DEL_USER_RESULT_CMD				"DelUserResponse"

#define USER_LIST_INFO_CMD				"UserListInfo"
#define DEVICE_LIST_INFO_CMD			"DeviceListInfo"
#define FACTORY_LIST_INFO_CMD			"FactoryListInfo"

#define GET_FACTORY_REQUEST_CMD			"GetFactoryInfo"
#define GET_FACTORY_RESPONSE_CMD		"GetFactoryInfoResponse"

#define FUZZY_FIND_USER_REQUEST_CMD		"FuzzyFindUser"
#define FUZZY_FIND_USER_RESULT_CMD		"FuzzyFindUserResponse"

#define MODIFY_PASSWORD_REQUEST_CMD		"ModifyPassword"
#define MODIFY_PASSWORD_RESULT_CMD		"ModifyPasswordResponse"

#define ADD_DEVICE_REQUEST_CMD			"AddProxyDevice"
#define ADD_DEVICE_RESULT_CMD			"AddProxyDeviceResponse"
#define DEL_DEVICE_REQUEST_CMD			"DelProxyDevice"
#define DEL_DEVICE_RESULT_CMD			"DelProxyDeviceResponse"
#define GET_DEVICE_INFO_REQUEST_CMD		"GetProxyDeviceInfo"
#define GET_DEVICE_INFO_RESULT_CMD		"GetProxyDeviceInfoResponse"
#define SET_DEVICE_INFO_REQUEST_CMD		"SetProxyDeviceInfo"
#define SET_DEVICE_INFO_RESULT_CMD		"SetProxyDeviceInfoResponse"

#define GET_ALL_DEVICE_ID_REQUEST_CMD	"GetAllDeviceId"
#define GET_ALL_DEVICE_ID_RESULT_CMD	"GetAllDeviceIdResponse"

#define BROADCAST_ADD_USER_CMD			"BroadcastAddUser"
#define BROADCAST_DEL_USER_CMD			"BroadcastDelUser"
#define BROADCAST_ADD_DEVICE_CMD		"BroadcastAddDevice"
#define BROADCAST_DEL_DEVICE_CMD		"BroadcastDelDevice"
#define BROADCAST_MODIFY_DEVICE_CMD		"BroadcastModifyDevice"
#define BROADCAST_DEVICE_STATUS_CMD		"BroadcastDeviceStatus"

#define GET_SERVER_CONFIG_REQUEST_CMD	"GetServerConfig"
#define GET_SERVER_CONFIG_RESPONSE_CMD	"GetServerConfigResponse"
#define SET_SERVER_CONFIG_REQUEST_CMD	"SetServerConfig"
#define SET_SERVER_CONFIG_RESULT_CMD	"SetServerConfigResponse"

#define DOWNLOAD_DATA_REQUEST_CMD		"DownloadData"
#define DOWNLOAD_DATA_RESPONSE_CMD		"DownloadDataResponse"
#define UPLOAD_DATA_REQUEST_CMD			"UploadData"
#define UPLOAD_DATA_RESPONSE_CMD		"UploadDataResponse"

#define LIMIT_BROADCASE_STATUE_CMD		"LimitBroadcastDeviceStatus"




#define OFFSET_STR					"offset"

#ifndef TOTAL_COUNT_STR
#define TOTAL_COUNT_STR				"totalCount"
#endif


#define USER_LIST_STR				"userList"

#define OLD_PASSWORD_STR			"oldPassword"
#define NEW_PASSWORD_STR			"newPassword"

#define DEVICE_ID_STR				"deviceId"
#define FACTORY_NAME_STR			"factoryName"
#define DEVICE_TYPE_STR				"machineType"
#define SDK_VERSION_STR				"sdkVersion"
#define DEVICE_PORT_STR				"devicePort"
#define PLATFORM_IP_STR				"platformIp"
#define PLATFORM_PORT_STR			"platformPort"

#define DEVICE_LIST_STR				"deviceList"
#define DEVICE_STR					"device"
#define ID_STR						"id"
#define CONDITION_STR				"condition"
#define DEVICE_STATUS_STR			"status"
#define ERROR_CODE_STR				"errCode"

#define DEVICE_ID_LIST_STR			"deviceIdList"

#define FACTORY_STR					"factory"
#define VERSION_LIST_STR			"versionList"
#define SDK_VERSION_STR				"sdkVersion"
#define MACHINE_LIST_STR			"machineList"

#define SERVER_CONFIG_STR			"serverConfig"
#define SERVER_IP_STR				"serverIp"
#define LISTEN_PORT_STR				"listenPort"
#define RTSP_PORT_STR				"rtspPort"

#define BACKUP_MAGIC_STR            "magic"
#define BACKUP_PORT_STR             "port"
#define BACKUP_SIZE_STR             "fileSize"

#define START_DEVICE_ID_STR         "startDeviceId"
#define END_DEVICE_ID_STR           "endDeviceId"




#define J_PRX_MAX_NAME_LEN			32
#define J_PRX_MAX_PWD_LEN			32


#define J_PRX_MAX_IP_LEN			64
#define J_PRX_MAX_ID_LEN			32
#define J_PRX_MAX_VERSION_LEN		16

#define J_PRX_MAX_TYPE_SIZE			8
#define J_PRX_MAX_VERSION_SIZE		8



enum //MachineType
{
	MachineType,
};


typedef struct _prx_modify_pwd
{
	char username[J_PRX_MAX_NAME_LEN];		//用户名
	char old_pwd[J_PRX_MAX_PWD_LEN];			//旧密码	
	char new_pwd[J_PRX_MAX_PWD_LEN];			//新密码	
}prx_modify_pwd;

typedef struct _prx_device_info
{
	int result;
	int device_id;						//设备ID
	int device_st;						//设备状态
	int device_err;						//错误码
	int pu_type;						//设备类型
	int device_port;					//设备端口
	int platform_port;					//平台端口
	int protocol;
	char pu_id[J_PRX_MAX_ID_LEN];				//PUID
	char factory[J_PRX_MAX_NAME_LEN];			//厂家
	char sdk_version[J_PRX_MAX_VERSION_LEN];	//软件版本
	char device_ip[J_PRX_MAX_IP_LEN];			//设备IP 地址
	char platform_ip[J_PRX_MAX_IP_LEN];		//平台IP 地址
	char username[J_PRX_MAX_NAME_LEN];		//用户名
	char password[J_PRX_MAX_PWD_LEN];			//密码
}prx_device_info;

typedef struct _prx_device_state
{
	int dev_id;							//设备ID
	int dev_state;						//设备状态
	int err_code;						//错误码
}prx_device_state;

typedef struct _prx_user_st
{
	int result;
	int count;
	JUserInfo *user_info;
}prx_user_st;

typedef struct _prx_page_user
{
	int total;
	int offset;
	int count;
	char username[J_PRX_MAX_NAME_LEN];
	JUserInfo *user_info;
}prx_page_user;

typedef struct _prx_page_device
{
	int total;
	int offset;
	int count;
	int dev_id;
	int machine_type;
	char sdk_version[J_PRX_MAX_VERSION_LEN];
	char factory_name[J_PRX_MAX_NAME_LEN];
	prx_device_info *dev_info;
}prx_page_device;

typedef struct _prx_device_id_st
{
	int count;
	int *device_id;
}prx_device_id_st;

typedef struct _prx_device_st
{
	int count;
	prx_device_info *device_info;
}prx_device_st;

typedef struct _prx_factory_info
{
	char factory_name[J_PRX_MAX_NAME_LEN];	//厂家
	int type_count;
	int type[J_PRX_MAX_TYPE_SIZE];
	int ver_count[J_PRX_MAX_TYPE_SIZE];
	char sdk_version[J_PRX_MAX_TYPE_SIZE][J_PRX_MAX_VERSION_SIZE][J_PRX_MAX_VERSION_LEN];	//软件版本
}prx_factory_info;

typedef struct _prx_factory_list
{
	int count;
	prx_factory_info *factory;
}prx_factory_list;

typedef struct _prx_store_log_info
{
	int record_type;
	JTime start_time;
	JTime stop_time;
	int file_size;
}prx_store_log_info;

typedef struct _prx_store_log_list
{
	int result;
	int log_count;
	prx_store_log_info store_log[J_SDK_MAX_STORE_LOG_SIZE];
}prx_store_log_list;

typedef struct _prx_server_config
{
	char server_ip[J_PRX_MAX_IP_LEN];
	int listen_port;
	int rtsp_port;
}prx_server_config;

typedef struct _prx_backup
{
    int magic;
    int port;
    int size;
    int result;
}prx_backup;

typedef struct _prx_limit
{
    int start;
    int end;
}prx_limit;


#endif	//__PROXY_INFO_H__

