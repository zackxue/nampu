#ifndef __NMP_MOD_MSS_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_MSS_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"


typedef struct _NmpMssRegister NmpMssRegister;
struct _NmpMssRegister
{
    gchar mss_id[MSS_ID_LEN];
    gchar  mss_version[VERSION_LEN];
};

typedef struct _NmpMssRegisterRes NmpMssRegisterRes;
struct _NmpMssRegisterRes
{
    NmpMsgErrCode       code;
    gchar                    domain_id[DOMAIN_ID_LEN];
    gint                       keep_alive_time;
    gint                       storage_type;
    gint                       mode;
    gchar			server_time[TIME_INFO_LEN];
    gchar              mss_name[MSS_NAME_LEN];
};

typedef struct _NmpMssHeart NmpMssHeart;
struct _NmpMssHeart
{
    gchar		mss_id[MSS_ID_LEN];
};

typedef struct _NmpMssHeartRes NmpMssHeartRes;
struct _NmpMssHeartRes
{
    NmpMsgErrCode	code;
    gchar			server_time[TIME_INFO_LEN];
};

typedef struct _NmpMssGetGuid NmpMssGetGuid;
struct _NmpMssGetGuid
{
    gchar		mss_id[MSS_ID_LEN];
    gint        req_num;
    gint        start_row;
};

typedef struct _NmpMssGuid NmpMssGuid;
struct _NmpMssGuid
{
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
};

typedef struct _NmpMssGetGuidRes NmpMssGetGuidRes;
struct _NmpMssGetGuidRes
{
    NmpMsgErrCode	code;
    gchar		   mss_id[MSS_ID_LEN];
    gint            total_count;
    gint            back_count;
    NmpMssGuid guid_info[0];
};

typedef struct _NmpMssGetRecordPolicy NmpMssGetRecordPolicy;
struct _NmpMssGetRecordPolicy
{
    gchar		   mss_id[MSS_ID_LEN];
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
};

typedef struct _NmpMssHdGroup NmpMssHdGroup;
struct _NmpMssHdGroup
{
    gchar              hd_group_id[HD_GROUP_ID_LEN];
};

typedef struct _NmpMssGetRecordPolicyRes NmpMssGetRecordPolicyRes;
struct _NmpMssGetRecordPolicyRes
{
    NmpMsgErrCode	code;
    gchar		    mss_id[MSS_ID_LEN];
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gint           level;
    NmpMssHdGroup		hd_group[HD_GROUP_NUM];
    gchar         policy[POLICY_LEN];
};

typedef struct _NmpMssGetRoute NmpMssGetRoute;
struct _NmpMssGetRoute
{
    gchar		   mss_id[MSS_ID_LEN];
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar         cms_ip[MAX_IP_LEN];
};

typedef struct _NmpMssGetRouteRes NmpMssGetRouteRes;
struct _NmpMssGetRouteRes
{
    NmpMsgErrCode	code;
    gchar		    mss_id[MSS_ID_LEN];
	 gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar		    url[MAX_URL_LEN];
};

typedef struct _NmpNotifyPolicyChange NmpNotifyPolicyChange;
struct _NmpNotifyPolicyChange
{
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar		    mss_id[MSS_ID_LEN];
    gint           all_changed;   //0:单个配置，1:修改该存储服务器所有的业务点策略
};

typedef struct _NmpMssGetMds NmpMssGetMds;
struct _NmpMssGetMds
{
    gchar		mss_id[MSS_ID_LEN];
    gint           req_num;
    gint           start_row;
};

typedef struct _NmpMssMds NmpMssMds;
struct _NmpMssMds
{
    gchar		mds_id[MDS_ID_LEN];
};

typedef struct _NmpMssGetMdsRes NmpMssGetMdsRes;
struct _NmpMssGetMdsRes
{
    NmpMsgErrCode	code;
    gchar		mss_id[MSS_ID_LEN];
    gint            total_count;
    gint            back_count;
    NmpMssMds  mds_info[0];
};

typedef struct _NmpMssGetMdsIp NmpMssGetMdsIp;
struct _NmpMssGetMdsIp
{
    gchar		mss_id[MSS_ID_LEN];
    gchar		mds_id[MDS_ID_LEN];
    gchar         cms_ip[MAX_IP_LEN];
};

typedef struct _NmpMssGetMdsIpRes NmpMssGetMdsIpRes;
struct _NmpMssGetMdsIpRes
{
    NmpMsgErrCode	code;
    gchar		       mss_id[MSS_ID_LEN];
    gchar		       mds_id[MDS_ID_LEN];
    gchar                mds_ip[MAX_IP_LEN];
    gint                   port;
};


#endif //__NMP_MOD_MSS_MESSAGES_EXTERNAL_H__



