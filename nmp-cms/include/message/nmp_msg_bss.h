#ifndef __NMP_MOD_BSS_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_BSS_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"
#include "search_device.h"


#define ONLINE_RATE_FLAG 1

typedef struct _JpfModifyAdmin             JpfModifyAdmin;
typedef struct _JpfModifyAdminResp         JpfModifyAdminResp;

typedef struct _JpfBssRes JpfBssRes;
struct _JpfBssRes
{
    JpfMsgErrCode     code;
    gchar			  bss_usr[USER_NAME_LEN];
};

typedef struct _JpfBssLoginInfo JpfBssLoginInfo;
struct _JpfBssLoginInfo
{
	gchar admin_name[USER_NAME_LEN];
	gchar password[USER_PASSWD_LEN];
};

typedef struct _JpfBssLoginRes JpfBssLoginRes;
struct _JpfBssLoginRes
{
    JpfMsgErrCode       code;
    gchar       admin_name[USER_NAME_LEN];
    gchar       domain_name[DOMAIN_NAME_LEN];
    gchar       domain_id[DOMAIN_ID_LEN];
    gint         module_sets;
};

typedef struct _JpfBssHeart JpfBssHeart;
struct _JpfBssHeart
{
    gchar			 admin_name[USER_NAME_LEN];;
};

typedef struct _JpfBssHeartResp JpfBssHeartResp;
struct _JpfBssHeartResp
{
    JpfMsgErrCode	code;
    gchar			server_time[TIME_INFO_LEN];
};

typedef struct _JpfPlatformUpgrade JpfPlatformUpgrade;
struct _JpfPlatformUpgrade
{
    gchar			 admin_name[USER_NAME_LEN];;
};

