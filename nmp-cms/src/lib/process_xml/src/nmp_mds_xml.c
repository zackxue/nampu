/********************************************************************
 * nmp_stream_xml.c  - deal xml of stream, parse and create xml
 * Function£ºparse or create xml relate to stream.
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
#include "nmp_mds_xml.h"
#include "nmp_msg_struct.h"
#include "nmp_share_errno.h"
#include "nmp_xml_shared.h"


/**
 * nmp_parse_mds_register: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_mds_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpMdsRegister req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsId")))
                nmp_deal_text(doc, cur, req_info.mds_id, MDS_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
                nmp_deal_text(doc, cur, req_info.cms_ip, MAX_IP_LEN );
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
 * nmp_create_mds_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_mds_register_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMdsRegisterRes *res_info;
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
                           BAD_CAST "mdsId",
                           BAD_CAST res_info->mds_id);
        snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->pu_port);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "puPort",
                           BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->rtsp_port);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "rtspPort",
                           BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->keep_alive_time);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "keepAliveTime",
                           BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_mds_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMdsHeart req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsId")))
            {
                nmp_deal_text(doc, cur, req_info.mds_id, MDS_ID_LEN);
                xml_error("cu_heart.session=%s\n",req_info.mds_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
                nmp_deal_text(doc, cur, req_info.cms_ip, MAX_IP_LEN );
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
nmp_create_mds_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMdsHeartRes *tmp = NULL;
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



