/********************************************************************
 * jpf_mss_xml.c  - deal xml of mss, parse and create xml
 * Function：parse or create xml relate to stream.
 * Author:yangy
 * Description:users can add parse or create message of stream,define
 *             struct about stream information
 * History:
 * 2011.06.01 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 ********************************************************************/

#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>
#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_mss_xml.h"
#include "nmp_msg_struct.h"
#include "nmp_errno.h"
#include "nmp_xml_shared.h"
#include "nmp_memory.h"

/**
 * jpf_parse_mss_register: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @return:         succeed JpfMsgInfo, else NULL
 */
JpfMsgInfo *
jpf_parse_mss_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    JpfMssRegister req_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
                jpf_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN );
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}


/**
 * jpf_create_mss_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
jpf_create_mss_register_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    JpfMssRegisterRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = jpf_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(res_info))
    {
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "domainId",
                           BAD_CAST res_info->domain_id);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "serverTime",
                           BAD_CAST res_info->server_time);
        snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->keep_alive_time);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "keepAliveTime",
                           BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->storage_type);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "storageType",
                           BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->mode);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mode",
                           BAD_CAST str);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mssName",
                           BAD_CAST res_info->mss_name);
    }

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfMssHeart req_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                jpf_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_heart.mss_id=%s\n",req_info.mss_id);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}

int
jpf_create_mss_heart_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    JpfMssHeartRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = jpf_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "serverTime",
                    BAD_CAST tmp->server_time);
    }

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_guid(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfMssGetGuid req_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                jpf_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_id=%s\n",req_info.mss_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"reqNum")))
            {
                jpf_deal_value(doc, cur, &req_info.req_num);
                xml_error("req_num=%d\n",req_info.req_num);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
            {
                jpf_deal_value(doc, cur, &req_info.start_row);
                xml_error("start_row=%d\n",req_info.start_row);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}


int
jpf_create_mss_get_guid_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    JpfMssGetGuidRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int i = 0;

    tmp = jpf_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->total_count);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->back_count);
        node = xmlNewNode(NULL, BAD_CAST "guInfo");
        xmlNewProp(node, BAD_CAST "guNum", BAD_CAST str);
        xmlAddChild(root_node, node);

        while ((tmp->back_count > 0)&&(i < tmp->back_count))
        {
            node1 = xmlNewNode(NULL, BAD_CAST "gu");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST tmp->guid_info[i].domain_id);
            xmlNewChild(node1,
                       NULL,
                       BAD_CAST "guId",
                       BAD_CAST tmp->guid_info[i].guid);
            i++;
        }
    }

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_record_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfMssGetRecordPolicy req_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                jpf_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_heart.mss_id=%s\n",req_info.mss_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
            {
                jpf_deal_text(doc, cur, req_info.guid, MAX_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
            {
                jpf_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}

int
jpf_create_mss_get_record_policy_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, new_node= NULL;
    xmlDocPtr doc_str;
    JpfMssGetRecordPolicyRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int xml_len = 0, i;

    tmp = jpf_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "domainId",
            BAD_CAST tmp->domain_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "guId",
            BAD_CAST tmp->guid);
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->level);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "level",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        node = xmlNewNode(NULL, BAD_CAST "hdGroupList");
        xmlAddChild(root_node, node);
        for (i = 0; i < HD_GROUP_NUM; i++)
        {
             if (strlen(tmp->hd_group[i].hd_group_id) != 0)
             {
                  node1 = xmlNewNode(NULL, BAD_CAST "hdGroup");
                  xmlAddChild(node, node1);
                  xmlNewChild(node1,
                       NULL,
                       BAD_CAST "hdGroupId",
                       BAD_CAST  tmp->hd_group[i].hd_group_id);
            }
        }

        xml_len = strlen(tmp->policy);
        if (xml_len != 0)
        {
            doc_str =xmlParseMemory(tmp->policy, xml_len);
            if (doc_str == NULL )
            {
                printf("Document not parsed successfully. \n");
                return -1;
            }

            node = xmlDocGetRootElement(doc_str); //确定文档根元素
            if (node == NULL)
            {
                xml_error("empty document\n");
                xmlFreeDoc(doc_str);
                return -1;
            }
            new_node = xmlDocCopyNode(node, doc, 1);
            xmlAddChild(root_node, new_node);
            xmlFreeDoc(doc_str);
        }
        else
        {
            node = xmlNewNode(NULL, BAD_CAST "timePolicy");
            xmlAddChild(root_node, node);
        }
    }

    return 0;
}


int
jpf_create_mss_notify_policy_change_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    JpfNotifyPolicyChange *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = jpf_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);


    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "guId",
                       BAD_CAST res_info->guid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST res_info->domain_id);
   snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->all_changed);
   node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "allChanged",
                       BAD_CAST str);

    return 0;
}


int
jpf_create_mss_gu_list_change(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_route(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfMssGetRoute req_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                jpf_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("--mss_id=%s\n",req_info.mss_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
            {
                jpf_deal_text(doc, cur, req_info.guid, MAX_ID_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
            {
                jpf_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
            {
                jpf_deal_text(doc, cur, req_info.cms_ip, MAX_IP_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}


int
jpf_create_mss_get_route_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL;
    JpfMssGetRouteRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = jpf_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    xmlNewTextChild(root_node,
                    NULL,
                    BAD_CAST "guId",
                    BAD_CAST res_info->guid);
    xmlNewTextChild(root_node,
                    NULL,
                    BAD_CAST "domainId",
                    BAD_CAST res_info->domain_id);
    if (!RES_CODE(res_info))
    {

	/* node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "url",
                           BAD_CAST res_info->url);	*/
         xmlNewTextChild(root_node,
                    NULL,
                    BAD_CAST "url",
                    BAD_CAST res_info->url);
    }

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfMssGetMds req_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                jpf_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_id=%s\n",req_info.mss_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"reqNum")))
            {
                jpf_deal_value(doc, cur, &req_info.req_num);
                xml_error("req_num=%d\n",req_info.req_num);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
            {
                jpf_deal_value(doc, cur, &req_info.start_row);
                xml_error("start_row=%d\n",req_info.start_row);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}

int
jpf_create_mss_get_mds_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    JpfMssGetMdsRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int i = 0;

    tmp = jpf_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->total_count);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->back_count);
        node = xmlNewNode(NULL, BAD_CAST "mdsInfo");
        xmlNewProp(node, BAD_CAST "mdsNum", BAD_CAST str);
        xmlAddChild(root_node, node);

        while ((tmp->back_count > 0)&&(i < tmp->back_count))
        {
            node1 = xmlNewNode(NULL, BAD_CAST "mds");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                           NULL,
                           BAD_CAST "mdsId",
                           BAD_CAST tmp->mds_info[i].mds_id);
             i++;
        }
    }

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfMssGetMdsIp req_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                jpf_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_id=%s\n",req_info.mss_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsId")))
            {
                jpf_deal_text(doc, cur, req_info.mds_id, MDS_ID_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
            {
                jpf_deal_text(doc, cur, req_info.cms_ip, MAX_IP_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}

int
jpf_create_mss_get_mds_ip_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    JpfMssGetMdsIpRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = jpf_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "mdsId",
                       BAD_CAST tmp->mds_id);

    if (!RES_CODE(tmp))
    {
        xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "mdsIp",
                       BAD_CAST tmp->mds_ip);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->port);
        xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "port",
                       BAD_CAST str);
    }

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_add_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfAddHdGroupRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))
            {
                jpf_deal_value(doc, cur, &res_info.hd_group_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_add_hd_group(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    JpfAddHdGroup *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "hdGroup");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    /*xmlNewChild(node,
                       NULL,
                       BAD_CAST "mssId",
                       BAD_CAST req_info->mss_id);*/
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "hdGroupName",
                       BAD_CAST req_info->hd_group_name);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_add_hd_to_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfAddHdToGroupRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	    else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_add_hd_to_group(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    JpfAddHdToGroup *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "hdGroup");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hd_id);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "hdId",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hd_group_id);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "hdGroupId",
                       BAD_CAST str);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_del_hd_from_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfDelHdFromGroupRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_del_hd_from_group(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    JpfDelHdFromGroup *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "hdGroup");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hd_group_id);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "hdGroupId",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hd_id);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "hdId",
                       BAD_CAST str);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_query_all_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfQueryAllHdGroupRes tmp;
    JpfQueryAllHdGroupRes *res_info = NULL;
    JpfMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL;
    gint i, j = 0, code = 0, size = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroupList";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
		  SET_CODE(&tmp.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"totalNum")))
            {
                jpf_deal_value(doc, cur, &tmp.total_num);
				printf("--------tmp.total_num=%d\n",tmp.total_num);
		  if (tmp.total_num >= 0)
		  {
		  	size = sizeof(JpfQueryAllHdGroupRes) + tmp.total_num*sizeof(JpfHdGroupInfo);
			printf("-------size=%d\n",size);
		  	res_info = jpf_mem_kalloc(size);
			if (res_info)
			    memset(res_info, 0, size);
		  }
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"list")))
            {
                if (!res_info)
		      goto end_query_all_hd_group;

                node = cur->xmlChildrenNode;
		  while (node != NULL)
		  {printf("-------------j=%d\n",j);
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"hdGroup")))
                    {
                         node1 = node->xmlChildrenNode;
                         while (node1 != NULL)
                         {
                             if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdGroupId")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->group_info[j].hd_group_id);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdGroupName")))
                           	 {
                           	       jpf_deal_text(doc, node1, res_info->group_info[j].hd_group_name, GROUP_NAME_LEN);
				 }
			        node1 = node1->next;
                         }

                        j++;
		      }
		      else
                         xml_error("not parse the node %s\n",node->name);
                    node = node->next;
		  }

            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
