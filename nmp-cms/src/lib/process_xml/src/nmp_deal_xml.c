
/**
 * @file     nmp_deal_xml.c
 * @author   Yang Ying
 * @section  LICENSE
 *
 * Copyright by Shenzhen NMP Electronic Co.Ltd, 2011.
 * Website: www.sznmp.com
 *
 * @section  DESCRIPTION
 *
 * 1. nmp_cmd_type_register
 *    Every new message must be registered before using
 *    it. this function can finish it.
 *
 * 2. nmp_init_cmd_id
 *    When adding a new message id, user only need to expand
 *    this function look piece "to do ...." in it.
 *
 * 3. nmp_deal_cmd:
 *    Used to find a specific command in XML document, if
 *    existed in XML, then call its processing function.
 *
 * history
 * 2011.06.03 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 * 2011.06.09 - Zhang Shiyong, add file description and code comments;
 */
#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>

//#include "nmp_xml_fun.h"

#include "nmp_cmd_type.h"
#include "nmp_pu_xml.h"
#include "nmp_cu_xml.h"
#include "nmp_bss_xml.h"
#include "nmp_mss_xml.h"
#include "nmp_mds_xml.h"
#include "nmp_xml_fun.h"
#include "nmp_ivs_xml.h"
#include "nmp_ams_xml.h"

#define CMD_ID_LEN              32
#define NODE_VALUE_LEN          64
#define ONLINE_RATE_FLAG 1
NmpCmdType nmp_cmd_sets;

/**
 * nmp_cmd_type_register: Register a new command type in platform
 *
 * @self:   current command set of platform
 * @id:     new command id
 * @p_xml:  callback function to process XML
 * @c_xml:  callback function to generate XML for the new command type
 * @return: succeed 0, else -1
 */
int
nmp_cmd_type_register(
	NmpCmdType      *self,
    NmpCmdID        id,
	NmpParseXml     p_xml,
    NmpCreateXml    c_xml
)
{
	if (self->counts >= MAX_CMD_ENTRIES)
	{
		xml_error("command number over max number\n");
		return -1;  //return ENOMEM ;
	}

	CMD_SET(self->cmd_enties[self->counts].cmd_id, id);
	GET_PARSE_XML_BYINDEX(self,self->counts)    = p_xml;
	GET_CREATE_XML_BYINDEX(self,self->counts)   = c_xml;

	++self->counts;

	return 0;
}

/**
 * nmp_init_cmd_id: initiate command set of platform
 *
 * When platform startup, firstly, invoke this function to initiate
 * all commands in command set
 *
 * @self: platform's command set
 * @return none
 */
