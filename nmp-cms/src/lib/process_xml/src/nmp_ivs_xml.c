/********************************************************************
 * nmp_ivs_xml.c  - deal xml of ivs, parse and create xml
 * Function£ºparse or create xml relate to stream.
 * Author:yangy
 * Description:users can add parse or create message of stream,define
 *             struct about stream information
 * History:
 * 2013.08.15 - Yang Ying, initiate to create;
 ********************************************************************/

#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>
#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_ivs_xml.h"
#include "nmp_msg_struct.h"
#include "nmp_errno.h"
#include "nmp_xml_shared.h"
#include "nmp_memory.h"

/**
 * nmp_parse_ivs_register: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_ivs_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpIvsRegister req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
                nmp_deal_text(doc, cur, req_info.ivs_id, IVS_ID_LEN );
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
 * nmp_create_ivs_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_ivs_register_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpIvsRegisterRes *res_info;
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
                           BAD_CAST "ivsName",
                           BAD_CAST res_info->ivs_name);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_ivs_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpIvsHeart req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
            {
                nmp_deal_text(doc, cur, req_info.ivs_id, IVS_ID_LEN);
                xml_error("ivs_heart.ivs_id=%s\n",req_info.ivs_id);
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
nmp_create_ivs_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpIvsHeartRes *tmp = NULL;
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


int
nmp_ivs_get_device_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetDeviceChannelInfo *req_info;

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
    return 0;
}


NmpMsgInfo *
nmp_ivs_set_device_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetDeviceParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    gint  code, i;

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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