end_query_all_hd_group:
    if (res_info)
   {
        memcpy(res_info, &tmp, sizeof(tmp));
		printf("-----------------parse session =%s,total_num=%d\n",res_info->session,res_info->total_num);
        sys_msg = jpf_msginfo_new_2(cmd, res_info, size, (JpfMsgInfoPrivDes)jpf_mem_kfree);
    }
    else
	 sys_msg = jpf_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
jpf_create_mss_query_all_hd_group(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    JpfQueryAllHdGroup *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "hdGroupList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_query_hd_group_info_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfQueryHdGroupInfoRes tmp;
    JpfQueryHdGroupInfoRes *res_info = NULL;
    JpfMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL;
    gint i, j = 0, code = 0, size = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroupInfo";

   app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    memset(&tmp, 0, sizeof(tmp));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    //memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
		  SET_CODE(&tmp.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"totalNum")))
            {
                jpf_deal_value(doc, cur, &tmp.total_num);
				printf("--------tmp.total_num=%d\n",tmp.total_num);
		  if (tmp.total_num >= 0)
		  {
		  	size = sizeof(JpfQueryAllHdRes) + tmp.total_num*sizeof(JpfHdInfo);
			printf("-------size=%d\n",size);
		  	res_info = jpf_mem_kalloc(size);
			if (res_info)
			    memset(res_info, 0, size);
		  }
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"list")))
            {
                if (!res_info)
		      goto end_query_all_hd;

                node = cur->xmlChildrenNode;
		  while (node != NULL)
		  {printf("-------------j=%d\n",j);
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"hardDisk")))
                    {
                         node1 = node->xmlChildrenNode;
                         while (node1 != NULL)
                         {
                             if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdId")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].hd_id);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdName")))
                           	 {
                           	       jpf_deal_text(doc, node1, res_info->hd_info[j].hd_name, HD_NAME_LEN);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"totalSize")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].total_size);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"usedSize")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].usedSize);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdStatus")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].hd_status);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"fsType")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].fs_type);
				 }
			        node1 = node1->next;
                         }

                        j++;
		      }
		      else
                         xml_error("not parse the node %s\n",node->name);
                    node = node->next;
		  }

            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