typedef struct _JpfPlatformUpgradeResp JpfPlatformUpgradeResp;
struct _JpfPlatformUpgradeResp
{
    JpfMsgErrCode	code;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAdminInfo JpfAdminInfo;
struct _JpfAdminInfo
{
	gchar admin_name[USER_NAME_LEN];
	gchar password[USER_PASSWD_LEN];
};

typedef struct _JpfAddAdmin JpfAddAdmin;
struct _JpfAddAdmin
{
	gchar admin_name[USER_NAME_LEN];
	gchar password[USER_PASSWD_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfDelAdmin JpfDelAdmin;
struct _JpfDelAdmin
{
	gchar admin_name[MULTI_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryAdmin JpfQueryAdmin;
struct _JpfQueryAdmin
{
	gint              req_num;
	gint              start_num;
	gint               type;   //0:按用户名查询，1:按类型查询
	gchar            key[USER_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryAdminRes JpfQueryAdminRes;
struct _JpfQueryAdminRes
{
	JpfMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint                     total_num;
	gint                     req_num;
	JpfAdminInfo       admin_info[0];
};

typedef struct _JpfAddUserGroup  JpfAddUserGroup;
struct _JpfAddUserGroup
{
	gint group_permissions;
	gint  group_rank;
	gchar group_name[GROUP_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfValidateUserGroup  JpfValidateUserGroup;
struct _JpfValidateUserGroup
{
	gchar group_name[GROUP_NAME_LEN];
	gint group_id;
};


typedef struct _JpfDelUserGroup JpfDelUserGroup;
struct _JpfDelUserGroup
{
	gchar group_id[MULTI_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfUserGroupInfo  JpfUserGroupInfo;
struct _JpfUserGroupInfo
{
	gint group_id;
	gint group_permissions;
	gint  group_rank;
	gchar group_name[GROUP_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryUserGroup JpfQueryUserGroup;
struct _JpfQueryUserGroup
{
	gint              req_num;
	gint              start_num;
	gint              type;   //0:按用户名查询，1:按类型查询
	gchar            key[USER_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryUserGroupRes JpfQueryUserGroupRes;
struct _JpfQueryUserGroupRes
{
	JpfMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint                     total_num;
	gint                     req_num;
	JpfUserGroupInfo       group_info[0];
};

typedef struct _JpfAddUser JpfAddUser;
struct _JpfAddUser
{
	gchar                username[USER_NAME_LEN];
	gchar                password[USER_PASSWD_LEN];
	gint                   group_id;
	gint                   sex;
	gchar                user_phone[PHONE_NUM_LEN];
	gchar                user_description[DESCRIPTION_INFO_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfValidateUser  JpfValidateUser;
struct _JpfValidateUser
{
    gchar username[USER_NAME_LEN];

};

typedef struct _JpfDelUser JpfDelUser;
struct _JpfDelUser
{
    gchar username[MULTI_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfForceUserOffline JpfForceUserOffline;
struct _JpfForceUserOffline
{
    gint reason;
};


typedef struct _JpfUserInfo  JpfUserInfo;
struct _JpfUserInfo
{
    gchar                username[USER_NAME_LEN];
    gchar                password[USER_PASSWD_LEN];
    gchar                group_name[GROUP_NAME_LEN];
    gint                   group_id;
    gint                   sex;
    gint                   user_id;
    gchar                user_phone[PHONE_NUM_LEN];
    gchar                user_description[DESCRIPTION_INFO_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryUser JpfQueryUser;
struct _JpfQueryUser
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryUserRes JpfQueryUserRes;
struct _JpfQueryUserRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    JpfUserInfo          user_info[0];
};

typedef struct _JpfAddDomain  JpfAddDomain;
struct _JpfAddDomain
{
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             domain_name[DOMAIN_NAME_LEN];
    gchar             domain_ip[MAX_IP_LEN];
    gint                domain_port;
    gint                domain_type;
};


typedef struct _JpfDomainInfo  JpfDomainInfo;
struct _JpfDomainInfo
{
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             domain_name[DOMAIN_NAME_LEN];
    gchar             domain_ip[MAX_IP_LEN];
    gint                domain_port;
    gint                domain_type;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfDelDomain JpfDelDomain;
struct _JpfDelDomain
{
    gchar             domain_id[MULTI_NAME_LEN];
};

typedef struct _JpfQueryDomain JpfQueryDomain;
struct _JpfQueryDomain
{
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryDomainRes JpfQueryDomainRes;
struct _JpfQueryDomainRes
{
    JpfMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    JpfDomainInfo           domain_info[0];
};

typedef struct _JpfModifyDomain   JpfModifyDomain;
struct _JpfModifyDomain
{
    gchar		dm_id[DOMAIN_ID_LEN];
    gchar		dm_name[DOMAIN_NAME_LEN];
};

typedef struct _JpfAddArea  JpfAddArea;
struct _JpfAddArea
{
    gchar             area_name[AREA_NAME_LEN];
    gint                area_parent;
};

typedef struct _JpfAddAreaRes JpfAddAreaRes;
struct _JpfAddAreaRes
{
    JpfMsgErrCode      code;
    gint                area_id;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfAreaInfo  JpfAreaInfo;
struct _JpfAreaInfo
{
    gint                area_id;
    gchar             area_name[AREA_NAME_LEN];
    gint                area_parent;
    gchar             user_name[USER_NAME_LEN];
    gchar             user_phone[PHONE_NUM_LEN];
    gchar             user_address[DESCRIPTION_INFO_LEN];
    gchar             description[DESCRIPTION_INFO_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryArea JpfQueryArea;
struct _JpfQueryArea
{
    gint               req_num;
    gint               start_num;
    gint               area_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryAreaRes JpfQueryAreaRes;
struct _JpfQueryAreaRes
{
    JpfMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      req_num;
    JpfAreaInfo           area_info[0];
};

typedef struct _JpfDelArea JpfDelArea;
struct _JpfDelArea
{
    gint                area_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfValidateArea  JpfValidateArea;
struct _JpfValidateArea
{
    gchar             area_name[AREA_NAME_LEN];
};

typedef struct _JpfAddPu JpfAddPu;
struct _JpfAddPu
{
    gchar             puid[MAX_ID_LEN];
    gchar             pu_info[PU_NAME_LEN];
    gint              pu_type;
    gint              pu_minor_type;
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             mds_id[MDS_ID_LEN];
    gchar             manufacturer[MF_ID_LEN];
    gint                area_id;
    gint                keep_alive_time;
    gchar             mss_id[MSS_ID_LEN];
    gint                pu_count;
    gint                av_count;
    gint                ai_count;
    gint                ao_count;
    gint                ds_count;
    gchar             ivs_id[IVS_ID_LEN];
    gchar             ams_id[AMS_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAddPuRes JpfAddPuRes;
struct _JpfAddPuRes
{
    JpfMsgErrCode     code;
    gchar			  bss_usr[USER_NAME_LEN];
    gint                  success_count;
};

typedef struct _JpfPuInfo JpfPuInfo;
struct _JpfPuInfo
{
    gchar             puid[MAX_ID_LEN];
    gchar             pu_info[PU_NAME_LEN];
    gint              pu_type;
    gint              pu_minor_type;
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             mds_id[MDS_ID_LEN];
    gchar             mds_name[MDS_NAME_LEN];
    gchar             ams_id[AMS_ID_LEN];
    gchar             mf_id[MF_ID_LEN];
    gchar             mf_name[MF_NAME_LEN];
    gchar             area_name[AREA_NAME_LEN];
    gint               area_id;
    gint               keep_alive_time;
    gint               pu_state;
    gchar             pu_last_alive[TIME_LEN];
    gchar             pu_last_ip[MAX_IP_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryPu JpfQueryPu;
struct _JpfQueryPu
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar             key[MAX_ID_LEN];
    gchar            domain_id[DOMAIN_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryPuRes JpfQueryPuRes;
struct _JpfQueryPuRes
{
    JpfMsgErrCode  code;
    gchar bss_usr[USER_NAME_LEN];
    gint                  total_num;
    gint                  req_num;
    gint                  type;
    JpfPuInfo          pu_info[0];
};

typedef struct _JpfMovePu JpfMovePu;
struct _JpfMovePu
{
    gchar             puid[MAX_ID_LEN];
    gchar             pu_info[PU_NAME_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    gint                area_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfPuPoint JpfPuPoint;
struct _JpfPuPoint
{
    gchar               domain_id[DOMAIN_ID_LEN];
    gchar               puid[MAX_ID_LEN];
};

typedef struct _JpfDelPu JpfDelPu;
struct _JpfDelPu
{
    gint       count;
    gint       type;
    gchar    key[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
    JpfPuPoint pu_list[0];
};

typedef struct _JpfValidatePu  JpfValidatePu;
struct _JpfValidatePu
{
    gchar              puid[MAX_ID_LEN];
};

typedef struct _JpfStoreServer JpfStoreServer;
struct _JpfStoreServer
{
    gchar              mss_id[MSS_ID_LEN];
    gchar              mss_name[MSS_NAME_LEN];
};

typedef struct _JpfAddGu JpfAddGu;
struct _JpfAddGu
{
    gchar             puid[MAX_ID_LEN];
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             gu_name[GU_NAME_LEN];
    gint                gu_type;
    gint                gu_attributes;
    gchar             ivs_id[IVS_ID_LEN];
    JpfStoreServer            mss[MSS_NUM];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfGuInfo JpfGuInfo;
struct _JpfGuInfo
{
    gchar            puid[MAX_ID_LEN];
    gchar            guid[MAX_ID_LEN];
    gchar            domain_id[DOMAIN_ID_LEN];
    gchar            pu_name[PU_NAME_LEN];
    gchar            gu_name[GU_NAME_LEN];
    gint               gu_type;
    gint               gu_attributes;
    gchar            pu_ip[MAX_IP_LEN];
    gint               pu_state;
    gint             pu_minor_type;
    gchar            area_name[AREA_NAME_LEN];
    gchar             ivs_id[IVS_ID_LEN];
    gchar             ivs_name[IVS_NAME_LEN];
    gchar             ams_name[AMS_NAME_LEN];
    JpfStoreServer            mss[MSS_NUM];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryGu JpfQueryGu;
struct _JpfQueryGu
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar puid[MAX_ID_LEN];
    gchar guid[MAX_ID_LEN];
    gchar domain_id[DOMAIN_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryGuRes JpfQueryGuRes;
struct _JpfQueryGuRes
{
    JpfMsgErrCode  code;
    gchar bss_usr[USER_NAME_LEN];
    gint        total_num;
    gint req_num;
    gint               type;
    JpfGuInfo    gu_info[0];
};

typedef struct _JpfDelGu JpfDelGu;
struct _JpfDelGu
{
    gchar domain_id[DOMAIN_ID_LEN];
    gchar  guid[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAddMds JpfAddMds;
struct _JpfAddMds
{
    gchar   mds_id[MDS_ID_LEN];
    gchar   mds_name[MDS_NAME_LEN];
    gint      mds_type;
    gint      keep_alive_freq;
    gint      mds_pu_port;
    gint      mds_rtsp_port;
    gint      get_ip_enable;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfMdsInfo  JpfMdsInfo;
struct _JpfMdsInfo
{
    gint     type;  // 0: 修改mds ; 1:修改get_ip_enable
    gchar   mds_id[MDS_ID_LEN];
    gchar   mds_name[MDS_NAME_LEN];
    gint      mds_type;
    gint      keep_alive_freq;
    gint      mds_pu_port;
    gint      mds_rtsp_port;
    gint      mds_state;
    gint      get_ip_enable;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryMds JpfQueryMds;
struct _JpfQueryMds
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询, 2:查询自动添加mds ip使能状态
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryMdsRes JpfQueryMdsRes;
struct _JpfQueryMdsRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint               type;
    gint                     total_num;
    gint                     req_num;
    JpfMdsInfo          mds_info[0];
};


typedef struct _JpfDelMds JpfDelMds;
struct _JpfDelMds
{
    gchar  mds_id[MULTI_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfAddMdsIp JpfAddMdsIp;
struct _JpfAddMdsIp
{
    gchar   mds_id[MDS_ID_LEN];
    gchar   cms_ip[MAX_IP_LEN];
    gchar   mds_ip[MAX_IP_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfMdsIpInfo  JpfMdsIpInfo;
struct _JpfMdsIpInfo
{
   //gchar   mds_id[MDS_ID_LEN];
    gchar   cms_ip[MAX_IP_LEN];
    gchar   mds_ip[MAX_IP_LEN];
};

typedef struct _JpfQueryMdsIp JpfQueryMdsIp;
struct _JpfQueryMdsIp
{
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryMdsIpRes JpfQueryMdsIpRes;
struct _JpfQueryMdsIpRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gchar   mds_id[MDS_ID_LEN];
    JpfMdsIpInfo          mds_ip_info[0];
};


typedef struct _JpfDelMdsIp JpfDelMdsIp;
struct _JpfDelMdsIp
{
    gchar  mds_id[MDS_ID_LEN];
    gchar   cms_ip[MAX_IP_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfAddMss JpfAddMss;
struct _JpfAddMss
{
    gchar   mss_id[MSS_ID_LEN];
    gchar   mss_name[MSS_NAME_LEN];
    gint      keep_alive_freq;
    gint      storage_type;  //磁盘类型：0：磁盘阵列，1：本地硬盘
    gint      mode;  //运行模式：0：录像模式，1：配置模式
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfMssInfo  JpfMssInfo;
struct _JpfMssInfo
{
    gchar   mss_id[MSS_ID_LEN];
    gchar   mss_name[MSS_NAME_LEN];
    gint      keep_alive_freq;
    gint      mss_state;
    gint      storage_type;  //磁盘类型：0：磁盘阵列，1：本地硬盘
    gint      mode;  //运行模式：0：录像模式，1：配置模式
    gchar   mss_last_ip[MAX_IP_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryMss JpfQueryMss;
struct _JpfQueryMss
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryMssRes JpfQueryMssRes;
struct _JpfQueryMssRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;

    JpfMssInfo          mss_info[0];
};


typedef struct _JpfDelMss JpfDelMss;
struct _JpfDelMss
{
    gchar  mss_id[MULTI_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfRecordGu  JpfRecordGu;
struct _JpfRecordGu
{
    gchar guid[MAX_ID_LEN];
    gchar domain_id[DOMAIN_ID_LEN];
    gchar puid[MAX_ID_LEN];
    gchar pu_name[PU_NAME_LEN];
    gchar gu_name[GU_NAME_LEN];
    gint   level;
    gchar mss_id[MSS_ID_LEN];
    gchar mss_name[MSS_NAME_LEN];
    gchar area_name[AREA_NAME_LEN];
};

typedef struct _JpfHdGroup JpfHdGroup;
struct _JpfHdGroup
{
    gchar              hd_group_id[HD_GROUP_ID_LEN];
};

typedef struct _JpfQueryRecordPolicy JpfQueryRecordPolicy;
struct _JpfQueryRecordPolicy
{
    gchar guid[MAX_ID_LEN];
    gchar domain_id[DOMAIN_ID_LEN];
    gchar  mss_id[MSS_ID_LEN];
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar          bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryRecordGuRes JpfQueryRecordGuRes;
struct _JpfQueryRecordGuRes
{
    JpfMsgErrCode     code;
    gchar                  guid[MAX_ID_LEN];
    gchar                  domain_id[DOMAIN_ID_LEN];
    gchar                  mss_id[MSS_ID_LEN];
    JpfHdGroup		     hd_group[HD_GROUP_NUM];
    gchar                  time_policy[POLICY_LEN];
};

typedef struct _JpfQueryRecordPolicyRes JpfQueryRecordPolicyRes;
struct _JpfQueryRecordPolicyRes
{
    JpfMsgErrCode      code;
    gchar          bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      req_num;
    gint                      type;   //0:按用户名查询，1:按类型查询	    JpfHdGroup		       hd_group[HD_GROUP_NUM];
    JpfHdGroup		       hd_group[HD_GROUP_NUM];
    gchar                    time_policy[POLICY_LEN];

    JpfRecordGu          record_policy[0];
};

typedef struct _JpfRecordPolicyConfig JpfRecordPolicyConfig;
struct _JpfRecordPolicyConfig
{
    gint                      type;
    gint                      level;
    gchar                    mss_id[MSS_ID_LEN];
    JpfHdGroup		        hd_group[HD_GROUP_NUM];
    gchar                    time_policy[POLICY_LEN];
    gchar bss_usr[USER_NAME_LEN];
    gint                       gu_count;
    JpfRecordGu          record_policy[0];
};

typedef struct _JpfRecordPolicyConfigRes JpfRecordPolicyConfigRes;
struct _JpfRecordPolicyConfigRes
{
    gint                      type;
    gchar                    mss_id[MSS_ID_LEN];
    gchar		bss_usr[USER_NAME_LEN];
    gint                       gu_count;
    JpfRecordGu          record_policy[0];
};

typedef struct _JpfAddModifyManufacturer  JpfAddModifyManufacturer;
struct _JpfAddModifyManufacturer
{
    gint type;
    gchar   mf_id[MF_ID_LEN];
    gchar  mf_name[MF_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfManufacturerInfo  JpfManufacturerInfo;
struct _JpfManufacturerInfo
{
    gchar   mf_id[MF_ID_LEN];
    gchar  mf_name[MF_NAME_LEN];
};

typedef struct _JpfQueryManufacturer JpfQueryManufacturer;
struct _JpfQueryManufacturer
{
    gint              req_num;
    gint              start_num;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryManufacturerRes JpfQueryManufacturerRes;
struct _JpfQueryManufacturerRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    JpfManufacturerInfo          manufacturer_info[0];
};

typedef struct _JpfDelManufacturer JpfDelManufacturer;
struct _JpfDelManufacturer
{
    gchar  mf_id[MF_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfGuToUserInfo JpfGuToUserInfo;
struct _JpfGuToUserInfo
{
    //gchar   username[USER_NAME_LEN];
    gchar   user_guid[MAX_ID_LEN];
    gchar   user_guid_domain[DOMAIN_ID_LEN];

};

typedef struct _JpfAddGuToUser JpfAddGuToUser;
struct _JpfAddGuToUser
{
    //struct  list_head list;
    gint total_num;
    gchar   username[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
    JpfGuToUserInfo gu_to_user_info[0];
};


typedef struct _JpfUserOwnGu  JpfUserOwnGu;
struct _JpfUserOwnGu
{
    gchar   user_guid[MAX_ID_LEN];
    gchar   guid_name[GU_NAME_LEN];
};

typedef struct _JpfQueryUserOwnGu JpfQueryUserOwnGu;
struct _JpfQueryUserOwnGu
{
    gint              req_num;
    gint              start_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryUserOwnGuRes JpfQueryUserOwnGuRes;
struct _JpfQueryUserOwnGuRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];

    JpfUserOwnGu      user_own_gu_info[0];
};

typedef struct _JpfDelGuFromUser JpfDelGuFromUser;
struct _JpfDelGuFromUser
{
    gint total_num;
    gchar   username[USER_NAME_LEN];
    JpfGuToUserInfo gu_to_user_info[0];
};

typedef struct _JpfTwToUserInfo JpfTwToUserInfo;
struct _JpfTwToUserInfo
{
    gint   tw_id;
};

typedef struct _JpfAddTwToUser JpfAddTwToUser;
struct _JpfAddTwToUser
{
    //struct  list_head list;
    gint total_num;
    gchar   username[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
    JpfTwToUserInfo tw_to_user_info[0];
};


typedef struct _JpfUserOwnTw  JpfUserOwnTw;
struct _JpfUserOwnTw
{
    gint   tw_id;
    gchar  tw_name[TW_NAME_LEN];
};

typedef struct _JpfQueryUserOwnTw JpfQueryUserOwnTw;
struct _JpfQueryUserOwnTw
{
    gint              req_num;
    gint              start_num;
    gchar user[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryUserOwnTwRes JpfQueryUserOwnTwRes;
struct _JpfQueryUserOwnTwRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];

    JpfUserOwnTw      user_own_tw_info[0];
};

typedef struct _JpfTourToUserInfo JpfTourToUserInfo;
struct _JpfTourToUserInfo
{
    gint   tour_id;
};

typedef struct _JpfAddTourToUser JpfAddTourToUser;
struct _JpfAddTourToUser
{
    gint total_num;
    gchar   username[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
    JpfTourToUserInfo tour_to_user_info[0];
};

typedef struct _JpfUserOwnTour  JpfUserOwnTour;
struct _JpfUserOwnTour
{
    gint   tour_id;
    gchar  tour_name[TOUR_NAME_LEN];
};

typedef struct _JpfQueryUserOwnTour JpfQueryUserOwnTour;
struct _JpfQueryUserOwnTour
{
    gint              req_num;
    gint              start_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryUserOwnTourRes JpfQueryUserOwnTourRes;
struct _JpfQueryUserOwnTourRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];

    JpfUserOwnTour      user_own_tour_info[0];
};

typedef struct _JpfSetServerTime JpfSetServerTime;
struct _JpfSetServerTime
{
    gchar time_zone[TIME_ZONE_LEN];
    gchar system_time[TIME_LEN];
};

typedef struct _JpfSetServerTimeRes JpfSetServerTimeRes;
struct _JpfSetServerTimeRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gchar time_zone[TIME_ZONE_LEN];
    gchar system_time[TIME_LEN];
};

typedef struct  _JpfQueryServerTime JpfQueryServerTime;
struct _JpfQueryServerTime
{
    gchar time_zone[TIME_ZONE_LEN];
};

typedef struct  _JpfQueryServerTimeRes JpfQueryServerTimeRes;
struct _JpfQueryServerTimeRes
{
    JpfMsgErrCode     code;
    //gchar time_zone[TIME_ZONE_LEN];
    gchar system_time[TIME_LEN];
};


typedef struct  _JpfDbBackup JpfDbBackup;
struct _JpfDbBackup
{
    gchar filename[FILENAME_LEN];
    gchar	bss_usr[USER_NAME_LEN];
};

typedef struct  _JpfDbImport JpfDbImport;
struct _JpfDbImport
{
    gchar filename[FILENAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct  _JpfDbImportRes JpfDbImportRes;
struct _JpfDbImportRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gchar error_desc[DESCRIPTION_INFO_LEN];
};

typedef struct _JpfErrRes JpfErrRes;
struct _JpfErrRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _JpfSetResult JpfSetResult;
struct _JpfSetResult
{
    JpfMsgErrCode	code;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfGetNetInterfaceConfigRes JpfGetNetInterfaceConfigRes;
struct _JpfGetNetInterfaceConfigRes
{
    JpfMsgErrCode      code;
    gchar                   network_interface[NET_INTERFACE_LEN];
};


typedef struct  _JpfGetNetworkConfig JpfGetNetworkConfig;
struct _JpfGetNetworkConfig
{
    gchar                   network_interface[NET_INTERFACE_LEN];
};

typedef struct
{
	gchar				ip[MAX_IP_LEN];
	gchar				netmask[MAX_IP_LEN];
	gchar				gateway[MAX_IP_LEN];
} IpInfo;

typedef struct _JpfGetNetworkConfigRes JpfGetNetworkConfigRes;
struct _JpfGetNetworkConfigRes
{
	JpfMsgErrCode		code;
	gchar				dns[MAX_IP_LEN];
	int					count;
	IpInfo				ip_list[0];
};

typedef struct _JpfSetNetworkConfig JpfSetNetworkConfig;
struct _JpfSetNetworkConfig
{
	gchar				network_interface[NET_INTERFACE_LEN];
	gchar				dns[MAX_IP_LEN];
	int					count;
	IpInfo				ip_list[0];
};

typedef struct _JpfAddHdGroup JpfAddHdGroup;
struct _JpfAddHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gchar   hd_group_name[GROUP_NAME_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfAddHdGroupRes JpfAddHdGroupRes;
struct _JpfAddHdGroupRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gchar   mss_id[MSS_ID_LEN];
    gint hd_group_id;
};

typedef struct _JpfAddHdToGroup JpfAddHdToGroup;
struct _JpfAddHdToGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;
    gint      hd_id;
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfAddHdToGroupRes JpfAddHdToGroupRes;
struct _JpfAddHdToGroupRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _JpfDelHdFromGroup JpfDelHdFromGroup;
struct _JpfDelHdFromGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;
    gint      hd_id;
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfDelHdFromGroupRes JpfDelHdFromGroupRes;
struct _JpfDelHdFromGroupRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _JpfRebootMss JpfRebootMss;
struct _JpfRebootMss
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfRebootMssRes JpfRebootMssRes;
struct _JpfRebootMssRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _JpfHdGroupInfo JpfHdGroupInfo;
struct _JpfHdGroupInfo
{
    gint hd_group_id;
    gchar   hd_group_name[GROUP_NAME_LEN];
};

typedef struct _JpfQueryAllHdGroup JpfQueryAllHdGroup;
struct _JpfQueryAllHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfQueryAllHdGroupRes JpfQueryAllHdGroupRes;
struct _JpfQueryAllHdGroupRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;
    JpfHdGroupInfo    group_info[0];
};

typedef struct _JpfHdInfo JpfHdInfo;
struct _JpfHdInfo
{
    gchar hd_name[HD_NAME_LEN];
    gint hd_id;
    gint total_size;
    gint usedSize;
    gint hd_status;
    gint fs_type;
};

typedef struct _JpfQueryHdGroupInfo JpfQueryHdGroupInfo;
struct _JpfQueryHdGroupInfo
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfQueryHdGroupInfoRes JpfQueryHdGroupInfoRes;
struct _JpfQueryHdGroupInfoRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;
    JpfHdInfo    hd_info[0];
};

typedef struct _JpfQueryAllHd JpfQueryAllHd;
struct _JpfQueryAllHd
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfQueryAllHdRes JpfQueryAllHdRes;
struct _JpfQueryAllHdRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;
    JpfHdInfo    hd_info[0];
};

typedef struct _JpfDelHdGroup JpfDelHdGroup;
struct _JpfDelHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfDelHdGroupRes JpfDelHdGroupRes;
struct _JpfDelHdGroupRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _JpfGetHdFormatProgress JpfGetHdFormatProgress;
struct _JpfGetHdFormatProgress
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfGetHdFormatProgressRes JpfGetHdFormatProgressRes;
struct _JpfGetHdFormatProgressRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gchar   mss_id[MSS_ID_LEN];
    gchar   hd_name[HD_NAME_LEN];
    gint      hd_group_id;
    gint      hd_id;
    gint      percent;
};

typedef struct _JpfQueryGuRecordStatus JpfQueryGuRecordStatus;
struct _JpfQueryGuRecordStatus
{
    gchar   mss_id[MSS_ID_LEN];
    gchar                  guid[MAX_ID_LEN];
    gchar                  domain_id[DOMAIN_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfQueryGuRecordStatusRes JpfQueryGuRecordStatusRes;
struct _JpfQueryGuRecordStatusRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gint                     status;
    gint                     status_code;
};

typedef struct _JpfMssId JpfMssId;
struct _JpfMssId
{
    gchar   mss_id[MSS_ID_LEN];
};

typedef struct _JpfMssEvent  JpfMssEvent;
struct _JpfMssEvent
{
    gint       mss_num;
    JpfMssId   mss_event[0];
};

typedef struct _JpfAddDefenceArea  JpfAddDefenceArea;
struct _JpfAddDefenceArea
{
    gint                defence_area_id;
    gint                enable;
    gchar             policy[POLICY_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAddDefenceAreaRes JpfAddDefenceAreaRes;
struct _JpfAddDefenceAreaRes
{
    JpfMsgErrCode      code;
};

typedef struct _JpfDefenceAreaInfo  JpfDefenceAreaInfo;
struct _JpfDefenceAreaInfo
{
    gint                defence_area_id;
    gchar             defence_area_name[AREA_NAME_LEN];
    gint                defence_enable;
    gchar             policy[POLICY_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryDefenceArea JpfQueryDefenceArea;
struct _JpfQueryDefenceArea
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:查询所有1:根据id查询
    gchar            key[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryDefenceAreaRes JpfQueryDefenceAreaRes;
struct _JpfQueryDefenceAreaRes
{
    JpfMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      type;   //0:查询所有1:根据id查询
    gint                      total_num;
    gint                      req_num;
    JpfDefenceAreaInfo           defence_area_info[0];
};

typedef struct _JpfDelDefenceArea JpfDelDefenceArea;
struct _JpfDelDefenceArea
{
    gint                defence_area_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAddDefenceMap  JpfAddDefenceMap;
struct _JpfAddDefenceMap
{
    gint                defence_area_id;
    gint                map_id;
    gchar             map_name[MAP_NAME_LEN];
    gchar             map_location[MAP_LOCATION_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfAddDefenceMapRes JpfAddDefenceMapRes;
struct _JpfAddDefenceMapRes
{
    JpfMsgErrCode      code;
};

typedef struct _JpfDefenceMapInfo  JpfDefenceMapInfo;
struct _JpfDefenceMapInfo
{
    gint                map_id;
    gchar             map_name[MAP_NAME_LEN];
    gchar             map_location[MAP_LOCATION_LEN];
};

typedef struct _JpfQueryDefenceMap JpfQueryDefenceMap;
struct _JpfQueryDefenceMap
{
    gint                defence_area_id;
    gint               req_num;
    gint               start_num;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryDefenceMapRes JpfQueryDefenceMapRes;
struct _JpfQueryDefenceMapRes
{
    JpfMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      back_num;
    JpfDefenceMapInfo           defence_map_info[0];
};

typedef struct _JpfDelDefenceMap JpfDelDefenceMap;
struct _JpfDelDefenceMap
{
    gint                defence_area_id;
    gint                map_id;
    gchar             map_location[MAP_LOCATION_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAddDefenceGu  JpfAddDefenceGu;
struct _JpfAddDefenceGu
{
    gint                map_id;
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    double            coordinate_x;
    double            coordinate_y;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAddDefenceGuRes JpfAddDefenceGuRes;
struct _JpfAddDefenceGuRes
{
    JpfMsgErrCode      code;
};

typedef struct _JpfDefenceGuInfo  JpfDefenceGuInfo;
struct _JpfDefenceGuInfo
{
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             pu_name[PU_NAME_LEN];
    gchar             gu_name[GU_NAME_LEN];
    gint                gu_type;
    double            coordinate_x;
    double            coordinate_y;
};

typedef struct _JpfModifyDefenceGu  JpfModifyDefenceGu;
struct _JpfModifyDefenceGu
{
    gint                map_id;
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    double            coordinate_x;
    double            coordinate_y;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryDefenceGu JpfQueryDefenceGu;
struct _JpfQueryDefenceGu
{
    gint               map_id;
    gint               req_num;
    gint               start_num;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryDefenceGuRes JpfQueryDefenceGuRes;
struct _JpfQueryDefenceGuRes
{
    JpfMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      back_num;
    JpfDefenceGuInfo           defence_gu_info[0];
};

typedef struct _JpfDelDefenceGu JpfDelDefenceGu;
struct _JpfDelDefenceGu
{
    gint                map_id;
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    gint               type;   //0:删除所有1:根据id查询
    gchar            key[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfSetMapHref  JpfSetMapHref;
struct _JpfSetMapHref
{
    gint                src_map_id;
    gint                dst_map_id;
    double            coordinate_x;
    double            coordinate_y;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfSetMapHrefRes JpfSetMapHrefRes;
struct _JpfSetMapHrefRes
{
    JpfMsgErrCode      code;
};

typedef struct _JpfMapHrefInfo  JpfMapHrefInfo;
struct _JpfMapHrefInfo
{
    gint                dst_map_id;
    gchar             map_name[MAP_NAME_LEN];
    gchar             map_location[MAP_LOCATION_LEN];
    double            coordinate_x;
    double            coordinate_y;
};

typedef struct _JpfModifyMapHref  JpfModifyMapHref;
struct _JpfModifyMapHref
{
    gint                src_map_id;
    gint                dst_map_id;
    double            coordinate_x;
    double            coordinate_y;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryMapHref JpfQueryMapHref;
struct _JpfQueryMapHref
{
    gint               map_id;
    gint               req_num;
    gint               start_num;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryMapHrefRes JpfQueryMapHrefRes;
struct _JpfQueryMapHrefRes
{
    JpfMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      back_num;
    JpfMapHrefInfo           map_href_info[0];
};

typedef struct _JpfDelMapHref JpfDelMapHref;
struct _JpfDelMapHref
{
    gint                src_map_id;
    gint                dst_map_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfChangeMss JpfChangeMss;
struct _JpfChangeMss
{
    gchar        mss_id[MSS_ID_LEN];
    gint      storage_type;  //磁盘类型：0：磁盘阵列，1：本地硬盘
    gint      mode;
};

typedef struct _JpfQueryAlarm JpfQueryAlarm;
struct _JpfQueryAlarm
{
    gint  		order_by;
    gint            req_num;
    gint            start_num;
    gint            alarm_state;
    gint            alarm_type;
    gchar	        start_time[TIME_INFO_LEN];
    gchar          end_time[TIME_INFO_LEN];
    gchar          bss_usr[USER_NAME_LEN];
};

typedef struct _JpfBssAlarm JpfBssAlarm;
struct _JpfBssAlarm
{
    gint           alarm_id;
    gchar        domain_id[DOMAIN_ID_LEN];
    gchar        puid[MAX_ID_LEN];
    gchar        guid[MAX_ID_LEN];
    gchar        pu_name[PU_NAME_LEN];
    gchar        gu_name[GU_NAME_LEN];
    gint           alarm_type;
    gint           state;
    gchar	      alarm_time[TIME_INFO_LEN];
    gchar	      alarm_info[ALARM_INFO_LEN];
    gchar	      operator[USER_NAME_LEN];
    gchar	      deal_time[TIME_INFO_LEN];
    gchar	      description[DESCRIPTION_INFO_LEN];
    gchar        submit_time[TIME_INFO_LEN];
};

typedef struct _JpfQueryAlarmRes JpfQueryAlarmRes;
struct _JpfQueryAlarmRes
{
    JpfMsgErrCode      code;
    gchar         bss_usr[USER_NAME_LEN];
    gint            req_num;
    gint            total_num;
    JpfBssAlarm	alarm_list[0];
};

typedef struct _JpfDelAlarm JpfDelAlarm;
struct _JpfDelAlarm
{
    gint               type;   //0:删除所有，1：根据告警ID删除，2：根据告警类型，时间段，告警处理情况
    gchar		alarm_ids[MULTI_NAME_LEN];
    gint            alarm_state;
    gint            alarm_type;
    gchar	        start_time[TIME_INFO_LEN];
    gchar          end_time[TIME_INFO_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfDelAlarmPolicy JpfDelAlarmPolicy;
struct _JpfDelAlarmPolicy
{
    gint            enable;
    gint            max_capacity;
    gint            del_alarm_num;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfQueryDelAlarmPolicyRes JpfQueryDelAlarmPolicyRes;
struct _JpfQueryDelAlarmPolicyRes
{
    JpfMsgErrCode      code;
    gint            enable;
    gint            max_capacity;
    gint            del_alarm_num;
};

typedef struct _JpfAddTw JpfAddTw;
struct _JpfAddTw
{
    gchar             tw_name[TW_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfModifyTw JpfModifyTw;
struct _JpfModifyTw
{
    gint                type;
    gint                tw_id;
    gchar             tw_name[TW_NAME_LEN];
    gint                line_num;
    gint                column_num;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfTwInfo JpfTwInfo;
struct _JpfTwInfo
{
    gint                tw_id;
    gchar             tw_name[TW_NAME_LEN];
    gint                line_num;
    gint                column_num;
};

typedef struct _JpfQueryTw JpfQueryTw;
struct _JpfQueryTw
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar         bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryTwRes JpfQueryTwRes;
struct _JpfQueryTwRes
{
    JpfMsgErrCode  code;
    gchar         bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    JpfTwInfo    tw_info[0];
};

typedef struct _JpfDelTw JpfDelTw;
struct _JpfDelTw
{
    gint                tw_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAddScreen JpfAddScreen;
struct _JpfAddScreen
{
    gint                tw_id;
    gchar             dis_domain[DOMAIN_ID_LEN];
    gchar             dis_guid[MAX_ID_LEN];
    double            coordinate_x;
    double            coordinate_y;
    double            length;
    double            width;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfModifyScreen JpfModifyScreen;
struct _JpfModifyScreen
{
    gchar             dis_domain[DOMAIN_ID_LEN];
    gchar             dis_guid[MAX_ID_LEN];
    gint                screen_id;
    double            coordinate_x;
    double            coordinate_y;
    double            length;
    double            width;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfScreenInfo JpfScreenInfo;
struct _JpfScreenInfo
{
    gint                screen_id;
    gint                tw_id;
    gchar             dis_domain[DOMAIN_ID_LEN];
    gchar             dis_guid[MAX_ID_LEN];
    double            coordinate_x;
    double            coordinate_y;
    double            length;
    double            width;
    gchar             puid[MAX_ID_LEN];
    gchar             pu_name[PU_NAME_LEN];
    gchar             gu_name[GU_NAME_LEN];
    gchar             pu_ip[MAX_IP_LEN];
    gint              pu_state;
    gint              pu_minor_type;
};

typedef struct _JpfQueryScreen JpfQueryScreen;
struct _JpfQueryScreen
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar         bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryScreenRes JpfQueryScreenRes;
struct _JpfQueryScreenRes
{
    JpfMsgErrCode  code;
    gchar         bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    JpfScreenInfo    screen_info[0];
};

typedef struct _JpfDelScreen JpfDelScreen;
struct _JpfDelScreen
{
    gint               type;   //0:根据显示通道删除电视墙下某个屏幕，1:删除某个电视墙下所有的屏幕
    gint                tw_id;
    gchar             dis_domain[DOMAIN_ID_LEN];
    gchar             dis_guid[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfScrDivInfo JpfScrDivInfo;
struct _JpfScrDivInfo
{
    gint                div_id;
    gchar             div_name[DIV_NAME_LEN];
    gchar             description[DESCRIPTION_INFO_LEN];
};

typedef struct _JpfQueryScrDiv JpfQueryScrDiv;
struct _JpfQueryScrDiv
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar         bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryScrDivRes JpfQueryScrDivRes;
struct _JpfQueryScrDivRes
{
    JpfMsgErrCode  code;
    gchar         bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    JpfScrDivInfo    scr_div_info[0];
};

typedef struct _JpfAddTour JpfAddTour;
struct _JpfAddTour
{
    gchar              tour_name[TOUR_NAME_LEN];
    gint                auto_jump;
    gchar		     bss_usr[USER_NAME_LEN];
};

typedef struct _JpfModifyTour JpfModifyTour;
struct _JpfModifyTour
{
    gint                tour_id;
    gchar             tour_name[TOUR_NAME_LEN];
    gint                auto_jump;
    gchar		     bss_usr[USER_NAME_LEN];
};


typedef struct _JpfTourInfo JpfTourInfo;
struct _JpfTourInfo
{
    gint                tour_id;
    gchar             tour_name[TOUR_NAME_LEN];
    gint                auto_jump;
};

typedef struct _JpfQueryTour JpfQueryTour;
struct _JpfQueryTour
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryTourRes JpfQueryTourRes;
struct _JpfQueryTourRes
{
    JpfMsgErrCode  code;
    gchar		bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    JpfTourInfo    tour_info[0];
};

typedef struct _JpfDelTour JpfDelTour;
struct _JpfDelTour
{
    gchar		     bss_usr[USER_NAME_LEN];
    gint                tour_id;
};

typedef struct _JpfTourStep JpfTourStep;
struct _JpfTourStep
{
    gint step_no;
    gint interval;
    gchar encoder_guid[MAX_ID_LEN];
    gchar encoder_domain[DOMAIN_ID_LEN];
    gchar gu_name[GU_NAME_LEN];
    gint level;
};

typedef struct _JpfAddTourStep JpfAddTourStep;
struct _JpfAddTourStep
{
    gint tour_id;
    gchar bss_usr[USER_NAME_LEN];
    gint total_num;
    JpfTourStep tour_step[0];
};

typedef struct _JpfTourStepInfo JpfTourStepInfo;
struct _JpfTourStepInfo
{
    gint                step_no;
    gint                interval;
    gchar   encoder_guid[MAX_ID_LEN];
    gchar   encoder_domain[DOMAIN_ID_LEN];
};

typedef struct _JpfQueryTourStep JpfQueryTourStep;
struct _JpfQueryTourStep
{
    gint              req_num;
    gint              start_num;
    gint              tour_id;
    gchar		  bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryTourStepRes JpfQueryTourStepRes;
struct _JpfQueryTourStepRes
{
    JpfMsgErrCode     code;
    gchar		     bss_usr[USER_NAME_LEN];
    gint                 total_num;
    gint                 back_num;
    gint                 tour_id;
    JpfTourStep     tour_step[0];
};

typedef struct _JpfAddGroup JpfAddGroup;
struct _JpfAddGroup
{
    gchar             group_name[GROUP_NAME_LEN];
    gint                tw_id;
    gchar		     bss_usr[USER_NAME_LEN];
};

typedef struct _JpfModifyGroup JpfModifyGroup;
struct _JpfModifyGroup
{
    gint               group_id;
    gchar             group_name[GROUP_NAME_LEN];
    gchar		     bss_usr[USER_NAME_LEN];
};


typedef struct _JpfGroupInfo JpfGroupInfo;
struct _JpfGroupInfo
{
    gint               group_id;
    gchar             group_name[GROUP_NAME_LEN];
    gint               tw_id;
    gchar             tw_name[TW_NAME_LEN];
};

typedef struct _JpfQueryGroup JpfQueryGroup;
struct _JpfQueryGroup
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:查询所有，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryGroupRes JpfQueryGroupRes;
struct _JpfQueryGroupRes
{
    JpfMsgErrCode  code;
    gchar		bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    JpfGroupInfo    group_info[0];
};

typedef struct _JpfDelGroup JpfDelGroup;
struct _JpfDelGroup
{
    gchar		     bss_usr[USER_NAME_LEN];
    gint               group_id;
};

typedef struct _JpfAddGroupStep JpfAddGroupStep;
struct _JpfAddGroupStep
{
    gint     		group_id;
    gint			step_no;
    gint			interval;
    gchar			bss_usr[USER_NAME_LEN];
};

typedef struct _JpfModifyGroupStep JpfModifyGroupStep;
struct _JpfModifyGroupStep
{
    gint			group_id;
    gint			step_no;
    gint			interval;
    gchar		     bss_usr[USER_NAME_LEN];
};


typedef struct _JpfGroupSteps JpfGroupSteps;
struct _JpfGroupSteps
{
    gint                group_id;
    gint			step_no;
    gint			interval;
};

typedef struct _JpfQueryGroupStep JpfQueryGroupStep;
struct _JpfQueryGroupStep
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:查询所有，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryGroupStepRes JpfQueryGroupStepRes;
struct _JpfQueryGroupStepRes
{
    JpfMsgErrCode  code;
    gchar		bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    JpfGroupSteps    group_step[0];
};

typedef struct _JpfDelGroupStep JpfDelGroupStep;
struct _JpfDelGroupStep
{
    gchar		      bss_usr[USER_NAME_LEN];
    gint                group_id;
    gint			step_no;
};

typedef struct _JpfGroupStep JpfGroupStep;
struct _JpfGroupStep
{
    gint div_no;
    gchar encoder_guid[MAX_ID_LEN];
    gchar encoder_domain[DOMAIN_ID_LEN];
    gint level;
};

typedef struct _JpfConfigGroupStep JpfConfigGroupStep;
struct _JpfConfigGroupStep
{
    gint group_id;
    gint step_no;
    gint scr_id;
    gint div_id;
    gchar bss_usr[USER_NAME_LEN];
    gint total_num;
    JpfGroupStep group_step[0];
};

typedef struct _JpfGroupStepInfo JpfGroupStepInfo;
struct _JpfGroupStepInfo
{
   // gint scr_id;
    //gint div_id;
    gint div_no;
    gchar encoder_guid[MAX_ID_LEN];
    gchar encoder_domain[DOMAIN_ID_LEN];
    gint level;
    gchar             puid[MAX_ID_LEN];
    gchar             pu_name[PU_NAME_LEN];
    gchar             gu_name[GU_NAME_LEN];
    gchar             pu_ip[MAX_IP_LEN];
    gint                pu_state;
};

typedef struct _JpfQueryGroupStepInfo JpfQueryGroupStepInfo;
struct _JpfQueryGroupStepInfo
{
    gint              req_num;
    gint              start_num;
    gint              group_id;
    gint              step_no;
    gint              scr_id;
    gint               type;   //0:查询所有，1:按类型查询
    gchar            key[USER_NAME_LEN];

    gchar		  bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryGroupStepInfoRes JpfQueryGroupStepInfoRes;
struct _JpfQueryGroupStepInfoRes
{
    JpfMsgErrCode     code;
    gchar		     bss_usr[USER_NAME_LEN];
    gint                 total_num;
    gint                 back_num;
    gint                 group_id;
    gint scr_id;
    gint div_id;
    JpfGroupStepInfo    group_step[0];
};

typedef struct _JpfModifyGroupStepInfo JpfModifyGroupStepInfo;
struct _JpfModifyGroupStepInfo
{
    gint group_id;
    gint step_no;
    gint scr_id;
    gint level;
    gint div_no;
    //JpfGroupStep step_info;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfDelGroupStepInfo JpfDelGroupStepInfo;
struct _JpfDelGroupStepInfo
{
	gchar bss_usr[USER_NAME_LEN];
	gint group_id;
	gint step_no;
	gint scr_id;
	gint div_no;
	gint type;
};

typedef struct _JpfQueryGroupStepDiv JpfQueryGroupStepDiv;
struct _JpfQueryGroupStepDiv
{
    gint              group_id;
    gint              scr_id;
    gchar		  bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryGroupStepDivRes JpfQueryGroupStepDivRes;
struct _JpfQueryGroupStepDivRes
{
    JpfMsgErrCode     code;
    gchar		     bss_usr[USER_NAME_LEN];
    gint div_id;
};

typedef struct _JpfLinkTimePolicyConfig JpfLinkTimePolicyConfig;
struct _JpfLinkTimePolicyConfig
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar time_policy[POLICY_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkTimePolicyConfig JpfModifyLinkTimePolicy;

typedef struct _JpfQueryLinkTimePolicy JpfQueryLinkTimePolicy;
struct _JpfQueryLinkTimePolicy
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkTimePolicyConfig JpfQueryLinkTimePolicyRes;

typedef struct _JpfDelLinkTimePolicy JpfDelLinkTimePolicy;
struct _JpfDelLinkTimePolicy
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkRecord JpfLinkRecord;
struct _JpfLinkRecord
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint time_len;
	JpfMssId mss[MSS_NUM];
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkRecord JpfModifyLinkRecord;

typedef struct _JpfQueryLinkRecord JpfQueryLinkRecord;
struct _JpfQueryLinkRecord
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};
typedef struct _JpfLinkRecordInfo JpfLinkRecordInfo;
struct _JpfLinkRecordInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint time_len;
	gchar gu_name[GU_NAME_LEN];
	JpfStoreServer            mss[MSS_NUM];
};

typedef struct _JpfQueryLinkRecordRes JpfQueryLinkRecordRes;
struct _JpfQueryLinkRecordRes
{
	JpfMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	JpfLinkRecordInfo link_record_info[0];
};

typedef struct _JpfDelLinkRecord JpfDelLinkRecord;
struct _JpfDelLinkRecord
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkIO JpfLinkIO;
struct _JpfLinkIO
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint time_len;
	gchar io_value[IO_VALUE_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkIO JpfModifyLinkIO;

typedef struct _JpfQueryLinkIO JpfQueryLinkIO;
struct _JpfQueryLinkIO
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkIOInfo JpfLinkIOInfo;
struct _JpfLinkIOInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint time_len;
	gchar gu_name[GU_NAME_LEN];
	gchar io_value[IO_VALUE_LEN];
};

typedef struct _JpfQueryLinkIORes JpfQueryLinkIORes;
struct _JpfQueryLinkIORes
{
	JpfMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	JpfLinkIOInfo link_io_info[0];
};

typedef struct _JpfDelLinkIO JpfDelLinkIO;
struct _JpfDelLinkIO
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkSnapshot JpfLinkSnapshot;
struct _JpfLinkSnapshot
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint picture_num;
	JpfMssId mss[MSS_NUM];
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkSnapshot JpfModifyLinkSnapshot;

typedef struct _JpfQueryLinkSnapshot JpfQueryLinkSnapshot;
struct _JpfQueryLinkSnapshot
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};
typedef struct _JpfLinkSnapshotInfo JpfLinkSnapshotInfo;
struct _JpfLinkSnapshotInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint picture_num;
	gchar gu_name[GU_NAME_LEN];
	JpfStoreServer            mss[MSS_NUM];
};

typedef struct _JpfQueryLinkSnapshotRes JpfQueryLinkSnapshotRes;
struct _JpfQueryLinkSnapshotRes
{
	JpfMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	JpfLinkSnapshotInfo link_snapshot_info[0];
};

typedef struct _JpfDelLinkSnapshot JpfDelLinkSnapshot;
struct _JpfDelLinkSnapshot
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkStepConfig JpfLinkStepConfig;
struct _JpfLinkStepConfig
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint tw_id;
	gint screen_id;
	gint div_num;
	gint div_id;
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkStepConfig JpfModifyLinkStep;

typedef struct _JpfQueryLinkStep JpfQueryLinkStep;
struct _JpfQueryLinkStep
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint tw_id;
	gint screen_id;
	gint div_num;
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};
typedef struct _JpfLinkStepInfo JpfLinkStepInfo;
struct _JpfLinkStepInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint time_len;
	gint tw_id;
	gint screen_id;
	gint div_num;
	gint div_id;
	gchar gu_name[GU_NAME_LEN];
	gchar screen_name[GU_NAME_LEN];
};

typedef struct _JpfQueryLinkStepRes JpfQueryLinkStepRes;
struct _JpfQueryLinkStepRes
{
	JpfMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint type;
	gint            total_num;
	gint            back_num;
	JpfLinkStepInfo link_step_info[0];
};

typedef struct _JpfDelLinkStep JpfDelLinkStep;
struct _JpfDelLinkStep
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint tw_id;
	gint screen_id;
	gint div_num;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkPreset JpfLinkPreset;
struct _JpfLinkPreset
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint preset_no;
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkPreset JpfModifyLinkPreset;

typedef struct _JpfQueryLinkPreset JpfQueryLinkPreset;
struct _JpfQueryLinkPreset
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkPresetInfo JpfLinkPresetInfo;
struct _JpfLinkPresetInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint preset_no;
	gchar gu_name[GU_NAME_LEN];
};

typedef struct _JpfQueryLinkPresetRes JpfQueryLinkPresetRes;
struct _JpfQueryLinkPresetRes
{
	JpfMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	JpfLinkPresetInfo link_preset_info[0];
};

typedef struct _JpfDelLinkPreset JpfDelLinkPreset;
struct _JpfDelLinkPreset
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkTourConfig JpfLinkTourConfig;
struct _JpfLinkTourConfig
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint tw_id;
	gint screen_id;
	gint div_num;
	gint div_id;
	gint tour_id;
	gint alarm_type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkTourConfig JpfModifyLinkTour;

typedef struct _JpfQueryLinkTour JpfQueryLinkTour;
struct _JpfQueryLinkTour
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint tw_id;
	gint screen_id;
	gint div_num;
	gint div_id;
	gint tour_id;
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkTourInfo JpfLinkTourInfo;
struct _JpfLinkTourInfo
{
	gint tw_id;
	gint screen_id;
	gint div_num;
	gint div_id;
	gint tour_id;
	gint alarm_type;
	gchar tour_name[TOUR_NAME_LEN];
};

typedef struct _JpfQueryLinkTourRes JpfQueryLinkTourRes;
struct _JpfQueryLinkTourRes
{
	JpfMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	JpfLinkTourInfo link_tour_info[0];
};

typedef struct _JpfDelLinkTour JpfDelLinkTour;
struct _JpfDelLinkTour
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint screen_id;
	gint div_num;
	gint div_id;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkGroupConfig JpfLinkGroupConfig;
struct _JpfLinkGroupConfig
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint group_id;
	gint alarm_type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkGroupConfig JpfModifyLinkGroup;

typedef struct _JpfQueryLinkGroup JpfQueryLinkGroup;
struct _JpfQueryLinkGroup
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint group_id;
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkGroupInfo JpfLinkGroupInfo;
struct _JpfLinkGroupInfo
{
	gint group_id;
	gint alarm_type;
	gchar group_name[GROUP_NAME_LEN];
};

typedef struct _JpfQueryLinkGroupRes JpfQueryLinkGroupRes;
struct _JpfQueryLinkGroupRes
{
	JpfMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	JpfLinkGroupInfo link_group_info[0];
};

typedef struct _JpfDelLinkGroup JpfDelLinkGroup;
struct _JpfDelLinkGroup
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint group_id;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkMap JpfLinkMap;
struct _JpfLinkMap
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint level;
	gchar bss_usr[USER_NAME_LEN];
};

typedef JpfLinkMap JpfModifyLinkMap;

typedef struct _JpfQueryLinkMap JpfQueryLinkMap;
struct _JpfQueryLinkMap
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfLinkMapInfo JpfLinkMapInfo;
struct _JpfLinkMapInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint level;
	gchar gu_name[GU_NAME_LEN];
};

typedef struct _JpfQueryLinkMapRes JpfQueryLinkMapRes;
struct _JpfQueryLinkMapRes
{
	JpfMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	JpfLinkMapInfo link_map_info[0];
};

typedef struct _JpfDelLinkMap JpfDelLinkMap;
struct _JpfDelLinkMap
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryServerResourceInfo JpfQueryServerResourceInfo;
struct _JpfQueryServerResourceInfo
{
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryServerResourceInfoRes JpfQueryServerResourceInfoRes;
struct _JpfQueryServerResourceInfoRes
{
	JpfMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint            enc_num;
	gint            enc_online_num;
	gint            dec_num;
	gint            dec_online_num;
	gint            av_num;
	gint            av_limited_num;
	gint            ds_num;
	gint            ds_limited_num;
	gint            ai_num;
	gint            ai_limited_num;
	gint            ao_num;
	gint            ao_limited_num;
	gint            system_version;
	gint            manufactor_type;
	gint            support_keyboard;
	JpfExpiredTime expired_time;
};

typedef JpfQueryServerResourceInfo JpfQueryTwAuthInfo;

typedef struct _JpfQueryTwAuthInfoRes JpfQueryTwAuthInfoRes;
struct _JpfQueryTwAuthInfoRes
{
	JpfMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint   tw_auth_type;
};

typedef JpfQueryServerResourceInfo JpfQueryAlarmLinkAuthInfo;

typedef struct _JpfQueryAlarmLinkAuthInfoRes JpfQueryAlarmLinkAuthInfoRes;
struct _JpfQueryAlarmLinkAuthInfoRes
{
	JpfMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint   alarm_link_auth_type;
};

typedef struct __search_pu_list
{
    int count;                          //条目
    search_result_t result[0];
}search_pu_list;

typedef struct _JpfSearchPuRes JpfSearchPuRes;
struct _JpfSearchPuRes
{
	JpfMsgErrCode     code;
	gint  pu_count;
};

typedef struct _JpfGetSearchedPu JpfGetSearchedPu;
struct _JpfGetSearchedPu
{
	gint               req_num;
	gint               start_num;
};

typedef struct _JpfGetSearchedPuRes JpfGetSearchedPuRes;
struct _JpfGetSearchedPuRes
{
	JpfMsgErrCode     code;
	gint  pu_count;  //设备总个数
	gint  res_count;  // 返回的设备个数
	search_result_t  search_pu[0];
};

typedef struct _JpfCmsIp JpfCmsIp;
struct _JpfCmsIp
{
	gchar ip[MAX_IP_LEN];
};

typedef struct _JpfQueryCmsAllIpRes JpfQueryCmsAllIpRes;
struct _JpfQueryCmsAllIpRes
{
	JpfMsgErrCode     code;
	gint count;
	JpfCmsIp ips[0];
};

typedef struct _JpfAutoAddPu JpfAutoAddPu;
struct _JpfAutoAddPu
{
	gchar bss_usr[USER_NAME_LEN];
	gchar		manufacturer[MF_ID_LEN];
	gint			pu_type;
	gchar 		puid[MAX_ID_LEN];
	gchar		dev_id[MAX_ID_LEN];
	gchar		dev_name[PU_NAME_LEN];
	gchar		domain_id[DOMAIN_ID_LEN];
	gint			area_id;
	gchar		mds_id[MDS_ID_LEN];
	gint			keep_alive_time;
	gint			av_num;
	gchar		mss_id[MSS_ID_LEN];
	gchar		cms_ip[MAX_IP_LEN];
	gint			cms_port;
	gint			connect_cms_enable;
};

typedef struct _JpfGetNextPuNo JpfGetNextPuNo;
struct _JpfGetNextPuNo
{
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfGetNextPuNoRes JpfGetNextPuNoRes;
struct _JpfGetNextPuNoRes
{
	JpfMsgErrCode     code;
	gchar pu_no[PU_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfGetInitName JpfGetInitName;
struct _JpfGetInitName
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfGetInitNameRes JpfGetInitNameRes;
struct _JpfGetInitNameRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gchar			   init_name[USER_NAME_LEN];
};

typedef struct _JpfSetInitName JpfSetInitName;
struct _JpfSetInitName
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   init_name[USER_NAME_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _JpfSetInitNameRes JpfSetInitNameRes;
struct _JpfSetInitNameRes
{
    JpfMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef JpfGetInitName JpfGetIpsanInfo;

typedef struct  _JpfIpsanInfo JpfIpsanInfo;
struct _JpfIpsanInfo
{
    gchar			   ip[MAX_IP_LEN];
    gint			   port;
};

typedef struct  _JpfGetIpsanInfoRes JpfGetIpsanInfoRes;
struct _JpfGetIpsanInfoRes
{
    JpfMsgErrCode     code;
    gchar			session[USER_NAME_LEN];
    gint			count;
    JpfIpsanInfo	ipsan_info[0];
};

typedef struct  _JpfAddOneIpsan JpfAddOneIpsan;
struct _JpfAddOneIpsan
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			session[USER_NAME_LEN];
    JpfIpsanInfo	ipsan_info;
};

typedef JpfSetInitNameRes JpfAddOneIpsanRes;

typedef JpfAddOneIpsan JpfDeleteOneIpsan;

typedef JpfSetInitNameRes JpfDeleteOneIpsanRes;

typedef JpfAddOneIpsan JpfGetOneIpsanDetail;

typedef struct _JpfGetOneIpsanDetailRes JpfGetOneIpsanDetailRes;
struct _JpfGetOneIpsanDetailRes
{
    JpfMsgErrCode     code;
    gchar			session[USER_NAME_LEN];
    gchar              target[USER_NAME_LEN];
    gint			connect_state;
};

/*typedef struct _JpfAddGuToUserList JpfAddGuToUserList;
struct _JpfAddGuToUserList
{
    struct  list_head list;
    gint cur_num;
    gint total_num;
    gint page;
    gchar username[USER_NAME_LEN];
    JpfAddGuToUser add_gu_to_user_head;
};

typedef struct _JpfAddGuToUserAllList JpfAddGuToUserAllList;
struct _JpfAddGuToUserAllList
{
    JpfAddGuToUserList add_gu_to_user_list_head;
    pthread_mutex_t  conn_lock;
};
*/

typedef struct _JpfQueryLog JpfQueryLog;
struct _JpfQueryLog
{
	gint			order_by;
	gint			req_count;
	gint			start_num;
	gint			log_level;
	gchar		start_time[TIME_INFO_LEN];
	gchar		end_time[TIME_INFO_LEN];
	gchar		bss_usr[USER_NAME_LEN];
};

typedef struct _JpfBssLog JpfBssLog;
struct _JpfBssLog
{
	gint			order_num;
	gint			log_level;
	gint			log_id;
	gint			result_code;
	gchar		log_time[TIME_INFO_LEN];
	gchar		user_name[USER_NAME_LEN];
	gchar		child_data1[LOG_CHILD_DATA_LEN];
	gchar		child_data2[LOG_CHILD_DATA_LEN];
	gchar		child_data3[LOG_CHILD_DATA_LEN];
};

typedef struct _JpfQueryLogRes JpfQueryLogRes;
struct _JpfQueryLogRes
{
	JpfMsgErrCode	code;
	gchar		bss_usr[USER_NAME_LEN];
	gint			total_num;
	gint			req_total;
	JpfBssLog	log_list[0];
};

typedef struct _JpfQueryAreaDevRate JpfQueryAreaDevRate;
struct _JpfQueryAreaDevRate
{
    gint              area_id;
    gint              type;
    gchar            key[TIME_LEN];
    gint			req_num;
	gint			start_num;
    gchar		  bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAreaDevRate JpfAreaDevRate;
struct _JpfAreaDevRate
{
	gint              area_id;
	gchar             area_name[AREA_NAME_LEN];
	gdouble           total_count;
	gdouble           online_count;
	gdouble rate;
};

typedef struct _JpfQueryAreaDevRateRes JpfQueryAreaDevRateRes;
struct _JpfQueryAreaDevRateRes
{
    JpfMsgErrCode     code;
    gchar		     bss_usr[USER_NAME_LEN];
    gint			total_num;
	gint			back_num;
	JpfAreaDevRate	area_dev_rate[0];
};

typedef struct _JpfValidateGuMap JpfValidateGuMap;
struct _JpfValidateGuMap
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfAddIvs JpfAddIvs;
struct _JpfAddIvs
{
    gchar   ivs_id[IVS_ID_LEN];
    gchar   ivs_name[IVS_NAME_LEN];
    gint      keep_alive_freq;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfIvsInfo  JpfIvsInfo;
struct _JpfIvsInfo
{
    gint     type;  // 0: 修改mds ; 1:修改get_ip_enable
    gchar   ivs_id[IVS_ID_LEN];
    gchar   ivs_name[IVS_NAME_LEN];
    gint      keep_alive_freq;
    gint      ivs_state;
    gchar   ivs_last_ip[MAX_IP_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryIvs JpfQueryIvs;
struct _JpfQueryIvs
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询, 2:查询自动添加mds ip使能状态
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _JpfQueryIvsRes JpfQueryIvsRes;
struct _JpfQueryIvsRes
{
    JpfMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint               type;
    gint                     total_num;
    gint                     req_num;
    JpfIvsInfo          ivs_info[0];
};


typedef struct _JpfDelIvs JpfDelIvs;
struct _JpfDelIvs
{
    gchar  ivs_id[MULTI_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _JpfGetServerFlagRes JpfGetServerFlagRes;
struct _JpfGetServerFlagRes
{
	JpfMsgErrCode		code;
	gint					server_flag;
};


typedef struct _JpfGetMdsConfigRes JpfGetMdsConfigRes;
struct _JpfGetMdsConfigRes
{
	JpfMsgErrCode		code;
	gchar				mds_id[MDS_ID_LEN];
	gint					state;
	gint					start_port;
	gint					end_port;
};


typedef struct _JpfSetMdsConfig JpfSetMdsConfig;
struct _JpfSetMdsConfig
{
	gchar				mds_id[MDS_ID_LEN];
	gint					start_port;
	gint					end_port;
};


typedef struct _JpfGetMdsState JpfGetMdsState;
struct _JpfGetMdsState
{
	gchar				mds_id[MDS_ID_LEN];
};

typedef struct _JpfGetMdsStateRes JpfGetMdsStateRes;
struct _JpfGetMdsStateRes
{
	JpfMsgErrCode		code;
	gint					state;
};


typedef struct _JpfGetMssConfigRes JpfGetMssConfigRes;
struct _JpfGetMssConfigRes
{
	JpfMsgErrCode		code;
	gchar				mss_id[MSS_ID_LEN];
	gchar				stor_type[MAX_STOR_TYPE_LEN];
	gint					state;
};


typedef struct _JpfSetMssConfig JpfSetMssConfig;
struct _JpfSetMssConfig
{
	gchar				mss_id[MSS_ID_LEN];
	gchar				stor_type[MAX_STOR_TYPE_LEN];
};


typedef struct _JpfGetMssState JpfGetMssState;
struct _JpfGetMssState
{
	gchar				mss_id[MDS_ID_LEN];
};

typedef struct _JpfGetMssStateRes JpfGetMssStateRes;
struct _JpfGetMssStateRes
{
	JpfMsgErrCode		code;
	gint					state;
};


typedef struct _JpfGetIvsConfigRes JpfGetIvsConfigRes;
struct _JpfGetIvsConfigRes
{
	JpfMsgErrCode		code;
	gchar				ivs_id[IVS_ID_LEN];
};


typedef struct _JpfSetIvsConfig JpfSetIvsConfig;
struct _JpfSetIvsConfig
{
	gchar				ivs_id[IVS_ID_LEN];
};


typedef struct _JpfGetIvsState JpfGetIvsState;
struct _JpfGetIvsState
{
	gchar				ivs_id[IVS_ID_LEN];
};

typedef struct _JpfGetIvsStateRes JpfGetIvsStateRes;
struct _JpfGetIvsStateRes
{
	JpfMsgErrCode		code;
	gint					state;
};


typedef struct _JpfAmsId JpfAmsId;
struct _JpfAmsId
{
	gchar				ams_id[AMS_ID_LEN];
};


typedef struct _JpfAddAms JpfAddAms;
struct _JpfAddAms
{
	gchar				ams_id[AMS_ID_LEN];
	gchar				ams_name[AMS_NAME_LEN];
	gint					keep_alive_freq;
	gchar				bss_usr[USER_NAME_LEN];
};


typedef JpfAddAms JpfModifyAms;


typedef struct _JpfQueryAms JpfQueryAms;
struct _JpfQueryAms
{
	gchar				ams_id[AMS_ID_LEN];
	gint					type;
	gint					req_num;
	gint					start_num;
	gchar				bss_usr[USER_NAME_LEN];
};


typedef struct _JpfAmsInfo JpfAmsInfo;
struct _JpfAmsInfo
{
	gchar				ams_id[AMS_ID_LEN];
	gchar				ams_name[AMS_NAME_LEN];
	gint					keep_alive_freq;
	gint					ams_state;
	gchar				ams_ip[MAX_IP_LEN];
};

typedef struct _JpfQueryAmsRes JpfQueryAmsRes;
struct _JpfQueryAmsRes
{
	JpfMsgErrCode		code;
	gchar				bss_usr[USER_NAME_LEN];
	gint					total_count;
	gint					back_count;
	JpfAmsInfo			ams_info[0];
};


typedef struct _JpfDelAms JpfDelAms;
struct _JpfDelAms
{
	gchar				ams_id[AMS_ID_LEN];
	gchar				bss_usr[USER_NAME_LEN];
};


typedef struct _JpfQueryAmsPu JpfQueryAmsPu;
struct _JpfQueryAmsPu
{
	gchar				ams_id[AMS_ID_LEN];
	gint					req_num;
	gint					start_num;
	gchar				bss_usr[USER_NAME_LEN];
};


typedef struct _JpfAmsPuInfo JpfAmsPuInfo;
struct _JpfAmsPuInfo
{
	gchar				puid[MAX_ID_LEN];
	gchar				domain[DOMAIN_ID_LEN];
	gchar				pu_name[PU_NAME_LEN];
	gchar				area_name[AREA_NAME_LEN];
	gchar				dev_name[AMS_DEV_NAME_LEN];
	gchar				dev_passwd[AMS_DEV_PASSWD_LEN];
	gchar				dev_ip[MAX_IP_LEN];
	gint					dev_port;
	gint					dev_state;
};

typedef struct _JpfQueryAmsPuRes JpfQueryAmsPuRes;
struct _JpfQueryAmsPuRes
{
	JpfMsgErrCode		code;
	gchar				bss_usr[USER_NAME_LEN];
	gint					total_count;
	gint					back_count;
	JpfAmsPuInfo			pu_info[0];
};


typedef struct _JpfModifyAmsPu JpfModifyAmsPu;
struct _JpfModifyAmsPu
{
	gchar				puid[MAX_ID_LEN];
	gchar				domain[DOMAIN_ID_LEN];
	gchar				dev_name[AMS_DEV_NAME_LEN];
	gchar				dev_passwd[AMS_DEV_PASSWD_LEN];
	gchar				dev_ip[MAX_IP_LEN];
	gint					dev_port;
	gchar				bss_usr[USER_NAME_LEN];
};


#endif //__NMP_MOD_BSS_MESSAGES_EXTERNAL_H__
