#ifndef __NMP_MOD_BSS_MESSAGES_EXTERNAL_H__
#define __NMP_MOD_BSS_MESSAGES_EXTERNAL_H__

#include "nmp_msg_share.h"
#include "nmp_list_head.h"
#include "search_device.h"


#define ONLINE_RATE_FLAG 1

typedef struct _NmpModifyAdmin             NmpModifyAdmin;
typedef struct _NmpModifyAdminResp         NmpModifyAdminResp;

typedef struct _NmpBssRes NmpBssRes;
struct _NmpBssRes
{
    NmpMsgErrCode     code;
    gchar			  bss_usr[USER_NAME_LEN];
};

typedef struct _NmpBssLoginInfo NmpBssLoginInfo;
struct _NmpBssLoginInfo
{
	gchar admin_name[USER_NAME_LEN];
	gchar password[USER_PASSWD_LEN];
};

typedef struct _NmpBssLoginRes NmpBssLoginRes;
struct _NmpBssLoginRes
{
    NmpMsgErrCode       code;
    gchar       admin_name[USER_NAME_LEN];
    gchar       domain_name[DOMAIN_NAME_LEN];
    gchar       domain_id[DOMAIN_ID_LEN];
    gint         module_sets;
};

typedef struct _NmpBssHeart NmpBssHeart;
struct _NmpBssHeart
{
    gchar			 admin_name[USER_NAME_LEN];;
};

typedef struct _NmpBssHeartResp NmpBssHeartResp;
struct _NmpBssHeartResp
{
    NmpMsgErrCode	code;
    gchar			server_time[TIME_INFO_LEN];
};

typedef struct _NmpPlatformUpgrade NmpPlatformUpgrade;
struct _NmpPlatformUpgrade
{
    gchar			 admin_name[USER_NAME_LEN];;
};