end_query_all_hd:
    if (res_info)
   {
        memcpy(res_info, &tmp, sizeof(tmp));
		printf("-----------------parse session =%s,total_num=%d\n",res_info->session,res_info->total_num);
        sys_msg = jpf_msginfo_new_2(cmd, res_info, size, (JpfMsgInfoPrivDes)jpf_mem_kfree);
    }
    else
	 sys_msg = jpf_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
jpf_create_mss_query_hd_group_info(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    JpfQueryHdGroupInfo  *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "hdGroupInfo");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hd_group_id);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "hdGroupId",
                       BAD_CAST str);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_query_all_hd_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfQueryAllHdRes tmp;
    JpfQueryAllHdRes *res_info = NULL;
    JpfMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL;
    gint i, j = 0, code = 0, size = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hardDiskList";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    memset(&tmp, 0, sizeof(tmp));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    //memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
		  SET_CODE(&tmp.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"totalNum")))
            {
                jpf_deal_value(doc, cur, &tmp.total_num);
				printf("--------tmp.total_num=%d\n",tmp.total_num);
		  if (tmp.total_num >= 0)
		  {
		  	size = sizeof(JpfQueryAllHdRes) + tmp.total_num*sizeof(JpfHdInfo);
			printf("-------size=%d\n",size);
		  	res_info = jpf_mem_kalloc(size);
			if (res_info)
			    memset(res_info, 0, size);
		  }
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"list")))
            {
                if (!res_info)
		      goto end_query_all_hd;

                node = cur->xmlChildrenNode;
		  while (node != NULL)
		  {printf("-------------j=%d\n",j);
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"hardDisk")))
                    {
                         node1 = node->xmlChildrenNode;
                         while (node1 != NULL)
                         {
                             if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdId")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].hd_id);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdName")))
                           	 {
                           	       jpf_deal_text(doc, node1, res_info->hd_info[j].hd_name, HD_NAME_LEN);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"totalSize")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].total_size);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"usedSize")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].usedSize);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdStatus")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].hd_status);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"fsType")))
                           	 {
                           	       jpf_deal_value(doc, node1, &res_info->hd_info[j].fs_type);
				 }
			        node1 = node1->next;
                         }

                        j++;
		      }
		      else
                         xml_error("not parse the node %s\n",node->name);
                    node = node->next;
		  }

            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
