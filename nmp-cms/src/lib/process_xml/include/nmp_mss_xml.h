/********************************************************************
 * jpf_pu_xml.h  - deal xml of pu, parse and create xml
 * Function£ºparse or create xml relate to pu.
 * Author:yangy
 * Description:users can add parse or create message of pu,define
 *             struct about pu information
 * History:
 * 2012.04.16 - Yang Ying, initiate to create;
 ********************************************************************/

#ifndef __NMP_MSS_XML__
#define __NMP_MSS_XML__

#include "nmp_xml_fun.h"

#define MSS_REGISTER_CMS				"MssRegister"
#define MSS_REGISTER_CMS_RESP			"MssRegisterResponse"
#define MSS_HEART						"MssHeart"
#define MSS_HEART_RESP					"MssHeartResponse"
#define MSS_GET_GUID					"MssGetGuid"
#define MSS_GET_GUID_RESP				"MssGetGuidResponse"
#define MSS_GET_RECORD_POLICY			"MssGetRecordPolicy"
#define MSS_GET_RECORD_POLICY_RESP	"MssGetRecordPolicyResponse"
#define MSS_NOTIFY_POLICY_CHANGE		"NotifytRecordPolicyChange"
#define MSS_GU_LIST_CHANGE				"GuListChange"
#define MSS_GET_ROUTE					"MssGetRoute"
#define MSS_GET_ROUTE_RESP				"MssGetRouteResponse"
#define MSS_GET_MDS                                "MssGetMds"
#define MSS_GET_MDS_RESP				"MssGetMdsResponse"
#define MSS_GET_MDS_IP					"MssGetMdsIp"
#define MSS_GET_MDS_IP_RESP			"MssGetMdsIpResponse"
#define MSS_NOTIFY_CHANGE_MSS		"NotifyModifyMss"
#define MSS_ALARM_LINK_RECORD			"AlarmLinkRecord"
#define MSS_ALARM_LINK_SNAPSHOT			"AlarmLinkSnapshot"

JpfMsgInfo *
jpf_parse_mss_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_register_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_heart_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_guid(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_guid_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_record_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_record_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_mss_notify_policy_change_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_mss_gu_list_change(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_route(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_route_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_mds_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_mds_ip_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_add_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_add_hd_group(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_add_hd_to_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_add_hd_to_group(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_del_hd_from_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_del_hd_from_group(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_reboot_mss_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_reboot_mss(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_query_all_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_query_all_hd_group(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_query_hd_group_info_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_query_hd_group_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_query_all_hd_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_query_all_hd(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_del_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_del_hd_group(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_hd_format_progress_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_hd_format_progress(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_mss_query_gu_record_status(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_query_gu_record_status_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

JpfMsgInfo *
jpf_parse_mss_get_store_log_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_store_log(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_mss_notify_mss_change_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_mss_alarm_link_record_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

int
jpf_create_mss_alarm_link_snapshot_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_initiator_name_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_initiator_name(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_set_initiator_name_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_set_initiator_name(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_ipsan_info_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_ipsan_info(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_add_one_ipsan_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_add_one_ipsan(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_delete_one_ipsan_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_delete_one_ipsan(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_get_one_ipsan_detail_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
jpf_create_mss_get_one_ipsan_detail(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo *
jpf_parse_mss_notify_message(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

#endif  //__NMP_MSS_XML__

