#ifndef __NMP_MOD_AMS_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_AMS_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"
#include "nmp_share_struct.h"


typedef struct _JpfAmsRegister JpfAmsRegister;
struct _JpfAmsRegister
{
    gchar ams_id[AMS_ID_LEN];
    gchar  ams_version[VERSION_LEN];
};

typedef struct _JpfAmsRegisterRes JpfAmsRegisterRes;
struct _JpfAmsRegisterRes
{
    JpfMsgErrCode       code;
    gchar                    domain_id[DOMAIN_ID_LEN];
    gint                       keep_alive_time;
    gchar			server_time[TIME_INFO_LEN];
};

typedef struct _JpfAmsHeart JpfAmsHeart;
struct _JpfAmsHeart
{
    gchar		ams_id[AMS_ID_LEN];
};

typedef struct _JpfAmsHeartRes JpfAmsHeartRes;
struct _JpfAmsHeartRes
{
    JpfMsgErrCode	code;
    gchar			server_time[TIME_INFO_LEN];
};


typedef struct _JpfActionPolicy JpfActionPolicy;
struct _JpfActionPolicy
{
	int			weekday_num;
	JpfWeekday	weekdays[WEEKDAYS];
};


typedef struct _JpfAmsActionRecord JpfAmsActionRecord;
struct _JpfAmsActionRecord
{
	JpfShareGuid	action_guid;
	gint			level;
	JpfShareMssId	mss_id;
	guint		time_len;
	guint		alarm_type;
};


typedef struct _JpfAmsActionIO JpfAmsActionIO;
struct _JpfAmsActionIO
{
	JpfShareGuid	action_guid;
	guint		time_len;
	gchar		io_value[IO_VALUE_LEN];
	gchar		session[SESSION_ID_LEN];
};

typedef struct _JpfAmsActionIORes JpfAmsActionIORes;
struct _JpfAmsActionIORes
{
    JpfMsgErrCode      code;
    gchar		              session[SESSION_ID_LEN];
    gchar                   domain_id[DOMAIN_ID_LEN];
    gchar                   guid[MAX_ID_LEN];
};


typedef struct _JpfAmsActionPreset JpfAmsActionPreset;
struct _JpfAmsActionPreset
{
	JpfShareGuid	action_guid;
	gint			preset_num;
	gchar		session[SESSION_ID_LEN];
};


typedef struct _JpfAmsActionSnapshot JpfAmsActionSnapshot;
struct _JpfAmsActionSnapshot
{
	JpfShareGuid	action_guid;
	gint			level;
	JpfShareMssId	mss_id;
	guint		picture_count;
	guint		alarm_type;
};


typedef struct _JpfAmsActionMapGu JpfAmsActionMapGu;
struct _JpfAmsActionMapGu
{
	JpfShareGuid	action_guid;
	gchar			gu_name[GU_NAME_LEN];
	gint		map_id;						//id
	gchar		map_name[MAP_NAME_LEN];
	gint		level;
};

typedef struct _JpfAmsActionMap JpfAmsActionMap;
struct _JpfAmsActionMap
{
	JpfShareGuid	action_guid;
	gint			alarm_type;
	gchar			alarm_time[TIME_INFO_LEN];
	gchar			gu_name[GU_NAME_LEN];
	gint			defence_id;
	gchar			defence_name[AREA_NAME_LEN];
	gint			map_id;
	gchar			map_name[MAP_NAME_LEN];
	guint			action_gu_count;
	JpfAmsActionMapGu	action_gu[AMS_MAX_LINK_MAP_GU];
	gint				cu_count;
	JpfAllCuOwnPu		cu_list[0];
};


typedef struct _JpfAmsGetDeviceInfo JpfAmsGetDeviceInfo;
struct _JpfAmsGetDeviceInfo
{
	gchar				ams_id[AMS_ID_LEN];
	gint					req_num;
	gint					start_num;
};


typedef struct _JpfAmsDeviceInfo JpfAmsDeviceInfo;
struct _JpfAmsDeviceInfo
{
	gchar				puid[MAX_ID_LEN];
	gchar				domain[DOMAIN_ID_LEN];
	gchar				dev_name[AMS_DEV_NAME_LEN];
	gchar				dev_passwd[AMS_DEV_PASSWD_LEN];
	gchar				dev_ip[MAX_IP_LEN];
	gint					dev_port;
};

typedef struct _JpfAmsGetDeviceInfoRes JpfAmsGetDeviceInfoRes;
struct _JpfAmsGetDeviceInfoRes
{
	JpfMsgErrCode		code;
	gchar				ams_id[AMS_ID_LEN];
	gint					total_count;
	gint					back_count;
	JpfAmsDeviceInfo		dev_info[0];
};


#endif //__NMP_MOD_AMS_MESSAGES_EXTERNAL_H__