typedef struct _NmpPlatformUpgradeResp NmpPlatformUpgradeResp;
struct _NmpPlatformUpgradeResp
{
    NmpMsgErrCode	code;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAdminInfo NmpAdminInfo;
struct _NmpAdminInfo
{
	gchar admin_name[USER_NAME_LEN];
	gchar password[USER_PASSWD_LEN];
};

typedef struct _NmpAddAdmin NmpAddAdmin;
struct _NmpAddAdmin
{
	gchar admin_name[USER_NAME_LEN];
	gchar password[USER_PASSWD_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpDelAdmin NmpDelAdmin;
struct _NmpDelAdmin
{
	gchar admin_name[MULTI_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryAdmin NmpQueryAdmin;
struct _NmpQueryAdmin
{
	gint              req_num;
	gint              start_num;
	gint               type;   //0:按用户名查询，1:按类型查询
	gchar            key[USER_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryAdminRes NmpQueryAdminRes;
struct _NmpQueryAdminRes
{
	NmpMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint                     total_num;
	gint                     req_num;
	NmpAdminInfo       admin_info[0];
};

typedef struct _NmpAddUserGroup  NmpAddUserGroup;
struct _NmpAddUserGroup
{
	gint group_permissions;
	gint  group_rank;
	gchar group_name[GROUP_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpValidateUserGroup  NmpValidateUserGroup;
struct _NmpValidateUserGroup
{
	gchar group_name[GROUP_NAME_LEN];
	gint group_id;
};


typedef struct _NmpDelUserGroup NmpDelUserGroup;
struct _NmpDelUserGroup
{
	gchar group_id[MULTI_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpUserGroupInfo  NmpUserGroupInfo;
struct _NmpUserGroupInfo
{
	gint group_id;
	gint group_permissions;
	gint  group_rank;
	gchar group_name[GROUP_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryUserGroup NmpQueryUserGroup;
struct _NmpQueryUserGroup
{
	gint              req_num;
	gint              start_num;
	gint              type;   //0:按用户名查询，1:按类型查询
	gchar            key[USER_NAME_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryUserGroupRes NmpQueryUserGroupRes;
struct _NmpQueryUserGroupRes
{
	NmpMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint                     total_num;
	gint                     req_num;
	NmpUserGroupInfo       group_info[0];
};

typedef struct _NmpAddUser NmpAddUser;
struct _NmpAddUser
{
	gchar                username[USER_NAME_LEN];
	gchar                password[USER_PASSWD_LEN];
	gint                   group_id;
	gint                   sex;
	gchar                user_phone[PHONE_NUM_LEN];
	gchar                user_description[DESCRIPTION_INFO_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpValidateUser  NmpValidateUser;
struct _NmpValidateUser
{
    gchar username[USER_NAME_LEN];

};

typedef struct _NmpDelUser NmpDelUser;
struct _NmpDelUser
{
    gchar username[MULTI_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpForceUserOffline NmpForceUserOffline;
struct _NmpForceUserOffline
{
    gint reason;
};


typedef struct _NmpUserInfo  NmpUserInfo;
struct _NmpUserInfo
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

typedef struct _NmpQueryUser NmpQueryUser;
struct _NmpQueryUser
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryUserRes NmpQueryUserRes;
struct _NmpQueryUserRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    NmpUserInfo          user_info[0];
};

typedef struct _NmpAddDomain  NmpAddDomain;
struct _NmpAddDomain
{
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             domain_name[DOMAIN_NAME_LEN];
    gchar             domain_ip[MAX_IP_LEN];
    gint                domain_port;
    gint                domain_type;
};


typedef struct _NmpDomainInfo  NmpDomainInfo;
struct _NmpDomainInfo
{
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             domain_name[DOMAIN_NAME_LEN];
    gchar             domain_ip[MAX_IP_LEN];
    gint                domain_port;
    gint                domain_type;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpDelDomain NmpDelDomain;
struct _NmpDelDomain
{
    gchar             domain_id[MULTI_NAME_LEN];
};

typedef struct _NmpQueryDomain NmpQueryDomain;
struct _NmpQueryDomain
{
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryDomainRes NmpQueryDomainRes;
struct _NmpQueryDomainRes
{
    NmpMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    NmpDomainInfo           domain_info[0];
};

typedef struct _NmpModifyDomain   NmpModifyDomain;
struct _NmpModifyDomain
{
    gchar		dm_id[DOMAIN_ID_LEN];
    gchar		dm_name[DOMAIN_NAME_LEN];
};

typedef struct _NmpAddArea  NmpAddArea;
struct _NmpAddArea
{
    gchar             area_name[AREA_NAME_LEN];
    gint                area_parent;
};

typedef struct _NmpAddAreaRes NmpAddAreaRes;
struct _NmpAddAreaRes
{
    NmpMsgErrCode      code;
    gint                area_id;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpAreaInfo  NmpAreaInfo;
struct _NmpAreaInfo
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

typedef struct _NmpQueryArea NmpQueryArea;
struct _NmpQueryArea
{
    gint               req_num;
    gint               start_num;
    gint               area_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryAreaRes NmpQueryAreaRes;
struct _NmpQueryAreaRes
{
    NmpMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      req_num;
    NmpAreaInfo           area_info[0];
};

typedef struct _NmpDelArea NmpDelArea;
struct _NmpDelArea
{
    gint                area_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpValidateArea  NmpValidateArea;
struct _NmpValidateArea
{
    gchar             area_name[AREA_NAME_LEN];
};

typedef struct _NmpAddPu NmpAddPu;
struct _NmpAddPu
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

typedef struct _NmpAddPuRes NmpAddPuRes;
struct _NmpAddPuRes
{
    NmpMsgErrCode     code;
    gchar			  bss_usr[USER_NAME_LEN];
    gint                  success_count;
};

typedef struct _NmpPuInfo NmpPuInfo;
struct _NmpPuInfo
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

typedef struct _NmpQueryPu NmpQueryPu;
struct _NmpQueryPu
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar             key[MAX_ID_LEN];
    gchar            domain_id[DOMAIN_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryPuRes NmpQueryPuRes;
struct _NmpQueryPuRes
{
    NmpMsgErrCode  code;
    gchar bss_usr[USER_NAME_LEN];
    gint                  total_num;
    gint                  req_num;
    gint                  type;
    NmpPuInfo          pu_info[0];
};

typedef struct _NmpMovePu NmpMovePu;
struct _NmpMovePu
{
    gchar             puid[MAX_ID_LEN];
    gchar             pu_info[PU_NAME_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    gint                area_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpPuPoint NmpPuPoint;
struct _NmpPuPoint
{
    gchar               domain_id[DOMAIN_ID_LEN];
    gchar               puid[MAX_ID_LEN];
};

typedef struct _NmpDelPu NmpDelPu;
struct _NmpDelPu
{
    gint       count;
    gint       type;
    gchar    key[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
    NmpPuPoint pu_list[0];
};

typedef struct _NmpValidatePu  NmpValidatePu;
struct _NmpValidatePu
{
    gchar              puid[MAX_ID_LEN];
};

typedef struct _NmpStoreServer NmpStoreServer;
struct _NmpStoreServer
{
    gchar              mss_id[MSS_ID_LEN];
    gchar              mss_name[MSS_NAME_LEN];
};

typedef struct _NmpAddGu NmpAddGu;
struct _NmpAddGu
{
    gchar             puid[MAX_ID_LEN];
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             gu_name[GU_NAME_LEN];
    gint                gu_type;
    gint                gu_attributes;
    gchar             ivs_id[IVS_ID_LEN];
    NmpStoreServer            mss[MSS_NUM];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpGuInfo NmpGuInfo;
struct _NmpGuInfo
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
    NmpStoreServer            mss[MSS_NUM];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryGu NmpQueryGu;
struct _NmpQueryGu
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

typedef struct _NmpQueryGuRes NmpQueryGuRes;
struct _NmpQueryGuRes
{
    NmpMsgErrCode  code;
    gchar bss_usr[USER_NAME_LEN];
    gint        total_num;
    gint req_num;
    gint               type;
    NmpGuInfo    gu_info[0];
};

typedef struct _NmpDelGu NmpDelGu;
struct _NmpDelGu
{
    gchar domain_id[DOMAIN_ID_LEN];
    gchar  guid[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAddMds NmpAddMds;
struct _NmpAddMds
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


typedef struct _NmpMdsInfo  NmpMdsInfo;
struct _NmpMdsInfo
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

typedef struct _NmpQueryMds NmpQueryMds;
struct _NmpQueryMds
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询, 2:查询自动添加mds ip使能状态
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryMdsRes NmpQueryMdsRes;
struct _NmpQueryMdsRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint               type;
    gint                     total_num;
    gint                     req_num;
    NmpMdsInfo          mds_info[0];
};


typedef struct _NmpDelMds NmpDelMds;
struct _NmpDelMds
{
    gchar  mds_id[MULTI_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpAddMdsIp NmpAddMdsIp;
struct _NmpAddMdsIp
{
    gchar   mds_id[MDS_ID_LEN];
    gchar   cms_ip[MAX_IP_LEN];
    gchar   mds_ip[MAX_IP_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpMdsIpInfo  NmpMdsIpInfo;
struct _NmpMdsIpInfo
{
   //gchar   mds_id[MDS_ID_LEN];
    gchar   cms_ip[MAX_IP_LEN];
    gchar   mds_ip[MAX_IP_LEN];
};

typedef struct _NmpQueryMdsIp NmpQueryMdsIp;
struct _NmpQueryMdsIp
{
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryMdsIpRes NmpQueryMdsIpRes;
struct _NmpQueryMdsIpRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gchar   mds_id[MDS_ID_LEN];
    NmpMdsIpInfo          mds_ip_info[0];
};


typedef struct _NmpDelMdsIp NmpDelMdsIp;
struct _NmpDelMdsIp
{
    gchar  mds_id[MDS_ID_LEN];
    gchar   cms_ip[MAX_IP_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpAddMss NmpAddMss;
struct _NmpAddMss
{
    gchar   mss_id[MSS_ID_LEN];
    gchar   mss_name[MSS_NAME_LEN];
    gint      keep_alive_freq;
    gint      storage_type;  //磁盘类型：0：磁盘阵列，1：本地硬盘
    gint      mode;  //运行模式：0：录像模式，1：配置模式
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpMssInfo  NmpMssInfo;
struct _NmpMssInfo
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

typedef struct _NmpQueryMss NmpQueryMss;
struct _NmpQueryMss
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryMssRes NmpQueryMssRes;
struct _NmpQueryMssRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;

    NmpMssInfo          mss_info[0];
};


typedef struct _NmpDelMss NmpDelMss;
struct _NmpDelMss
{
    gchar  mss_id[MULTI_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpRecordGu  NmpRecordGu;
struct _NmpRecordGu
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

typedef struct _NmpHdGroup NmpHdGroup;
struct _NmpHdGroup
{
    gchar              hd_group_id[HD_GROUP_ID_LEN];
};

typedef struct _NmpQueryRecordPolicy NmpQueryRecordPolicy;
struct _NmpQueryRecordPolicy
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

typedef struct _NmpQueryRecordGuRes NmpQueryRecordGuRes;
struct _NmpQueryRecordGuRes
{
    NmpMsgErrCode     code;
    gchar                  guid[MAX_ID_LEN];
    gchar                  domain_id[DOMAIN_ID_LEN];
    gchar                  mss_id[MSS_ID_LEN];
    NmpHdGroup		     hd_group[HD_GROUP_NUM];
    gchar                  time_policy[POLICY_LEN];
};

typedef struct _NmpQueryRecordPolicyRes NmpQueryRecordPolicyRes;
struct _NmpQueryRecordPolicyRes
{
    NmpMsgErrCode      code;
    gchar          bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      req_num;
    gint                      type;   //0:按用户名查询，1:按类型查询	    NmpHdGroup		       hd_group[HD_GROUP_NUM];
    NmpHdGroup		       hd_group[HD_GROUP_NUM];
    gchar                    time_policy[POLICY_LEN];

    NmpRecordGu          record_policy[0];
};

typedef struct _NmpRecordPolicyConfig NmpRecordPolicyConfig;
struct _NmpRecordPolicyConfig
{
    gint                      type;
    gint                      level;
    gchar                    mss_id[MSS_ID_LEN];
    NmpHdGroup		        hd_group[HD_GROUP_NUM];
    gchar                    time_policy[POLICY_LEN];
    gchar bss_usr[USER_NAME_LEN];
    gint                       gu_count;
    NmpRecordGu          record_policy[0];
};

typedef struct _NmpRecordPolicyConfigRes NmpRecordPolicyConfigRes;
struct _NmpRecordPolicyConfigRes
{
    gint                      type;
    gchar                    mss_id[MSS_ID_LEN];
    gchar		bss_usr[USER_NAME_LEN];
    gint                       gu_count;
    NmpRecordGu          record_policy[0];
};

typedef struct _NmpAddModifyManufacturer  NmpAddModifyManufacturer;
struct _NmpAddModifyManufacturer
{
    gint type;
    gchar   mf_id[MF_ID_LEN];
    gchar  mf_name[MF_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpManufacturerInfo  NmpManufacturerInfo;
struct _NmpManufacturerInfo
{
    gchar   mf_id[MF_ID_LEN];
    gchar  mf_name[MF_NAME_LEN];
};

typedef struct _NmpQueryManufacturer NmpQueryManufacturer;
struct _NmpQueryManufacturer
{
    gint              req_num;
    gint              start_num;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryManufacturerRes NmpQueryManufacturerRes;
struct _NmpQueryManufacturerRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    NmpManufacturerInfo          manufacturer_info[0];
};

typedef struct _NmpDelManufacturer NmpDelManufacturer;
struct _NmpDelManufacturer
{
    gchar  mf_id[MF_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpGuToUserInfo NmpGuToUserInfo;
struct _NmpGuToUserInfo
{
    //gchar   username[USER_NAME_LEN];
    gchar   user_guid[MAX_ID_LEN];
    gchar   user_guid_domain[DOMAIN_ID_LEN];

};

typedef struct _NmpAddGuToUser NmpAddGuToUser;
struct _NmpAddGuToUser
{
    //struct  list_head list;
    gint total_num;
    gchar   username[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
    NmpGuToUserInfo gu_to_user_info[0];
};


typedef struct _NmpUserOwnGu  NmpUserOwnGu;
struct _NmpUserOwnGu
{
    gchar   user_guid[MAX_ID_LEN];
    gchar   guid_name[GU_NAME_LEN];
};

typedef struct _NmpQueryUserOwnGu NmpQueryUserOwnGu;
struct _NmpQueryUserOwnGu
{
    gint              req_num;
    gint              start_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryUserOwnGuRes NmpQueryUserOwnGuRes;
struct _NmpQueryUserOwnGuRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];

    NmpUserOwnGu      user_own_gu_info[0];
};

typedef struct _NmpDelGuFromUser NmpDelGuFromUser;
struct _NmpDelGuFromUser
{
    gint total_num;
    gchar   username[USER_NAME_LEN];
    NmpGuToUserInfo gu_to_user_info[0];
};

typedef struct _NmpTwToUserInfo NmpTwToUserInfo;
struct _NmpTwToUserInfo
{
    gint   tw_id;
};

typedef struct _NmpAddTwToUser NmpAddTwToUser;
struct _NmpAddTwToUser
{
    //struct  list_head list;
    gint total_num;
    gchar   username[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
    NmpTwToUserInfo tw_to_user_info[0];
};


typedef struct _NmpUserOwnTw  NmpUserOwnTw;
struct _NmpUserOwnTw
{
    gint   tw_id;
    gchar  tw_name[TW_NAME_LEN];
};

typedef struct _NmpQueryUserOwnTw NmpQueryUserOwnTw;
struct _NmpQueryUserOwnTw
{
    gint              req_num;
    gint              start_num;
    gchar user[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryUserOwnTwRes NmpQueryUserOwnTwRes;
struct _NmpQueryUserOwnTwRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];

    NmpUserOwnTw      user_own_tw_info[0];
};

typedef struct _NmpTourToUserInfo NmpTourToUserInfo;
struct _NmpTourToUserInfo
{
    gint   tour_id;
};

typedef struct _NmpAddTourToUser NmpAddTourToUser;
struct _NmpAddTourToUser
{
    gint total_num;
    gchar   username[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
    NmpTourToUserInfo tour_to_user_info[0];
};

typedef struct _NmpUserOwnTour  NmpUserOwnTour;
struct _NmpUserOwnTour
{
    gint   tour_id;
    gchar  tour_name[TOUR_NAME_LEN];
};

typedef struct _NmpQueryUserOwnTour NmpQueryUserOwnTour;
struct _NmpQueryUserOwnTour
{
    gint              req_num;
    gint              start_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryUserOwnTourRes NmpQueryUserOwnTourRes;
struct _NmpQueryUserOwnTourRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint                     total_num;
    gint                     req_num;
    gchar domain_id[DOMAIN_ID_LEN];
    gchar user[USER_NAME_LEN];

    NmpUserOwnTour      user_own_tour_info[0];
};

typedef struct _NmpSetServerTime NmpSetServerTime;
struct _NmpSetServerTime
{
    gchar time_zone[TIME_ZONE_LEN];
    gchar system_time[TIME_LEN];
};

typedef struct _NmpSetServerTimeRes NmpSetServerTimeRes;
struct _NmpSetServerTimeRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gchar time_zone[TIME_ZONE_LEN];
    gchar system_time[TIME_LEN];
};

typedef struct  _NmpQueryServerTime NmpQueryServerTime;
struct _NmpQueryServerTime
{
    gchar time_zone[TIME_ZONE_LEN];
};

typedef struct  _NmpQueryServerTimeRes NmpQueryServerTimeRes;
struct _NmpQueryServerTimeRes
{
    NmpMsgErrCode     code;
    //gchar time_zone[TIME_ZONE_LEN];
    gchar system_time[TIME_LEN];
};


typedef struct  _NmpDbBackup NmpDbBackup;
struct _NmpDbBackup
{
    gchar filename[FILENAME_LEN];
    gchar	bss_usr[USER_NAME_LEN];
};

typedef struct  _NmpDbImport NmpDbImport;
struct _NmpDbImport
{
    gchar filename[FILENAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct  _NmpDbImportRes NmpDbImportRes;
struct _NmpDbImportRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gchar error_desc[DESCRIPTION_INFO_LEN];
};

typedef struct _NmpErrRes NmpErrRes;
struct _NmpErrRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _NmpSetResult NmpSetResult;
struct _NmpSetResult
{
    NmpMsgErrCode	code;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpGetNetInterfaceConfigRes NmpGetNetInterfaceConfigRes;
struct _NmpGetNetInterfaceConfigRes
{
    NmpMsgErrCode      code;
    gchar                   network_interface[NET_INTERFACE_LEN];
};


typedef struct  _NmpGetNetworkConfig NmpGetNetworkConfig;
struct _NmpGetNetworkConfig
{
    gchar                   network_interface[NET_INTERFACE_LEN];
};

typedef struct
{
	gchar				ip[MAX_IP_LEN];
	gchar				netmask[MAX_IP_LEN];
	gchar				gateway[MAX_IP_LEN];
} IpInfo;

typedef struct _NmpGetNetworkConfigRes NmpGetNetworkConfigRes;
struct _NmpGetNetworkConfigRes
{
	NmpMsgErrCode		code;
	gchar				dns[MAX_IP_LEN];
	int					count;
	IpInfo				ip_list[0];
};

typedef struct _NmpSetNetworkConfig NmpSetNetworkConfig;
struct _NmpSetNetworkConfig
{
	gchar				network_interface[NET_INTERFACE_LEN];
	gchar				dns[MAX_IP_LEN];
	int					count;
	IpInfo				ip_list[0];
};

typedef struct _NmpAddHdGroup NmpAddHdGroup;
struct _NmpAddHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gchar   hd_group_name[GROUP_NAME_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpAddHdGroupRes NmpAddHdGroupRes;
struct _NmpAddHdGroupRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gchar   mss_id[MSS_ID_LEN];
    gint hd_group_id;
};

typedef struct _NmpAddHdToGroup NmpAddHdToGroup;
struct _NmpAddHdToGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;
    gint      hd_id;
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpAddHdToGroupRes NmpAddHdToGroupRes;
struct _NmpAddHdToGroupRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _NmpDelHdFromGroup NmpDelHdFromGroup;
struct _NmpDelHdFromGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;
    gint      hd_id;
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpDelHdFromGroupRes NmpDelHdFromGroupRes;
struct _NmpDelHdFromGroupRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _NmpRebootMss NmpRebootMss;
struct _NmpRebootMss
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpRebootMssRes NmpRebootMssRes;
struct _NmpRebootMssRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _NmpHdGroupInfo NmpHdGroupInfo;
struct _NmpHdGroupInfo
{
    gint hd_group_id;
    gchar   hd_group_name[GROUP_NAME_LEN];
};

typedef struct _NmpQueryAllHdGroup NmpQueryAllHdGroup;
struct _NmpQueryAllHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpQueryAllHdGroupRes NmpQueryAllHdGroupRes;
struct _NmpQueryAllHdGroupRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;
    NmpHdGroupInfo    group_info[0];
};

typedef struct _NmpHdInfo NmpHdInfo;
struct _NmpHdInfo
{
    gchar hd_name[HD_NAME_LEN];
    gint hd_id;
    gint total_size;
    gint usedSize;
    gint hd_status;
    gint fs_type;
};

typedef struct _NmpQueryHdGroupInfo NmpQueryHdGroupInfo;
struct _NmpQueryHdGroupInfo
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpQueryHdGroupInfoRes NmpQueryHdGroupInfoRes;
struct _NmpQueryHdGroupInfoRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;
    NmpHdInfo    hd_info[0];
};

typedef struct _NmpQueryAllHd NmpQueryAllHd;
struct _NmpQueryAllHd
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpQueryAllHdRes NmpQueryAllHdRes;
struct _NmpQueryAllHdRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gint                     total_num;
    NmpHdInfo    hd_info[0];
};

typedef struct _NmpDelHdGroup NmpDelHdGroup;
struct _NmpDelHdGroup
{
    gchar   mss_id[MSS_ID_LEN];
    gint      hd_group_id;
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpDelHdGroupRes NmpDelHdGroupRes;
struct _NmpDelHdGroupRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef struct _NmpGetHdFormatProgress NmpGetHdFormatProgress;
struct _NmpGetHdFormatProgress
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpGetHdFormatProgressRes NmpGetHdFormatProgressRes;
struct _NmpGetHdFormatProgressRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gchar   mss_id[MSS_ID_LEN];
    gchar   hd_name[HD_NAME_LEN];
    gint      hd_group_id;
    gint      hd_id;
    gint      percent;
};

typedef struct _NmpQueryGuRecordStatus NmpQueryGuRecordStatus;
struct _NmpQueryGuRecordStatus
{
    gchar   mss_id[MSS_ID_LEN];
    gchar                  guid[MAX_ID_LEN];
    gchar                  domain_id[DOMAIN_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpQueryGuRecordStatusRes NmpQueryGuRecordStatusRes;
struct _NmpQueryGuRecordStatusRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gint                     status;
    gint                     status_code;
};

typedef struct _NmpMssId NmpMssId;
struct _NmpMssId
{
    gchar   mss_id[MSS_ID_LEN];
};

typedef struct _NmpMssEvent  NmpMssEvent;
struct _NmpMssEvent
{
    gint       mss_num;
    NmpMssId   mss_event[0];
};

typedef struct _NmpAddDefenceArea  NmpAddDefenceArea;
struct _NmpAddDefenceArea
{
    gint                defence_area_id;
    gint                enable;
    gchar             policy[POLICY_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAddDefenceAreaRes NmpAddDefenceAreaRes;
struct _NmpAddDefenceAreaRes
{
    NmpMsgErrCode      code;
};

typedef struct _NmpDefenceAreaInfo  NmpDefenceAreaInfo;
struct _NmpDefenceAreaInfo
{
    gint                defence_area_id;
    gchar             defence_area_name[AREA_NAME_LEN];
    gint                defence_enable;
    gchar             policy[POLICY_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryDefenceArea NmpQueryDefenceArea;
struct _NmpQueryDefenceArea
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:查询所有1:根据id查询
    gchar            key[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryDefenceAreaRes NmpQueryDefenceAreaRes;
struct _NmpQueryDefenceAreaRes
{
    NmpMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      type;   //0:查询所有1:根据id查询
    gint                      total_num;
    gint                      req_num;
    NmpDefenceAreaInfo           defence_area_info[0];
};

typedef struct _NmpDelDefenceArea NmpDelDefenceArea;
struct _NmpDelDefenceArea
{
    gint                defence_area_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAddDefenceMap  NmpAddDefenceMap;
struct _NmpAddDefenceMap
{
    gint                defence_area_id;
    gint                map_id;
    gchar             map_name[MAP_NAME_LEN];
    gchar             map_location[MAP_LOCATION_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpAddDefenceMapRes NmpAddDefenceMapRes;
struct _NmpAddDefenceMapRes
{
    NmpMsgErrCode      code;
};

typedef struct _NmpDefenceMapInfo  NmpDefenceMapInfo;
struct _NmpDefenceMapInfo
{
    gint                map_id;
    gchar             map_name[MAP_NAME_LEN];
    gchar             map_location[MAP_LOCATION_LEN];
};

typedef struct _NmpQueryDefenceMap NmpQueryDefenceMap;
struct _NmpQueryDefenceMap
{
    gint                defence_area_id;
    gint               req_num;
    gint               start_num;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryDefenceMapRes NmpQueryDefenceMapRes;
struct _NmpQueryDefenceMapRes
{
    NmpMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      back_num;
    NmpDefenceMapInfo           defence_map_info[0];
};

typedef struct _NmpDelDefenceMap NmpDelDefenceMap;
struct _NmpDelDefenceMap
{
    gint                defence_area_id;
    gint                map_id;
    gchar             map_location[MAP_LOCATION_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAddDefenceGu  NmpAddDefenceGu;
struct _NmpAddDefenceGu
{
    gint                map_id;
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    double            coordinate_x;
    double            coordinate_y;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAddDefenceGuRes NmpAddDefenceGuRes;
struct _NmpAddDefenceGuRes
{
    NmpMsgErrCode      code;
};

typedef struct _NmpDefenceGuInfo  NmpDefenceGuInfo;
struct _NmpDefenceGuInfo
{
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    gchar             pu_name[PU_NAME_LEN];
    gchar             gu_name[GU_NAME_LEN];
    gint                gu_type;
    double            coordinate_x;
    double            coordinate_y;
};

typedef struct _NmpModifyDefenceGu  NmpModifyDefenceGu;
struct _NmpModifyDefenceGu
{
    gint                map_id;
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    double            coordinate_x;
    double            coordinate_y;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryDefenceGu NmpQueryDefenceGu;
struct _NmpQueryDefenceGu
{
    gint               map_id;
    gint               req_num;
    gint               start_num;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryDefenceGuRes NmpQueryDefenceGuRes;
struct _NmpQueryDefenceGuRes
{
    NmpMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      back_num;
    NmpDefenceGuInfo           defence_gu_info[0];
};

typedef struct _NmpDelDefenceGu NmpDelDefenceGu;
struct _NmpDelDefenceGu
{
    gint                map_id;
    gchar             guid[MAX_ID_LEN];
    gchar             domain_id[DOMAIN_ID_LEN];
    gint               type;   //0:删除所有1:根据id查询
    gchar            key[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpSetMapHref  NmpSetMapHref;
struct _NmpSetMapHref
{
    gint                src_map_id;
    gint                dst_map_id;
    double            coordinate_x;
    double            coordinate_y;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpSetMapHrefRes NmpSetMapHrefRes;
struct _NmpSetMapHrefRes
{
    NmpMsgErrCode      code;
};

typedef struct _NmpMapHrefInfo  NmpMapHrefInfo;
struct _NmpMapHrefInfo
{
    gint                dst_map_id;
    gchar             map_name[MAP_NAME_LEN];
    gchar             map_location[MAP_LOCATION_LEN];
    double            coordinate_x;
    double            coordinate_y;
};

typedef struct _NmpModifyMapHref  NmpModifyMapHref;
struct _NmpModifyMapHref
{
    gint                src_map_id;
    gint                dst_map_id;
    double            coordinate_x;
    double            coordinate_y;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryMapHref NmpQueryMapHref;
struct _NmpQueryMapHref
{
    gint               map_id;
    gint               req_num;
    gint               start_num;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryMapHrefRes NmpQueryMapHrefRes;
struct _NmpQueryMapHrefRes
{
    NmpMsgErrCode      code;
    gchar bss_usr[USER_NAME_LEN];
    gint                      total_num;
    gint                      back_num;
    NmpMapHrefInfo           map_href_info[0];
};

typedef struct _NmpDelMapHref NmpDelMapHref;
struct _NmpDelMapHref
{
    gint                src_map_id;
    gint                dst_map_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpChangeMss NmpChangeMss;
struct _NmpChangeMss
{
    gchar        mss_id[MSS_ID_LEN];
    gint      storage_type;  //磁盘类型：0：磁盘阵列，1：本地硬盘
    gint      mode;
};

typedef struct _NmpQueryAlarm NmpQueryAlarm;
struct _NmpQueryAlarm
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

typedef struct _NmpBssAlarm NmpBssAlarm;
struct _NmpBssAlarm
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

typedef struct _NmpQueryAlarmRes NmpQueryAlarmRes;
struct _NmpQueryAlarmRes
{
    NmpMsgErrCode      code;
    gchar         bss_usr[USER_NAME_LEN];
    gint            req_num;
    gint            total_num;
    NmpBssAlarm	alarm_list[0];
};

typedef struct _NmpDelAlarm NmpDelAlarm;
struct _NmpDelAlarm
{
    gint               type;   //0:删除所有，1：根据告警ID删除，2：根据告警类型，时间段，告警处理情况
    gchar		alarm_ids[MULTI_NAME_LEN];
    gint            alarm_state;
    gint            alarm_type;
    gchar	        start_time[TIME_INFO_LEN];
    gchar          end_time[TIME_INFO_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpDelAlarmPolicy NmpDelAlarmPolicy;
struct _NmpDelAlarmPolicy
{
    gint            enable;
    gint            max_capacity;
    gint            del_alarm_num;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpQueryDelAlarmPolicyRes NmpQueryDelAlarmPolicyRes;
struct _NmpQueryDelAlarmPolicyRes
{
    NmpMsgErrCode      code;
    gint            enable;
    gint            max_capacity;
    gint            del_alarm_num;
};

typedef struct _NmpAddTw NmpAddTw;
struct _NmpAddTw
{
    gchar             tw_name[TW_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpModifyTw NmpModifyTw;
struct _NmpModifyTw
{
    gint                type;
    gint                tw_id;
    gchar             tw_name[TW_NAME_LEN];
    gint                line_num;
    gint                column_num;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpTwInfo NmpTwInfo;
struct _NmpTwInfo
{
    gint                tw_id;
    gchar             tw_name[TW_NAME_LEN];
    gint                line_num;
    gint                column_num;
};

typedef struct _NmpQueryTw NmpQueryTw;
struct _NmpQueryTw
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar         bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryTwRes NmpQueryTwRes;
struct _NmpQueryTwRes
{
    NmpMsgErrCode  code;
    gchar         bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    NmpTwInfo    tw_info[0];
};

typedef struct _NmpDelTw NmpDelTw;
struct _NmpDelTw
{
    gint                tw_id;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAddScreen NmpAddScreen;
struct _NmpAddScreen
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

typedef struct _NmpModifyScreen NmpModifyScreen;
struct _NmpModifyScreen
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

typedef struct _NmpScreenInfo NmpScreenInfo;
struct _NmpScreenInfo
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

typedef struct _NmpQueryScreen NmpQueryScreen;
struct _NmpQueryScreen
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar         bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryScreenRes NmpQueryScreenRes;
struct _NmpQueryScreenRes
{
    NmpMsgErrCode  code;
    gchar         bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    NmpScreenInfo    screen_info[0];
};

typedef struct _NmpDelScreen NmpDelScreen;
struct _NmpDelScreen
{
    gint               type;   //0:根据显示通道删除电视墙下某个屏幕，1:删除某个电视墙下所有的屏幕
    gint                tw_id;
    gchar             dis_domain[DOMAIN_ID_LEN];
    gchar             dis_guid[MAX_ID_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpScrDivInfo NmpScrDivInfo;
struct _NmpScrDivInfo
{
    gint                div_id;
    gchar             div_name[DIV_NAME_LEN];
    gchar             description[DESCRIPTION_INFO_LEN];
};

typedef struct _NmpQueryScrDiv NmpQueryScrDiv;
struct _NmpQueryScrDiv
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar         bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryScrDivRes NmpQueryScrDivRes;
struct _NmpQueryScrDivRes
{
    NmpMsgErrCode  code;
    gchar         bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    NmpScrDivInfo    scr_div_info[0];
};

typedef struct _NmpAddTour NmpAddTour;
struct _NmpAddTour
{
    gchar              tour_name[TOUR_NAME_LEN];
    gint                auto_jump;
    gchar		     bss_usr[USER_NAME_LEN];
};

typedef struct _NmpModifyTour NmpModifyTour;
struct _NmpModifyTour
{
    gint                tour_id;
    gchar             tour_name[TOUR_NAME_LEN];
    gint                auto_jump;
    gchar		     bss_usr[USER_NAME_LEN];
};


typedef struct _NmpTourInfo NmpTourInfo;
struct _NmpTourInfo
{
    gint                tour_id;
    gchar             tour_name[TOUR_NAME_LEN];
    gint                auto_jump;
};

typedef struct _NmpQueryTour NmpQueryTour;
struct _NmpQueryTour
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:按用户名查询，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryTourRes NmpQueryTourRes;
struct _NmpQueryTourRes
{
    NmpMsgErrCode  code;
    gchar		bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    NmpTourInfo    tour_info[0];
};

typedef struct _NmpDelTour NmpDelTour;
struct _NmpDelTour
{
    gchar		     bss_usr[USER_NAME_LEN];
    gint                tour_id;
};

typedef struct _NmpTourStep NmpTourStep;
struct _NmpTourStep
{
    gint step_no;
    gint interval;
    gchar encoder_guid[MAX_ID_LEN];
    gchar encoder_domain[DOMAIN_ID_LEN];
    gchar gu_name[GU_NAME_LEN];
    gint level;
};

typedef struct _NmpAddTourStep NmpAddTourStep;
struct _NmpAddTourStep
{
    gint tour_id;
    gchar bss_usr[USER_NAME_LEN];
    gint total_num;
    NmpTourStep tour_step[0];
};

typedef struct _NmpTourStepInfo NmpTourStepInfo;
struct _NmpTourStepInfo
{
    gint                step_no;
    gint                interval;
    gchar   encoder_guid[MAX_ID_LEN];
    gchar   encoder_domain[DOMAIN_ID_LEN];
};

typedef struct _NmpQueryTourStep NmpQueryTourStep;
struct _NmpQueryTourStep
{
    gint              req_num;
    gint              start_num;
    gint              tour_id;
    gchar		  bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryTourStepRes NmpQueryTourStepRes;
struct _NmpQueryTourStepRes
{
    NmpMsgErrCode     code;
    gchar		     bss_usr[USER_NAME_LEN];
    gint                 total_num;
    gint                 back_num;
    gint                 tour_id;
    NmpTourStep     tour_step[0];
};

typedef struct _NmpAddGroup NmpAddGroup;
struct _NmpAddGroup
{
    gchar             group_name[GROUP_NAME_LEN];
    gint                tw_id;
    gchar		     bss_usr[USER_NAME_LEN];
};

typedef struct _NmpModifyGroup NmpModifyGroup;
struct _NmpModifyGroup
{
    gint               group_id;
    gchar             group_name[GROUP_NAME_LEN];
    gchar		     bss_usr[USER_NAME_LEN];
};


typedef struct _NmpGroupInfo NmpGroupInfo;
struct _NmpGroupInfo
{
    gint               group_id;
    gchar             group_name[GROUP_NAME_LEN];
    gint               tw_id;
    gchar             tw_name[TW_NAME_LEN];
};

typedef struct _NmpQueryGroup NmpQueryGroup;
struct _NmpQueryGroup
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:查询所有，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryGroupRes NmpQueryGroupRes;
struct _NmpQueryGroupRes
{
    NmpMsgErrCode  code;
    gchar		bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    NmpGroupInfo    group_info[0];
};

typedef struct _NmpDelGroup NmpDelGroup;
struct _NmpDelGroup
{
    gchar		     bss_usr[USER_NAME_LEN];
    gint               group_id;
};

typedef struct _NmpAddGroupStep NmpAddGroupStep;
struct _NmpAddGroupStep
{
    gint     		group_id;
    gint			step_no;
    gint			interval;
    gchar			bss_usr[USER_NAME_LEN];
};

typedef struct _NmpModifyGroupStep NmpModifyGroupStep;
struct _NmpModifyGroupStep
{
    gint			group_id;
    gint			step_no;
    gint			interval;
    gchar		     bss_usr[USER_NAME_LEN];
};


typedef struct _NmpGroupSteps NmpGroupSteps;
struct _NmpGroupSteps
{
    gint                group_id;
    gint			step_no;
    gint			interval;
};

typedef struct _NmpQueryGroupStep NmpQueryGroupStep;
struct _NmpQueryGroupStep
{
    gint               req_num;
    gint               start_num;
    gint               type;   //0:查询所有，1:按类型查询
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryGroupStepRes NmpQueryGroupStepRes;
struct _NmpQueryGroupStepRes
{
    NmpMsgErrCode  code;
    gchar		bss_usr[USER_NAME_LEN];
    gint            total_num;
    gint            back_num;
    gint            type;
    NmpGroupSteps    group_step[0];
};

typedef struct _NmpDelGroupStep NmpDelGroupStep;
struct _NmpDelGroupStep
{
    gchar		      bss_usr[USER_NAME_LEN];
    gint                group_id;
    gint			step_no;
};

typedef struct _NmpGroupStep NmpGroupStep;
struct _NmpGroupStep
{
    gint div_no;
    gchar encoder_guid[MAX_ID_LEN];
    gchar encoder_domain[DOMAIN_ID_LEN];
    gint level;
};

typedef struct _NmpConfigGroupStep NmpConfigGroupStep;
struct _NmpConfigGroupStep
{
    gint group_id;
    gint step_no;
    gint scr_id;
    gint div_id;
    gchar bss_usr[USER_NAME_LEN];
    gint total_num;
    NmpGroupStep group_step[0];
};

typedef struct _NmpGroupStepInfo NmpGroupStepInfo;
struct _NmpGroupStepInfo
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

typedef struct _NmpQueryGroupStepInfo NmpQueryGroupStepInfo;
struct _NmpQueryGroupStepInfo
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

typedef struct _NmpQueryGroupStepInfoRes NmpQueryGroupStepInfoRes;
struct _NmpQueryGroupStepInfoRes
{
    NmpMsgErrCode     code;
    gchar		     bss_usr[USER_NAME_LEN];
    gint                 total_num;
    gint                 back_num;
    gint                 group_id;
    gint scr_id;
    gint div_id;
    NmpGroupStepInfo    group_step[0];
};

typedef struct _NmpModifyGroupStepInfo NmpModifyGroupStepInfo;
struct _NmpModifyGroupStepInfo
{
    gint group_id;
    gint step_no;
    gint scr_id;
    gint level;
    gint div_no;
    //NmpGroupStep step_info;
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpDelGroupStepInfo NmpDelGroupStepInfo;
struct _NmpDelGroupStepInfo
{
	gchar bss_usr[USER_NAME_LEN];
	gint group_id;
	gint step_no;
	gint scr_id;
	gint div_no;
	gint type;
};

typedef struct _NmpQueryGroupStepDiv NmpQueryGroupStepDiv;
struct _NmpQueryGroupStepDiv
{
    gint              group_id;
    gint              scr_id;
    gchar		  bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryGroupStepDivRes NmpQueryGroupStepDivRes;
struct _NmpQueryGroupStepDivRes
{
    NmpMsgErrCode     code;
    gchar		     bss_usr[USER_NAME_LEN];
    gint div_id;
};

typedef struct _NmpLinkTimePolicyConfig NmpLinkTimePolicyConfig;
struct _NmpLinkTimePolicyConfig
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar time_policy[POLICY_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef NmpLinkTimePolicyConfig NmpModifyLinkTimePolicy;

typedef struct _NmpQueryLinkTimePolicy NmpQueryLinkTimePolicy;
struct _NmpQueryLinkTimePolicy
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef NmpLinkTimePolicyConfig NmpQueryLinkTimePolicyRes;

typedef struct _NmpDelLinkTimePolicy NmpDelLinkTimePolicy;
struct _NmpDelLinkTimePolicy
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkRecord NmpLinkRecord;
struct _NmpLinkRecord
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint time_len;
	NmpMssId mss[MSS_NUM];
	gchar bss_usr[USER_NAME_LEN];
};

typedef NmpLinkRecord NmpModifyLinkRecord;

typedef struct _NmpQueryLinkRecord NmpQueryLinkRecord;
struct _NmpQueryLinkRecord
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
typedef struct _NmpLinkRecordInfo NmpLinkRecordInfo;
struct _NmpLinkRecordInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint time_len;
	gchar gu_name[GU_NAME_LEN];
	NmpStoreServer            mss[MSS_NUM];
};

typedef struct _NmpQueryLinkRecordRes NmpQueryLinkRecordRes;
struct _NmpQueryLinkRecordRes
{
	NmpMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	NmpLinkRecordInfo link_record_info[0];
};

typedef struct _NmpDelLinkRecord NmpDelLinkRecord;
struct _NmpDelLinkRecord
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkIO NmpLinkIO;
struct _NmpLinkIO
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

typedef NmpLinkIO NmpModifyLinkIO;

typedef struct _NmpQueryLinkIO NmpQueryLinkIO;
struct _NmpQueryLinkIO
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

typedef struct _NmpLinkIOInfo NmpLinkIOInfo;
struct _NmpLinkIOInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint time_len;
	gchar gu_name[GU_NAME_LEN];
	gchar io_value[IO_VALUE_LEN];
};

typedef struct _NmpQueryLinkIORes NmpQueryLinkIORes;
struct _NmpQueryLinkIORes
{
	NmpMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	NmpLinkIOInfo link_io_info[0];
};

typedef struct _NmpDelLinkIO NmpDelLinkIO;
struct _NmpDelLinkIO
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkSnapshot NmpLinkSnapshot;
struct _NmpLinkSnapshot
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint picture_num;
	NmpMssId mss[MSS_NUM];
	gchar bss_usr[USER_NAME_LEN];
};

typedef NmpLinkSnapshot NmpModifyLinkSnapshot;

typedef struct _NmpQueryLinkSnapshot NmpQueryLinkSnapshot;
struct _NmpQueryLinkSnapshot
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
typedef struct _NmpLinkSnapshotInfo NmpLinkSnapshotInfo;
struct _NmpLinkSnapshotInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint level;
	gint alarm_type;
	gint picture_num;
	gchar gu_name[GU_NAME_LEN];
	NmpStoreServer            mss[MSS_NUM];
};

typedef struct _NmpQueryLinkSnapshotRes NmpQueryLinkSnapshotRes;
struct _NmpQueryLinkSnapshotRes
{
	NmpMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	NmpLinkSnapshotInfo link_snapshot_info[0];
};

typedef struct _NmpDelLinkSnapshot NmpDelLinkSnapshot;
struct _NmpDelLinkSnapshot
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkStepConfig NmpLinkStepConfig;
struct _NmpLinkStepConfig
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

typedef NmpLinkStepConfig NmpModifyLinkStep;

typedef struct _NmpQueryLinkStep NmpQueryLinkStep;
struct _NmpQueryLinkStep
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
typedef struct _NmpLinkStepInfo NmpLinkStepInfo;
struct _NmpLinkStepInfo
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

typedef struct _NmpQueryLinkStepRes NmpQueryLinkStepRes;
struct _NmpQueryLinkStepRes
{
	NmpMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint type;
	gint            total_num;
	gint            back_num;
	NmpLinkStepInfo link_step_info[0];
};

typedef struct _NmpDelLinkStep NmpDelLinkStep;
struct _NmpDelLinkStep
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint tw_id;
	gint screen_id;
	gint div_num;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkPreset NmpLinkPreset;
struct _NmpLinkPreset
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint preset_no;
	gchar bss_usr[USER_NAME_LEN];
};

typedef NmpLinkPreset NmpModifyLinkPreset;

typedef struct _NmpQueryLinkPreset NmpQueryLinkPreset;
struct _NmpQueryLinkPreset
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

typedef struct _NmpLinkPresetInfo NmpLinkPresetInfo;
struct _NmpLinkPresetInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint preset_no;
	gchar gu_name[GU_NAME_LEN];
};

typedef struct _NmpQueryLinkPresetRes NmpQueryLinkPresetRes;
struct _NmpQueryLinkPresetRes
{
	NmpMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	NmpLinkPresetInfo link_preset_info[0];
};

typedef struct _NmpDelLinkPreset NmpDelLinkPreset;
struct _NmpDelLinkPreset
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkTourConfig NmpLinkTourConfig;
struct _NmpLinkTourConfig
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

typedef NmpLinkTourConfig NmpModifyLinkTour;

typedef struct _NmpQueryLinkTour NmpQueryLinkTour;
struct _NmpQueryLinkTour
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

typedef struct _NmpLinkTourInfo NmpLinkTourInfo;
struct _NmpLinkTourInfo
{
	gint tw_id;
	gint screen_id;
	gint div_num;
	gint div_id;
	gint tour_id;
	gint alarm_type;
	gchar tour_name[TOUR_NAME_LEN];
};

typedef struct _NmpQueryLinkTourRes NmpQueryLinkTourRes;
struct _NmpQueryLinkTourRes
{
	NmpMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	NmpLinkTourInfo link_tour_info[0];
};

typedef struct _NmpDelLinkTour NmpDelLinkTour;
struct _NmpDelLinkTour
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint screen_id;
	gint div_num;
	gint div_id;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkGroupConfig NmpLinkGroupConfig;
struct _NmpLinkGroupConfig
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint group_id;
	gint alarm_type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef NmpLinkGroupConfig NmpModifyLinkGroup;

typedef struct _NmpQueryLinkGroup NmpQueryLinkGroup;
struct _NmpQueryLinkGroup
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint group_id;
	gint               req_num;
	gint               start_num;
	gint type;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkGroupInfo NmpLinkGroupInfo;
struct _NmpLinkGroupInfo
{
	gint group_id;
	gint alarm_type;
	gchar group_name[GROUP_NAME_LEN];
};

typedef struct _NmpQueryLinkGroupRes NmpQueryLinkGroupRes;
struct _NmpQueryLinkGroupRes
{
	NmpMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	NmpLinkGroupInfo link_group_info[0];
};

typedef struct _NmpDelLinkGroup NmpDelLinkGroup;
struct _NmpDelLinkGroup
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gint group_id;
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpLinkMap NmpLinkMap;
struct _NmpLinkMap
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint level;
	gchar bss_usr[USER_NAME_LEN];
};

typedef NmpLinkMap NmpModifyLinkMap;

typedef struct _NmpQueryLinkMap NmpQueryLinkMap;
struct _NmpQueryLinkMap
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

typedef struct _NmpLinkMapInfo NmpLinkMapInfo;
struct _NmpLinkMapInfo
{
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gint alarm_type;
	gint level;
	gchar gu_name[GU_NAME_LEN];
};

typedef struct _NmpQueryLinkMapRes NmpQueryLinkMapRes;
struct _NmpQueryLinkMapRes
{
	NmpMsgErrCode     code;
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
	gint            total_num;
	gint            back_num;
	NmpLinkMapInfo link_map_info[0];
};

typedef struct _NmpDelLinkMap NmpDelLinkMap;
struct _NmpDelLinkMap
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar link_guid[MAX_ID_LEN];
	gchar link_domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryServerResourceInfo NmpQueryServerResourceInfo;
struct _NmpQueryServerResourceInfo
{
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryServerResourceInfoRes NmpQueryServerResourceInfoRes;
struct _NmpQueryServerResourceInfoRes
{
	NmpMsgErrCode     code;
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
	NmpExpiredTime expired_time;
};

typedef NmpQueryServerResourceInfo NmpQueryTwAuthInfo;

typedef struct _NmpQueryTwAuthInfoRes NmpQueryTwAuthInfoRes;
struct _NmpQueryTwAuthInfoRes
{
	NmpMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint   tw_auth_type;
};

typedef NmpQueryServerResourceInfo NmpQueryAlarmLinkAuthInfo;

typedef struct _NmpQueryAlarmLinkAuthInfoRes NmpQueryAlarmLinkAuthInfoRes;
struct _NmpQueryAlarmLinkAuthInfoRes
{
	NmpMsgErrCode     code;
	gchar bss_usr[USER_NAME_LEN];
	gint   alarm_link_auth_type;
};

typedef struct __search_pu_list
{
    int count;                          //条目
    search_result_t result[0];
}search_pu_list;

typedef struct _NmpSearchPuRes NmpSearchPuRes;
struct _NmpSearchPuRes
{
	NmpMsgErrCode     code;
	gint  pu_count;
};

typedef struct _NmpGetSearchedPu NmpGetSearchedPu;
struct _NmpGetSearchedPu
{
	gint               req_num;
	gint               start_num;
};

typedef struct _NmpGetSearchedPuRes NmpGetSearchedPuRes;
struct _NmpGetSearchedPuRes
{
	NmpMsgErrCode     code;
	gint  pu_count;  //设备总个数
	gint  res_count;  // 返回的设备个数
	search_result_t  search_pu[0];
};

typedef struct _NmpCmsIp NmpCmsIp;
struct _NmpCmsIp
{
	gchar ip[MAX_IP_LEN];
};

typedef struct _NmpQueryCmsAllIpRes NmpQueryCmsAllIpRes;
struct _NmpQueryCmsAllIpRes
{
	NmpMsgErrCode     code;
	gint count;
	NmpCmsIp ips[0];
};

typedef struct _NmpAutoAddPu NmpAutoAddPu;
struct _NmpAutoAddPu
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

typedef struct _NmpGetNextPuNo NmpGetNextPuNo;
struct _NmpGetNextPuNo
{
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpGetNextPuNoRes NmpGetNextPuNoRes;
struct _NmpGetNextPuNoRes
{
	NmpMsgErrCode     code;
	gchar pu_no[PU_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpGetInitName NmpGetInitName;
struct _NmpGetInitName
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpGetInitNameRes NmpGetInitNameRes;
struct _NmpGetInitNameRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
    gchar			   init_name[USER_NAME_LEN];
};

typedef struct _NmpSetInitName NmpSetInitName;
struct _NmpSetInitName
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			   init_name[USER_NAME_LEN];
    gchar			   session[USER_NAME_LEN];
};

typedef struct  _NmpSetInitNameRes NmpSetInitNameRes;
struct _NmpSetInitNameRes
{
    NmpMsgErrCode     code;
    gchar			   session[USER_NAME_LEN];
};

typedef NmpGetInitName NmpGetIpsanInfo;

typedef struct  _NmpIpsanInfo NmpIpsanInfo;
struct _NmpIpsanInfo
{
    gchar			   ip[MAX_IP_LEN];
    gint			   port;
};

typedef struct  _NmpGetIpsanInfoRes NmpGetIpsanInfoRes;
struct _NmpGetIpsanInfoRes
{
    NmpMsgErrCode     code;
    gchar			session[USER_NAME_LEN];
    gint			count;
    NmpIpsanInfo	ipsan_info[0];
};

typedef struct  _NmpAddOneIpsan NmpAddOneIpsan;
struct _NmpAddOneIpsan
{
    gchar   mss_id[MSS_ID_LEN];
    gchar			session[USER_NAME_LEN];
    NmpIpsanInfo	ipsan_info;
};

typedef NmpSetInitNameRes NmpAddOneIpsanRes;

typedef NmpAddOneIpsan NmpDeleteOneIpsan;

typedef NmpSetInitNameRes NmpDeleteOneIpsanRes;

typedef NmpAddOneIpsan NmpGetOneIpsanDetail;

typedef struct _NmpGetOneIpsanDetailRes NmpGetOneIpsanDetailRes;
struct _NmpGetOneIpsanDetailRes
{
    NmpMsgErrCode     code;
    gchar			session[USER_NAME_LEN];
    gchar              target[USER_NAME_LEN];
    gint			connect_state;
};

/*typedef struct _NmpAddGuToUserList NmpAddGuToUserList;
struct _NmpAddGuToUserList
{
    struct  list_head list;
    gint cur_num;
    gint total_num;
    gint page;
    gchar username[USER_NAME_LEN];
    NmpAddGuToUser add_gu_to_user_head;
};

typedef struct _NmpAddGuToUserAllList NmpAddGuToUserAllList;
struct _NmpAddGuToUserAllList
{
    NmpAddGuToUserList add_gu_to_user_list_head;
    pthread_mutex_t  conn_lock;
};
*/

typedef struct _NmpQueryLog NmpQueryLog;
struct _NmpQueryLog
{
	gint			order_by;
	gint			req_count;
	gint			start_num;
	gint			log_level;
	gchar		start_time[TIME_INFO_LEN];
	gchar		end_time[TIME_INFO_LEN];
	gchar		bss_usr[USER_NAME_LEN];
};

typedef struct _NmpBssLog NmpBssLog;
struct _NmpBssLog
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

typedef struct _NmpQueryLogRes NmpQueryLogRes;
struct _NmpQueryLogRes
{
	NmpMsgErrCode	code;
	gchar		bss_usr[USER_NAME_LEN];
	gint			total_num;
	gint			req_total;
	NmpBssLog	log_list[0];
};

typedef struct _NmpQueryAreaDevRate NmpQueryAreaDevRate;
struct _NmpQueryAreaDevRate
{
    gint              area_id;
    gint              type;
    gchar            key[TIME_LEN];
    gint			req_num;
	gint			start_num;
    gchar		  bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAreaDevRate NmpAreaDevRate;
struct _NmpAreaDevRate
{
	gint              area_id;
	gchar             area_name[AREA_NAME_LEN];
	gdouble           total_count;
	gdouble           online_count;
	gdouble rate;
};

typedef struct _NmpQueryAreaDevRateRes NmpQueryAreaDevRateRes;
struct _NmpQueryAreaDevRateRes
{
    NmpMsgErrCode     code;
    gchar		     bss_usr[USER_NAME_LEN];
    gint			total_num;
	gint			back_num;
	NmpAreaDevRate	area_dev_rate[0];
};

typedef struct _NmpValidateGuMap NmpValidateGuMap;
struct _NmpValidateGuMap
{
	gchar guid[MAX_ID_LEN];
	gchar domain[DOMAIN_ID_LEN];
	gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpAddIvs NmpAddIvs;
struct _NmpAddIvs
{
    gchar   ivs_id[IVS_ID_LEN];
    gchar   ivs_name[IVS_NAME_LEN];
    gint      keep_alive_freq;
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpIvsInfo  NmpIvsInfo;
struct _NmpIvsInfo
{
    gint     type;  // 0: 修改mds ; 1:修改get_ip_enable
    gchar   ivs_id[IVS_ID_LEN];
    gchar   ivs_name[IVS_NAME_LEN];
    gint      keep_alive_freq;
    gint      ivs_state;
    gchar   ivs_last_ip[MAX_IP_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryIvs NmpQueryIvs;
struct _NmpQueryIvs
{
    gint              req_num;
    gint              start_num;
    gint               type;   //0:按用户名查询，1:按类型查询, 2:查询自动添加mds ip使能状态
    gchar            key[USER_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};

typedef struct _NmpQueryIvsRes NmpQueryIvsRes;
struct _NmpQueryIvsRes
{
    NmpMsgErrCode     code;
    gchar bss_usr[USER_NAME_LEN];
    gint               type;
    gint                     total_num;
    gint                     req_num;
    NmpIvsInfo          ivs_info[0];
};


typedef struct _NmpDelIvs NmpDelIvs;
struct _NmpDelIvs
{
    gchar  ivs_id[MULTI_NAME_LEN];
    gchar bss_usr[USER_NAME_LEN];
};


typedef struct _NmpGetServerFlagRes NmpGetServerFlagRes;
struct _NmpGetServerFlagRes
{
	NmpMsgErrCode		code;
	gint					server_flag;
};


typedef struct _NmpGetMdsConfigRes NmpGetMdsConfigRes;
struct _NmpGetMdsConfigRes
{
	NmpMsgErrCode		code;
	gchar				mds_id[MDS_ID_LEN];
	gint					state;
	gint					start_port;
	gint					end_port;
};


typedef struct _NmpSetMdsConfig NmpSetMdsConfig;
struct _NmpSetMdsConfig
{
	gchar				mds_id[MDS_ID_LEN];
	gint					start_port;
	gint					end_port;
};


typedef struct _NmpGetMdsState NmpGetMdsState;
struct _NmpGetMdsState
{
	gchar				mds_id[MDS_ID_LEN];
};

typedef struct _NmpGetMdsStateRes NmpGetMdsStateRes;
struct _NmpGetMdsStateRes
{
	NmpMsgErrCode		code;
	gint					state;
};


typedef struct _NmpGetMssConfigRes NmpGetMssConfigRes;
struct _NmpGetMssConfigRes
{
	NmpMsgErrCode		code;
	gchar				mss_id[MSS_ID_LEN];
	gchar				stor_type[MAX_STOR_TYPE_LEN];
	gint					state;
};


typedef struct _NmpSetMssConfig NmpSetMssConfig;
struct _NmpSetMssConfig
{
	gchar				mss_id[MSS_ID_LEN];
	gchar				stor_type[MAX_STOR_TYPE_LEN];
};


typedef struct _NmpGetMssState NmpGetMssState;
struct _NmpGetMssState
{
	gchar				mss_id[MDS_ID_LEN];
};

typedef struct _NmpGetMssStateRes NmpGetMssStateRes;
struct _NmpGetMssStateRes
{
	NmpMsgErrCode		code;
	gint					state;
};


typedef struct _NmpGetIvsConfigRes NmpGetIvsConfigRes;
struct _NmpGetIvsConfigRes
{
	NmpMsgErrCode		code;
	gchar				ivs_id[IVS_ID_LEN];
};


typedef struct _NmpSetIvsConfig NmpSetIvsConfig;
struct _NmpSetIvsConfig
{
	gchar				ivs_id[IVS_ID_LEN];
};


typedef struct _NmpGetIvsState NmpGetIvsState;
struct _NmpGetIvsState
{
	gchar				ivs_id[IVS_ID_LEN];
};

typedef struct _NmpGetIvsStateRes NmpGetIvsStateRes;
struct _NmpGetIvsStateRes
{
	NmpMsgErrCode		code;
	gint					state;
};


typedef struct _NmpAmsId NmpAmsId;
struct _NmpAmsId
{
	gchar				ams_id[AMS_ID_LEN];
};


typedef struct _NmpAddAms NmpAddAms;
struct _NmpAddAms
{
	gchar				ams_id[AMS_ID_LEN];
	gchar				ams_name[AMS_NAME_LEN];
	gint					keep_alive_freq;
	gchar				bss_usr[USER_NAME_LEN];
};


typedef NmpAddAms NmpModifyAms;


typedef struct _NmpQueryAms NmpQueryAms;
struct _NmpQueryAms
{
	gchar				ams_id[AMS_ID_LEN];
	gint					type;
	gint					req_num;
	gint					start_num;
	gchar				bss_usr[USER_NAME_LEN];
};


typedef struct _NmpAmsInfo NmpAmsInfo;
struct _NmpAmsInfo
{
	gchar				ams_id[AMS_ID_LEN];
	gchar				ams_name[AMS_NAME_LEN];
	gint					keep_alive_freq;
	gint					ams_state;
	gchar				ams_ip[MAX_IP_LEN];
};

typedef struct _NmpQueryAmsRes NmpQueryAmsRes;
struct _NmpQueryAmsRes
{
	NmpMsgErrCode		code;
	gchar				bss_usr[USER_NAME_LEN];
	gint					total_count;
	gint					back_count;
	NmpAmsInfo			ams_info[0];
};


typedef struct _NmpDelAms NmpDelAms;
struct _NmpDelAms
{
	gchar				ams_id[AMS_ID_LEN];
	gchar				bss_usr[USER_NAME_LEN];
};


typedef struct _NmpQueryAmsPu NmpQueryAmsPu;
struct _NmpQueryAmsPu
{
	gchar				ams_id[AMS_ID_LEN];
	gint					req_num;
	gint					start_num;
	gchar				bss_usr[USER_NAME_LEN];
};


typedef struct _NmpAmsPuInfo NmpAmsPuInfo;
struct _NmpAmsPuInfo
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

typedef struct _NmpQueryAmsPuRes NmpQueryAmsPuRes;
struct _NmpQueryAmsPuRes
{
	NmpMsgErrCode		code;
	gchar				bss_usr[USER_NAME_LEN];
	gint					total_count;
	gint					back_count;
	NmpAmsPuInfo			pu_info[0];
};


typedef struct _NmpModifyAmsPu NmpModifyAmsPu;
struct _NmpModifyAmsPu
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