end_query_all_hd:
    if (res_info)
   {
        memcpy(res_info, &tmp, sizeof(tmp));
		printf("-----------------parse session =%s,total_num=%d\n",res_info->session,res_info->total_num);
        sys_msg = jpf_msginfo_new_2(cmd, res_info, size, (JpfMsgInfoPrivDes)jpf_mem_kfree);
    }
    else
	 sys_msg = jpf_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
jpf_create_mss_query_all_hd(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    JpfQueryAllHd *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "hardDiskList");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "mssId",
                       BAD_CAST req_info->mss_id);
    return 0;
}


JpfMsgInfo *
jpf_parse_mss_del_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfDelHdGroupRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
		  SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_del_hd_group(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    JpfDelHdGroup *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "hdGroup");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "mssId",
                       BAD_CAST req_info->mss_id);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hd_group_id);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "hdGroupId",
                       BAD_CAST str);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_reboot_mss_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfRebootMssRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
		  SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_reboot_mss(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfRebootMss *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "mssId",
                       BAD_CAST req_info->mss_id);
    return 0;
}

int
jpf_create_mss_get_hd_format_progress(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfGetHdFormatProgress *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
   // node = xmlNewNode(NULL, BAD_CAST "hdGroup");
  //  xmlAddChild(root_node, node);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_hd_format_progress_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfGetHdFormatProgressRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
		  SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))
            {
                jpf_deal_value(doc, cur, &res_info.hd_group_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdId")))
            {
                jpf_deal_value(doc, cur, &res_info.hd_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdName")))
            {
                jpf_deal_text(doc, cur, res_info.hd_name,HD_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"percent")))
            {
                jpf_deal_value(doc, cur, &res_info.percent);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_query_gu_record_status(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfQueryGuRecordStatus *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "guId",
                       BAD_CAST req_info->guid);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_query_gu_record_status_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfQueryGuRecordStatusRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
		  SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"status")))
            {
                jpf_deal_value(doc, cur, &res_info.status);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"statusCode")))
            {
                jpf_deal_value(doc, cur, &res_info.status_code);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


JpfMsgInfo *
jpf_parse_mss_get_store_log_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    JpfGetStoreLogRes *res_info = NULL;
    JpfGetStoreLogRes	tmp;
    JpfMsgInfo *sys_msg = NULL;
    gint  code, i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node = NULL, node1 = NULL;
    char *path = "/message";
    char *back_count = NULL;
    int j = 0, size;

    memset(&tmp, 0, sizeof(tmp));
    app_result =  jpf_get_node(doc, (const xmlChar *)path);
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
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&tmp, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                jpf_deal_text(doc, cur, tmp.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                jpf_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                jpf_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessId")))
                jpf_deal_value(doc, cur, &tmp.sessId);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"logCount")))
                jpf_deal_value(doc, cur, &tmp.total_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"recInfo")))
            {
                back_count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (back_count)
                {
                    tmp.req_num = atoi(back_count);
                    xmlFree(back_count);
                }
		  else
		  	tmp.req_num = 0;
                size = sizeof(JpfGetStoreLogRes) + tmp.req_num*sizeof(JpfStoreLog);
                res_info	= jpf_mem_kalloc(size);
                if (!res_info)
                    return NULL;
                memset(res_info, 0, sizeof(res_info));
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (tmp.req_num <= 0)
		           break;
                    if  ((!xmlStrcmp(node->name, (const xmlChar *)"recNode")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"recType")))
                                jpf_deal_value(doc, node1, &res_info->store_list[j].record_type);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"begTime")))
                                jpf_deal_text(doc, node1, res_info->store_list[j].begin_time, TIME_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"endTime")))
                                jpf_deal_text(doc, node1, res_info->store_list[j].end_time, TIME_LEN);
			       else if ((!xmlStrcmp(node1->name, (const xmlChar *)"property")))
                                jpf_deal_text(doc, node1, res_info->store_list[j].property, FILE_PROPERTY_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"fileSize")))
                                jpf_deal_value(doc, node1, &res_info->store_list[j].file_size);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < tmp.req_num)
                            j++;
                    }
                    node = node->next;
                }
            }
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if (res_info)
        memcpy(res_info, &tmp, sizeof(tmp));
    else
    {
        size = sizeof(JpfGetStoreLogRes);
        res_info	= jpf_mem_kalloc(size);
        if (res_info)
            memcpy(res_info, &tmp, sizeof(tmp));
        else
            return NULL;
    }
    sys_msg = jpf_msginfo_new_2(cmd, res_info, size, (JpfMsgInfoPrivDes)jpf_mem_kfree);
    //sys_msg = jpf_msginfo_new(cmd, res_info, size);
    return sys_msg;
}


