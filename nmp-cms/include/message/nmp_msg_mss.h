#ifndef __NMP_MOD_MSS_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_MSS_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"


typedef struct _JpfMssRegister JpfMssRegister;
struct _JpfMssRegister
{
    gchar mss_id[MSS_ID_LEN];
    gchar  mss_version[VERSION_LEN];
};

typedef struct _JpfMssRegisterRes JpfMssRegisterRes;
struct _JpfMssRegisterRes
{
    JpfMsgErrCode       code;
    gchar                    domain_id[DOMAIN_ID_LEN];
    gint                       keep_alive_time;
    gint                       storage_type;
    gint                       mode;
    gchar			server_time[TIME_INFO_LEN];
    gchar              mss_name[MSS_NAME_LEN];
};

typedef struct _JpfMssHeart JpfMssHeart;
struct _JpfMssHeart
{
    gchar		mss_id[MSS_ID_LEN];
};

typedef struct _JpfMssHeartRes JpfMssHeartRes;
struct _JpfMssHeartRes
{
    JpfMsgErrCode	code;
    gchar			server_time[TIME_INFO_LEN];
};

typedef struct _JpfMssGetGuid JpfMssGetGuid;
struct _JpfMssGetGuid
{
    gchar		mss_id[MSS_ID_LEN];
    gint        req_num;
    gint        start_row;
};

typedef struct _JpfMssGuid JpfMssGuid;
struct _JpfMssGuid
{
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
};

typedef struct _JpfMssGetGuidRes JpfMssGetGuidRes;
struct _JpfMssGetGuidRes
{
    JpfMsgErrCode	code;
    gchar		   mss_id[MSS_ID_LEN];
    gint            total_count;
    gint            back_count;
    JpfMssGuid guid_info[0];
};

typedef struct _JpfMssGetRecordPolicy JpfMssGetRecordPolicy;
struct _JpfMssGetRecordPolicy
{
    gchar		   mss_id[MSS_ID_LEN];
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
};

typedef struct _JpfMssHdGroup JpfMssHdGroup;
struct _JpfMssHdGroup
{
    gchar              hd_group_id[HD_GROUP_ID_LEN];
};

typedef struct _JpfMssGetRecordPolicyRes JpfMssGetRecordPolicyRes;
struct _JpfMssGetRecordPolicyRes
{
    JpfMsgErrCode	code;
    gchar		    mss_id[MSS_ID_LEN];
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gint           level;
    JpfMssHdGroup		hd_group[HD_GROUP_NUM];
    gchar         policy[POLICY_LEN];
};

typedef struct _JpfMssGetRoute JpfMssGetRoute;
struct _JpfMssGetRoute
{
    gchar		   mss_id[MSS_ID_LEN];
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar         cms_ip[MAX_IP_LEN];
};

typedef struct _JpfMssGetRouteRes JpfMssGetRouteRes;
struct _JpfMssGetRouteRes
{
    JpfMsgErrCode	code;
    gchar		    mss_id[MSS_ID_LEN];
	 gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar		    url[MAX_URL_LEN];
};

typedef struct _JpfNotifyPolicyChange JpfNotifyPolicyChange;
struct _JpfNotifyPolicyChange
{
    gchar         guid[MAX_ID_LEN];
    gchar         domain_id[DOMAIN_ID_LEN];
    gchar		    mss_id[MSS_ID_LEN];
    gint           all_changed;   //0:单个配置，1:修改该存储服务器所有的业务点策略
};

typedef struct _JpfMssGetMds JpfMssGetMds;
struct _JpfMssGetMds
{
    gchar		mss_id[MSS_ID_LEN];
    gint           req_num;
    gint           start_row;
};

typedef struct _JpfMssMds JpfMssMds;
struct _JpfMssMds
{
    gchar		mds_id[MDS_ID_LEN];
};

typedef struct _JpfMssGetMdsRes JpfMssGetMdsRes;
struct _JpfMssGetMdsRes
{
    JpfMsgErrCode	code;
    gchar		mss_id[MSS_ID_LEN];
    gint            total_count;
    gint            back_count;
    JpfMssMds  mds_info[0];
};

typedef struct _JpfMssGetMdsIp JpfMssGetMdsIp;
struct _JpfMssGetMdsIp
{
    gchar		mss_id[MSS_ID_LEN];
    gchar		mds_id[MDS_ID_LEN];
    gchar         cms_ip[MAX_IP_LEN];
};

typedef struct _JpfMssGetMdsIpRes JpfMssGetMdsIpRes;
struct _JpfMssGetMdsIpRes
{
    JpfMsgErrCode	code;
    gchar		       mss_id[MSS_ID_LEN];
    gchar		       mds_id[MDS_ID_LEN];
    gchar                mds_ip[MAX_IP_LEN];
    gint                   port;
};


#endif //__NMP_MOD_MSS_MESSAGES_EXTERNAL_H__