void
nmp_init_cmd_id(NmpCmdType *self)
{
	//memset(self, 0 ,sizeof(NmpCmdType));
    nmp_cmd_type_register(self,
        PU_REGISTER_CMS,
        nmp_parse_pu_register,
        NULL);

    nmp_cmd_type_register(self,
        PU_REGISTER_CMS_RESP,
        NULL,
        nmp_create_pu_register_resp);

    nmp_cmd_type_register(self,
        PU_HEART,
        nmp_parse_pu_heart,
        NULL);

    nmp_cmd_type_register(self,
        PU_HEART_RESP,
        NULL,
        nmp_create_pu_heart_resp);

    nmp_cmd_type_register(self,
        PU_CHANGE_DISPATCH,
        NULL,
        nmp_create_pu_change_dispatch);

    nmp_cmd_type_register(self,
        PU_GET_MDS_INFO,
        nmp_parse_pu_get_mds_info,
        NULL);

    nmp_cmd_type_register(self,
        PU_GET_MDS_INFO_RESP,
        NULL,
        nmp_create_pu_get_mds_info_resp);

    nmp_cmd_type_register(self,
        MDS_REGISTER_CMS,
        nmp_parse_mds_register,
        NULL);

    nmp_cmd_type_register(self,
        MDS_REGISTER_CMS_RESP,
        NULL,
        nmp_create_mds_register_resp);

    nmp_cmd_type_register(self,
        MDS_HEART,
        nmp_parse_mds_heart,
        NULL);

    nmp_cmd_type_register(self,
        MDS_HEART_RESP,
        NULL,
        nmp_create_mds_heart_resp);

    nmp_cmd_type_register(self,
        MSS_REGISTER_CMS,
        nmp_parse_mss_register,
        NULL);

    nmp_cmd_type_register(self,
        MSS_REGISTER_CMS_RESP,
        NULL,
        nmp_create_mss_register_resp);

    nmp_cmd_type_register(self,
        MSS_HEART,
        nmp_parse_mss_heart,
        NULL);

    nmp_cmd_type_register(self,
        MSS_HEART_RESP,
        NULL,
        nmp_create_mss_heart_resp);

    //begin register cu command
    nmp_cmd_type_register(self,
        CU_LOGIN,
        nmp_parse_cu_login,
        NULL);

    nmp_cmd_type_register(self,
        CU_LOGIN_RESP,
        NULL,
        nmp_create_cu_login_resp);

    nmp_cmd_type_register(self,
        CU_HEART,
        nmp_parse_cu_heart,
        NULL);

    nmp_cmd_type_register(self,
        CU_HEART_RESP,
        NULL,
        nmp_create_cu_heart_resp);

    nmp_cmd_type_register(self,
        CMS_FORCE_USR_OFFLINE,
        NULL,
        nmp_create_force_usr_offline);

    nmp_cmd_type_register(self,
        BSS_LOGIN,
        nmp_parse_bss_login,
        NULL);

    nmp_cmd_type_register(self,
        BSS_LOGIN_RES,
        NULL,
        nmp_create_bss_login_resp);

    nmp_cmd_type_register(self,
        BSS_HEART,
        nmp_parse_bss_heart,
        NULL);

    nmp_cmd_type_register(self,
        BSS_HEART_RES,
        NULL,
        nmp_create_bss_heart_resp);

    nmp_cmd_type_register(self,
        ADD_ADMIN,
        nmp_parse_add_admin,
        NULL);

    nmp_cmd_type_register(self,
        ADD_ADMIN_RES,
        NULL,
        nmp_create_add_admin_resp);

    nmp_cmd_type_register(self,
        MODIFY_ADMIN,
        nmp_parse_modify_admin,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_ADMIN_RES,
        NULL,
        nmp_create_modify_admin_resp);

    nmp_cmd_type_register(self,
        DEL_ADMIN,
        nmp_parse_del_admin,
        NULL);

    nmp_cmd_type_register(self,
        DEL_ADMIN_RES,
        NULL,
        nmp_create_del_admin_resp);

    nmp_cmd_type_register(self,
        QUERY_ADMIN,
        nmp_parse_query_admin,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_ADMIN_RES,
        NULL,
        nmp_create_query_admin_resp);

    nmp_cmd_type_register(self,
        VALIDATA_ADMIN,
        nmp_parse_validata_admin,
        NULL);

    nmp_cmd_type_register(self,
        VALIDATA_ADMIN_RES,
        NULL,
        nmp_create_validata_admin_resp);

    nmp_cmd_type_register(self,
        ADD_USER_GROUP,
        nmp_parse_add_user_group,
        NULL);

    nmp_cmd_type_register(self,
        ADD_USER_GROUP_RES,
        NULL,
        nmp_create_add_user_group_resp);

    nmp_cmd_type_register(self,
        VALIDATA_USER_GROUP,
        nmp_parse_validata_user_group,
        NULL);

    nmp_cmd_type_register(self,
        VALIDATA_USER_GROUP_RES,
        NULL,
        nmp_create_validata_user_group_resp);

    nmp_cmd_type_register(self,
        QUERY_USER_GROUP,
        nmp_parse_query_user_group,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_USER_GROUP_RES,
        NULL,
        nmp_create_query_user_group_resp);

    nmp_cmd_type_register(self,
        MODIFY_USER_GROUP,
        nmp_parse_modify_user_group,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_USER_GROUP_RES,
        NULL,
        nmp_create_modify_user_group_resp);

    nmp_cmd_type_register(self,
        DEL_USER_GROUP,
        nmp_parse_del_user_group,
        NULL);

    nmp_cmd_type_register(self,
        DEL_USER_GROUP_RES,
        NULL,
        nmp_create_del_user_group_resp);

    nmp_cmd_type_register(self,
        ADD_USER,
        nmp_parse_add_user,
        NULL);

    nmp_cmd_type_register(self,
        ADD_USER_RES,
        NULL,
        nmp_create_add_user_resp);

    nmp_cmd_type_register(self,
        VALIDATA_USER,
        nmp_parse_validata_user,
        NULL);

    nmp_cmd_type_register(self,
        VALIDATA_USER_RES,
        NULL,
        nmp_create_validata_user_resp);

    nmp_cmd_type_register(self,
        QUERY_USER,
        nmp_parse_query_user,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_USER_RES,
        NULL,
        nmp_create_query_user_resp);

    nmp_cmd_type_register(self,
        MODIFY_USER,
        nmp_parse_modify_user,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_USER_RES,
        NULL,
        nmp_create_modify_user_resp);

    nmp_cmd_type_register(self,
        DEL_USER,
        nmp_parse_del_user,
        NULL);

    nmp_cmd_type_register(self,
        DEL_USER_RES,
        NULL,
        nmp_create_del_user_resp);

    nmp_cmd_type_register(self,
        QUERY_DOMAIN,
        nmp_parse_query_domain,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_DOMAIN_RES,
        NULL,
        nmp_create_query_domain_resp);

    nmp_cmd_type_register(self,
        MODIFY_DOMAIN,
        nmp_parse_modify_domain,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_DOMAIN_RES,
        NULL,
        nmp_create_modify_domain_resp);

    nmp_cmd_type_register(self,
        QUERY_AREA,
        nmp_parse_query_area,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_AREA_RES,
        NULL,
        nmp_create_query_area_resp);


    nmp_cmd_type_register(self,
        ADD_MODIFY_AREA,
        nmp_parse_add_modify_area,
        NULL);

    nmp_cmd_type_register(self,
        ADD_MODIFY_AREA_RES,
        NULL,
        nmp_create_add_modify_area_resp);

    nmp_cmd_type_register(self,
        DEL_AREA,
        nmp_parse_del_area,
        NULL);

    nmp_cmd_type_register(self,
        DEL_AREA_RES,
        NULL,
        nmp_create_del_area_resp);

    nmp_cmd_type_register(self,
        ADD_PU,
        nmp_parse_add_pu,
        NULL);

    nmp_cmd_type_register(self,
        ADD_PU_RES,
        NULL,
        nmp_create_add_pu_resp);

    nmp_cmd_type_register(self,
        MODIFY_PU,
        nmp_parse_modify_pu,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_PU_RES,
        NULL,
        nmp_create_modify_pu_resp);

    nmp_cmd_type_register(self,
        VALIDATA_PU,
        nmp_parse_validata_pu,
        NULL);

    nmp_cmd_type_register(self,
        VALIDATA_PU_RES,
        NULL,
        nmp_create_validata_pu_resp);

    nmp_cmd_type_register(self,
        QUERY_PU,
        nmp_parse_query_pu,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_PU_RES,
        NULL,
        nmp_create_query_pu_resp);

    nmp_cmd_type_register(self,
        DEL_PU,
        nmp_parse_del_pu,
        NULL);

    nmp_cmd_type_register(self,
        DEL_PU_RES,
        NULL,
        nmp_create_del_pu_resp);

    nmp_cmd_type_register(self,
        ADD_GU,
        nmp_parse_add_gu,
        NULL);

    nmp_cmd_type_register(self,
        ADD_GU_RES,
        NULL,
        nmp_create_add_gu_resp);

    nmp_cmd_type_register(self,
        QUERY_GU,
        nmp_parse_query_gu,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_GU_RES,
        NULL,
        nmp_create_query_gu_resp);

    nmp_cmd_type_register(self,
        MODIFY_GU,
        nmp_parse_modify_gu,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_GU_RES,
        NULL,
        nmp_create_modify_gu_resp);

    nmp_cmd_type_register(self,
        DEL_GU,
        nmp_parse_del_gu,
        NULL);

    nmp_cmd_type_register(self,
        DEL_GU_RES,
        NULL,
        nmp_create_del_gu_resp);

    nmp_cmd_type_register(self,
        ADD_MDS,
        nmp_parse_add_mds,
        NULL);

    nmp_cmd_type_register(self,
        ADD_MDS_RES,
        NULL,
        nmp_create_add_mds_resp);

    nmp_cmd_type_register(self,
        QUERY_MDS,
        nmp_parse_query_mds,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_MDS_RES,
        NULL,
        nmp_create_query_mds_resp);


    nmp_cmd_type_register(self,
        MODIFY_MDS,
        nmp_parse_modify_mds,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_MDS_RES,
        NULL,
        nmp_create_modify_mds_resp);

    nmp_cmd_type_register(self,
        DEL_MDS,
        nmp_parse_del_mds,
        NULL);

    nmp_cmd_type_register(self,
        DEL_MDS_RES,
        NULL,
        nmp_create_del_mds_resp);

    nmp_cmd_type_register(self,
        ADD_MDS_IP,
        nmp_parse_add_mds_ip,
        NULL);

    nmp_cmd_type_register(self,
        ADD_MDS_IP_RES,
        NULL,
        nmp_create_add_mds_ip_resp);

    nmp_cmd_type_register(self,
        QUERY_MDS_IP,
        nmp_parse_query_mds_ip,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_MDS_IP_RES,
        NULL,
        nmp_create_query_mds_ip_resp);

    nmp_cmd_type_register(self,
        DEL_MDS_IP,
        nmp_parse_del_mds_ip,
        NULL);

    nmp_cmd_type_register(self,
        DEL_MDS_IP_RES,
        NULL,
        nmp_create_del_mds_ip_resp);

    nmp_cmd_type_register(self,
        ADD_MSS,
        nmp_parse_add_mss,
        NULL);

    nmp_cmd_type_register(self,
        ADD_MSS_RES,
        NULL,
        nmp_create_add_mss_resp);


    nmp_cmd_type_register(self,
        QUERY_MSS,
        nmp_parse_query_mss,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_MSS_RES,
        NULL,
        nmp_create_query_mss_resp);


    nmp_cmd_type_register(self,
        MODIFY_MSS,
        nmp_parse_modify_mss,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_MSS_RES,
        NULL,
        nmp_create_modify_mss_resp);

    nmp_cmd_type_register(self,
        DEL_MSS,
        nmp_parse_del_mss,
        NULL);

    nmp_cmd_type_register(self,
        DEL_MSS_RES,
        NULL,
        nmp_create_del_mss_resp);

    nmp_cmd_type_register(self,
        QUERY_RECORD_POLICY,
        nmp_parse_query_record_policy,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_RECORD_POLICY_RES,
        NULL,
        nmp_create_query_record_policy_resp);

    nmp_cmd_type_register(self,
        RECORD_POLICY_CONFIG,
        nmp_parse_record_policy_config,
        NULL);

    nmp_cmd_type_register(self,
        RECORD_POLICY_CONFIG_RES,
        NULL,
        nmp_create_record_policy_config_resp);

    nmp_cmd_type_register(self,
        QUERY_MANUFACTURER,
        nmp_parse_query_manufacturer,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_MANUFACTURER_RES,
        NULL,
        nmp_create_query_manufacturer_resp);

    nmp_cmd_type_register(self,
        ADD_MODIFY_MANUFACTURER,
        nmp_parse_add_modify_manufacturer,
        NULL);

    nmp_cmd_type_register(self,
        ADD_MODIFY_MANUFACTURER_RES,
        NULL,
        nmp_create_add_modify_manufacturer_resp);

    nmp_cmd_type_register(self,
        DEL_MANUFACTURER,
        nmp_parse_del_manufacturer,
        NULL);

    nmp_cmd_type_register(self,
        DEL_MANUFACTURER_RES,
        NULL,
        nmp_create_del_manufacturer_resp);

    nmp_cmd_type_register(self,
        ADD_GU_TO_USER,
        nmp_parse_add_gu_to_user,
        NULL);

    nmp_cmd_type_register(self,
        ADD_GU_TO_USER_RES,
        NULL,
        nmp_create_add_gu_to_user_resp);

    nmp_cmd_type_register(self,
        QUERY_USER_OWN_GU,
        nmp_parse_query_user_own_gu,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_USER_OWN_GU_RES,
        NULL,
        nmp_create_query_user_own_gu_resp);

    nmp_cmd_type_register(self,
        ADD_TW_TO_USER,
        nmp_parse_add_tw_to_user,
        NULL);

    nmp_cmd_type_register(self,
        ADD_TW_TO_USER_RES,
        NULL,
        nmp_create_add_tw_to_user_resp);

    nmp_cmd_type_register(self,
        QUERY_USER_OWN_TW,
        nmp_parse_query_user_own_tw,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_USER_OWN_TW_RES,
        NULL,
        nmp_create_query_user_own_tw_resp);

    nmp_cmd_type_register(self,
        ADD_TOUR_TO_USER,
        nmp_parse_add_tour_to_user,
        NULL);

    nmp_cmd_type_register(self,
        ADD_TOUR_TO_USER_RES,
        NULL,
        nmp_create_add_tour_to_user_resp);

    nmp_cmd_type_register(self,
        QUERY_USER_OWN_TOUR,
        nmp_parse_query_user_own_tour,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_USER_OWN_TOUR_RES,
        NULL,
        nmp_create_query_user_own_tour_resp);

    nmp_cmd_type_register(self,
        QUERY_SERVER_TIME,
        nmp_parse_query_system_time,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_SERVER_TIME_RES,
        NULL,
        nmp_create_query_system_time_resp);

    nmp_cmd_type_register(self,
        SET_SERVER_TIME,
        nmp_parse_set_system_time,
        NULL);

    nmp_cmd_type_register(self,
        SET_SERVER_TIME_RES,
        NULL,
        nmp_create_set_system_time_resp
        );

    nmp_cmd_type_register(self,
        DATABASE_BACKUP,
        nmp_parse_database_backup,
        NULL);

    nmp_cmd_type_register(self,
        DATABASE_BACKUP_RES,
        NULL,
        nmp_create_database_backup_resp
        );

    nmp_cmd_type_register(self,
        DATABASE_IMPORT,
        nmp_parse_database_import,
        NULL);

    nmp_cmd_type_register(self,
        DATABASE_IMPORT_RES,
        NULL,
        nmp_create_database_import_resp
        );

    nmp_cmd_type_register(self,
        CU_GET_ALL_AREA,
        nmp_parse_get_all_area,
        NULL);

    nmp_cmd_type_register(self,
        CU_GET_ALL_AREA_RESP,
        NULL,
        nmp_create_get_all_area_resp);

    nmp_cmd_type_register(self,
        CU_GET_AREA_INFO,
        nmp_parse_get_area_info,
        NULL);

    nmp_cmd_type_register(self,
        CU_GET_AREA_INFO_RESP,
        NULL,
        nmp_create_get_area_info_resp);

    nmp_cmd_type_register(self,
        CU_GET_DEVICE_LIST,
        nmp_parse_get_device_list,
        NULL);

    nmp_cmd_type_register(self,
        CU_GET_DEVICE_LIST_RESP,
        NULL,
        nmp_create_get_device_list_resp);

    nmp_cmd_type_register(self,
        CU_GET_AREA_DEVICE,
        nmp_parse_get_area_device,
        NULL);

    nmp_cmd_type_register(self,
        CU_GET_AREA_DEVICE_RESP,
        NULL,
        nmp_create_get_area_device_resp);

    nmp_cmd_type_register(self,
        CU_REQUEST_MEDIA,
        nmp_parse_get_media_url,
        nmp_create_pu_get_media_url_para);

    nmp_cmd_type_register(self,
        CU_REQUEST_MEDIA_RESP,
        nmp_parse_pu_get_media_url_resp,
        nmp_create_get_media_url_resp);

    nmp_cmd_type_register(self,
        CHANGE_PU_ONLINE_STATE,
        NULL,
        nmp_create_change_pu_online_state);

    nmp_cmd_type_register(self,
        BROADCAST_GENERAL_MSG,
        nmp_parse_mss_notify_message,
        nmp_create_broadcast_general_msg);

    nmp_cmd_type_register(self,
        NOTIFY_MODIFY_DOMAIN,
        NULL,
        nmp_create_notify_modify_domain);

    nmp_cmd_type_register(self,
        GET_PLATFORM_INFO,
        nmp_parse_cu_get_platform_info,
        nmp_create_pu_get_platform_info);

    nmp_cmd_type_register(self,
        GET_PLATFORM_INFO_RESP,
        nmp_parse_pu_get_platform_info_resp,
        nmp_create_cu_get_platform_info_resp);

    nmp_cmd_type_register(self,
        SET_PLATFORM_INFO,
        nmp_parse_cu_set_platform_info,
        nmp_create_pu_set_platform_info);

    nmp_cmd_type_register(self,
        SET_PLATFORM_INFO_RESP,
        nmp_parse_pu_set_platform_info_resp,
        nmp_create_cu_set_platform_info_resp);

    nmp_cmd_type_register(self,
        GET_DEVICE_INFO,
        nmp_parse_cu_get_device_info,
        nmp_create_pu_get_device_info);

    nmp_cmd_type_register(self,
        GET_DEVICE_INFO_RESP,
        nmp_parse_pu_get_device_info_resp,
        nmp_create_cu_get_device_info_resp);

    nmp_cmd_type_register(self,
        GET_NETWORK_INFO,
        nmp_parse_cu_get_network_info,
        nmp_create_pu_get_network_info);

    nmp_cmd_type_register(self,
        GET_NETWORK_INFO_RESP,
        nmp_parse_pu_get_network_info_resp,
        nmp_create_cu_get_network_info_resp);

    nmp_cmd_type_register(self,
        SET_NETWORK_INFO,
        nmp_parse_cu_set_network_info,
        nmp_create_pu_set_network_info);

    nmp_cmd_type_register(self,
        SET_NETWORK_INFO_RESP,
        nmp_parse_pu_set_network_info_resp,
        nmp_create_cu_set_network_info_resp);

    nmp_cmd_type_register(self,
        GET_PPPOE_INFO,
        nmp_parse_cu_get_pppoe_para,
        nmp_create_pu_get_pppoe_para);

    nmp_cmd_type_register(self,
        GET_PPPOE_INFO_RESP,
        nmp_parse_pu_get_pppoe_para_resp,
        nmp_create_cu_get_pppoe_para_resp);

	nmp_cmd_type_register(self,
        SET_PPPOE_INFO,
        nmp_parse_cu_set_pppoe_para,
        nmp_create_pu_set_pppoe_para);

    nmp_cmd_type_register(self,
        SET_PPPOE_INFO_RESP,
        nmp_parse_pu_set_pppoe_para_resp,
        nmp_create_cu_set_pppoe_para_resp);

    nmp_cmd_type_register(self,
        GET_ENCODE_PARA,
        nmp_parse_cu_get_encode_para,
        nmp_create_pu_get_encode_para);

    nmp_cmd_type_register(self,
        GET_ENCODE_PARA_RESP,
        nmp_parse_pu_get_encode_para_resp,
        nmp_create_cu_get_encode_para_resp);

    nmp_cmd_type_register(self,
        SET_ENCODE_PARA,
        nmp_parse_cu_set_encode_para,
        nmp_create_pu_set_encode_para);

    nmp_cmd_type_register(self,
        SET_ENCODE_PARA_RESP,
        nmp_parse_pu_set_encode_para_resp,
        nmp_create_cu_set_encode_para_resp);

    nmp_cmd_type_register(self,
        GET_DISPLAY_PARA,
        nmp_parse_cu_get_display_para,
        nmp_create_pu_get_display_para);

    nmp_cmd_type_register(self,
        GET_DISPLAY_PARA_RESP,
        nmp_parse_pu_get_display_para_resp,
        nmp_create_cu_get_display_para_resp);

    nmp_cmd_type_register(self,
        GET_DEF_DISPLAY_PARA,
        nmp_parse_cu_get_def_display_para,
        nmp_create_pu_get_def_display_para);

    nmp_cmd_type_register(self,
        GET_DEF_DISPLAY_PARA_RESP,
        nmp_parse_pu_get_def_display_para_resp,
        nmp_create_cu_get_def_display_para_resp);

    nmp_cmd_type_register(self,
        SET_DISPLAY_PARA,
        nmp_parse_cu_set_display_para,
        nmp_create_pu_set_display_para);

    nmp_cmd_type_register(self,
        SET_DISPLAY_PARA_RESP,
        nmp_parse_pu_set_display_para_resp,
        nmp_create_cu_set_display_para_resp);

    nmp_cmd_type_register(self,
        GET_OSD_PARA,
        nmp_parse_cu_get_OSD_para,
        nmp_create_pu_get_OSD_para);

    nmp_cmd_type_register(self,
        GET_OSD_PARA_RESP,
        nmp_parse_pu_get_OSD_para_resp,
        nmp_create_cu_get_OSD_para_resp);

    nmp_cmd_type_register(self,
        SET_OSD_PARA,
        nmp_parse_cu_set_OSD_para,
        nmp_create_pu_set_OSD_para);

    nmp_cmd_type_register(self,
        SET_OSD_PARA_RESP,
        nmp_parse_pu_set_OSD_para_resp,
        nmp_create_cu_set_OSD_para_resp);

    nmp_cmd_type_register(self,
        GET_MOVE_ALARM_PARA,
        nmp_parse_cu_get_move_detection,
        nmp_create_pu_get_move_detection);

    nmp_cmd_type_register(self,
        GET_MOVE_ALARM_PARA_RESP,
        nmp_parse_pu_get_move_detection_resp,
        nmp_create_cu_get_move_detection_resp);

    nmp_cmd_type_register(self,
        SET_MOVE_ALARM_PARA,
        nmp_parse_cu_set_move_detection,
        nmp_create_pu_set_move_detection);

    nmp_cmd_type_register(self,
        SET_MOVE_ALARM_PARA_RESP,
        nmp_parse_pu_set_move_detection_resp,
        nmp_create_cu_set_move_detection_resp);


    nmp_cmd_type_register(self,
        GET_VIDEO_LOST_PARA,
        nmp_parse_cu_get_video_lost,
        nmp_create_pu_get_video_lost);

    nmp_cmd_type_register(self,
        GET_VIDEO_LOST_PARA_RESP,
        nmp_parse_pu_get_video_lost_resp,
        nmp_create_cu_get_video_lost_resp);

    nmp_cmd_type_register(self,
        SET_VIDEO_LOST_PARA,
        nmp_parse_cu_set_video_lost,
        nmp_create_pu_set_video_lost);

    nmp_cmd_type_register(self,
        SET_VIDEO_LOST_PARA_RESP,
        nmp_parse_pu_set_video_lost_resp,
        nmp_create_cu_set_video_lost_resp);

    nmp_cmd_type_register(self,
        GET_HIDE_ALARM_PARA,
        nmp_parse_cu_get_hide_alarm,
        nmp_create_pu_get_hide_alarm);

    nmp_cmd_type_register(self,
        GET_HIDE_ALARM_PARA_RESP,
        nmp_parse_pu_get_hide_alarm_resp,
        nmp_create_cu_get_hide_alarm_resp);

    nmp_cmd_type_register(self,
        SET_HIDE_ALARM_PARA,
        nmp_parse_cu_set_hide_alarm,
        nmp_create_pu_set_hide_alarm);

    nmp_cmd_type_register(self,
        SET_HIDE_ALARM_PARA_RESP,
        nmp_parse_pu_set_hide_alarm_resp,
        nmp_create_cu_set_hide_alarm_resp);

    nmp_cmd_type_register(self,
        GET_HIDE_PARA,
        nmp_parse_cu_get_hide_para,
        nmp_create_pu_get_hide_para);

    nmp_cmd_type_register(self,
        GET_HIDE_PARA_RESP,
        nmp_parse_pu_get_hide_para_resp,
        nmp_create_cu_get_hide_para_resp);

    nmp_cmd_type_register(self,
        SET_HIDE_PARA,
        nmp_parse_cu_set_hide_para,
        nmp_create_pu_set_hide_para);

    nmp_cmd_type_register(self,
        SET_HIDE_PARA_RESP,
        nmp_parse_pu_set_hide_para_resp,
        nmp_create_cu_set_hide_para_resp);

    nmp_cmd_type_register(self,
        GET_IO_ALARM_PARA,
        nmp_parse_cu_get_io_alarm,
        nmp_create_pu_get_io_alarm);

    nmp_cmd_type_register(self,
        GET_IO_ALARM_PARA_RESP,
        nmp_parse_pu_get_io_alarm_resp,
        nmp_create_cu_get_io_alarm_resp);

    nmp_cmd_type_register(self,
        SET_IO_ALARM_PARA,
        nmp_parse_cu_set_io_alarm,
        nmp_create_pu_set_io_alarm);

    nmp_cmd_type_register(self,
        SET_IO_ALARM_PARA_RESP,
        nmp_parse_pu_set_io_alarm_resp,
        nmp_create_cu_set_io_alarm_resp);

    nmp_cmd_type_register(self,
        GET_JOINT_PARA,
        nmp_parse_cu_get_joint_para,
        nmp_create_pu_get_joint_para);

    nmp_cmd_type_register(self,
        GET_JOINT_PARA_RESP,
        nmp_parse_pu_get_joint_para_resp,
        nmp_create_cu_get_joint_para_resp);

    nmp_cmd_type_register(self,
        SET_JOINT_PARA,
        nmp_parse_cu_set_joint_para,
        nmp_create_pu_set_joint_para);

    nmp_cmd_type_register(self,
        SET_JOINT_PARA_RESP,
        nmp_parse_pu_set_joint_para_resp,
        nmp_create_cu_set_joint_para_resp);

    nmp_cmd_type_register(self,
        GET_RECORD_PARA,
        nmp_parse_cu_get_record_para,
        nmp_create_pu_get_record_para);

    nmp_cmd_type_register(self,
        GET_RECORD_PARA_RESP,
        nmp_parse_pu_get_record_para_resp,
        nmp_create_cu_get_record_para_resp);

    nmp_cmd_type_register(self,
        SET_RECORD_PARA,
        nmp_parse_cu_set_record_para,
        nmp_create_pu_set_record_para);

    nmp_cmd_type_register(self,
        SET_RECORD_PARA_RESP,
        nmp_parse_pu_set_record_para_resp,
        nmp_create_cu_set_record_para_resp);

    nmp_cmd_type_register(self,
        GET_PTZ_PARA,
        nmp_parse_cu_get_ptz_para,
        nmp_create_pu_get_ptz_para);

    nmp_cmd_type_register(self,
        GET_PTZ_PARA_RESP,
        nmp_parse_pu_get_ptz_para_resp,
        nmp_create_cu_get_ptz_para_resp);

	nmp_cmd_type_register(self,
        SET_PTZ_PARA,
        nmp_parse_cu_set_ptz_para,
        nmp_create_pu_set_ptz_para);

    nmp_cmd_type_register(self,
        SET_PTZ_PARA_RESP,
        nmp_parse_pu_set_ptz_para_resp,
        nmp_create_cu_set_ptz_para_resp);


    nmp_cmd_type_register(self,
        CONTROL_PTZ,
        nmp_parse_cu_control_ptz,
        nmp_create_pu_control_ptz);

    nmp_cmd_type_register(self,
        CONTROL_PTZ_RESP,
        nmp_parse_pu_control_ptz_resp,
        nmp_create_cu_control_ptz_resp);

    nmp_cmd_type_register(self,
        GET_PRESET_POINT,
        nmp_parse_cu_get_preset_point,
        nmp_create_pu_get_preset_point);

    nmp_cmd_type_register(self,
        GET_PRESET_POINT_RESP,
        nmp_parse_pu_get_preset_point_resp,
        nmp_create_cu_get_preset_point_resp);

    nmp_cmd_type_register(self,
        SET_PRESET_POINT,
        nmp_parse_cu_set_preset_point,
        nmp_create_pu_set_preset_point);

    nmp_cmd_type_register(self,
        SET_PRESET_POINT_RESP,
        nmp_parse_pu_set_preset_point_resp,
        nmp_create_cu_set_preset_point_resp);

    nmp_cmd_type_register(self,
        GET_CRUISE_WAY_SET,
        nmp_parse_cu_get_cruise_way_set,
        nmp_create_pu_get_cruise_way_set);

    nmp_cmd_type_register(self,
        GET_CRUISE_WAY_SET_RESP,
        nmp_parse_pu_get_cruise_way_set_resp,
        nmp_create_cu_get_cruise_way_set_resp);

    nmp_cmd_type_register(self,
        GET_CRUISE_WAY,
        nmp_parse_cu_get_cruise_way,
        nmp_create_pu_get_cruise_way);

    nmp_cmd_type_register(self,
        GET_CRUISE_WAY_RESP,
        nmp_parse_pu_get_cruise_way_resp,
        nmp_create_cu_get_cruise_way_resp);

    nmp_cmd_type_register(self,
        ADD_CRUISE_WAY,
        nmp_parse_cu_add_cruise_way,
        nmp_create_pu_add_cruise_way);

    nmp_cmd_type_register(self,
        ADD_CRUISE_WAY_RESP,
        nmp_parse_pu_add_cruise_way_resp,
        nmp_create_cu_add_cruise_way_resp);

    nmp_cmd_type_register(self,
        MODIFY_CRUISE_WAY,
        nmp_parse_cu_modify_cruise_way,
        nmp_create_pu_modify_cruise_way);

    nmp_cmd_type_register(self,
        MODIFY_CRUISE_WAY_RESP,
        nmp_parse_pu_modify_cruise_way_resp,
        nmp_create_cu_modify_cruise_way_resp);

    nmp_cmd_type_register(self,
        SET_CRUISE_WAY,
        nmp_parse_cu_set_cruise_way,
        nmp_create_pu_set_cruise_way);

    nmp_cmd_type_register(self,
        SET_CRUISE_WAY_RESP,
        nmp_parse_pu_set_cruise_way_resp,
        nmp_create_cu_set_cruise_way_resp);

    nmp_cmd_type_register(self,
        THREE_D_CONTROL,
        nmp_parse_cu_3D_control,
        nmp_create_pu_3D_control);

    nmp_cmd_type_register(self,
        THREE_D_CONTROL_RESP,
        nmp_parse_pu_3D_control_resp,
        nmp_create_cu_3D_control_resp);

    nmp_cmd_type_register(self,
        THREE_D_GOBACK,
        nmp_parse_cu_3D_goback,
        nmp_create_pu_3D_goback);

    nmp_cmd_type_register(self,
        THREE_D_GOBACK_RESP,
        nmp_parse_pu_3D_goback_resp,
        nmp_create_cu_3D_goback_resp);

    nmp_cmd_type_register(self,
         GET_SERIAL_PARA,
         nmp_parse_cu_get_serial_para,
         nmp_create_pu_get_serial_para);

    nmp_cmd_type_register(self,
        GET_SERIAL_PARA_RESP,
        nmp_parse_pu_get_serial_para_resp,
        nmp_create_cu_get_serial_para_resp);

    nmp_cmd_type_register(self,
        SET_SERIAL_PARA,
        nmp_parse_cu_set_serial_para,
        nmp_create_pu_set_serial_para);

    nmp_cmd_type_register(self,
        SET_SERIAL_PARA_RESP,
        nmp_parse_pu_set_serial_para_resp,
        nmp_create_cu_set_serial_para_resp);

    nmp_cmd_type_register(self,
         GET_DEVICE_TIME,
         nmp_parse_cu_get_device_time,
         nmp_create_pu_get_device_time);

    nmp_cmd_type_register(self,
        GET_DEVICE_TIME_RESP,
        nmp_parse_pu_get_device_time_resp,
        nmp_create_cu_get_device_time_resp);

    nmp_cmd_type_register(self,
        SET_DEVICE_TIME,
        nmp_parse_cu_set_device_time,
        nmp_create_pu_set_device_time);

    nmp_cmd_type_register(self,
        SET_DEVICE_TIME_RESP,
        nmp_parse_pu_set_device_time_resp,
        nmp_create_cu_set_device_time_resp);

    nmp_cmd_type_register(self,
         GET_NTP_INFO,
         nmp_parse_cu_get_ntp_info,
         nmp_create_pu_get_ntp_info);

    nmp_cmd_type_register(self,
        GET_NTP_INFO_RESP,
        nmp_parse_pu_get_ntp_info_resp,
        nmp_create_cu_get_ntp_info_resp);

    nmp_cmd_type_register(self,
        SET_NTP_INFO,
        nmp_parse_cu_set_ntp_info,
        nmp_create_pu_set_ntp_info);

    nmp_cmd_type_register(self,
        SET_NTP_INFO_RESP,
        nmp_parse_pu_set_ntp_info_resp,
        nmp_create_cu_set_ntp_info_resp);

    nmp_cmd_type_register(self,
        GET_FTP_PARA,
        nmp_parse_cu_get_ftp_para,
        nmp_create_pu_get_ftp_para);

    nmp_cmd_type_register(self,
        GET_FTP_PARA_RESP,
        nmp_parse_pu_get_ftp_para_resp,
        nmp_create_cu_get_ftp_para_resp);

    nmp_cmd_type_register(self,
        SET_FTP_PARA,
        nmp_parse_cu_set_ftp_para,
        nmp_create_pu_set_ftp_para);

    nmp_cmd_type_register(self,
        SET_FTP_PARA_RESP,
        nmp_parse_pu_set_ftp_para_resp,
        nmp_create_cu_set_ftp_para_resp);

    nmp_cmd_type_register(self,
        GET_SMTP_PARA,
        nmp_parse_cu_get_smtp_para,
        nmp_create_pu_get_smtp_para);

    nmp_cmd_type_register(self,
        GET_SMTP_PARA_RESP,
        nmp_parse_pu_get_smtp_para_resp,
        nmp_create_cu_get_smtp_para_resp);

	nmp_cmd_type_register(self,
        SET_SMTP_PARA,
        nmp_parse_cu_set_smtp_para,
        nmp_create_pu_set_smtp_para);

    nmp_cmd_type_register(self,
        SET_SMTP_PARA_RESP,
        nmp_parse_pu_set_smtp_para_resp,
        nmp_create_cu_set_smtp_para_resp);

    nmp_cmd_type_register(self,
        GET_UPNP_PARA,
        nmp_parse_cu_get_upnp_para,
        nmp_create_pu_get_upnp_para);

    nmp_cmd_type_register(self,
        GET_UPNP_PARA_RESP,
        nmp_parse_pu_get_upnp_para_resp,
        nmp_create_cu_get_upnp_para_resp);

    nmp_cmd_type_register(self,
        SET_UPNP_PARA,
        nmp_parse_cu_set_upnp_para,
        nmp_create_pu_set_upnp_para);

    nmp_cmd_type_register(self,
        SET_UPNP_PARA_RESP,
        nmp_parse_pu_set_upnp_para_resp,
        nmp_create_cu_set_upnp_para_resp);

    nmp_cmd_type_register(self,
        GET_TRANSPARENT_PARA,
        nmp_parse_cu_get_transparent_para,
        nmp_create_pu_get_transparent_para);

    nmp_cmd_type_register(self,
        GET_TRANSPARENT_PARA_RESP,
        nmp_parse_pu_get_transparent_para_resp,
        nmp_create_cu_get_transparent_para_resp);

    nmp_cmd_type_register(self,
        SET_TRANSPARENT_PARA,
        nmp_parse_cu_set_transparent_para,
        nmp_create_pu_set_transparent_para);

    nmp_cmd_type_register(self,
        SET_TRANSPARENT_PARA_RESP,
        nmp_parse_pu_set_transparent_para_resp,
        nmp_create_cu_set_transparent_para_resp);

    nmp_cmd_type_register(self,
        GET_DDNS_PARA,
        nmp_parse_cu_get_ddns_para,
        nmp_create_pu_get_ddns_para);

    nmp_cmd_type_register(self,
        GET_DDNS_PARA_RESP,
        nmp_parse_pu_get_ddns_para_resp,
        nmp_create_cu_get_ddns_para_resp);

    nmp_cmd_type_register(self,
        SET_DDNS_PARA,
        nmp_parse_cu_set_ddns_para,
        nmp_create_pu_set_ddns_para);

    nmp_cmd_type_register(self,
        SET_DDNS_PARA_RESP,
        nmp_parse_pu_set_ddns_para_resp,
        nmp_create_cu_set_ddns_para_resp);


    nmp_cmd_type_register(self,
        GET_DISK_INFO,
        nmp_parse_cu_get_disk_info,
        nmp_create_pu_get_disk_info);

    nmp_cmd_type_register(self,
        GET_DISK_INFO_RESP,
        nmp_parse_pu_get_disk_info_resp,
        nmp_create_cu_get_disk_info_resp);

    nmp_cmd_type_register(self,
        GET_RESOLUTION_INFO,
        nmp_parse_cu_get_resolution_info,
        nmp_create_pu_get_resolution_info);

    nmp_cmd_type_register(self,
        GET_RESOLUTION_INFO_RESP,
        nmp_parse_pu_get_resolution_info_resp,
        nmp_create_cu_get_resolution_info_resp);

    nmp_cmd_type_register(self,
        SET_RESOLUTION_INFO,
        nmp_parse_cu_set_resolution_info,
        nmp_create_pu_set_resolution_info);

    nmp_cmd_type_register(self,
        SET_RESOLUTION_INFO_RESP,
        nmp_parse_pu_set_resolution_info_resp,
        nmp_create_cu_set_resolution_info_resp);

    nmp_cmd_type_register(self,
        GET_IRCUT_CONTROL_INFO,
        nmp_parse_cu_get_ircut_control_info,
        nmp_create_pu_get_ircut_control_info);

    nmp_cmd_type_register(self,
        GET_IRCUT_CONTROL_INFO_RESP,
        nmp_parse_pu_get_ircut_control_info_resp,
        nmp_create_cu_get_ircut_control_info_resp);

    nmp_cmd_type_register(self,
        SET_IRCUT_CONTROL_INFO,
        nmp_parse_cu_set_ircut_control_info,
        nmp_create_pu_set_ircut_control_info);

    nmp_cmd_type_register(self,
        SET_IRCUT_CONTROL_INFO_RESP,
        nmp_parse_pu_set_ircut_control_info_resp,
        nmp_create_cu_set_ircut_control_info_resp);

    nmp_cmd_type_register(self,
        FORMAT_DISK,
        nmp_parse_cu_format_disk,
        nmp_create_pu_format_disk);

    nmp_cmd_type_register(self,
        FORMAT_DISK_RESP,
        nmp_parse_pu_format_disk_resp,
        nmp_create_cu_format_disk_resp);

    nmp_cmd_type_register(self,
        FORMAR_DISK_PROGRESS,
        nmp_parse_pu_submit_format_pro,
        nmp_create_cu_submit_format_pro);

    nmp_cmd_type_register(self,
        SUBMIT_ALARM,
        nmp_parse_pu_submit_alarm,
        nmp_create_cu_submit_alarm);

    nmp_cmd_type_register(self,
        GET_ALARM,
        nmp_parse_cu_get_alarm,
        NULL);

    nmp_cmd_type_register(self,
        GET_ALARM_RESP,
        NULL,
        nmp_create_cu_get_alarm_resp);

    nmp_cmd_type_register(self,
        GET_ALARM_STATE,
        nmp_parse_cu_get_alarm_state,
        NULL);

    nmp_cmd_type_register(self,
        GET_ALARM_STATE_RESP,
        NULL,
        nmp_create_cu_get_alarm_state_resp);

    nmp_cmd_type_register(self,
        DEAL_ALARM,
        nmp_parse_cu_deal_alarm,
        NULL);

    nmp_cmd_type_register(self,
        DEAL_ALARM_RESP,
        NULL,
        nmp_create_cu_deal_alarm_resp);

    nmp_cmd_type_register(self,
        GET_STORE_LOG,
        nmp_parse_cu_get_store_log,
        nmp_create_pu_get_store_log);

    nmp_cmd_type_register(self,
        GET_STORE_LOG_RESP,
        nmp_parse_pu_get_store_log_resp,
        nmp_create_cu_get_store_log_resp);

    nmp_cmd_type_register(self,
        GET_MSS_STORE_LOG,
        nmp_parse_cu_get_mss_store_log,
        nmp_create_mss_get_store_log);

    nmp_cmd_type_register(self,
        GET_MSS_STORE_LOG_RESP,
        nmp_parse_mss_get_store_log_resp,
        nmp_create_cu_get_mss_store_log_resp);

    nmp_cmd_type_register(self,
        FIRMWARE_UPGRADE,
        nmp_parse_cu_firmware_upgrade,
        nmp_create_pu_firmware_upgrade);

    nmp_cmd_type_register(self,
        FIRMWARE_UPGRADE_RESP,
        nmp_parse_pu_firmware_upgrade_resp,
        nmp_create_cu_firmware_upgrade_resp);

    nmp_cmd_type_register(self,
        GET_GU_MSS,
        nmp_parse_cu_get_gu_mss,
        NULL);

    nmp_cmd_type_register(self,
        GET_GU_MSS_RESP,
        NULL,
        nmp_create_cu_get_gu_mss_resp);

    nmp_cmd_type_register(self,
        MSS_GET_GUID,
        nmp_parse_mss_get_guid,
        NULL);

    nmp_cmd_type_register(self,
        MSS_GET_GUID_RESP,
        NULL,
        nmp_create_mss_get_guid_resp);

    nmp_cmd_type_register(self,
        MSS_GET_RECORD_POLICY,
        nmp_parse_mss_get_record_policy,
        NULL);

    nmp_cmd_type_register(self,
        MSS_GET_RECORD_POLICY_RESP,
        NULL,
        nmp_create_mss_get_record_policy_resp);

    nmp_cmd_type_register(self,
        MSS_NOTIFY_POLICY_CHANGE,
        NULL,
        nmp_create_mss_notify_policy_change_resp);

    nmp_cmd_type_register(self,
        MSS_GU_LIST_CHANGE,
        NULL,
        nmp_create_mss_gu_list_change);

    nmp_cmd_type_register(self,
        MSS_GET_ROUTE,
        nmp_parse_mss_get_route,
        NULL);

    nmp_cmd_type_register(self,
        MSS_GET_ROUTE_RESP,
        NULL,
        nmp_create_mss_get_route_resp);

    nmp_cmd_type_register(self,
        MSS_GET_MDS,
        nmp_parse_mss_get_mds,
        NULL);

    nmp_cmd_type_register(self,
        MSS_GET_MDS_RESP,
        NULL,
        nmp_create_mss_get_mds_resp);

    nmp_cmd_type_register(self,
        MSS_GET_MDS_IP,
        nmp_parse_mss_get_mds_ip,
        NULL);

    nmp_cmd_type_register(self,
        MSS_GET_MDS_IP_RESP,
        NULL,
        nmp_create_mss_get_mds_ip_resp);

    nmp_cmd_type_register(self,
        CONTROL_DEVICE,
        nmp_parse_cu_control_device,
        nmp_create_pu_control_device);

    nmp_cmd_type_register(self,
        CONTROL_DEVICE_RESP,
        nmp_parse_pu_control_device_resp,
        nmp_create_cu_control_device_resp);

    nmp_cmd_type_register(self,
        ADD_HD_GROUP,
        nmp_parse_bss_add_hd_group,
        nmp_create_mss_add_hd_group);

    nmp_cmd_type_register(self,
        ADD_HD_GROUP_RES,
        nmp_parse_mss_add_hd_group_resp,
        nmp_create_bss_add_hd_group_resp);

    nmp_cmd_type_register(self,
        ADD_HD_TO_GROUP,
        nmp_parse_bss_add_hd_to_group,
        nmp_create_mss_add_hd_to_group);

    nmp_cmd_type_register(self,
        ADD_HD_TO_GROUP_RES,
        nmp_parse_mss_add_hd_to_group_resp,
        nmp_create_bss_add_hd_to_group_resp);

    nmp_cmd_type_register(self,
        DEL_HD_FROM_GROUP,
        nmp_parse_bss_del_hd_from_group,
        nmp_create_mss_del_hd_from_group);

    nmp_cmd_type_register(self,
        DEL_HD_FROM_GROUP_RES,
        nmp_parse_mss_del_hd_from_group_resp,
        nmp_create_bss_del_hd_from_group_resp);

    nmp_cmd_type_register(self,
        REBOOT_MSS,
        nmp_parse_bss_reboot_mss,
        nmp_create_mss_reboot_mss);

    nmp_cmd_type_register(self,
        REBOOT_MSS_RES,
        nmp_parse_mss_reboot_mss_resp,
        nmp_create_bss_reboot_mss_resp);

    nmp_cmd_type_register(self,
        QUERY_ALL_HD_GROUP,
        nmp_parse_bss_query_all_hd_group,
        nmp_create_mss_query_all_hd_group);

    nmp_cmd_type_register(self,
        QUERY_ALL_HD_GROUP_RES,
        nmp_parse_mss_query_all_hd_group_resp,
        nmp_create_bss_query_all_hd_group_resp);

    nmp_cmd_type_register(self,
        QUERY_HD_GROUP_INFO,
        nmp_parse_bss_query_hd_group_info,
        nmp_create_mss_query_hd_group_info);

    nmp_cmd_type_register(self,
        QUERY_HD_GROUP_INFO_RES,
        nmp_parse_mss_query_hd_group_info_resp,
        nmp_create_bss_query_hd_group_info_resp);

    nmp_cmd_type_register(self,
        QUERY_ALL_HD,
        nmp_parse_bss_query_all_hd,
        nmp_create_mss_query_all_hd);

    nmp_cmd_type_register(self,
        QUERY_ALL_HD_RES,
        nmp_parse_mss_query_all_hd_resp,
        nmp_create_bss_query_all_hd_resp);

    nmp_cmd_type_register(self,
        DEL_HD_GROUP,
        nmp_parse_bss_del_hd_group,
        nmp_create_mss_del_hd_group);

    nmp_cmd_type_register(self,
        DEL_HD_GROUP_RES,
        nmp_parse_mss_del_hd_group_resp,
        nmp_create_bss_del_hd_group_resp);

    nmp_cmd_type_register(self,
        GET_HD_FORMAT_PROGRESS,
        nmp_parse_bss_get_hd_format_progress,
        nmp_create_mss_get_hd_format_progress);

    nmp_cmd_type_register(self,
        GET_HD_FORMAT_PROGRESS_RES,
        nmp_parse_mss_get_hd_format_progress_resp,
        nmp_create_bss_get_hd_format_progress_resp);

    nmp_cmd_type_register(self,
        QUERY_GU_RECORD_STATUS,
        nmp_parse_bss_query_gu_record_status,
        nmp_create_mss_query_gu_record_status);

    nmp_cmd_type_register(self,
        QUERY_GU_RECORD_STATUS_RES,
        nmp_parse_mss_query_gu_record_status_resp,
        nmp_create_bss_query_gu_record_status_resp);

    nmp_cmd_type_register(self,
        ADD_DEFENCE_AREA,
        nmp_parse_add_defence_area,
        NULL);

    nmp_cmd_type_register(self,
        ADD_DEFENCE_AREA_RES,
        NULL,
        nmp_create_add_defence_area_resp);

    nmp_cmd_type_register(self,
        MODIFY_DEFENCE_AREA,
        nmp_parse_modify_defence_area,
        NULL);

    nmp_cmd_type_register(self,
       MODIFY_DEFENCE_AREA_RES,
        NULL,
        nmp_create_modify_defence_area_resp);

    nmp_cmd_type_register(self,
        QUERY_DEFENCE_AREA,
        nmp_parse_query_defence_area,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_DEFENCE_AREA_RES,
        NULL,
        nmp_create_query_defence_area_resp);

    nmp_cmd_type_register(self,
        DEL_DEFENCE_AREA,
        nmp_parse_del_defence_area,
        NULL);

    nmp_cmd_type_register(self,
        DEL_DEFENCE_AREA_RES,
        NULL,
        nmp_create_del_defence_area_resp);

    nmp_cmd_type_register(self,
        ADD_DEFENCE_MAP,
        nmp_parse_add_defence_map,
        NULL);

    nmp_cmd_type_register(self,
        ADD_DEFENCE_MAP_RES,
        NULL,
        nmp_create_add_defence_map_resp);

    nmp_cmd_type_register(self,
        QUERY_DEFENCE_MAP,
        nmp_parse_query_defence_map,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_DEFENCE_MAP_RES,
        NULL,
        nmp_create_query_defence_map_resp);

    nmp_cmd_type_register(self,
        DEL_DEFENCE_MAP,
        nmp_parse_del_defence_map,
        NULL);

    nmp_cmd_type_register(self,
        DEL_DEFENCE_MAP_RES,
        NULL,
        nmp_create_del_defence_map_resp);

    nmp_cmd_type_register(self,
        ADD_DEFENCE_GU,
        nmp_parse_add_defence_gu,
        NULL);

    nmp_cmd_type_register(self,
        ADD_DEFENCE_GU_RES,
        NULL,
        nmp_create_add_defence_gu_resp);

    nmp_cmd_type_register(self,
        MODIFY_DEFENCE_GU,
        nmp_parse_modify_defence_gu,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_DEFENCE_GU_RES,
        NULL,
        nmp_create_modify_defence_gu_resp);

    nmp_cmd_type_register(self,
        QUERY_DEFENCE_GU,
        nmp_parse_query_defence_gu,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_DEFENCE_GU_RES,
        NULL,
        nmp_create_query_defence_gu_resp);

    nmp_cmd_type_register(self,
        DEL_DEFENCE_GU,
        nmp_parse_del_defence_gu,
        NULL);

    nmp_cmd_type_register(self,
        DEL_DEFENCE_GU_RES,
        NULL,
        nmp_create_del_defence_gu_resp);

    nmp_cmd_type_register(self,
        SET_MAP_HREF,
        nmp_parse_set_map_href,
        NULL);

    nmp_cmd_type_register(self,
        SET_MAP_HREF_RES,
        NULL,
        nmp_create_set_map_href_resp);

    nmp_cmd_type_register(self,
        MODIFY_MAP_HREF,
        nmp_parse_modify_map_href,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_MAP_HREF_RES,
        NULL,
        nmp_create_modify_map_href_resp);

    nmp_cmd_type_register(self,
        QUERY_MAP_HREF,
        nmp_parse_query_map_href,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_MAP_HREF_RES,
        NULL,
        nmp_create_query_map_href_resp);

    nmp_cmd_type_register(self,
        DEL_MAP_HREF,
        nmp_parse_del_map_href,
        NULL);

    nmp_cmd_type_register(self,
        DEL_MAP_HREF_RES,
        NULL,
        nmp_create_del_map_href_resp);

    nmp_cmd_type_register(self,
        ADD_TW,
        nmp_parse_add_tw,
        NULL);

    nmp_cmd_type_register(self,
        ADD_TW_RES,
        NULL,
        nmp_create_add_tw_resp);

    nmp_cmd_type_register(self,
        MODIFY_TW,
        nmp_parse_modify_tw,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_TW_RES,
        NULL,
        nmp_create_modify_tw_resp);

    nmp_cmd_type_register(self,
        QUERY_TW,
        nmp_parse_query_tw,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_TW_RES,
        NULL,
        nmp_create_query_tw_resp);

    nmp_cmd_type_register(self,
        DEL_TW,
        nmp_parse_del_tw,
        NULL);

    nmp_cmd_type_register(self,
        DEL_TW_RES,
        NULL,
        nmp_create_del_tw_resp);

    nmp_cmd_type_register(self,
        ADD_SCREEN,
        nmp_parse_add_screen,
        NULL);

    nmp_cmd_type_register(self,
        ADD_SCREEN_RES,
        NULL,
        nmp_create_add_screen_resp);

    nmp_cmd_type_register(self,
        MODIFY_SCREEN,
        nmp_parse_modify_screen,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_SCREEN_RES,
        NULL,
        nmp_create_modify_screen_resp);

    nmp_cmd_type_register(self,
        QUERY_SCREEN,
        nmp_parse_query_screen,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_SCREEN_RES,
        NULL,
        nmp_create_query_screen_resp);

    nmp_cmd_type_register(self,
        DEL_SCREEN,
        nmp_parse_del_screen,
        NULL);

    nmp_cmd_type_register(self,
        DEL_SCREEN_RES,
        NULL,
        nmp_create_del_screen_resp);

    nmp_cmd_type_register(self,
        QUERY_SCR_DIV,
        nmp_parse_query_scr_div,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_SCR_DIV_RES,
        NULL,
        nmp_create_query_scr_div_resp);

    nmp_cmd_type_register(self,
        ADD_TOUR,
        nmp_parse_add_tour,
        NULL);

    nmp_cmd_type_register(self,
        ADD_TOUR_RES,
        NULL,
        nmp_create_add_tour_resp);

    nmp_cmd_type_register(self,
        MODIFY_TOUR,
        nmp_parse_modify_tour,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_TOUR_RES,
        NULL,
        nmp_create_modify_tour_resp);

    nmp_cmd_type_register(self,
        QUERY_TOUR,
        nmp_parse_query_tour,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_TOUR_RES,
        NULL,
        nmp_create_query_tour_resp);

    nmp_cmd_type_register(self,
        DEL_TOUR,
        nmp_parse_del_tour,
        NULL);

    nmp_cmd_type_register(self,
        DEL_TOUR_RES,
        NULL,
        nmp_create_del_tour_resp);

    nmp_cmd_type_register(self,
        ADD_TOUR_STEP,
        nmp_parse_add_tour_step,
        NULL);

    nmp_cmd_type_register(self,
        ADD_TOUR_STEP_RES,
        NULL,
        nmp_create_add_tour_step_resp);

    nmp_cmd_type_register(self,
        QUERY_TOUR_STEP,
        nmp_parse_query_tour_step,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_TOUR_STEP_RES,
        NULL,
        nmp_create_query_tour_step_resp);

    nmp_cmd_type_register(self,
        ADD_GROUP,
        nmp_parse_add_group,
        NULL);

    nmp_cmd_type_register(self,
        ADD_GROUP_RES,
        NULL,
        nmp_create_add_group_resp);

    nmp_cmd_type_register(self,
        MODIFY_GROUP,
        nmp_parse_modify_group,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_GROUP_RES,
        NULL,
        nmp_create_modify_group_resp);

    nmp_cmd_type_register(self,
        QUERY_GROUP,
        nmp_parse_query_group,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_GROUP_RES,
        NULL,
        nmp_create_query_group_resp);

    nmp_cmd_type_register(self,
        DEL_GROUP,
        nmp_parse_del_group,
        NULL);

    nmp_cmd_type_register(self,
        DEL_GROUP_RES,
        NULL,
        nmp_create_del_group_resp);

    nmp_cmd_type_register(self,
        ADD_GROUP_STEP,
        nmp_parse_add_group_step,
        NULL);

    nmp_cmd_type_register(self,
        ADD_GROUP_STEP_RES,
        NULL,
        nmp_create_add_group_step_resp);

    nmp_cmd_type_register(self,
        MODIFY_GROUP_STEP,
        nmp_parse_modify_group_step,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_GROUP_STEP_RES,
        NULL,
        nmp_create_modify_group_step_resp);

    nmp_cmd_type_register(self,
        DEL_GROUP_STEP,
        nmp_parse_del_group_step,
        NULL);

    nmp_cmd_type_register(self,
        DEL_GROUP_STEP_RES,
        NULL,
        nmp_create_del_group_step_resp);

    nmp_cmd_type_register(self,
        QUERY_GROUP_STEP,
        nmp_parse_query_group_step,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_GROUP_STEP_RES,
        NULL,
        nmp_create_query_group_step_resp);

    nmp_cmd_type_register(self,
        CONFIG_GROUP_STEP,
        nmp_parse_config_group_step,
        NULL);

    nmp_cmd_type_register(self,
        CONFIG_GROUP_STEP_RES,
        NULL,
        nmp_create_config_group_step_resp);

    nmp_cmd_type_register(self,
        MODIFY_GROUP_STEP_INFO,
        nmp_parse_modify_group_step_info,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_GROUP_STEP_INFO_RES,
        NULL,
        nmp_create_modify_group_step_info_resp);

    nmp_cmd_type_register(self,
        DEL_GROUP_STEP_INFO,
        nmp_parse_del_group_step_info,
        NULL);

    nmp_cmd_type_register(self,
        DEL_GROUP_STEP_INFO_RES,
        NULL,
        nmp_create_del_group_step_info_resp);

    nmp_cmd_type_register(self,
        QUERY_GROUP_STEP_INFO,
        nmp_parse_query_group_step_info,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_GROUP_STEP_INFO_RES,
        NULL,
        nmp_create_query_group_step_info_resp);

    nmp_cmd_type_register(self,
        QUERY_GROUP_STEP_DIV,
        nmp_parse_query_group_step_div,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_GROUP_STEP_DIV_RES,
        NULL,
        nmp_create_query_group_step_div_resp);

    nmp_cmd_type_register(self,
        QUERY_ALARM,
        nmp_parse_query_alarm,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_ALARM_RES,
        NULL,
        nmp_create_query_alarm_resp);

    nmp_cmd_type_register(self,
        DEL_ALARM,
        nmp_parse_del_alarm,
        NULL);

    nmp_cmd_type_register(self,
        DEL_ALARM_RES,
        NULL,
        nmp_create_del_alarm_resp);

    nmp_cmd_type_register(self,
        SET_DEL_ALARM_POLICY,
        nmp_parse_set_del_alarm_policy,
        NULL);

    nmp_cmd_type_register(self,
        SET_DEL_ALARM_POLICY_RES,
        NULL,
        nmp_create_set_del_alarm_policy_resp);

    nmp_cmd_type_register(self,
        QUERY_DEL_ALARM_POLICY,
        nmp_parse_query_del_alarm_policy,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_DEL_ALARM_POLICY_RES,
        NULL,
        nmp_create_query_del_alarm_policy_resp);

    nmp_cmd_type_register(self,
        MSS_NOTIFY_CHANGE_MSS,
        NULL,
        nmp_create_mss_notify_mss_change_resp);

    nmp_cmd_type_register(self,
        GET_DEFENCE_AREA,
        nmp_parse_get_defence_area,
        NULL);

    nmp_cmd_type_register(self,
        GET_DEFENCE_AREA_RESP,
        NULL,
        nmp_create_get_defence_area_resp);

    nmp_cmd_type_register(self,
        GET_DEFENCE_MAP,
        nmp_parse_get_defence_map,
        NULL);

    nmp_cmd_type_register(self,
        GET_DEFENCE_MAP_RESP,
        NULL,
        nmp_create_get_defence_map_resp);

    nmp_cmd_type_register(self,
        GET_DEFENCE_GU,
        nmp_parse_get_defence_gu,
        NULL);

    nmp_cmd_type_register(self,
        GET_DEFENCE_GU_RESP,
        NULL,
        nmp_create_get_defence_gu_resp);

    nmp_cmd_type_register(self,
        GET_MAP_HREF,
        nmp_parse_get_map_href,
        NULL);

    nmp_cmd_type_register(self,
        GET_MAP_HREF_RESP,
        NULL,
        nmp_create_get_map_href_resp);

    nmp_cmd_type_register(self,
        GET_GU_MAP_LOCATION,
        nmp_parse_get_gu_map_location,
        NULL);

    nmp_cmd_type_register(self,
        GET_GU_MAP_LOCATION_RESP,
        NULL,
        nmp_create_get_gu_map_location_resp);

    nmp_cmd_type_register(self,
        GET_TW,
        nmp_parse_get_tw,
        NULL);

    nmp_cmd_type_register(self,
        GET_TW_RESP,
        NULL,
        nmp_create_get_tw_resp);

    nmp_cmd_type_register(self,
        GET_SCREEN,
        nmp_parse_get_screen,
        NULL);

    nmp_cmd_type_register(self,
        GET_SCREEN_RESP,
        NULL,
        nmp_create_get_screen_resp);

    nmp_cmd_type_register(self,
        GET_SCR_DIV,
        nmp_parse_get_div_mode,
        NULL);

    nmp_cmd_type_register(self,
        GET_SCR_DIV_RESP,
        NULL,
        nmp_create_get_div_mode_resp);

    nmp_cmd_type_register(self,
        PU_GET_DIV_MODE,
        nmp_parse_pu_get_div_mode,
        NULL);

    nmp_cmd_type_register(self,
        PU_GET_DIV_MODE_RESP,
        NULL,
        nmp_create_pu_get_div_mode_resp);

    nmp_cmd_type_register(self,
        GET_SCREEN_STATE,
        nmp_parse_get_scr_state,
        nmp_create_pu_get_scr_state);

    nmp_cmd_type_register(self,
        GET_SCREEN_STATE_RESP,
        nmp_parse_pu_get_scr_state_resp,
        nmp_create_get_scr_state_resp);

    nmp_cmd_type_register(self,
        CHANGE_DIV_MODE,
        nmp_parse_change_div_mode,
        nmp_create_pu_change_div_mode);

    nmp_cmd_type_register(self,
        CHANGE_DIV_MODE_RESP,
        nmp_parse_pu_change_div_mode_resp,
        nmp_create_change_div_mode_resp);

    nmp_cmd_type_register(self,
        RUN_STEP,
        nmp_parse_run_step,
        NULL);

    nmp_cmd_type_register(self,
        RUN_STEP_RESP,
        NULL,
        nmp_create_run_step_resp);

    nmp_cmd_type_register(self,
        FULL_SCREEN,
        nmp_parse_full_screen,
        nmp_create_pu_full_screen);

    nmp_cmd_type_register(self,
        FULL_SCREEN_RESP,
        nmp_parse_pu_full_screen_resp,
        nmp_create_full_screen_resp);

    nmp_cmd_type_register(self,
        EXIT_FULL_SCREEN,
        nmp_parse_exit_full_screen,
        nmp_create_pu_exit_full_screen);

    nmp_cmd_type_register(self,
        EXIT_FULL_SCREEN_RESP,
        nmp_parse_pu_exit_full_screen_resp,
        nmp_create_exit_full_screen_resp);

    nmp_cmd_type_register(self,
        TW_CLEAR_DIVISION,
        nmp_parse_clear_division,
        nmp_create_pu_clear_division);

    nmp_cmd_type_register(self,
        TW_CLEAR_DIVISION_RESP,
        nmp_parse_pu_clear_division_resp,
        nmp_create_clear_division_resp);

    nmp_cmd_type_register(self,
        TW_OPERATE_NOTIFY,
        NULL,
        nmp_create_tw_operate_notify);

    nmp_cmd_type_register(self,
        DEC_ONLINE_STATE_NOTIFY,
        NULL,
        nmp_create_tw_decoder_online_state_notify);

    nmp_cmd_type_register(self,
        TVWALL_PLAY,
        NULL,
        nmp_create_pu_tw_play);

    nmp_cmd_type_register(self,
        TVWALL_PLAY_RESP,
        nmp_parse_pu_tw_play_resp,
        NULL);

    nmp_cmd_type_register(self,
        TW_PLAY_NOTIFY,
        NULL,
        nmp_create_tw_play_notify);

    nmp_cmd_type_register(self,
        GET_TOUR,
        nmp_parse_get_tour,
        NULL);

    nmp_cmd_type_register(self,
        GET_TOUR_RESP,
        NULL,
        nmp_create_get_tour_resp);

    nmp_cmd_type_register(self,
        GET_TOUR_STEP,
        nmp_parse_get_tour_step,
        NULL);

    nmp_cmd_type_register(self,
        GET_TOUR_STEP_RESP,
        NULL,
        nmp_create_get_tour_step_resp);

    nmp_cmd_type_register(self,
        RUN_TOUR,
        nmp_parse_run_tour,
        NULL);

    nmp_cmd_type_register(self,
        RUN_TOUR_RESP,
        NULL,
        nmp_create_run_tour_resp);

    nmp_cmd_type_register(self,
        STOP_TOUR,
        nmp_parse_stop_tour,
        NULL);

    nmp_cmd_type_register(self,
        STOP_TOUR_RESP,
        NULL,
        nmp_create_stop_tour_resp);

    nmp_cmd_type_register(self,
        GET_GROUP,
        nmp_parse_get_group,
        NULL);

    nmp_cmd_type_register(self,
        GET_GROUP_RESP,
        NULL,
        nmp_create_get_group_resp);

    nmp_cmd_type_register(self,
        RUN_GROUP,
        nmp_parse_run_group,
        NULL);

    nmp_cmd_type_register(self,
        RUN_GROUP_RESP,
        NULL,
        nmp_create_run_group_resp);

    nmp_cmd_type_register(self,
        STOP_GROUP,
        nmp_parse_stop_group,
        NULL);

    nmp_cmd_type_register(self,
        STOP_GROUP_RESP,
        NULL,
        nmp_create_stop_group_resp);

    nmp_cmd_type_register(self,
        PLATFORM_UPGRADE,
        nmp_parse_bss_platform_upgrade,
        NULL);

    nmp_cmd_type_register(self,
        PLATFORM_UPGRADE_RES,
        NULL,
        nmp_create_bss_platform_upgrade_resp);

    nmp_cmd_type_register(self,
        GET_NETINTERFACE_CONFIG,
        nmp_parse_get_net_interface_config,
        NULL);

    nmp_cmd_type_register(self,
        GET_NETINTERFACE_CONFIG_RES,
        NULL,
        nmp_create_get_net_interface_config_resp);

    nmp_cmd_type_register(self,
        GET_NETWORK_CONFIG,
        nmp_parse_get_network_config,
        NULL);

    nmp_cmd_type_register(self,
        GET_NETWORK_CONFIG_RES,
        NULL,
        nmp_create_get_network_config_resp);

    nmp_cmd_type_register(self,
        SET_NETWORK_CONFIG,
        nmp_parse_set_network_config,
        NULL);

    nmp_cmd_type_register(self,
        SET_NETWORK_CONFIG_RES,
        NULL,
        nmp_create_set_network_config_resp);

    nmp_cmd_type_register(self,
        AMS_REGISTER_CMS,
        nmp_parse_ams_register,
        NULL);

    nmp_cmd_type_register(self,
        AMS_REGISTER_CMS_RESP,
        NULL,
        nmp_create_ams_register_resp);

    nmp_cmd_type_register(self,
        AMS_HEART,
        nmp_parse_ams_heart,
        NULL);

    nmp_cmd_type_register(self,
        AMS_HEART_RESP,
        NULL,
        nmp_create_ams_heart_resp);

    nmp_cmd_type_register(self,
        LINK_TIME_POLICY_CONFIG,
        nmp_parse_link_time_policy,
        NULL);

    nmp_cmd_type_register(self,
        LINK_TIME_POLICY_CONFIG_RES,
        NULL,
        nmp_create_link_time_policy_resp);

    nmp_cmd_type_register(self,
        MODIFY_LINK_TIME_POLICY,
        nmp_parse_modify_link_time_policy,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_LINK_TIME_POLICY_RES,
        NULL,
        nmp_create_modify_link_time_policy_resp);

    nmp_cmd_type_register(self,
        DEL_LINK_TIME_POLICY,
        nmp_parse_del_link_time_policy,
        NULL);

    nmp_cmd_type_register(self,
        DEL_LINK_TIME_POLICY_RES,
        NULL,
        nmp_create_del_link_time_policy_resp);

    nmp_cmd_type_register(self,
        QUERY_LINK_TIME_POLICY,
        nmp_parse_query_link_time_policy,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_LINK_TIME_POLICY_RES,
        NULL,
        nmp_create_query_link_time_policy_resp);

    nmp_cmd_type_register(self,
        LINK_RECORD,
        nmp_parse_link_record,
        NULL);

    nmp_cmd_type_register(self,
        LINK_RECORD_RES,
        NULL,
        nmp_create_link_record_resp);

    nmp_cmd_type_register(self,
        MODIFY_LINK_RECORD,
        nmp_parse_modify_link_record,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_LINK_RECORD_RES,
        NULL,
        nmp_create_modify_link_record_resp);

    nmp_cmd_type_register(self,
        DEL_LINK_RECORD,
        nmp_parse_del_link_record,
        NULL);

    nmp_cmd_type_register(self,
        DEL_LINK_RECORD_RES,
        NULL,
        nmp_create_del_link_record_resp);

    nmp_cmd_type_register(self,
        QUERY_LINK_RECORD,
        nmp_parse_query_link_record,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_LINK_RECORD_RES,
        NULL,
        nmp_create_query_link_record_resp);

    nmp_cmd_type_register(self,
        MSS_ALARM_LINK_RECORD,
        NULL,
        nmp_create_mss_alarm_link_record_resp);

    nmp_cmd_type_register(self,
        MSS_ALARM_LINK_SNAPSHOT,
        NULL,
        nmp_create_mss_alarm_link_snapshot_resp);

    nmp_cmd_type_register(self,
        LINK_IO,
        nmp_parse_link_io,
        NULL);

    nmp_cmd_type_register(self,
        LINK_IO_RES,
        NULL,
        nmp_create_link_io_resp);

    nmp_cmd_type_register(self,
        MODIFY_LINK_IO,
        nmp_parse_modify_link_io,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_LINK_IO_RES,
        NULL,
        nmp_create_modify_link_io_resp);

    nmp_cmd_type_register(self,
        DEL_LINK_IO,
        nmp_parse_del_link_io,
        NULL);

    nmp_cmd_type_register(self,
        DEL_LINK_IO_RES,
        NULL,
        nmp_create_del_link_io_resp);

    nmp_cmd_type_register(self,
        QUERY_LINK_IO,
        nmp_parse_query_link_io,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_LINK_IO_RES,
        NULL,
        nmp_create_query_link_io_resp);

    nmp_cmd_type_register(self,
        ALARM_LINK_IO,
        nmp_parse_cu_alarm_link_io,
        nmp_create_pu_alarm_link_io);

    nmp_cmd_type_register(self,
        ALARM_LINK_IO_RESP,
        nmp_parse_pu_alarm_link_io_resp,
        nmp_create_cu_alarm_link_io_resp);

    nmp_cmd_type_register(self,
        LINK_SNAPSHOT,
        nmp_parse_link_snapshot,
        NULL);

    nmp_cmd_type_register(self,
        LINK_SNAPSHOT_RES,
        NULL,
        nmp_create_link_snapshot_resp);

    nmp_cmd_type_register(self,
        MODIFY_LINK_SNAPSHOT,
        nmp_parse_modify_link_snapshot,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_LINK_SNAPSHOT_RES,
        NULL,
        nmp_create_modify_link_snapshot_resp);

    nmp_cmd_type_register(self,
        DEL_LINK_SNAPSHOT,
        nmp_parse_del_link_snapshot,
        NULL);

    nmp_cmd_type_register(self,
        DEL_LINK_SNAPSHOT_RES,
        NULL,
        nmp_create_del_link_snapshot_resp);

    nmp_cmd_type_register(self,
        QUERY_LINK_SNAPSHOT,
        nmp_parse_query_link_snapshot,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_LINK_SNAPSHOT_RES,
        NULL,
        nmp_create_query_link_snapshot_resp);

    nmp_cmd_type_register(self,
        LINK_PRESET,
        nmp_parse_link_preset,
        NULL);

    nmp_cmd_type_register(self,
        LINK_PRESET_RES,
        NULL,
        nmp_create_link_preset_resp);

    nmp_cmd_type_register(self,
        MODIFY_LINK_PRESET,
        nmp_parse_modify_link_preset,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_LINK_PRESET_RES,
        NULL,
        nmp_create_modify_link_preset_resp);

    nmp_cmd_type_register(self,
        DEL_LINK_PRESET,
        nmp_parse_del_link_preset,
        NULL);

    nmp_cmd_type_register(self,
        DEL_LINK_PRESET_RES,
        NULL,
        nmp_create_del_link_preset_resp);

    nmp_cmd_type_register(self,
        QUERY_LINK_PRESET,
        nmp_parse_query_link_preset,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_LINK_PRESET_RES,
        NULL,
        nmp_create_query_link_preset_resp);

    nmp_cmd_type_register(self,
        ALARM_LINK_PRESET,
        NULL,
        nmp_create_pu_alarm_link_preset);

    nmp_cmd_type_register(self,
        LINK_STEP,
        nmp_parse_link_step,
        NULL);

    nmp_cmd_type_register(self,
        LINK_STEP_RES,
        NULL,
        nmp_create_link_step_resp);

    nmp_cmd_type_register(self,
        MODIFY_LINK_STEP,
        nmp_parse_modify_link_step,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_LINK_STEP_RES,
        NULL,
        nmp_create_modify_link_step_resp);

    nmp_cmd_type_register(self,
        DEL_LINK_STEP,
        nmp_parse_del_link_step,
        NULL);

    nmp_cmd_type_register(self,
        DEL_LINK_STEP_RES,
        NULL,
        nmp_create_del_link_step_resp);

    nmp_cmd_type_register(self,
        QUERY_LINK_STEP,
        nmp_parse_query_link_step,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_LINK_STEP_RES,
        NULL,
        nmp_create_query_link_step_resp);

    nmp_cmd_type_register(self,
        LINK_MAP,
        nmp_parse_link_map,
        NULL);

    nmp_cmd_type_register(self,
        LINK_MAP_RES,
        NULL,
        nmp_create_link_map_resp);

    nmp_cmd_type_register(self,
        MODIFY_LINK_MAP,
        nmp_parse_modify_link_map,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_LINK_MAP_RES,
        NULL,
        nmp_create_modify_link_map_resp);

    nmp_cmd_type_register(self,
        DEL_LINK_MAP,
        nmp_parse_del_link_map,
        NULL);

    nmp_cmd_type_register(self,
        DEL_LINK_MAP_RES,
        NULL,
        nmp_create_del_link_map_resp);

    nmp_cmd_type_register(self,
        QUERY_LINK_MAP,
        nmp_parse_query_link_map,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_LINK_MAP_RES,
        NULL,
        nmp_create_query_link_map_resp);

    nmp_cmd_type_register(self,
        QUERY_SERVER_RESOURCE_INFO,
        nmp_parse_query_server_resource_info,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_SERVER_RESOURCE_INFO_RES,
        NULL,
        nmp_create_query_server_resource_info_resp);

    nmp_cmd_type_register(self,
        GET_LICENSE_INFO,
        nmp_parse_get_license_info,
        NULL);

    nmp_cmd_type_register(self,
        GET_LICENSE_INFO_RESP,
        NULL,
        nmp_create_get_license_info_resp);

    nmp_cmd_type_register(self,
        SEARCH_PU,
        nmp_parse_search_device,
        NULL);

    nmp_cmd_type_register(self,
        SEARCH_PU_RES,
        NULL,
        nmp_create_search_device_resp);

    nmp_cmd_type_register(self,
        GET_SEARCHED_PUS,
        nmp_parse_get_searched_device,
        NULL);

    nmp_cmd_type_register(self,
        GET_SEARCHED_PUS_RES,
        NULL,
        nmp_create_get_searched_device_resp);

    nmp_cmd_type_register(self,
        QUERY_CMS_ALL_IPS,
        nmp_parse_query_cms_all_ips,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_CMS_ALL_IPS_RES,
        NULL,
        nmp_create_query_cms_all_ips_resp);

    nmp_cmd_type_register(self,
        QUERY_TW_AUTH_INFO,
        nmp_parse_query_tw_auth_info,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_TW_AUTH_INFO_RES,
        NULL,
        nmp_create_query_tw_auth_info_resp);

    nmp_cmd_type_register(self,
        QUERY_ALARM_LINK_AUTH_INFO,
        nmp_parse_query_alarm_link_auth_info,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_ALARM_LINK_AUTH_INFO_RES,
        NULL,
        nmp_create_query_alarm_link_auth_info_resp);

    nmp_cmd_type_register(self,
        GET_TW_LICENSE_INFO,
        nmp_parse_get_tw_auth_info,
        NULL);

    nmp_cmd_type_register(self,
        GET_TW_LICENSE_INFO_RESP,
        NULL,
        nmp_create_get_tw_auth_info_resp);

    nmp_cmd_type_register(self,
        AUTO_ADD_PU,
        nmp_parse_auto_add_pu,
        NULL);

    nmp_cmd_type_register(self,
        AUTO_ADD_PU_RES,
        NULL,
        nmp_create_auto_add_pu_resp);

    nmp_cmd_type_register(self,
        GET_NEXT_PUNO,
        nmp_parse_get_next_puno,
        NULL);

    nmp_cmd_type_register(self,
        GET_NEXT_PUNO_RES,
        NULL,
        nmp_create_get_next_puno_resp);

    nmp_cmd_type_register(self,
        GET_INIT_NAME,
        nmp_parse_bss_get_initiator_name,
        nmp_create_mss_get_initiator_name);

    nmp_cmd_type_register(self,
        GET_INIT_NAME_RES,
        nmp_parse_mss_get_initiator_name_resp,
        nmp_create_bss_get_initiator_name_resp);

    nmp_cmd_type_register(self,
        SET_INIT_NAME,
        nmp_parse_bss_set_initiator_name,
        nmp_create_mss_set_initiator_name);

    nmp_cmd_type_register(self,
        SET_INIT_NAME_RES,
        nmp_parse_mss_set_initiator_name_resp,
        nmp_create_bss_set_initiator_name_resp);

    nmp_cmd_type_register(self,
        GET_IPSAN_INFO,
        nmp_parse_bss_get_ipsan_info,
        nmp_create_mss_get_ipsan_info);

    nmp_cmd_type_register(self,
        GET_IPSAN_INFO_RES,
        nmp_parse_mss_get_ipsan_info_resp,
        nmp_create_bss_get_ipsan_info_resp);

    nmp_cmd_type_register(self,
        ADD_ONE_IPSAN,
        nmp_parse_bss_add_one_ipsan,
        nmp_create_mss_add_one_ipsan);

    nmp_cmd_type_register(self,
        ADD_ONE_IPSAN_RES,
        nmp_parse_mss_add_one_ipsan_resp,
        nmp_create_bss_add_one_ipsan_resp);

    nmp_cmd_type_register(self,
        DELETE_ONE_IPSAN,
        nmp_parse_bss_delete_one_ipsan,
        nmp_create_mss_delete_one_ipsan);

    nmp_cmd_type_register(self,
        DELETE_ONE_IPSAN_RES,
        nmp_parse_mss_delete_one_ipsan_resp,
        nmp_create_bss_delete_one_ipsan_resp);

    nmp_cmd_type_register(self,
        GET_ONE_IPSAN_DETAIL,
        nmp_parse_bss_get_one_ipsan_detail,
        nmp_create_mss_get_one_ipsan_detail);

    nmp_cmd_type_register(self,
        GET_ONE_IPSAN_DETAIL_RES,
        nmp_parse_mss_get_one_ipsan_detail_resp,
        nmp_create_bss_get_one_ipsan_detail_resp);

    nmp_cmd_type_register(self,
        QUERY_LOG,
        nmp_parse_query_log,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_LOG_RES,
        NULL,
        nmp_create_query_log_resp);
#if ONLINE_RATE_FLAG
    nmp_cmd_type_register(self,
        QUERY_AREA_DEV_ONLINE_RATE,
        nmp_parse_query_area_dev_online_rate,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_AREA_DEV_ONLINE_RATE_RES,
        NULL,
        nmp_create_query_area_dev_online_rate_resp);
#endif

    nmp_cmd_type_register(self,
        VALIDATA_GU_MAP,
        nmp_parse_validate_gu_map,
        NULL);

    nmp_cmd_type_register(self,
        VALIDATA_GU_MAP_RES,
        NULL,
        nmp_create_validate_gu_map_resp);

    nmp_cmd_type_register(self,
        ALARM_LINK_MAP,
        NULL,
        nmp_create_cu_alarm_link_map);

    nmp_cmd_type_register(self,
        CU_MODIFY_USER,
        nmp_parse_cu_modify_user_pwd,
        NULL);

    nmp_cmd_type_register(self,
        CU_MODIFY_USER_RES,
        NULL,
        nmp_create_cu_modify_user_pwd_resp);

        nmp_cmd_type_register(self,
        ADD_IVS,
        nmp_parse_add_ivs,
        NULL);

    nmp_cmd_type_register(self,
        ADD_IVS_RES,
        NULL,
        nmp_create_add_ivs_resp);

    nmp_cmd_type_register(self,
        QUERY_IVS,
        nmp_parse_query_ivs,
        NULL);

    nmp_cmd_type_register(self,
        QUERY_IVS_RES,
        NULL,
        nmp_create_query_ivs_resp);


    nmp_cmd_type_register(self,
        MODIFY_IVS,
        nmp_parse_modify_ivs,
        NULL);

    nmp_cmd_type_register(self,
        MODIFY_IVS_RES,
        NULL,
        nmp_create_modify_ivs_resp);

    nmp_cmd_type_register(self,
        DEL_IVS,
        nmp_parse_del_ivs,
        NULL);

    nmp_cmd_type_register(self,
        DEL_IVS_RES,
        NULL,
        nmp_create_del_ivs_resp);

    nmp_cmd_type_register(self,
        IVS_REGISTER_CMS,
        nmp_parse_ivs_register,
        NULL);

    nmp_cmd_type_register(self,
        IVS_REGISTER_CMS_RESP,
        NULL,
        nmp_create_ivs_register_resp);

    nmp_cmd_type_register(self,
        IVS_HEART,
        nmp_parse_ivs_heart,
        NULL);

    nmp_cmd_type_register(self,
        IVS_HEART_RESP,
        NULL,
        nmp_create_ivs_heart_resp);

	nmp_cmd_type_register(self,
		GET_SERVER_FALG,
		nmp_parse_get_server_flag,
		NULL);

	nmp_cmd_type_register(self,
		GET_SERVER_FALG_RES,
		NULL,
		nmp_create_get_server_flag_resp);

	nmp_cmd_type_register(self,
		GET_MDS_CONFIG,
		nmp_parse_get_mds_config,
		NULL);

	nmp_cmd_type_register(self,
		GET_MDS_CONFIG_RES,
		NULL,
		nmp_create_get_mds_config_resp);

	nmp_cmd_type_register(self,
		SET_MDS_CONFIG,
		nmp_parse_set_mds_config,
		NULL);

	nmp_cmd_type_register(self,
		SET_MDS_CONFIG_RES,
		NULL,
		nmp_create_set_mds_config_resp);

	nmp_cmd_type_register(self,
		GET_MDS_STATE,
		nmp_parse_get_mds_state,
		NULL);

	nmp_cmd_type_register(self,
		GET_MDS_STATE_RES,
		NULL,
		nmp_create_get_mds_state_resp);

	nmp_cmd_type_register(self,
		GET_MSS_CONFIG,
		nmp_parse_get_mss_config,
		NULL);

	nmp_cmd_type_register(self,
		GET_MSS_CONFIG_RES,
		NULL,
		nmp_create_get_mss_config_resp);

	nmp_cmd_type_register(self,
		SET_MSS_CONFIG,
		nmp_parse_set_mss_config,
		NULL);

	nmp_cmd_type_register(self,
		SET_MSS_CONFIG_RES,
		NULL,
		nmp_create_set_mss_config_resp);

	nmp_cmd_type_register(self,
		GET_MSS_STATE,
		nmp_parse_get_mss_state,
		NULL);

	nmp_cmd_type_register(self,
		GET_MSS_STATE_RES,
		NULL,
		nmp_create_get_mss_state_resp);

	nmp_cmd_type_register(self,
		GET_IVS_CONFIG,
		nmp_parse_get_ivs_config,
		NULL);

	nmp_cmd_type_register(self,
		GET_IVS_CONFIG_RES,
		NULL,
		nmp_create_get_ivs_config_resp);

	nmp_cmd_type_register(self,
		SET_IVS_CONFIG,
		nmp_parse_set_ivs_config,
		NULL);

	nmp_cmd_type_register(self,
		SET_IVS_CONFIG_RES,
		NULL,
		nmp_create_set_ivs_config_resp);

	nmp_cmd_type_register(self,
		GET_IVS_STATE,
		nmp_parse_get_ivs_state,
		NULL);

	nmp_cmd_type_register(self,
		GET_IVS_STATE_RES,
		NULL,
		nmp_create_get_ivs_state_resp);

	nmp_cmd_type_register(self,
		CMS_SHUTDOWN,
		nmp_parse_cms_shutdown,
		NULL);

	nmp_cmd_type_register(self,
		CMS_SHUTDOWN_RES,
		NULL,
		nmp_create_cms_shutdown_resp);

	nmp_cmd_type_register(self,
		CMS_REBOOT,
		nmp_parse_cms_reboot,
		NULL);

	nmp_cmd_type_register(self,
		CMS_REBOOT_RES,
		NULL,
		nmp_create_cms_reboot_resp);

	nmp_cmd_type_register(self,
		ADD_AMS,
		nmp_parse_add_ams,
		NULL);

	nmp_cmd_type_register(self,
		ADD_AMS_RES,
		NULL,
		nmp_create_add_ams_resp);

	nmp_cmd_type_register(self,
		MODIFY_AMS,
		nmp_parse_modify_ams,
		NULL);

	nmp_cmd_type_register(self,
		MODIFY_AMS_RES,
		NULL,
		nmp_create_modify_ams_resp);

	nmp_cmd_type_register(self,
		DEL_AMS,
		nmp_parse_del_ams,
		NULL);

	nmp_cmd_type_register(self,
		DEL_AMS_RES,
		NULL,
		nmp_create_del_ams_resp);

	nmp_cmd_type_register(self,
		QUERY_AMS,
		nmp_parse_query_ams,
		NULL);

	nmp_cmd_type_register(self,
		QUERY_AMS_RES,
		NULL,
		nmp_create_query_ams_resp);

	nmp_cmd_type_register(self,
		QUERY_AMS_PU,
		nmp_parse_query_ams_pu,
		NULL);

	nmp_cmd_type_register(self,
		QUERY_AMS_PU_RES,
		NULL,
		nmp_create_query_ams_pu_resp);

	nmp_cmd_type_register(self,
		MODIFY_AMS_PU,
		nmp_parse_modify_ams_pu,
		NULL);

	nmp_cmd_type_register(self,
		MODIFY_AMS_PU_RES,
		NULL,
		nmp_create_modify_ams_pu_resp);

	nmp_cmd_type_register(self,
		AMS_DEVICE_INFO_CHANGE,
		NULL,
		nmp_create_ams_device_info_change);

	nmp_cmd_type_register(self,
		AMS_GET_DEVICE_INFO,
		nmp_parse_ams_get_device_info,
		NULL);

	nmp_cmd_type_register(self,
		AMS_GET_DEVICE_INFO_RESP,
		NULL,
		nmp_create_ams_get_device_info_resp);

	nmp_cmd_type_register(self,
		CU_QUERY_GUID,
		nmp_parse_cu_query_guid,
		NULL);

	nmp_cmd_type_register(self,
		CU_QUERY_GUID_RESP,
		NULL,
		nmp_create_cu_query_guid_resp);

	nmp_cmd_type_register(self,
		CU_QUERY_SCREEN_ID,
		nmp_parse_cu_query_screen_id,
		NULL);

	nmp_cmd_type_register(self,
		CU_QUERY_SCREEN_ID_RESP,
		NULL,
		nmp_create_cu_query_screen_id_resp);

	nmp_cmd_type_register(self,
		CU_QUERY_USER_GUIDS,
		nmp_parse_cu_query_user_guids,
		NULL);

	nmp_cmd_type_register(self,
		CU_QUERY_USER_GUIDS_RESP,
		NULL,
		nmp_create_cu_query_user_guids_resp);

	nmp_cmd_type_register(self,
		CU_SET_USER_GUIDS,
		nmp_parse_cu_set_user_guids,
		NULL);

	nmp_cmd_type_register(self,
		CU_SET_USER_GUIDS_RESP,
		NULL,
		nmp_create_cu_set_user_guids_resp);

	nmp_cmd_type_register(self,
		CU_SET_SCREEN_NUM,
		nmp_parse_cu_set_screen_num,
		NULL);

	nmp_cmd_type_register(self,
		CU_SET_SCREEN_NUM_RESP,
		NULL,
		nmp_create_cu_set_screen_num_resp);

	nmp_cmd_type_register(self,
		CU_QUERY_TOUR_ID,
		nmp_parse_cu_query_tour_id,
		NULL);

	nmp_cmd_type_register(self,
		CU_QUERY_TOUR_ID_RESP,
		NULL,
		nmp_create_cu_query_tour_id_resp);

	nmp_cmd_type_register(self,
		CU_SET_TOUR_NUM,
		nmp_parse_cu_set_tour_num,
		NULL);

	nmp_cmd_type_register(self,
		CU_SET_TOUR_NUM_RESP,
		NULL,
		nmp_create_cu_set_tour_num_resp);

	nmp_cmd_type_register(self,
		CU_QUERY_GROUP_ID,
		nmp_parse_cu_query_group_id,
		NULL);

	nmp_cmd_type_register(self,
		CU_QUERY_GROUP_ID_RESP,
		NULL,
		nmp_create_cu_query_group_id_resp);

	nmp_cmd_type_register(self,
		CU_SET_GROUP_NUM,
		nmp_parse_cu_set_group_num,
		NULL);

	nmp_cmd_type_register(self,
		CU_SET_GROUP_NUM_RESP,
		NULL,
		nmp_create_cu_set_group_num_resp);

    printf("init enter nmp_create_xml cmd num=%d\n",CMD_TYPE_COUNTS(self));
    // to do ..., add new command id here
}


void
nmp_init_xml_cmd()
{
	nmp_init_cmd_id(&nmp_cmd_sets);

}