int
jpf_create_mss_get_store_log(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    JpfGetMssStoreLog *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "guId",
                       BAD_CAST req_info->guid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "mssId",
                       BAD_CAST req_info->mss_id);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "hdGroupId",
                       BAD_CAST req_info->hd_group_id);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->record_type);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "recType",
                       BAD_CAST str);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "begTime",
                       BAD_CAST req_info->begin_time);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "endTime",
                       BAD_CAST req_info->end_time);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->begin_node);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "begNode",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->end_node);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "endNode",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->sessId);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessId",
                       BAD_CAST str);
    return 0;
}


int
jpf_create_mss_notify_mss_change_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    JpfChangeMss *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = jpf_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->storage_type);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "storageType",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->mode);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "mode",
                       BAD_CAST str);

    return 0;
}


int
jpf_create_mss_alarm_link_record_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    JpfAmsActionRecord *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = jpf_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "guId",
                       BAD_CAST res_info->action_guid.guid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST res_info->action_guid.domain_id);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->level);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "level",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->time_len);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeLen",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->alarm_type);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "alarmType",
                       BAD_CAST str);

    return 0;
}


int
jpf_create_mss_alarm_link_snapshot_resp(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    JpfAmsActionSnapshot *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = jpf_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "guId",
                       BAD_CAST res_info->action_guid.guid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST res_info->action_guid.domain_id);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->level);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "level",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->picture_count);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "pictureCount",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->alarm_type);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "alarmType",
                       BAD_CAST str);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_initiator_name_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfGetInitNameRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"initiatorName")))
                jpf_deal_text(doc, cur, res_info.init_name, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_get_initiator_name(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfGetInitName *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_set_initiator_name_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfSetInitNameRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_set_initiator_name(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfSetInitName *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "initiatorName",
                       BAD_CAST req_info->init_name);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_ipsan_info_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfGetIpsanInfoRes *res_info = NULL;
    JpfGetIpsanInfoRes	tmp;
    JpfMsgInfo *sys_msg = NULL;
    gint  code, i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node = NULL, node1 = NULL;
    char *path = "/message";
    char *count = NULL;
    int j = 0, size;

    memset(&tmp, 0, sizeof(tmp));
    app_result =  jpf_get_node(doc, (const xmlChar *)path);
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
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&tmp, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                jpf_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ipsans")))
            {
                count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (count)
                {
                    tmp.count = atoi(count);
                    xmlFree(count);
                }
		  else
		  	tmp.count = 0;
                size = sizeof(JpfGetIpsanInfoRes) + tmp.count*sizeof(JpfIpsanInfo);
                res_info	= jpf_mem_kalloc(size);
                if (!res_info)
                    return NULL;
                memset(res_info, 0, sizeof(res_info));
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (tmp.count <= 0)
		           break;
                    if  ((!xmlStrcmp(node->name, (const xmlChar *)"ipsan")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"ip")))
                                jpf_deal_text(doc, node1, res_info->ipsan_info[j].ip, MAX_IP_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"port")))
                                jpf_deal_value(doc, node1, &res_info->ipsan_info[j].port);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < tmp.count)
                            j++;
                        else
                            break;
                    }
                    node = node->next;
                }
            }
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if (res_info)
        memcpy(res_info, &tmp, sizeof(tmp));
    else
    {
        size = sizeof(JpfGetIpsanInfoRes);
        res_info	= jpf_mem_kalloc(size);
        if (res_info)
            memcpy(res_info, &tmp, sizeof(tmp));
        else
            return NULL;
    }
    sys_msg = jpf_msginfo_new_2(cmd, res_info, size, (JpfMsgInfoPrivDes)jpf_mem_kfree);
    return sys_msg;
}


