#ifndef __NMP_MESSAGE_SHARE_STRUCT_H__
#define __NMP_MESSAGE_SHARE_STRUCT_H__

#include <glib.h>


#define MULTI_NAME_LEN		       512
#define USER_NAME_LEN		       64
#define USER_PASSWD_LEN		128
#define SESSION_ID_LEN		       16
#define DOMAIN_NAME_LEN		64
#define PU_NAME_LEN                     64
#define PU_ID_LEN                          16
#define GU_NAME_LEN                     64
#define GU_ID_LEN                          24
#define DOMAIN_ID_LEN			16
#define AREA_NAME_LEN      		64
#define MAX_ID_LEN          	       32
#define MDS_ID_LEN		   		16
#define MDS_NAME_LEN		   	64
#define MSS_ID_LEN		   		16
#define MSS_NAME_LEN		   	64
#define MAX_STOR_TYPE_LEN		32
#define MAX_IP_LEN                        128
#define GROUP_NAME_LEN               64
#define PHONE_NUM_LEN                 64
#define DESCRIPTION_INFO_LEN     256
#define MF_ID_LEN		   		4
#define MF_NAME_LEN		   	64
#define TIME_INFO_LEN		       32
#define MAX_URL_LEN                     128
#define TIME_LEN                            64
#define TIME_ZONE_LEN                 16
#define TIME_SEG_LEN                   32
#define TIME_SEG_NUM                  8
#define MAX_URL_LEN                    128
#define DETECT_AREA_NUM           32
#define WEEKDAYS                         8
#define VERSION_LEN                     64
#define FILENAME_LEN                  128
#define OFFLINE_REASON_LEN      128
#define STRING_LEN                      512
#define MSS_NUM                           5
#define POLICY_LEN                       20480
#define HD_GROUP_ID_LEN           8
#define HD_GROUP_NUM                5
#define PATH_LEN                         128
#define MAIL_ADDR_LEN               64
#define ALARM_INFO_LEN             64
#define TEXT_DATA_LEN               64
#define DECODE_NAME_LEN          64
#define FIRMWARE_HEAD_LEN     132
#define HD_NAME_LEN                  64
#define FILE_PROPERTY_LEN        128
#define MAP_NAME_LEN                64
#define MAP_LOCATION_LEN         256
#define TW_NAME_LEN                  64
#define SCREEN_NAME_LEN           64
#define DIV_NAME_LEN                  64
#define GENERAL_MSG_PARM_LEN 64
#define TOUR_NAME_LEN                64
#define GROUP_NAME_LEN              64
#define NET_INTERFACE_LEN        64
#define AMS_ID_LEN		   		16
#define AMS_NAME_LEN		   	64
#define AMS_MAX_MSS_COUNT	3
#define AMS_MAX_GUID_COUNT	16
#define PRESET_NAME_LEN		64
#define CRUISE_NAME_LEN		64
#define SRE_NAME_LEN			64
#define DEV_TYPE_LEN			4
#define IO_VALUE_LEN                   64
#define LOG_CHILD_DATA_LEN		32
#define AMS_MAX_LINK_MAP_GU	8
#define IVS_ID_LEN		   		16
#define IVS_NAME_LEN		   	64
#define AMS_DEV_NAME_LEN		64
#define AMS_DEV_PASSWD_LEN	64


#define RES_CODE(res) (((JpfMsgErrCode*)(res))->err_no)
#define SET_CODE(res, code) (RES_CODE(res) = (code))

typedef struct _JpfMsgErrCode JpfMsgErrCode;
struct _JpfMsgErrCode
{
	gint			err_no;
};

typedef struct _JpfMsgCode JpfMsgCode;
struct _JpfMsgCode
{
	JpfMsgErrCode			code;
	glong      affect_num;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfDivMode JpfDivMode;
struct _JpfDivMode
{
    gint                div_id;
    gchar             div_name[DIV_NAME_LEN];
    gchar             description[DESCRIPTION_INFO_LEN];
};

typedef struct _JpfGetDivMode JpfGetDivMode;
struct _JpfGetDivMode
{
    gchar			  session[SESSION_ID_LEN];
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
};

typedef struct _JpfGetDivModeRes JpfGetDivModeRes;
struct _JpfGetDivModeRes
{
    JpfMsgErrCode  code;
    gchar			  session[MAX_ID_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    JpfDivMode    scr_div_info[0];
};

typedef struct __JpfExpiredTime JpfExpiredTime;
struct __JpfExpiredTime
{
	gint type;  //到期时间类型:WDD_TIME_TYPE
	gchar expired_time[TIME_LEN];
};

typedef struct _JpfAllCuOwnPu JpfAllCuOwnPu;
struct _JpfAllCuOwnPu
{
    gchar username[USER_NAME_LEN];
};


typedef struct _JpfResult JpfResult;
struct _JpfResult
{
	JpfMsgErrCode	code;
	gchar			session[SESSION_ID_LEN];
};

#endif	//__NMP_MESSAGE_SHARE_STRUCT_H__
