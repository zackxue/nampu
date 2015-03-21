#ifndef __NMP_MOD_AMS_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_AMS_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"
#include "nmp_share_struct.h"


typedef struct _NmpAmsRegister NmpAmsRegister;
struct _NmpAmsRegister
{
    gchar ams_id[AMS_ID_LEN];
    gchar  ams_version[VERSION_LEN];
};

typedef struct _NmpAmsRegisterRes NmpAmsRegisterRes;
struct _NmpAmsRegisterRes
{
    NmpMsgErrCode       code;
    gchar                    domain_id[DOMAIN_ID_LEN];
    gint                       keep_alive_time;
    gchar			server_time[TIME_INFO_LEN];
};

typedef struct _NmpAmsHeart NmpAmsHeart;
struct _NmpAmsHeart
{
    gchar		ams_id[AMS_ID_LEN];
};

typedef struct _NmpAmsHeartRes NmpAmsHeartRes;
struct _NmpAmsHeartRes
{
    NmpMsgErrCode	code;
    gchar			server_time[TIME_INFO_LEN];
};


typedef struct _NmpActionPolicy NmpActionPolicy;
struct _NmpActionPolicy
{
	int			weekday_num;
	NmpWeekday	weekdays[WEEKDAYS];
};


typedef struct _NmpAmsActionRecord NmpAmsActionRecord;
struct _NmpAmsActionRecord
{
	NmpShareGuid	action_guid;
	gint			level;
	NmpShareMssId	mss_id;
	guint		time_len;
	guint		alarm_type;
};


typedef struct _NmpAmsActionIO NmpAmsActionIO;
struct _NmpAmsActionIO
{
	NmpShareGuid	action_guid;
	guint		time_len;
	gchar		io_value[IO_VALUE_LEN];
	gchar		session[SESSION_ID_LEN];
};

typedef struct _NmpAmsActionIORes NmpAmsActionIORes;
struct _NmpAmsActionIORes
{
    NmpMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
};


typedef struct _NmpAmsActionPreset NmpAmsActionPreset;
struct _NmpAmsActionPreset
{
	NmpShareGuid	action_guid;
	gint			preset_num;
	gchar		session[SESSION_ID_LEN];
};


typedef struct _NmpAmsActionSnapshot NmpAmsActionSnapshot;
struct _NmpAmsActionSnapshot
{
	NmpShareGuid	action_guid;
	gint			level;
	NmpShareMssId	mss_id;
	guint		picture_count;
	guint		alarm_type;
};


typedef struct _NmpAmsActionMapGu NmpAmsActionMapGu;
struct _NmpAmsActionMapGu
{
	NmpShareGuid	action_guid;
	gchar			gu_name[GU_NAME_LEN];
	gint		map_id;						//id
	gchar		map_name[MAP_NAME_LEN];
	gint		level;
};

typedef struct _NmpAmsActionMap NmpAmsActionMap;
struct _NmpAmsActionMap
{
	NmpShareGuid	action_guid;
	gint			alarm_type;
	gchar			alarm_time[TIME_INFO_LEN];
	gchar			gu_name[GU_NAME_LEN];
	gint			defence_id;
	gchar			defence_name[AREA_NAME_LEN];
	gint			map_id;
	gchar			map_name[MAP_NAME_LEN];
	guint			action_gu_count;
	NmpAmsActionMapGu	action_gu[AMS_MAX_LINK_MAP_GU];
	gint				cu_count;
	NmpAllCuOwnPu		cu_list[0];
};


typedef struct _NmpAmsGetDeviceInfo NmpAmsGetDeviceInfo;
struct _NmpAmsGetDeviceInfo
{
	gchar				ams_id[AMS_ID_LEN];
	gint					req_num;
	gint					start_num;
};


typedef struct _NmpAmsDeviceInfo NmpAmsDeviceInfo;
struct _NmpAmsDeviceInfo
{
	gchar				puid[MAX_ID_LEN];
	gchar				domain[DOMAIN_ID_LEN];
	gchar				dev_name[AMS_DEV_NAME_LEN];
	gchar				dev_passwd[AMS_DEV_PASSWD_LEN];
	gchar				dev_ip[MAX_IP_LEN];
	gint					dev_port;
};

typedef struct _NmpAmsGetDeviceInfoRes NmpAmsGetDeviceInfoRes;
struct _NmpAmsGetDeviceInfoRes
{
	NmpMsgErrCode		code;
	gchar				ams_id[AMS_ID_LEN];
	gint					total_count;
	gint					back_count;
	NmpAmsDeviceInfo		dev_info[0];
};


#endif //__NMP_MOD_AMS_MESSAGES_EXTERNAL_H__