int
jpf_create_mss_get_ipsan_info(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfGetIpsanInfo *req_info;

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_add_one_ipsan_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfAddOneIpsanRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_add_one_ipsan(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfAddOneIpsan *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ip",
                       BAD_CAST req_info->ipsan_info.ip);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->ipsan_info.port);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "port",
                       BAD_CAST str);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_delete_one_ipsan_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfDeleteOneIpsanRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_delete_one_ipsan(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfDeleteOneIpsan *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ip",
                       BAD_CAST req_info->ipsan_info.ip);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->ipsan_info.port);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "port",
                       BAD_CAST str);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_get_one_ipsan_detail_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfGetOneIpsanDetailRes res_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, 0, sizeof(res_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                jpf_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                jpf_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"target")))
                jpf_deal_text(doc, cur, res_info.target, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"connectState")))
                jpf_deal_value(doc, cur, &res_info.connect_state);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
jpf_create_mss_get_one_ipsan_detail(xmlDocPtr doc, JpfMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    JpfGetOneIpsanDetail *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = jpf_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ip",
                       BAD_CAST req_info->ipsan_info.ip);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->ipsan_info.port);
    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "port",
                       BAD_CAST str);

    return 0;
}


JpfMsgInfo *
jpf_parse_mss_notify_message(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    JpfNotifyMessage req_info;
    JpfMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  jpf_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"messageId")))
            {
                jpf_deal_value(doc, cur, &req_info.msg_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameter1")))
            {
                jpf_deal_text(doc, cur, req_info.param1, GENERAL_MSG_PARM_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameter2")))
            {
                jpf_deal_text(doc, cur, req_info.param2, GENERAL_MSG_PARM_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameter3")))
            {
                jpf_deal_text(doc, cur, req_info.param3, GENERAL_MSG_PARM_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"content")))
            {
                jpf_deal_text(doc, cur, req_info.content, DESCRIPTION_INFO_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = jpf_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}


