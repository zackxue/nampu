/********************************************************************
 * nmp_mss_xml.c  - deal xml of mss, parse and create xml
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
 * nmp_parse_mss_register: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_mss_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpMssRegister req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN );
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
nmp_create_mss_register_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMssRegisterRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = nmp_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
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


NmpMsgInfo *
nmp_parse_mss_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssHeart req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_heart.mss_id=%s\n",req_info.mss_id);
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
nmp_create_mss_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMssHeartRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_get_guid(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetGuid req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_id=%s\n",req_info.mss_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"reqNum")))
            {
                nmp_deal_value(doc, cur, &req_info.req_num);
                xml_error("req_num=%d\n",req_info.req_num);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
            {
                nmp_deal_value(doc, cur, &req_info.start_row);
                xml_error("start_row=%d\n",req_info.start_row);
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
nmp_create_mss_get_guid_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpMssGetGuidRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int i = 0;

    tmp = nmp_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_get_record_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetRecordPolicy req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_heart.mss_id=%s\n",req_info.mss_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
            {
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
            {
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
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
nmp_create_mss_get_record_policy_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, new_node= NULL;
    xmlDocPtr doc_str;
    NmpMssGetRecordPolicyRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int xml_len = 0, i;

    tmp = nmp_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
nmp_create_mss_notify_policy_change_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpNotifyPolicyChange *res_info;
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
nmp_create_mss_gu_list_change(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    return 0;
}


NmpMsgInfo *
nmp_parse_mss_get_route(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetRoute req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("--mss_id=%s\n",req_info.mss_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
            {
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
            {
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
            {
                nmp_deal_text(doc, cur, req_info.cms_ip, MAX_IP_LEN);
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
nmp_create_mss_get_route_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL;
    NmpMssGetRouteRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = nmp_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
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


NmpMsgInfo *
nmp_parse_mss_get_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetMds req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_id=%s\n",req_info.mss_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"reqNum")))
            {
                nmp_deal_value(doc, cur, &req_info.req_num);
                xml_error("req_num=%d\n",req_info.req_num);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
            {
                nmp_deal_value(doc, cur, &req_info.start_row);
                xml_error("start_row=%d\n",req_info.start_row);
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
nmp_create_mss_get_mds_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpMssGetMdsRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int i = 0;

    tmp = nmp_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_get_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssGetMdsIp req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
            {
                nmp_deal_text(doc, cur, req_info.mss_id, MSS_ID_LEN);
                xml_error("mss_id=%s\n",req_info.mss_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsId")))
            {
                nmp_deal_text(doc, cur, req_info.mds_id, MDS_ID_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
            {
                nmp_deal_text(doc, cur, req_info.cms_ip, MAX_IP_LEN);
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
nmp_create_mss_get_mds_ip_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMssGetMdsIpRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_add_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddHdGroupRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))
            {
                nmp_deal_value(doc, cur, &res_info.hd_group_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_add_hd_group(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    NmpAddHdGroup *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
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


NmpMsgInfo *
nmp_parse_mss_add_hd_to_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddHdToGroupRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	    else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_add_hd_to_group(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    NmpAddHdToGroup *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
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


NmpMsgInfo *
nmp_parse_mss_del_hd_from_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelHdFromGroupRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_del_hd_from_group(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    NmpDelHdFromGroup *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
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


NmpMsgInfo *
nmp_parse_mss_query_all_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryAllHdGroupRes tmp;
    NmpQueryAllHdGroupRes *res_info = NULL;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL;
    gint i, j = 0, code = 0, size = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroupList";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
		  SET_CODE(&tmp.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"totalNum")))
            {
                nmp_deal_value(doc, cur, &tmp.total_num);
				printf("--------tmp.total_num=%d\n",tmp.total_num);
		  if (tmp.total_num >= 0)
		  {
		  	size = sizeof(NmpQueryAllHdGroupRes) + tmp.total_num*sizeof(NmpHdGroupInfo);
			printf("-------size=%d\n",size);
		  	res_info = nmp_mem_kalloc(size);
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
                           	       nmp_deal_value(doc, node1, &res_info->group_info[j].hd_group_id);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdGroupName")))
                           	 {
                           	       nmp_deal_text(doc, node1, res_info->group_info[j].hd_group_name, GROUP_NAME_LEN);
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
        sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    }
    else
	 sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_mss_query_all_hd_group(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    NmpQueryAllHdGroup *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "hdGroupList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


NmpMsgInfo *
nmp_parse_mss_query_hd_group_info_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryHdGroupInfoRes tmp;
    NmpQueryHdGroupInfoRes *res_info = NULL;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL;
    gint i, j = 0, code = 0, size = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroupInfo";

   app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
		  SET_CODE(&tmp.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"totalNum")))
            {
                nmp_deal_value(doc, cur, &tmp.total_num);
				printf("--------tmp.total_num=%d\n",tmp.total_num);
		  if (tmp.total_num >= 0)
		  {
		  	size = sizeof(NmpQueryAllHdRes) + tmp.total_num*sizeof(NmpHdInfo);
			printf("-------size=%d\n",size);
		  	res_info = nmp_mem_kalloc(size);
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
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].hd_id);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdName")))
                           	 {
                           	       nmp_deal_text(doc, node1, res_info->hd_info[j].hd_name, HD_NAME_LEN);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"totalSize")))
                           	 {
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].total_size);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"usedSize")))
                           	 {
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].usedSize);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdStatus")))
                           	 {
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].hd_status);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"fsType")))
                           	 {
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].fs_type);
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
        sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    }
    else
	 sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_mss_query_hd_group_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    NmpQueryHdGroupInfo  *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
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


NmpMsgInfo *
nmp_parse_mss_query_all_hd_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryAllHdRes tmp;
    NmpQueryAllHdRes *res_info = NULL;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL, node1 = NULL;
    gint i, j = 0, code = 0, size = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hardDiskList";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
		  SET_CODE(&tmp.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"totalNum")))
            {
                nmp_deal_value(doc, cur, &tmp.total_num);
				printf("--------tmp.total_num=%d\n",tmp.total_num);
		  if (tmp.total_num >= 0)
		  {
		  	size = sizeof(NmpQueryAllHdRes) + tmp.total_num*sizeof(NmpHdInfo);
			printf("-------size=%d\n",size);
		  	res_info = nmp_mem_kalloc(size);
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
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].hd_id);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdName")))
                           	 {
                           	       nmp_deal_text(doc, node1, res_info->hd_info[j].hd_name, HD_NAME_LEN);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"totalSize")))
                           	 {
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].total_size);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"usedSize")))
                           	 {
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].usedSize);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"hdStatus")))
                           	 {
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].hd_status);
				 }
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"fsType")))
                           	 {
                           	       nmp_deal_value(doc, node1, &res_info->hd_info[j].fs_type);
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
        sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    }
    else
	 sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_mss_query_all_hd(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    NmpQueryAllHd *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_del_hd_group_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelHdGroupRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/hdGroup";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
		  SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_del_hd_group(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL;
    NmpDelHdGroup *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
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


NmpMsgInfo *
nmp_parse_mss_reboot_mss_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpRebootMssRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
		  SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_reboot_mss(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpRebootMss *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

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
nmp_create_mss_get_hd_format_progress(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetHdFormatProgress *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);
   // node = xmlNewNode(NULL, BAD_CAST "hdGroup");
  //  xmlAddChild(root_node, node);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


NmpMsgInfo *
nmp_parse_mss_get_hd_format_progress_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetHdFormatProgressRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
		  SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))
            {
                nmp_deal_value(doc, cur, &res_info.hd_group_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdId")))
            {
                nmp_deal_value(doc, cur, &res_info.hd_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdName")))
            {
                nmp_deal_text(doc, cur, res_info.hd_name,HD_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"percent")))
            {
                nmp_deal_value(doc, cur, &res_info.percent);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_query_gu_record_status(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpQueryGuRecordStatus *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_query_gu_record_status_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryGuRecordStatusRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
		  SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            {
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"status")))
            {
                nmp_deal_value(doc, cur, &res_info.status);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"statusCode")))
            {
                nmp_deal_value(doc, cur, &res_info.status_code);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


NmpMsgInfo *
nmp_parse_mss_get_store_log_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetStoreLogRes *res_info = NULL;
    NmpGetStoreLogRes	tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node = NULL, node1 = NULL;
    char *path = "/message";
    char *back_count = NULL;
    int j = 0, size;

    memset(&tmp, 0, sizeof(tmp));
    app_result =  nmp_get_node(doc, (const xmlChar *)path);
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
                SET_CODE(&tmp, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessId")))
                nmp_deal_value(doc, cur, &tmp.sessId);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"logCount")))
                nmp_deal_value(doc, cur, &tmp.total_num);
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
                size = sizeof(NmpGetStoreLogRes) + tmp.req_num*sizeof(NmpStoreLog);
                res_info	= nmp_mem_kalloc(size);
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
                                nmp_deal_value(doc, node1, &res_info->store_list[j].record_type);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"begTime")))
                                nmp_deal_text(doc, node1, res_info->store_list[j].begin_time, TIME_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"endTime")))
                                nmp_deal_text(doc, node1, res_info->store_list[j].end_time, TIME_LEN);
			       else if ((!xmlStrcmp(node1->name, (const xmlChar *)"property")))
                                nmp_deal_text(doc, node1, res_info->store_list[j].property, FILE_PROPERTY_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"fileSize")))
                                nmp_deal_value(doc, node1, &res_info->store_list[j].file_size);
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
        size = sizeof(NmpGetStoreLogRes);
        res_info	= nmp_mem_kalloc(size);
        if (res_info)
            memcpy(res_info, &tmp, sizeof(tmp));
        else
            return NULL;
    }
    sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    //sys_msg = nmp_msginfo_new(cmd, res_info, size);
    return sys_msg;
}


int
nmp_create_mss_get_store_log(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetMssStoreLog *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

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
nmp_create_mss_notify_mss_change_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpChangeMss *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

    res_info = nmp_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
nmp_create_mss_alarm_link_record_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpAmsActionRecord *res_info;
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
nmp_create_mss_alarm_link_snapshot_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpAmsActionSnapshot *res_info;
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


NmpMsgInfo *
nmp_parse_mss_get_initiator_name_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetInitNameRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"initiatorName")))
                nmp_deal_text(doc, cur, res_info.init_name, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_get_initiator_name(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetInitName *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


NmpMsgInfo *
nmp_parse_mss_set_initiator_name_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpSetInitNameRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_set_initiator_name(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpSetInitName *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_get_ipsan_info_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetIpsanInfoRes *res_info = NULL;
    NmpGetIpsanInfoRes	tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node = NULL, node1 = NULL;
    char *path = "/message";
    char *count = NULL;
    int j = 0, size;

    memset(&tmp, 0, sizeof(tmp));
    app_result =  nmp_get_node(doc, (const xmlChar *)path);
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
                SET_CODE(&tmp, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
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
                size = sizeof(NmpGetIpsanInfoRes) + tmp.count*sizeof(NmpIpsanInfo);
                res_info	= nmp_mem_kalloc(size);
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
                                nmp_deal_text(doc, node1, res_info->ipsan_info[j].ip, MAX_IP_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"port")))
                                nmp_deal_value(doc, node1, &res_info->ipsan_info[j].port);
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
        size = sizeof(NmpGetIpsanInfoRes);
        res_info	= nmp_mem_kalloc(size);
        if (res_info)
            memcpy(res_info, &tmp, sizeof(tmp));
        else
            return NULL;
    }
    sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    return sys_msg;
}


int
nmp_create_mss_get_ipsan_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetIpsanInfo *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sessionId",
                       BAD_CAST req_info->session);

    return 0;
}


NmpMsgInfo *
nmp_parse_mss_add_one_ipsan_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddOneIpsanRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_add_one_ipsan(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpAddOneIpsan *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_delete_one_ipsan_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDeleteOneIpsanRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_delete_one_ipsan(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpDeleteOneIpsan *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_get_one_ipsan_detail_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetOneIpsanDetailRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i, code = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&res_info.code, code);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"target")))
                nmp_deal_text(doc, cur, res_info.target, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"connectState")))
                nmp_deal_value(doc, cur, &res_info.connect_state);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }
    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_mss_get_one_ipsan_detail(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetOneIpsanDetail *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

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


NmpMsgInfo *
nmp_parse_mss_notify_message(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpNotifyMessage req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"messageId")))
            {
                nmp_deal_value(doc, cur, &req_info.msg_id);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameter1")))
            {
                nmp_deal_text(doc, cur, req_info.param1, GENERAL_MSG_PARM_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameter2")))
            {
                nmp_deal_text(doc, cur, req_info.param2, GENERAL_MSG_PARM_LEN);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"parameter3")))
            {
                nmp_deal_text(doc, cur, req_info.param3, GENERAL_MSG_PARM_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"content")))
            {
                nmp_deal_text(doc, cur, req_info.content, DESCRIPTION_INFO_LEN);
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


