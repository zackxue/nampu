/********************************************************************
 * nmp_pu_xml.h  - deal xml of pu, parse and create xml
 * Function£ºparse or create xml relate to pu. 
 * Author:yangy
 * Description:users can add parse or create message of pu,define 
 *             struct about pu information
 * History: 
 * 2012.04.13 - Yang Ying, initiate to create;
 ********************************************************************/
 
#ifndef __NMP_CMS_XML__
#define __NMP_CMS_XML__

//#include "nmp_xml_data_type.h"
#include <glib.h>
#include "nmp_xml_fun.h"	

#define MAX_GU_ENTRIES             16

#define MSS_REGISTER_CMS            "MssRegister"
#define MSS_REGISTER_CMS_RESP       "MssRegisterResponse"
#define MSS_HEART                   "MssHeart"
#define MSS_HEART_RESP              "MssHeartResponse"
#define MSS_GET_GUID                "MssGetGuid"
#define MSS_GET_GUID_RESP           "MssGetGuidResponse"
#define MSS_GET_RECORD_POLICY       "MssGetRecordPolicy"
#define MSS_GET_RECORD_POLICY_RESP  "MssGetRecordPolicyResponse"
#define MSS_GET_ROUTE               "MssGetRoute"
#define MSS_GET_ROUTE_RESP          "MssGetRouteResponse"
#define MSS_GET_MDS                 "MssGetMds"
#define MSS_GET_MDS_RESP            "MssGetMdsResponse"
#define MSS_GET_MDS_IP              "MssGetMdsIp"
#define MSS_GET_MDS_IP_RESP         "MssGetMdsIpResponse"
#define MSS_NOTIFY_POLICY_CHANGE    "NotifytRecordPolicyChange"
#define MSS_GU_LIST_CHANGE          "GuListChange"

#define QUERY_RECORD_STATUS	        "QueryGuRecordStatus"
#define QUERY_RECORD_STATUS_RES     "QueryGuRecordStatusResponse"
#define ADD_HD_GROUP                "AddHdGroup"
#define ADD_HD_GROUP_RES            "AddHdGroupResponse"
#define ADD_HD_TO_GROUP             "AddHdToGroup"
#define ADD_HD_TO_GROUP_RES         "AddHdToGroupResponse"
#define DEL_HD_FROM_GROUP           "DelHdFromGroup"
#define DEL_HD_FROM_GROUP_RES       "DelHdFromGroupResponse"
#define QUERY_ALL_HD_GROUP          "QueryAllHdGroup"
#define QUERY_ALL_HD_GROUP_RES      "QueryAllHdGroupResponse"
#define QUERY_HD_GROUP_INFO         "QueryHdGroupInfo"
#define QUERY_HD_GROUP_INFO_RES     "QueryHdGroupInfoResponse"
#define QUERY_ALL_HD                "QueryAllHd"
#define QUERY_ALL_HD_RES            "QueryAllHdResponse"
#define DEL_HD_GROUP                "DelHdGroup"
#define DEL_HD_GROUP_RES            "DelHdGroupResponse"
#define GET_HD_FORMAT_PROGRESS      "GetHdFormatProgress"
#define GET_HD_FORMAT_PROGRESS_RES  "GetHdFormatProgressResponse"
#define GET_STORE_LOG               "GetMssStoreLog"
#define GET_STORE_LOG_RES           "GetMssStoreLogResponse"
#define MSS_NOTIFY_MSS_CHANGE       "NotifyModifyMss"
#define ADD_ONE_IPSAN			"AddOneIpsan"
#define ADD_ONE_IPSAN_RES			"AddOneIpsanResponse"
#define GET_IPSANS				"GetIpsanInfo"
#define GET_IPSANS_RES				"GetIpsanInfoResponse"
#define SET_IPSANS				"SetIpsanInfo"
#define SET_IPSANS_RES				"SetIpsanInfoResponse"
#define GET_INITIATOR_NAME		"GetInitiatorName"
#define GET_INITIATOR_NAME_RES		"GetInitiatorNameResponse"
#define SET_INITIATOR_NAME		"SetInitiatorName"
#define SET_INITIATOR_NAME_RES		"SetInitiatorNameResponse"
#define GET_ONE_IPSAN_DETAIL	"GetOneIpsanDetail"
#define GET_ONE_IPSAN_DETAIL_RES	"GetOneIpsanDetailResponse"
#define DEL_ONE_IPSAN			"DeleteOneIpsan"
#define DEL_ONE_IPSAN_RES			"DeleteOneIpsanResponse"
#define NOTIFY_MESSAGE			"NotifyMessage"
#define ALARM_LINK_RECORD			"AlarmLinkRecord"
#define REBOOT_MSS				"RebootMss"
#define REBOOT_MSS_RES				"RebootMssResponse"


NmpMsgInfo *  
nmp_parse_mss_register_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int 
nmp_create_mss_register(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_mss_heart_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_mss_heart(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_mss_get_guid_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_mss_get_guid(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_get_record_policy_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_get_record_policy(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_get_route_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_get_route(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_notify_policy_change(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

NmpMsgInfo * 
nmp_parse_mss_gu_list_change(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

NmpMsgInfo * 
nmp_parse_get_mds_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_get_mds(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_get_mds_ip_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_get_mds_ip(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_query_record_status(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_query_record_status_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_add_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_add_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_add_hd_to_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_add_hd_to_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_del_hd_from_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_del_hd_from_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_query_all_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_query_all_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_query_hd_group_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_query_hd_group_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_query_all_hd(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_query_all_hd_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_del_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_del_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_get_hd_format_progress(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_get_hd_format_progress_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *  
nmp_parse_get_store_log(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_get_store_log_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo * 
nmp_parse_notify_mss_change(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

NmpMsgInfo *  
nmp_parse_add_one_ipsan(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_add_one_ipsan_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *  
nmp_parse_get_ipsans(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_get_ipsans_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *  
nmp_parse_set_ipsans(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_set_ipsans_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *  
nmp_parse_get_initiator_name(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_get_initiator_name_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *  
nmp_parse_set_initiator_name(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_set_initiator_name_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *  
nmp_parse_get_ipsan_detail(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_get_ipsan_detail_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *  
nmp_parse_del_one_ipsan(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_del_one_ipsan_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

gint
nmp_create_notify_message(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *  
nmp_parse_alarm_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

NmpMsgInfo *  
nmp_parse_reboot_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

gint
nmp_create_reboot_mss_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

#endif

