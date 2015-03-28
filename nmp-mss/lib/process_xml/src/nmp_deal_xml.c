
/**
 * @file     nmp_deal_xml.c
 * @author   Yang Ying
 * @section  LICENSE
 *
 * Copyright by Shenzhen JXJ Electronic Co.Ltd, 2011.
 * Website: www.szjxj.com
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
 * 2012.04.14 - Yang Ying, initiate to create;
 */
#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>

#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_cms_xml.h"

#define CMD_ID_LEN              32
#define NODE_VALUE_LEN          64

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
	 nmp_cmd_type_register(self,
        MSS_REGISTER_CMS, 
        NULL, 
        nmp_create_mss_register);
			
    nmp_cmd_type_register(self,
        MSS_REGISTER_CMS_RESP, 
        nmp_parse_mss_register_resp, 
        NULL);

    nmp_cmd_type_register(self,
        MSS_HEART, 
        NULL, 
        nmp_create_mss_heart);
		
    nmp_cmd_type_register(self,
        MSS_HEART_RESP, 
        nmp_parse_mss_heart_resp, 
        NULL);
 
    nmp_cmd_type_register(self,
        MSS_GET_GUID, 
        NULL, 
        nmp_create_mss_get_guid);
		
    nmp_cmd_type_register(self,
        MSS_GET_GUID_RESP, 
        nmp_parse_mss_get_guid_resp, 
        NULL);
        
    nmp_cmd_type_register(self,
        MSS_GET_RECORD_POLICY, 
        NULL, 
        nmp_create_get_record_policy);
		
    nmp_cmd_type_register(self,
        MSS_GET_RECORD_POLICY_RESP, 
        nmp_parse_get_record_policy_resp, 
        NULL);

    nmp_cmd_type_register(self,
        MSS_GET_ROUTE, 
        NULL, 
        nmp_create_get_route);
		
    nmp_cmd_type_register(self,
        MSS_GET_ROUTE_RESP, 
        nmp_parse_get_route_resp, 
        NULL);
   
   nmp_cmd_type_register(self,
        MSS_GET_MDS, 
        NULL, 
        nmp_create_get_mds);

    nmp_cmd_type_register(self,
        MSS_GET_MDS_RESP, 
        nmp_parse_get_mds_resp, 
        NULL);
        
   nmp_cmd_type_register(self,
        MSS_GET_MDS_IP, 
        NULL, 
        nmp_create_get_mds_ip);
		
    nmp_cmd_type_register(self,
        MSS_GET_MDS_IP_RESP, 
        nmp_parse_get_mds_ip_resp, 
        NULL);
                      
    nmp_cmd_type_register(self,
        MSS_NOTIFY_POLICY_CHANGE, 
        nmp_parse_notify_policy_change, 
        NULL);

    nmp_cmd_type_register(self,
        MSS_GU_LIST_CHANGE, 
        nmp_parse_mss_gu_list_change, 
        NULL);

    nmp_cmd_type_register(self,
        QUERY_RECORD_STATUS, 
        nmp_parse_query_record_status, 
        NULL);

    nmp_cmd_type_register(self,
        QUERY_RECORD_STATUS_RES, 
        NULL, 
        nmp_create_query_record_status_resp);

    nmp_cmd_type_register(self,
        ADD_HD_GROUP, 
        nmp_parse_add_hd_group, 
        NULL);
        
    nmp_cmd_type_register(self,
        ADD_HD_GROUP_RES, 
        NULL, 
        nmp_create_add_hd_group_resp);
        
    nmp_cmd_type_register(self,
        ADD_HD_TO_GROUP, 
        nmp_parse_add_hd_to_group, 
        NULL);
        
    nmp_cmd_type_register(self,
        ADD_HD_TO_GROUP_RES, 
        NULL, 
        nmp_create_add_hd_to_group_resp);
        
    nmp_cmd_type_register(self,
        DEL_HD_FROM_GROUP, 
        nmp_parse_del_hd_from_group, 
        NULL);
        
    nmp_cmd_type_register(self,
        DEL_HD_FROM_GROUP_RES, 
        NULL, 
        nmp_create_del_hd_from_group_resp);
  
    nmp_cmd_type_register(self,
        QUERY_ALL_HD_GROUP, 
        nmp_parse_query_all_hd_group, 
        NULL);
        
    nmp_cmd_type_register(self,
        QUERY_ALL_HD_GROUP_RES, 
        NULL, 
        nmp_create_query_all_hd_group_resp);
         
    nmp_cmd_type_register(self,
        QUERY_HD_GROUP_INFO, 
        nmp_parse_query_hd_group_info, 
        NULL);
        
    nmp_cmd_type_register(self,
        QUERY_HD_GROUP_INFO_RES, 
        NULL, 
        nmp_create_query_hd_group_info_resp);
    nmp_cmd_type_register(self,
        QUERY_ALL_HD, 
        nmp_parse_query_all_hd, 
        NULL);
        
    nmp_cmd_type_register(self,
        QUERY_ALL_HD_RES, 
        NULL, 
        nmp_create_query_all_hd_resp);
            
    nmp_cmd_type_register(self,
        DEL_HD_GROUP, 
        nmp_parse_del_hd_group, 
        NULL);
        
    nmp_cmd_type_register(self,
        DEL_HD_GROUP_RES, 
        NULL, 
        nmp_create_del_hd_group_resp);
  
    nmp_cmd_type_register(self,
        GET_HD_FORMAT_PROGRESS, 
        nmp_parse_get_hd_format_progress, 
        NULL);
        
    nmp_cmd_type_register(self,
        GET_HD_FORMAT_PROGRESS_RES, 
        NULL, 
        nmp_create_get_hd_format_progress_resp);
        
    nmp_cmd_type_register(self,
        GET_STORE_LOG, 
        nmp_parse_get_store_log, 
        NULL);
	
    nmp_cmd_type_register(self,
        GET_STORE_LOG_RES, 
        NULL, 
        nmp_create_get_store_log_resp);
	
    nmp_cmd_type_register(self,
        MSS_NOTIFY_MSS_CHANGE, 
        nmp_parse_notify_mss_change, 
        NULL);

	nmp_cmd_type_register(self,
		ADD_ONE_IPSAN, 
		nmp_parse_add_one_ipsan, 
		NULL);

	nmp_cmd_type_register(self,
		ADD_ONE_IPSAN_RES, 
		NULL, 
		nmp_create_add_one_ipsan_resp);

	nmp_cmd_type_register(self,
		GET_IPSANS, 
		nmp_parse_get_ipsans, 
		NULL);

	nmp_cmd_type_register(self,
		GET_IPSANS_RES, 
		NULL, 
		nmp_create_get_ipsans_resp);

	nmp_cmd_type_register(self,
		SET_IPSANS, 
		nmp_parse_set_ipsans, 
		NULL);

	nmp_cmd_type_register(self,
		SET_IPSANS_RES, 
		NULL, 
		nmp_create_set_ipsans_resp);

	nmp_cmd_type_register(self,
		GET_INITIATOR_NAME, 
		nmp_parse_get_initiator_name, 
		NULL);

	nmp_cmd_type_register(self,
		GET_INITIATOR_NAME_RES, 
		NULL, 
		nmp_create_get_initiator_name_resp);

	nmp_cmd_type_register(self,
		SET_INITIATOR_NAME, 
		nmp_parse_set_initiator_name, 
		NULL);

	nmp_cmd_type_register(self,
		SET_INITIATOR_NAME_RES, 
		NULL, 
		nmp_create_set_initiator_name_resp);

	nmp_cmd_type_register(self,
		GET_ONE_IPSAN_DETAIL, 
		nmp_parse_get_ipsan_detail, 
		NULL);

	nmp_cmd_type_register(self,
		GET_ONE_IPSAN_DETAIL_RES, 
		NULL, 
		nmp_create_get_ipsan_detail_resp);

	nmp_cmd_type_register(self,
		DEL_ONE_IPSAN, 
		nmp_parse_del_one_ipsan, 
		NULL);

	nmp_cmd_type_register(self,
		DEL_ONE_IPSAN_RES, 
		NULL, 
		nmp_create_del_one_ipsan_resp);

	nmp_cmd_type_register(self,
		NOTIFY_MESSAGE, 
		NULL, 
		nmp_create_notify_message);

	nmp_cmd_type_register(self,
		ALARM_LINK_RECORD, 
		nmp_parse_alarm_link_record, 
		NULL);

	nmp_cmd_type_register(self,
		REBOOT_MSS, 
		nmp_parse_reboot_mss, 
		NULL);

	nmp_cmd_type_register(self,
		REBOOT_MSS_RES, 
		NULL, 
		nmp_create_reboot_mss_resp);
	
    // to do ..., add new command id here
}


void
nmp_init_xml_cmd()
{
	nmp_init_cmd_id(&nmp_cmd_sets);
	
}

