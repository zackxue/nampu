#ifndef __PROXY_INFO_H__
#define __PROXY_INFO_H__



#include "stdafx.h"


#ifdef HAVE_PROXY_INFO


#define USER_LONGIN_REQUEST_CMD		"UserLogin"
#define USER_LONGIN_RESULT_CMD		"UserLoginResponse"

#define ADD_USER_REQUEST_CMD		"AddUser"
#define ADD_USER_RESULT_CMD			"AddUserResponse"
#define DEL_USER_REQUEST_CMD		"DelUser"
#define DEL_USER_RESULT_CMD			"DelUserResponse"

#define USER_LIST_INFO_CMD			"UserListInfo"
#define DEVICE_LIST_INFO_CMD		"DeviceListInfo"

#define FUZZY_FIND_USER_REQUEST_CMD	"FuzzyFindUser"
#define FUZZY_FIND_USER_RESULT_CMD	"FuzzyFindUserResponse"

#define MODIFY_PASSWORD_REQUEST_CMD	"ModifyPassword"
#define MODIFY_PASSWORD_RESULT_CMD	"ModifyPasswordResponse"

#define ADD_DEVICE_REQUEST_CMD		"AddProxyDevice"
#define ADD_DEVICE_RESULT_CMD		"AddProxyDeviceResponse"
#define DEL_DEVICE_REQUEST_CMD		"DelProxyDevice"
#define DEL_DEVICE_RESULT_CMD		"DelProxyDeviceResponse"
#define GET_DEVICE_INFO_REQUEST_CMD	"GetProxyDeviceInfo"
#define GET_DEVICE_INFO_RESULT_CMD	"GetProxyDeviceInfoResponse"
#define SET_DEVICE_INFO_REQUEST_CMD	"SetProxyDeviceInfo"
#define SET_DEVICE_INFO_RESULT_CMD	"SetProxyDeviceInfoResponse"

#define GET_ALL_DEVICE_ID_REQUEST_CMD	"GetAllDeviceId"
#define GET_ALL_DEVICE_ID_RESULT_CMD	"GetAllDeviceIdResponse"







#define USERNAME_STR		"userName"
#define PASSWORD_STR		"password"
#define RESULT_STR			"result"

#define USER_LIST_STR		"userList"
#define COUNT_STR			"count"

#define OLD_PASSWORD_STR	"oldPassword"
#define NEW_PASSWORD_STR	"newPassword"


//#define PU_ID_STR			"puId"
#define DEVICE_ID_STR		"deviceId"
#define FACTORY_NAME_STR	"factoryName"
#define DEVICE_TYPE_STR		"machineType"
#define SDK_VERSION_STR		"sdkVersion"
#define DEVICE_IP_STR		"deviceIp"
#define DEVICE_PORT_STR		"devicePort"
#define PLATFORM_IP_STR		"platformIp"
#define PLATFORM_PORT_STR	"platformPort"

#define DEVICE_LIST_STR		"deviceList"
//#define DEVICE_ID_STR		"id"

#define DEVICE_ID_LIST_STR	"deviceIdList"



#define MAX_ID_LEN				32
#define MAX_NAME_LEN			64
#define MAX_PWD_LEN				64
#define MAX_VERSION_LEN			16
#define MAX_IP_LEN				16


enum //MachineType
{
	MachineType,
};




typedef struct _prx_user_info
{
	char username[MAX_NAME_LEN];		//用户名
	char password[MAX_PWD_LEN];			//密码	
}prx_user_info;

typedef struct _prx_modify_pwd
{
	char username[MAX_NAME_LEN];		//用户名
	char old_pwd[MAX_PWD_LEN];			//旧密码	
	char new_pwd[MAX_PWD_LEN];			//新密码	
}prx_modify_pwd;

typedef struct _prx_device_info
{
	int result;
	int type;							//设备类型
	int device_port;					//设备端口
	int platform_port;					//平台端口
	int device_id;						//设备ID
	char pu_id[MAX_ID_LEN];				//PUID
	char factory[MAX_NAME_LEN];			//厂家
	char sdk_version[MAX_VERSION_LEN];	//软件版本
	char device_ip[MAX_IP_LEN];			//设备IP 地址
	char platform_ip[MAX_IP_LEN];		//平台IP 地址
	char username[MAX_NAME_LEN];		//用户名
	char password[MAX_PWD_LEN];			//密码
}prx_device_info;

typedef struct _prx_user_st
{
	int result;
	int count;
	prx_user_info* user_info;
}prx_user_st;

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








#endif  //HAVE_PROXY_INFO

#endif	//__PROXY_INFO_H__

