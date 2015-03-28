/********************************************************************
 * nmp_stream_xml.c  - deal xml of stream, parse and create xml
 * Function£ºparse or create xml relate to stream. 
 * Author:yangy
 * Description:users can add parse or create message of stream,define 
 *             struct about stream information
 * History: 
 * 2012.04.14 - Yang Ying, initiate to create;
 ********************************************************************/
 
#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>
#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_cms_xml.h"
#include "nmp_errno.h"
#include "nmp_message.h"
#include "nmp_memory.h"

/**
 * nmp_parse_mss_register: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_mss_register_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpmssRegisterRes req_info;
    NmpMsgInfo *sys_msg = NULL;
    int i, code;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (app_result == NULL)   
        return NULL;
    
    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&req_info, 0, sizeof(req_info));
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))   
            {  
            	nmp_deal_value(doc, cur, &code);    
                SET_CODE(&req_info, code);             
            }        	
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId"))) 
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN );     
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId"))) 
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN );  
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime"))) 
                nmp_deal_value(doc, cur, &req_info.keep_alive_time);     
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"storageType"))) 
                nmp_deal_value(doc, cur, &req_info.storage_type);   
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mode"))) 
                nmp_deal_value(doc, cur, &req_info.mode);                                
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"serverTime")))   
            {               
                nmp_deal_text(doc, cur, req_info.server_time, TIME_INFO_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssName")))   
            {               
                nmp_deal_text(doc, cur, req_info.mss_name, MSS_NAME_LEN);
            }
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);    
            
            cur = cur->next;   
        }
    }
		
	xmlXPathFreeObject(app_result);	
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
    
    return sys_msg;
}


/**
 * nmp_create_mss_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_mss_register(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL; 
    NmpMssRegister *res_info;
    char str[INT_TO_CHAR_LEN] = {0};
		
    res_info = nmp_get_msginfo_data(sys_msg); 
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;
		
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewChild(root_node, 
                       NULL, 
                       BAD_CAST "mssId",
                       BAD_CAST res_info->mss_id);

    return 0;
}


NmpMsgInfo * 
nmp_parse_mss_heart_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssHeartRes req_info;
    NmpMsgInfo *sys_msg = NULL;
    int i, code;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (app_result == NULL)   
        return NULL;
    
    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&req_info, 0, sizeof(req_info));
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))   
            {  
            	nmp_deal_value(doc, cur, &code);    
                SET_CODE(&req_info, code);             
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"serverTime")))   
            {               
                nmp_deal_text(doc, cur, req_info.server_time, TIME_INFO_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);     
    
            cur = cur->next;   
        }
    }
	xmlXPathFreeObject(app_result);	
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
    return sys_msg;
}

int 
nmp_create_mss_heart(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL; 
    NmpMssHeart *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "mssId",
                BAD_CAST tmp->mss_id);	
                	
    return 0;
}


NmpMsgInfo * 
nmp_parse_mss_get_guid_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetGuidRes *res_info = NULL;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL;
    int i, j = 0, code = 0, num = 0, total_count = 0, size = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *mss_num = NULL;
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (app_result == NULL)   
        return NULL;
    
    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i]; 
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {   
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"guInfo")))   
        {  
            mss_num = (char *)xmlGetProp(cur, (const xmlChar *)"guNum");
            if (mss_num)
            {
                num = atoi(mss_num);
                xmlFree(mss_num);								
            }
            size = sizeof(NmpMssGetGuidRes) + num*sizeof(NmpMssGuid);
            res_info = (NmpMssGetGuidRes*)nmp_mem_kalloc(size);
            if (!res_info)
            	return NULL;
            memset(res_info, 0, sizeof(res_info));
            res_info->back_count = num;
            node = cur->xmlChildrenNode;  
            while (node != NULL)
            {  
                if ((!xmlStrcmp(node->name, (const xmlChar *)"gu"))) 
                {
                    node1 = node->xmlChildrenNode; 
                    while (node1 != NULL)
                    {
                        if ((!xmlStrcmp(node1->name, (const xmlChar *)"guId"))) 
                            nmp_deal_text(doc, node1, res_info->guid_info[j].guid, MAX_ID_LEN );   
                        else if ((!xmlStrcmp(node1->name, (const xmlChar *)"domainId")))            
                            nmp_deal_text(doc, node1, res_info->guid_info[j].domain_id, DOMAIN_ID_LEN);  
                        else
                            xml_warning("Warning, not parse the node %s \n", node1->name);    
                        
                        node1 = node1->next;  
                    }
                    j++;
                }      
                node = node->next;  
            }			                 
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode"))) 
        {
            nmp_deal_value(doc, cur, &code);  
            size = sizeof(NmpMssGetGuidRes);
            res_info = (NmpMssGetGuidRes*)nmp_mem_kalloc(size);
            if (!res_info)
               return NULL;
            memset(res_info, 0, sizeof(res_info));  
            SET_CODE(res_info, code);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"totalCount"))) 
        {
            nmp_deal_value(doc, cur, &total_count);  
        }	
        cur = cur->next;     
    }  
    }
	xmlXPathFreeObject(app_result);	
    res_info->total_count = total_count;
	sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);   
    return sys_msg;
}

int 
nmp_create_mss_get_guid(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL; 
    NmpMssGetGuid *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "mssId",
                BAD_CAST tmp->mss_id);	
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->req_num);            
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "reqNum",
                BAD_CAST str);	       
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->start_row);            
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "startRow",
                BAD_CAST str);	                       	
    return 0;
}


NmpMsgInfo * 
nmp_parse_get_record_policy_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetRecordPolicyRes req_info;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL, node2 = NULL, node3 = NULL;
    int i, week_num = 0, seg_num = 0, day_num = 0, hd_group_num = 0, code;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (app_result == NULL)   
        return NULL;
    
    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&req_info, 0, sizeof(req_info));
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))   
            {  
            	nmp_deal_value(doc, cur, &code);    
                SET_CODE(&req_info, code);             
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))   
            {               
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))   
            {               
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))   
            {               
                nmp_deal_value(doc, cur, &req_info.level);
            } 
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupList")))    
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"hdGroup")))
                   	{ 
                   	    node1 = node->xmlChildrenNode;
					
                        while (node1 != NULL)
                        {
                           if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdGroupId")))
                   	       { 
                   	       	   if (hd_group_num >= HD_GROUP_NUM)
                   	       	   	   break;
                               nmp_deal_text(doc, node1, req_info.hd_group[hd_group_num].hd_group_id, HD_GROUP_ID_LEN);   
                           }
				           node1 = node1->next;
                       }
                       hd_group_num++;
                   	}
                    node = node->next;
                }              
            }		            
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timePolicy")))   
            {               
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"weekDays"))) 
                    {
                	    node1 = node->xmlChildrenNode;	
                	    while (node1 != NULL)
                	    {
                	    	if ((!xmlStrcmp(node1->name, (const xmlChar *)"weekDay")))
                	    	{
                	    		if (week_num >= WEEKDAYS)
                	                break;
                	    		node2 = node1->xmlChildrenNode;	
                	            while (node2 != NULL)
                	            {
                	            	if ((!xmlStrcmp(node2->name, (const xmlChar *)"week")))
                	            	{                	            		
                	            		nmp_deal_value(doc, node2, &req_info.record_policy.weekdays[week_num].weekday);               	            		                	            	    	
                	            	}
                	            	else if ((!xmlStrcmp(node2->name, (const xmlChar *)"timeList")))
                	            	{
                	            		seg_num = 0;
                	            	    node3 = node2->xmlChildrenNode;		
                	            		while (node3 != NULL)
                	            		{
                	            		    if ((!xmlStrcmp(node3->name, (const xmlChar *)"timeSeg")))
                	            		    {
                	            		    	if (seg_num >= TIME_SEG_NUM)
                	            			       break;                	            		    	
                	            		    	nmp_deal_text(doc, node3, 
                	            		    	    req_info.record_policy.weekdays[week_num].time_segs[seg_num].time_seg, TIME_SEG_LEN - 1);
                	            		        seg_num++;
                	            		    }
                	            		    node3 = node3->next;
                	            		}	
                	            		req_info.record_policy.weekdays[week_num].time_seg_num = seg_num;
                	            	}
                	            	else
                	            		xml_error("not parse the node %s\n",cur->name); 
                	            	node2 = node2->next;                	            	
                	            }    
                	            week_num++;           	            
                	    	}
                	    	
                	    	node1 = node1->next;                	        
                	    }
                    }  
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"date")))
                    {              		 
                	    node1 = node->xmlChildrenNode;	
                	    while (node1 != NULL)
                	    {
                	    	if ((!xmlStrcmp(node1->name, (const xmlChar *)"days")))
                	    	{
                	    		if (day_num >= MAX_DAY_NUM)
                	                break;
                	    		node2 = node1->xmlChildrenNode;	
                	            while (node2 != NULL)
                	            {
                	            	if ((!xmlStrcmp(node2->name, (const xmlChar *)"day")))
                	            	{               	            		
                	            		nmp_deal_text(doc, node2, req_info.record_policy.day[day_num].date, MAX_DATE_LEN - 1);	               	            		         		                	            	    
                	            	}
                	            	else if ((!xmlStrcmp(node2->name, (const xmlChar *)"timeList")))
                	            	{
                	            		seg_num = 0;
                	            		node3 = node2->xmlChildrenNode;		
                	            		while (node3 != NULL)
                	            		{
                	            		    if ((!xmlStrcmp(node3->name, (const xmlChar *)"timeSeg")))
                	            		    {
                	            		    	if (seg_num >= TIME_SEG_NUM)
                	            			       break;
                	            		    	nmp_deal_text(doc, node3, 
                	            		    	    req_info.record_policy.day[day_num].time_segs[seg_num].time_seg, TIME_SEG_LEN - 1);
                	            		       seg_num++;
                	            		    }
                	            		    node3 = node3->next;
                	            		}
                	            		req_info.record_policy.day[day_num].time_seg_num = seg_num;
                	            	}
                	            	else
                	            		xml_error("not parse the node %s\n",cur->name); 
                	            	node2 = node2->next;              	            	
                	            }    
                	            day_num++;            	                  	            	
                	    	}               	    	
                	    	node1 = node1->next;        
                	    }                     
                    }
                    
                    node = node->next;
                }
            }      
            else
                xml_error("not parse the node %s\n",cur->name);     
    
            cur = cur->next;   
        }
    }
	req_info.record_policy.weekday_num = week_num;
	req_info.record_policy.day_num = day_num;	
	xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
		
    return sys_msg;
}

int 
nmp_create_get_record_policy(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL; 
    NmpMssGetRecordPolicy *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "mssId",
                BAD_CAST tmp->mss_id);	
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "guId",
                BAD_CAST tmp->guid);
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "domainId",
                BAD_CAST tmp->domain_id);                                              	
    return 0;
}


NmpMsgInfo * 
nmp_parse_get_route_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetRouteRes req_info;
    NmpMsgInfo *sys_msg = NULL;
    int i, code;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (app_result == NULL)   
        return NULL;
    
    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&req_info, 0, sizeof(req_info));
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))   
            {  
            	nmp_deal_value(doc, cur, &code);    
                SET_CODE(&req_info, code);             
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))   
            {               
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))   
            {               
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            }   
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"url")))   
            {               
                nmp_deal_text(doc, cur, req_info.url, MAX_URL_LEN);
            }                    
            else
                xml_error("not parse the node %s\n",cur->name);     
    
            cur = cur->next;   
        }
    }
	xmlXPathFreeObject(app_result);	
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
		
    return sys_msg;
}

int 
nmp_create_get_route(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL; 
    NmpMssGetRoute *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "mssId",
                BAD_CAST tmp->mss_id);	
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "guId",
                BAD_CAST tmp->guid);
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "domainId",
                BAD_CAST tmp->domain_id);     
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "cmsIp",
                BAD_CAST tmp->cms_ip);                                           	
    return 0;
}


NmpMsgInfo * 
nmp_parse_notify_policy_change(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpNotifyPolicyChange tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))     
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);  
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))     
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);  	
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"allChanged")))
            	nmp_deal_value(doc, cur, &tmp.all_changed);
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


NmpMsgInfo * 
nmp_parse_mss_gu_list_change(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssId tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))     
                nmp_deal_text(doc, cur, tmp.mss_id, MAX_ID_LEN);  
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


NmpMsgInfo * 
nmp_parse_query_record_status(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryRecordStatus tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
	        if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            	nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
            	nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
            	nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


gint
nmp_create_query_record_status_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpQueryRecordStatusRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->status);
 	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "status",
		BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->status_code);
 	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "statusCode",
		BAD_CAST str); 

	return 0;
}


NmpMsgInfo * 
nmp_parse_add_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddHdGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupName")))     
                nmp_deal_text(doc, cur, tmp.hd_group_name, GROUP_NAME_LEN);  
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);  	
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


/**
 * nmp_create_add_hd_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_add_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL; 
    NmpAddHdGroupRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    
    node = xmlNewNode(NULL, BAD_CAST "hdGroup");
    xmlAddChild(root_node, node);
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));   
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);  
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session);         
    if (!RES_CODE(tmp))   
    {                        
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_group_id);   
        xmlNewChild(node, 
                NULL, 
                BAD_CAST "hdGroupId",
                BAD_CAST str);  	
    }                 
    return 0;
}

NmpMsgInfo * 
nmp_parse_add_hd_to_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddHdToGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);    
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))     
                nmp_deal_value(doc, cur, &tmp.hd_group_id);  
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdId")))     
                nmp_deal_value(doc, cur, &tmp.hd_id);                 
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


/**
 * nmp_create_hd_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_add_hd_to_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL; 
    NmpAddHdToGroupRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    
    node = xmlNewNode(NULL, BAD_CAST "hdGroup");
    xmlAddChild(root_node, node);
    
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);  
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session);      
    return 0;
}



NmpMsgInfo * 
nmp_parse_del_hd_from_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelHdFromGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
	        if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);  
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))     
                nmp_deal_value(doc, cur, &tmp.hd_group_id);  
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdId")))     
                nmp_deal_value(doc, cur, &tmp.hd_id);                 
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


/**
 * nmp_create_del_hd_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_del_hd_from_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL; 
    NmpDelHdFromGroupRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    
    node = xmlNewNode(NULL, BAD_CAST "hdGroup");
    xmlAddChild(root_node, node);
    
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);  
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session);     
    return 0;
}


NmpMsgInfo * 
nmp_parse_query_all_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryAllHdGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroupList";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
	        if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);  
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


/**
 * nmp_create_query_all_hd_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_query_all_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL; 
    NmpQueryAllHdGroupRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int total_num, i;
	
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    
    node = xmlNewNode(NULL, BAD_CAST "hdGroupList");
    xmlAddChild(root_node, node);
    
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);  
                  
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session); 
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);             
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "totalNum",
                BAD_CAST str);                
    if (!RES_CODE(tmp))   
    { 
        total_num = tmp->total_num;	
        
        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;
        while (total_num > 0)  
        {
            node2 = xmlNewNode(NULL, BAD_CAST "hdGroup");
            xmlAddChild(node1, node2);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->group_info[i].hd_group_id);          
            xmlNewChild(node2, 
                        NULL, 
                        BAD_CAST "hdGroupId",
                        BAD_CAST  str);    
  			
            xmlNewChild(node2, 
                        NULL, 
                        BAD_CAST "hdGroupName",
                        BAD_CAST tmp->group_info[i].hd_group_name);      
			
            i++;
            total_num--;
        }    
    }            

    return 0;
}


NmpMsgInfo * 
nmp_parse_query_hd_group_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryHdGroupInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroupInfo";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
	        if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);  
 	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))     
                nmp_deal_value(doc, cur, &tmp.hd_group_id);                 
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


/**
 * nmp_create_query_hd_group_info_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_query_hd_group_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL; 
    NmpQueryHdGroupInfoRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int total_num, i;
	
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    
    node = xmlNewNode(NULL, BAD_CAST "hdGroupInfo");
    xmlAddChild(root_node, node);
    
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);  
                  
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session); 
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);             
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "totalNum",
                BAD_CAST str);                
    if (!RES_CODE(tmp))   
    { 
        total_num = tmp->total_num;	
        
        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;
        while (total_num > 0)  
        {
            node2 = xmlNewNode(NULL, BAD_CAST "hardDisk");
            xmlAddChild(node1, node2);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_info[i].hd_id);          
            xmlNewChild(node2, 
                        NULL, 
                        BAD_CAST "hdId",
                        BAD_CAST  str);    
            xmlNewChild(node2, 
                        NULL, 
                        BAD_CAST "hdName",
                        BAD_CAST  tmp->hd_info[i].hd_name);                         
            snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_info[i].total_size);   			
            xmlNewChild(node2, 
                        NULL, 
                        BAD_CAST "totalSize",
                        BAD_CAST str);   
                                  
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->hd_info[i].usedSize);      
            xmlNewChild(node2, 
                         NULL, 
                         BAD_CAST "usedSize",
                         BAD_CAST str);  
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->hd_info[i].hd_status);      
            xmlNewChild(node2, 
                         NULL, 
                         BAD_CAST "hdStatus",
                         BAD_CAST str);                  
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->hd_info[i].fs_type);      
            xmlNewChild(node2, 
                         NULL, 
                         BAD_CAST "fsType",
                         BAD_CAST str);  
			
            i++;
            total_num--;
        }    
    }            

    return 0;
}


NmpMsgInfo * 
nmp_parse_query_all_hd(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryAllHd tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hardDiskList";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
	        if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);  
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


/**
 * nmp_create_query_all_hd_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_query_all_hd_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL; 
    NmpQueryAllHdRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int total_num, i;
	
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    
    node = xmlNewNode(NULL, BAD_CAST "hardDiskList");
    xmlAddChild(root_node, node);
    
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);  
                  
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session); 
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);             
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "totalNum",
                BAD_CAST str);                
    if (!RES_CODE(tmp))   
    { 
        total_num = tmp->total_num;	
        
        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;
        while (total_num > 0)  
        {
            node2 = xmlNewNode(NULL, BAD_CAST "hardDisk");
            xmlAddChild(node1, node2);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_info[i].hd_id);          
            xmlNewChild(node2, 
                        NULL, 
                        BAD_CAST "hdId",
                        BAD_CAST  str);    
            xmlNewChild(node2, 
                        NULL, 
                        BAD_CAST "hdName",
                        BAD_CAST  tmp->hd_info[i].hd_name);                         
            snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_info[i].total_size);   			
            xmlNewChild(node2, 
                        NULL, 
                        BAD_CAST "totalSize",
                        BAD_CAST str);   
                                  
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->hd_info[i].usedSize);      
            xmlNewChild(node2, 
                         NULL, 
                         BAD_CAST "usedSize",
                         BAD_CAST str);  
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->hd_info[i].hd_status);      
            xmlNewChild(node2, 
                         NULL, 
                         BAD_CAST "hdStatus",
                         BAD_CAST str);                  
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->hd_info[i].fs_type);      
            xmlNewChild(node2, 
                         NULL, 
                         BAD_CAST "fsType",
                         BAD_CAST str);  
			
            i++;
            total_num--;
        }    
    }            

    return 0;
}

NmpMsgInfo * 
nmp_parse_del_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelHdGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))     
                nmp_deal_value(doc, cur, &tmp.hd_group_id);  
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);  	
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


/**
 * nmp_create_del_hd_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_del_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL; 
    NmpDelHdGroupRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    
    node = xmlNewNode(NULL, BAD_CAST "hdGroup");
    xmlAddChild(root_node, node);
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));   
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);  
    xmlNewChild(node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session);                 
	    
    return 0;
}


NmpMsgInfo * 
nmp_parse_get_hd_format_progress(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetHdFormatProgress tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
	        if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))     
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);  	
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


int 
nmp_create_get_hd_format_progress_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL; 
    NmpGetHdFormatProgressRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    
    //node = xmlNewNode(NULL, BAD_CAST "hdGroup");
   // xmlAddChild(root_node, node);
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));   
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);  
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session);                 
    if (!RES_CODE(tmp))   
    {                 
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_group_id);   
        xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "hdGroupId",
                BAD_CAST str);  
        xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "hdName",
                BAD_CAST tmp->hd_name);                
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_id);   
        xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "hdId",
                BAD_CAST str);   
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->percent);   
        xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "percent",
                BAD_CAST str);     
    }                                      	    
    return 0;
}

NmpMsgInfo * 
nmp_parse_get_mds_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetMdsRes *res_info = NULL;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL;
    int i, j = 0, code = 0, num = 0, total_count = 0, size = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *mds_num = NULL;
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (app_result == NULL)   
        return NULL;
    
    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i]; 
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {   
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsInfo")))   
        {  
            mds_num = (char *)xmlGetProp(cur, (const xmlChar *)"mdsNum");
            if (mds_num)
            {
                num = atoi(mds_num);
                xmlFree(mds_num);								
            }
            size = sizeof(NmpMssGetMdsRes) + num*sizeof(NmpMssMds);
            res_info = (NmpMssGetMdsRes*)nmp_mem_kalloc(size);
            if (!res_info)
            	return NULL;
            memset(res_info, 0, sizeof(res_info));
            res_info->back_count = num;
            node = cur->xmlChildrenNode;  
            while (node != NULL)
            {  
                if ((!xmlStrcmp(node->name, (const xmlChar *)"mds"))) 
                {
                    node1 = node->xmlChildrenNode; 
                    while (node1 != NULL)
                    {
                        if ((!xmlStrcmp(node1->name, (const xmlChar *)"mdsId"))) 
                            nmp_deal_text(doc, node1, res_info->mds_info[j].mds_id, MDS_ID_LEN );                              
                        else
                            xml_warning("Warning, not parse the node %s \n", node1->name);    
                        
                        node1 = node1->next;  
                    }
                    j++;
                }      
                node = node->next;  
            }			                 
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode"))) 
        {
            nmp_deal_value(doc, cur, &code);  
            size = sizeof(NmpMssGetMdsRes);
            res_info = nmp_mem_kalloc(size);
            if (!res_info)
               return NULL;
            memset(res_info, 0, sizeof(res_info));  
            SET_CODE(res_info, code);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"totalCount"))) 
        {
            nmp_deal_value(doc, cur, &total_count);  
        }	
        cur = cur->next;     
    }  
    }
	xmlXPathFreeObject(app_result);	
    res_info->total_count = total_count;
	sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);   
    return sys_msg;
}

int 
nmp_create_get_mds(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL; 
    NmpMssGetMds *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "mssId",
                BAD_CAST tmp->mss_id);	
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->req_num);            
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "reqNum",
                BAD_CAST str);	       
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->start_row);            
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "startRow",
                BAD_CAST str);	                       	
    return 0;
}


NmpMsgInfo * 
nmp_parse_get_mds_ip_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetMdsIpRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    int i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    
    memset(&res_info, 0, sizeof(res_info));
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (app_result == NULL)   
        return NULL;
    
    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i]; 
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {   
        	if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))   
            {  
            	nmp_deal_value(doc, cur, &code);    
                SET_CODE(&res_info, code);             
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsId")))   
            {               
                nmp_deal_text(doc, cur, res_info.mds_id, MAX_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsIp")))   
            {               
                nmp_deal_text(doc, cur, res_info.mds_ip, DOMAIN_ID_LEN);
            }  
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))   
            {               
                nmp_deal_value(doc, cur, &res_info.port);
            }             
            cur = cur->next;     
        }  
    }
	xmlXPathFreeObject(app_result);	
	sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));   
    return sys_msg;
}

int 
nmp_create_get_mds_ip(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL; 
    NmpMssGetMdsIp *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    
    tmp = nmp_get_msginfo_data(sys_msg); 
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "mssId",
                BAD_CAST tmp->mss_id);	         
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "mdsId",
                BAD_CAST tmp->mds_id);	                
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "cmsIp",
                BAD_CAST tmp->cms_ip);	                       	
    return 0;
}


NmpMsgInfo *  
nmp_parse_get_store_log(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetMssStoreLog req_info;
    NmpMsgInfo *sys_msg = NULL;

    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (app_result == NULL)   
        return NULL;
    
    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&req_info, 0, sizeof(req_info));
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId"))) 
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);    
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId"))) 
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN); 
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId"))) 
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN); 
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId"))) 
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);   
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId"))) 
                nmp_deal_text(doc, cur, req_info.hd_group_id, HD_GROUP_ID_LEN);			
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"recType"))) 
                nmp_deal_value(doc, cur, &req_info.record_type);  	
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"begTime"))) 
                nmp_deal_text(doc, cur, req_info.begin_time, TIME_LEN); 
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"endTime"))) 
                nmp_deal_text(doc, cur, req_info.end_time, TIME_LEN); 			 
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"begNode"))) 
                nmp_deal_value(doc, cur, &req_info.begin_node);   
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"endNode"))) 
                nmp_deal_value(doc, cur, &req_info.end_node);  	
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessId"))) 
                nmp_deal_value(doc, cur, &req_info.sessId);  			 
            else
            {
                xml_warning("Warning, not parse the node %s \n", cur->name);        
            }
    
            cur = cur->next;   
        }
   	}

    xmlXPathFreeObject(app_result);		
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
    
    return sys_msg;
}


/**
 * nmp_create_get_store_log_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_get_store_log_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
   
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetStoreLogRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    gint count, i;
		
    tmp = nmp_get_msginfo_data(sys_msg); 
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;
        
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "resultCode",
                BAD_CAST str);      
    xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "sessionId",
                BAD_CAST tmp->session);                 
    if (!RES_CODE(tmp))   
    { 
        count = tmp->req_num;   
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "logCount",
                BAD_CAST str);  
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->sessId);
        xmlNewChild(root_node, 
                NULL, 
                BAD_CAST "sessId",
                BAD_CAST str);  		
        node = xmlNewNode(NULL, BAD_CAST "recInfo");
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->req_num);		
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);				
        xmlAddChild(root_node, node);
        i = 0;
        while (count > 0)   
        {
            node1 = xmlNewNode(NULL, BAD_CAST "recNode");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->store_list[i].record_type);      
            xmlNewChild(node1, 
                        NULL, 
                        BAD_CAST "recType",
                        BAD_CAST str);  						
            xmlNewChild(node1, 
                        NULL, 
                        BAD_CAST "begTime",
                        BAD_CAST tmp->store_list[i].begin_time);       
            xmlNewChild(node1, 
                        NULL, 
                        BAD_CAST "endTime",
                        BAD_CAST tmp->store_list[i].end_time);     		
            xmlNewChild(node1, 
                        NULL, 
                        BAD_CAST "property",
                        BAD_CAST tmp->store_list[i].property);                           				
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->store_list[i].file_size);      
            xmlNewChild(node1, 
                        NULL, 
                        BAD_CAST "fileSize",
                        BAD_CAST str);            
            
            i++;
            count--;
        }    
    }            

    return 0;                                               
}


NmpMsgInfo * 
nmp_parse_notify_mss_change(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssChange tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);    
    if (!app_result)      
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval; 
    memset(&tmp, 0, sizeof(tmp));
    
    for (i=0; i < nodeset->nodeNr; i++)     
    {   
        cur = nodeset->nodeTab[i];    
        cur = cur->xmlChildrenNode;  
        while (cur != NULL)
        {  
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"storageType")))     
                nmp_deal_value(doc, cur, &tmp.storage_type);  
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mode")))     
                nmp_deal_value(doc, cur, &tmp.mode);  	
            else
                xml_error("not parse the node %s\n",cur->name);     

            cur = cur->next;   
            while ( cur && xmlIsBlankNode (cur) ) 
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    
    return sys_msg;
}


NmpMsgInfo *  
nmp_parse_add_one_ipsan(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMssAddOneIpsan req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
				nmp_deal_text(doc, cur, req_info.session, USER_NAME_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ip")))
				nmp_deal_text(doc, cur, req_info.info.ip, MAX_IP_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
				nmp_deal_value(doc, cur, &req_info.info.port);
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


gint
nmp_create_add_one_ipsan_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpMssAddOneIpsanRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);

	return 0;
}


NmpMsgInfo *  
nmp_parse_get_ipsans(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMssGetIpsans req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
				nmp_deal_text(doc, cur, req_info.session, USER_NAME_LEN);
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


gint
nmp_create_get_ipsans_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpMssGetIpsansRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};
	gint count, i;

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);

	node = xmlNewNode(NULL, BAD_CAST "ipsans");
	snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->info.total);
	xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
	xmlAddChild(root_node, node);

	i = 0;
	count = tmp->info.total;
	while (count--)
	{
		node1 = xmlNewNode(NULL, BAD_CAST "ipsan");
		xmlAddChild(node, node1);

		xmlNewChild(node1, 
			NULL, 
			BAD_CAST "ip",
			BAD_CAST tmp->info.ips[i].ip);

		snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->info.ips[i].port);
		xmlNewChild(node1, 
			NULL, 
			BAD_CAST "port",
			BAD_CAST str);

		i++;
	}

	return 0;
}


NmpMsgInfo *  
nmp_parse_set_ipsans(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMssSetIpsans req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	xmlNodePtr node, node1;
	char *count;
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
				nmp_deal_text(doc, cur, req_info.session, USER_NAME_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ipsans")))
			{
				count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
				if (count)
				{
					req_info.info.total = atoi(count);
					xmlFree(count);
				}
				else
					req_info.info.total = 0;

				if (req_info.info.total > MAX_IPSAN_NUM)
					req_info.info.total = MAX_IPSAN_NUM;

				node = cur->xmlChildrenNode;
				gint j = 0;
				while (node != NULL)
				{
					if (req_info.info.total <= 0)
						break;
					if ((!xmlStrcmp(node->name, (const xmlChar *)"ipsan")))
					{
						node1 = node->xmlChildrenNode;
						while (node1 != NULL)
						{
							if ((!xmlStrcmp(node1->name, (const xmlChar *)"ip")))
								nmp_deal_text(doc, cur, req_info.info.ips[j].ip, 
								MAX_IP_LEN);
							else if ((!xmlStrcmp(node1->name, (const xmlChar *)"port")))
								nmp_deal_value(doc, cur, &req_info.info.ips[j].port);
							else
								xml_warning("Warning, not parse the node %s \n", 
								node1->name);
							node1 = node1->next;
						}
					}

					if (j < req_info.info.total)
						j++;
					node = node->next;
				}
			}
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


gint
nmp_create_set_ipsans_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpMssSetIpsansRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);

	return 0;
}


NmpMsgInfo *  
nmp_parse_get_initiator_name(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMssGetInitiatorName req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
				nmp_deal_text(doc, cur, req_info.session, USER_NAME_LEN);
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


gint
nmp_create_get_initiator_name_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpMssGetInitiatorNameRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);

	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "initiatorName",
		BAD_CAST tmp->name);

	return 0;
}


NmpMsgInfo *  
nmp_parse_set_initiator_name(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMssSetInitiatorName req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
				nmp_deal_text(doc, cur, req_info.session, USER_NAME_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"initiatorName")))
				nmp_deal_text(doc, cur, req_info.name, MAX_INITIATOR_NAME_LEN);
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


gint
nmp_create_set_initiator_name_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpMssSetInitiatorNameRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);

	return 0;
}


NmpMsgInfo *  
nmp_parse_get_ipsan_detail(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMssGetOneIpsanDetail req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
				nmp_deal_text(doc, cur, req_info.session, USER_NAME_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ip")))
				nmp_deal_text(doc, cur, req_info.info.ip, MAX_IP_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
				nmp_deal_value(doc, cur, &req_info.info.port);
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


gint
nmp_create_get_ipsan_detail_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpMssGetOneIpsanDetailRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);

	snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->connect_state);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "connectState",
		BAD_CAST str);

	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "target",
		BAD_CAST tmp->target_name);

	return 0;
}


NmpMsgInfo *  
nmp_parse_del_one_ipsan(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMssDelOneIpsan req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
				nmp_deal_text(doc, cur, req_info.session, USER_NAME_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ip")))
				nmp_deal_text(doc, cur, req_info.info.ip, MAX_IP_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
				nmp_deal_value(doc, cur, &req_info.info.port);
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


gint
nmp_create_del_one_ipsan_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpMssDelOneIpsanRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);

	return 0;
}


gint
nmp_create_notify_message(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpNotifyMessage *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->msg_id);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "messageId",
		BAD_CAST str);
	
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "parameter1",
		BAD_CAST tmp->param1);

	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "parameter2",
		BAD_CAST tmp->param2);

	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "parameter3",
		BAD_CAST tmp->param3);

	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "content",
		BAD_CAST tmp->content);

	return 0;
}


NmpMsgInfo *  
nmp_parse_alarm_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpAlarmLinkRecord req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
				nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
				nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeLen")))
				nmp_deal_value(doc, cur, (gint*)&req_info.time_len);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
				nmp_deal_value(doc, cur, (gint*)&req_info.alarm_type);
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


NmpMsgInfo *  
nmp_parse_reboot_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpSystemReboot req_info;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	gint i;

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (app_result == NULL)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	memset(&req_info, 0, sizeof(req_info));
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
				nmp_deal_text(doc, cur, req_info.session, USER_NAME_LEN);
			else
			{
				xml_warning("Warning, not parse the node %s \n", cur->name);
			}

			cur = cur->next;
		}
	}

	xmlXPathFreeObject(app_result);
	sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

	return sys_msg;
}


gint
nmp_create_reboot_mss_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	
	xmlNodePtr root_node = NULL;
	NmpSystemRebootRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;
	
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "resultCode",
		BAD_CAST str);
	xmlNewChild(root_node, 
		NULL, 
		BAD_CAST "sessionId",
		BAD_CAST tmp->session);

	return 0;
}

