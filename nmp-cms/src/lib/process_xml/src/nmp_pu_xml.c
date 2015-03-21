/********************************************************************
 * nmp_pu_xml.c  - deal xml of pu, parse and create xml
 * Function£ºparse or create xml relate to pu.
 * Author:yangy
 * Description:users can add parse or create message of pu
 * History:
 * 2011.06.01 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 ********************************************************************/

#include <stdio.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>
#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_pu_xml.h"
#include "nmp_msg_struct.h"
#include "nmp_errno.h"
#include "nmp_xml_shared.h"
#include "nmp_memory.h"
#include "nmp_tw_interface.h"


static int
nmp_get_chan_from_guid(char *guid, int *channel)
{
	assert(guid != NULL);

	char temp[GU_ID_LEN] = {0};

	return sscanf(guid, "%22s%2d", temp, channel) != 1;
}


/**
 * nmp_parse_pu_register: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_pu_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpPuRegInfo req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, req_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
                nmp_deal_text(doc, cur, req_info.cms_ip, MAX_IP_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puType")))
                nmp_deal_value(doc, cur, &req_info.pu_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"deviceIp")))
                nmp_deal_text(doc, cur, req_info.pu_ip, MAX_IP_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puVersion")))
                nmp_deal_value(doc, cur, &req_info.pu_version);
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
 * nmp_create_pu_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_pu_register_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpPuRegRes *res_info;
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
                           BAD_CAST "puId",
                           BAD_CAST res_info->puid);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mdsIp",
                           BAD_CAST res_info->mdu_ip);
        snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->mdu_port);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mdsPort",
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
nmp_parse_pu_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpPuHeart req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
            {
                nmp_deal_text(doc, cur, req_info.puid, MAX_ID_LEN);
                xml_error("pu_heart.puid=%s\n",req_info.puid);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"deviceIp")))
                nmp_deal_text(doc, cur, req_info.pu_ip, MAX_IP_LEN );
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
nmp_create_pu_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpPuHeartResp *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
nmp_parse_pu_get_mds_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetMdsInfo req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
            {
                nmp_deal_text(doc, cur, req_info.puid, MAX_ID_LEN);
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
nmp_create_pu_get_mds_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpGetMdsInfoRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "puId",
                    BAD_CAST tmp->puid);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "ip",
                    BAD_CAST tmp->mdu_ip);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mdu_port);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "port",
                    BAD_CAST str);
    }

    return 0;
}


int
nmp_create_pu_change_dispatch(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpChangeDispatch *req_info;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);

    return 0;
}


int
nmp_pu_get_configure_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
  xmlNodePtr root_node = NULL, node = NULL;
    NmpGetDeviceInfo *req_info;

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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    return 0;
}


int
nmp_pu_get_device_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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
nmp_pu_set_device_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetDeviceInfoRes res_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
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


NmpMsgInfo *
nmp_pu_set_device_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
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


NmpMsgInfo *
nmp_parse_pu_get_platform_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetPlatformInfoRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
                nmp_deal_text(doc, cur, res_info.cms_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsPort")))
                nmp_deal_value(doc, cur, &res_info.cms_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsIp")))
                nmp_deal_text(doc, cur, res_info.mds_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsPort")))
                nmp_deal_value(doc, cur, &res_info.mds_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"protocol")))
                nmp_deal_value(doc, cur, &res_info.protocol);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"isConCms")))
                nmp_deal_value(doc, cur, &res_info.is_conn_cms);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


/**
 * nmp_create_pu_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_pu_get_platform_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_platform_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_platform_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetPlatformInfo *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "cmsIp",
                       BAD_CAST req_info->cms_ip);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->cms_port);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "cmsPort",
                           BAD_CAST str);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mdsIp",
                           BAD_CAST req_info->mds_ip);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->mds_port);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mdsPort",
                           BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->protocol);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "protocol",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->is_conn_cms);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "isConCms",
                           BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_device_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetDeviceInfoRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"manuInfo")))
                nmp_deal_text(doc, cur, res_info.manu_info, DESCRIPTION_INFO_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"releaseDate")))
                nmp_deal_text(doc, cur, res_info.release_date, TIME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"devVersion")))
                nmp_deal_text(doc, cur, res_info.dev_version, VERSION_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hwVersion")))
                nmp_deal_text(doc, cur, res_info.hardware_version, VERSION_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puType")))
                nmp_deal_value(doc, cur, &res_info.pu_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puSubType")))
                nmp_deal_value(doc, cur, &res_info.pu_sub_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"diNum")))
                nmp_deal_value(doc, cur, &res_info.di_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"doNum")))
                nmp_deal_value(doc, cur, &res_info.do_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"channelNum")))
                nmp_deal_value(doc, cur, &res_info.channel_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"RS232Num")))
                nmp_deal_value(doc, cur, &res_info.rs232_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"RS485Num")))
                nmp_deal_value(doc, cur, &res_info.rs485_num);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_device_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_get_network_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetNetworkInfoRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i, j = 0;
    xmlXPathObjectPtr app_result;
	xmlNodePtr node = NULL;
    char *xpath = "/message";
    char *type = NULL;

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&res_info, -1, sizeof(res_info));
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mainDns")))
    	         nmp_deal_text(doc, cur, res_info.main_dns, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"backupDns")))
    	         nmp_deal_text(doc, cur, res_info.sub_dns, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"autoDnsEnable")))
    	         nmp_deal_value(doc, cur, &res_info.auto_dns_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmdPort")))
    	         nmp_deal_value(doc, cur, &res_info.cmd_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dataPort")))
    	         nmp_deal_value(doc, cur, &res_info.data_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"webPort")))
    	         nmp_deal_value(doc, cur, &res_info.web_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"network")))
            {
                type = (char *)xmlGetProp(cur, (const xmlChar *)"type");
                if (!type)
                {
                    res_info.network[j].network_type = -1;
                }
                else
                {
                    res_info.network[j].network_type = atoi(type);
                    xmlFree(type);
                }

                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"ip")))
                        nmp_deal_text(doc, node, res_info.network[j].ip, MAX_IP_LEN );
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"netmask")))
                        nmp_deal_text(doc, node, res_info.network[j].netmask, MAX_IP_LEN );
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"gateway")))
                        nmp_deal_text(doc, node, res_info.network[j].gateway, MAX_IP_LEN );
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"mac")))
                        nmp_deal_text(doc, node, res_info.network[j].mac, MAX_IP_LEN);
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"dhcpEnable")))
                        nmp_deal_value(doc, node, &res_info.network[j].dhcp_enable);
                    else
                        xml_warning("Warning, not parse the node %s \n", node->name);
                    node = node->next;
                }
                j++;
            }
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_network_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_network_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_network_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpSetNetworkInfo *req_info;
    char str[INT_TO_CHAR_LEN] = {0};
    gint i = 0, type;

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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "mainDns",
                       BAD_CAST req_info->main_dns);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "backupDns",
                       BAD_CAST req_info->sub_dns);
    while (i < 3)
    {
        type = req_info->network[i].network_type;
        if ((type >= 0) &&(type < 3))
        {
            snprintf(str, INT_TO_CHAR_LEN,  "%d", type);
            node = xmlNewNode(NULL, BAD_CAST "network");
            xmlNewProp(node, BAD_CAST "type", BAD_CAST str);
            xmlAddChild(root_node, node);
            node1 = xmlNewChild(node,
                           NULL,
                           BAD_CAST "ip",
                           BAD_CAST req_info->network[i].ip);
            node1 = xmlNewChild(node,
                           NULL,
                           BAD_CAST "netmask",
                           BAD_CAST req_info->network[i].netmask);
            node1 = xmlNewChild(node,
                           NULL,
                           BAD_CAST "gateway",
                           BAD_CAST req_info->network[i].gateway);
            node1 = xmlNewChild(node,
                           NULL,
                           BAD_CAST "mac",
                           BAD_CAST req_info->network[i].mac);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->network[i].dhcp_enable);
            node1 = xmlNewChild(node,
                           NULL,
                           BAD_CAST "dhcpEnable",
                           BAD_CAST str);
        }
        i++;
    }

    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->auto_dns_enable);
    node = xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "autoDnsEnable",
                   BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->cmd_port);
    node = xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "cmdPort",
                   BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->data_port);
    node = xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "dataPort",
                   BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->web_port);
    node = xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "webPort",
                   BAD_CAST str);


    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_pppoe_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetPppoeInfoRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoeAccount")))
                nmp_deal_text(doc, cur, res_info.account, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoePasswd")))
                nmp_deal_text(doc, cur, res_info.passwd, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoeInterface")))
                nmp_deal_value(doc, cur, &res_info.interfaces);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoeEnable")))
                nmp_deal_value(doc, cur, &res_info.pppoeEnable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoeIp")))
                nmp_deal_text(doc, cur, res_info.pppoeIp, MAX_IP_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

	 xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


/**
 * nmp_create_pu_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_pu_get_pppoe_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_pppoe_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_pppoe_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetPppoeInfo *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "pppoeAccount",
                       BAD_CAST req_info->account);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "pppoePasswd",
                       BAD_CAST req_info->passwd);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->interfaces);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "pppoeInterface",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->pppoeEnable);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "pppoeEnable",
                       BAD_CAST str);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "pppoeIp",
                       BAD_CAST req_info->pppoeIp);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_media_url_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetMediaUrlRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
    	     }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"url")))
                nmp_deal_text(doc, cur, res_info.url, MAX_URL_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_media_url_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetMediaUrl *req_info;
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
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->media);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "media",
                       BAD_CAST str);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "cuIp",
                       BAD_CAST req_info->cu_ip);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_encode_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetEncodeParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
    	     }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"videoFormat")))
                nmp_deal_value(doc, cur, &res_info.video_format);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"frameRate")))
                nmp_deal_value(doc, cur, &res_info.frame_rate);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"iFrameInterval")))
                nmp_deal_value(doc, cur, &res_info.i_frame_interval);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"videoType")))
                nmp_deal_value(doc, cur, &res_info.video_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"resolution")))
                nmp_deal_value(doc, cur, &res_info.resolution);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bitRateType")))
                nmp_deal_value(doc, cur, &res_info.bit_rate_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"QpValue")))
                nmp_deal_value(doc, cur, &res_info.Qp_value);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"codeRate")))
                nmp_deal_value(doc, cur, &res_info.code_rate);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"framePriority")))
                nmp_deal_value(doc, cur, &res_info.frame_priority);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"audioType")))
                nmp_deal_value(doc, cur, &res_info.audio_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"audioEnable")))
                nmp_deal_value(doc, cur, &res_info.audio_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"audioInputMode")))
                nmp_deal_value(doc, cur, &res_info.audio_input_mode);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"encodeLevel")))
                nmp_deal_value(doc, cur, &res_info.encodeLevel);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_encode_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_encode_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_encode_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetEncodePara *req_info;
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
        BAD_CAST req_info->session
    );
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "guId",
        BAD_CAST req_info->guid
    );
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "domainId",
        BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->video_format);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "videoFormat",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->frame_rate);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "frameRate",
        BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->i_frame_interval);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "iFrameInterval",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->video_type);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "videoType",
        BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->resolution);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "resolution",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->bit_rate_type);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "bitRateType",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->Qp_value);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "QpValue",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->code_rate);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "codeRate",
        BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->frame_priority);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "framePriority",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->audio_type);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "audioType",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->audio_enable);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "audioEnable",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->audio_input_mode);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "audioInputMode",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->encodeLevel);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "encodeLevel",
        BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_display_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetDisplayParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"contrast")))
                nmp_deal_value(doc, cur, &res_info.contrast);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bright")))
                nmp_deal_value(doc, cur, &res_info.bright);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hue")))
                nmp_deal_value(doc, cur, &res_info.hue);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"saturation")))
                nmp_deal_value(doc, cur, &res_info.saturation);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sharpness")))
                nmp_deal_value(doc, cur, &res_info.sharpness);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_display_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_get_def_display_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
	return nmp_parse_pu_get_display_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_get_def_display_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	return nmp_create_pu_get_display_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_display_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_display_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetDisplayPara *req_info;
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
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->contrast);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "contrast",
                           BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->bright);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "bright",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->hue);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "hue",
                           BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->saturation);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "saturation",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->sharpness);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "sharpness",
                           BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_OSD_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetOSDParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayTimeEnable")))
                nmp_deal_value(doc, cur, &res_info.time_display_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayX")))
                nmp_deal_value(doc, cur, &res_info.time_display_x);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayY")))
                nmp_deal_value(doc, cur, &res_info.time_display_y);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayColor")))
                nmp_deal_value(doc, cur, &res_info.time_display_color);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayTextEnable")))
                nmp_deal_value(doc, cur, &res_info.text_display_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayX")))
                nmp_deal_value(doc, cur, &res_info.text_display_x);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayY")))
                nmp_deal_value(doc, cur, &res_info.text_display_y);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayColor")))
                nmp_deal_value(doc, cur, &res_info.text_display_color);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayData")))
                nmp_deal_text(doc, cur, res_info.text_display_data, TEXT_DATA_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxWidth")))
                nmp_deal_value(doc, cur, &res_info.max_width);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxHeight")))
                nmp_deal_value(doc, cur, &res_info.max_height);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayStreamEnable")))
                nmp_deal_value(doc, cur, &res_info.stream_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayW")))
                nmp_deal_value(doc, cur, &res_info.time_display_w);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayH")))
                nmp_deal_value(doc, cur, &res_info.time_display_h);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayW")))
                nmp_deal_value(doc, cur, &res_info.text_display_w);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayH")))
                nmp_deal_value(doc, cur, &res_info.text_display_h);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_OSD_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_OSD_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_OSD_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetOSDPara *req_info;
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

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_display_enable);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "displayTimeEnable",
                           BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_display_x);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "timeDisplayX",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_display_y);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "timeDisplayY",
                           BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_display_color);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "timeDisplayColor",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->text_display_enable);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "displayTextEnable",
                           BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->text_display_x);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "textDisplayX",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->text_display_y);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "textDisplayY",
                           BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->text_display_color);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "textDisplayColor",
                           BAD_CAST str);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "textDisplayData",
                           BAD_CAST req_info->text_display_data);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->max_width);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "maxWidth",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->max_height);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "maxHeight",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->stream_enable);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "displayStreamEnable",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_display_w);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "timeDisplayW",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_display_h);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "timeDisplayH",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->text_display_w);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "textDisplayW",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->text_display_h);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "textDisplayH",
                           BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_record_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetRecordParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code = 0, i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *weekpath = "/message/weekDays/weekDay";

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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"autoCover")))
                nmp_deal_value(doc, cur, &res_info.auto_cover);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"preRecord")))
                nmp_deal_value(doc, cur, &res_info.pre_record);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"allDayEnable")))
                nmp_deal_value(doc, cur, &res_info.all_day_enable);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if(!code)
    {
        nmp_get_weekday(doc, weekpath, &res_info.weekdays[0], &res_info.weekday_num);
    }

    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_record_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_record_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_record_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetRecordPara *req_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if (!req_info)
    return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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

    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->auto_cover);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "autoCover",
                       BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->pre_record);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "preRecord",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->all_day_enable);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "allDayEnable",
                       BAD_CAST str);
    node = xmlNewNode(NULL, BAD_CAST "weekDays");
    xmlAddChild(root_node, node);

    nmp_set_weekday(node, &req_info->weekdays[0], req_info->weekday_num);

    return 0;

}

NmpMsgInfo *
nmp_parse_pu_get_hide_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetHideParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *rectpath = "/message/hideArea/rect";
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hideEnable")))
                nmp_deal_value(doc, cur, &res_info.hide_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hideColor")))
                nmp_deal_value(doc, cur, &res_info.hide_color);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxHeight")))
                nmp_deal_value(doc, cur, &res_info.max_height);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxWidth")))
                nmp_deal_value(doc, cur, &res_info.max_width);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if(!code)
    {
        nmp_get_rectarea(doc, rectpath, &res_info.detect_area[0], &res_info.detect_num);
    }
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_hide_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_hide_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_hide_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetHidePara *req_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if (!req_info)
    return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hide_enable);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "hideEnable",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hide_color);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "hideColor",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->max_height);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxHeight",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->max_width);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxWidth",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->detect_num);
    node = xmlNewNode(NULL, BAD_CAST "hideArea");
    xmlNewProp(node, BAD_CAST "hideNum", BAD_CAST str);
    xmlAddChild(root_node, node);
    nmp_set_rectarea(node, &req_info->detect_area[0], req_info->detect_num);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_serial_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetSerialParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"serialNo")))
                nmp_deal_value(doc, cur, &res_info.serial_no);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"baudRate")))
                nmp_deal_value(doc, cur, &res_info.baud_rate);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dataBit")))
                nmp_deal_value(doc, cur, &res_info.data_bit);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stopBit")))
                nmp_deal_value(doc, cur, &res_info.stop_bit);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"verify")))
                nmp_deal_value(doc, cur, &res_info.verify);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

	 xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_serial_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetSerialPara *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->serial_no);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "serialNo",
                       BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_set_serial_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_serial_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetSerialPara *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->serial_no);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "serialNo",
                       BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->baud_rate);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "baudRate",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->data_bit);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "dataBit",
                       BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->stop_bit);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "stopBit",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->verify);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "verify",
                       BAD_CAST str);
    return 0;
}



NmpMsgInfo *
nmp_parse_pu_get_move_detection_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetMoveAlarmParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *weekpath = "/message/weekDays/weekDay";
    char *rectpath = "/message/detectArea/rect";
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"moveEnable")))
                nmp_deal_value(doc, cur, &res_info.move_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sensitiveLevel")))
                nmp_deal_value(doc, cur, &res_info.sensitive_level);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"detectInterval")))
                nmp_deal_value(doc, cur, &res_info.detect_interval);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxWidth")))
                nmp_deal_value(doc, cur, &res_info.max_width);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxHeight")))
                nmp_deal_value(doc, cur, &res_info.max_height);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if(!code)
    {
        nmp_get_rectarea(doc, rectpath, &res_info.detect_area[0], &res_info.detect_num);
        nmp_get_weekday(doc, weekpath, &res_info.weekdays[0], &res_info.weekday_num);
    }
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_move_detection(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_move_detection_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_move_detection(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetMoveAlarmPara *req_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if (!req_info)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->move_enable);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "moveEnable",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->sensitive_level);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sensitiveLevel",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->detect_interval);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "detectInterval",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->max_width);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxWidth",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->max_height);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxHeight",
                       BAD_CAST str);
    node = xmlNewNode(NULL, BAD_CAST "detectArea");
    xmlAddChild(root_node, node);
    nmp_set_rectarea(node, &req_info->detect_area[0], req_info->detect_num);

    node = xmlNewNode(NULL, BAD_CAST "weekDays");
    xmlAddChild(root_node, node);
    nmp_set_weekday(node, &req_info->weekdays[0], req_info->weekday_num);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_video_lost_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetVideoLostParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code = 0, i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *weekpath = "/message/weekDays/weekDay";

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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"lostEnable")))
                nmp_deal_value(doc, cur, &res_info.lost_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"detectInterval")))
                nmp_deal_value(doc, cur, &res_info.detect_interval);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if(!code)
    {
        nmp_get_weekday(doc, weekpath, &res_info.weekdays[0], &res_info.weekday_num);
    }

    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_video_lost(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}

NmpMsgInfo *
nmp_parse_pu_set_video_lost_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_video_lost(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetVideoLostPara *req_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if (!req_info)
    return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->lost_enable);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "lostEnable",
                       BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->detect_interval);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "detectInterval",
                       BAD_CAST str);


    node = xmlNewNode(NULL, BAD_CAST "weekDays");
    xmlAddChild(root_node, node);

    nmp_set_weekday(node, &req_info->weekdays[0], req_info->weekday_num);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_hide_alarm_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetHideAlarmParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *weekpath = "/message/weekDays/weekDay";
    char *rectpath = "/message/detectArea/rect";
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hideEnable")))
                nmp_deal_value(doc, cur, &res_info.hide_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"detectInterval")))
                nmp_deal_value(doc, cur, &res_info.detect_interval);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxHeight")))
                nmp_deal_value(doc, cur, &res_info.max_height);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxWidth")))
                nmp_deal_value(doc, cur, &res_info.max_width);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if(!code)
    {
        nmp_get_rectarea(doc, rectpath, &res_info.detect_area[0], &res_info.detect_num);
        nmp_get_weekday(doc, weekpath, &res_info.weekdays[0], &res_info.weekday_num);
    }
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_hide_alarm(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_hide_alarm_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_hide_alarm(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetHideAlarmPara *req_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if (!req_info)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->hide_enable);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "hideEnable",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->detect_interval);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "detectInterval",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->max_height);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxHeight",
                       BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->max_width);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxWidth",
                       BAD_CAST str);
    node = xmlNewNode(NULL, BAD_CAST "detectArea");
    xmlAddChild(root_node, node);
    nmp_set_rectarea(node, &req_info->detect_area[0], req_info->detect_num);

    node = xmlNewNode(NULL, BAD_CAST "weekDays");
    xmlAddChild(root_node, node);
    nmp_set_weekday(node, &req_info->weekdays[0], req_info->weekday_num);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_io_alarm_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetIOAlarmParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code = 0, i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *weekpath = "/message/weekDays/weekDay";

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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ioEnable")))
                nmp_deal_value(doc, cur, &res_info.io_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ioType")))
                nmp_deal_value(doc, cur, &res_info.io_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"detectInterval")))
                nmp_deal_value(doc, cur, &res_info.detect_interval);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if(!code)
    {
        nmp_get_weekday(doc, weekpath, &res_info.weekdays[0], &res_info.weekday_num);
    }

    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_io_alarm(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_io_alarm_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_io_alarm(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetIOAlarmPara *req_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if (!req_info)
    return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->io_enable);
    node = xmlNewChild(root_node,
                      NULL,
                      BAD_CAST "ioEnable",
                      BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->io_type);
    node = xmlNewChild(root_node,
                      NULL,
                      BAD_CAST "ioType",
                      BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->detect_interval);
    node = xmlNewChild(root_node,
                      NULL,
                      BAD_CAST "detectInterval",
                      BAD_CAST str);


    node = xmlNewNode(NULL, BAD_CAST "weekDays");
    xmlAddChild(root_node, node);
    nmp_set_weekday(node, &req_info->weekdays[0], req_info->weekday_num);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_joint_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetJointParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code = 0, i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node = NULL, node1;
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
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
            nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
            nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
            nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
            nmp_deal_value(doc, cur, &res_info.alarm_type);
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"jointAction")))
        {
            node = cur->xmlChildrenNode;
            while (node != NULL)
            {
                if ((!xmlStrcmp(node->name, (const xmlChar *)"jointRecord")))
                {
                    node1 = node->xmlChildrenNode;
                    while (node1 != NULL)
                    {
                        if ((!xmlStrcmp(node1->name, (const xmlChar *)"recordEnableChannel")))
                            nmp_deal_value(doc, node1, &res_info.record_channel);
                        else if((!xmlStrcmp(node1->name, (const xmlChar *)"recordSecond")))
                            nmp_deal_value(doc, node1, &res_info.record_second);
                        else
                            xml_warning("Warning, not parse the node %s \n", node1->name);

                        node1 = node1->next;
                    }
                }
                else if ((!xmlStrcmp(node->name, (const xmlChar *)"jointIO")))
                {
                    node1 = node->xmlChildrenNode;
                    while (node1 != NULL)
                    {
                        if ((!xmlStrcmp(node1->name, (const xmlChar *)"beepEnable")))
                            nmp_deal_value(doc, node1, &res_info.beep_enable);
                        else if((!xmlStrcmp(node1->name, (const xmlChar *)"beepSecond")))
                            nmp_deal_value(doc, node1, &res_info.beep_second);
                        else if((!xmlStrcmp(node1->name, (const xmlChar *)"outputEnableChannel")))
                            nmp_deal_value(doc, node1, &res_info.output_channel);
                        else if((!xmlStrcmp(node1->name, (const xmlChar *)"outputTimes")))
                            nmp_deal_value(doc, node1, &res_info.output_second);
                        else
                            xml_warning("Warning, not parse the node %s \n", node1->name);
                        node1 = node1->next;
                    }
                }
                else if ((!xmlStrcmp(node->name, (const xmlChar *)"jointSnap")))
                {
                    node1 = node->xmlChildrenNode;
                    while (node1 != NULL)
                    {
                        if ((!xmlStrcmp(node1->name, (const xmlChar *)"snapEnableChannel")))
                            nmp_deal_value(doc, node1, &res_info.snap_channel);
                        else if((!xmlStrcmp(node1->name, (const xmlChar *)"snapInterval")))
                            nmp_deal_value(doc, node1, &res_info.snap_interval);
                        else if((!xmlStrcmp(node1->name, (const xmlChar *)"snapTimes")))
                            nmp_deal_value(doc, node1, &res_info.snap_times);
                        else
                            xml_warning("Warning, not parse the node %s \n", node1->name);

                        node1 = node1->next;
                    }
                }
		  else if ((!xmlStrcmp(node->name, (const xmlChar *)"jointEmail")))
                {
                    node1 = node->xmlChildrenNode;
                    while (node1 != NULL)
                    {
                        if ((!xmlStrcmp(node1->name, (const xmlChar *)"emailEnable")))
                            nmp_deal_value(doc, node1, &res_info.email_enable);
                        else
                            xml_warning("Warning, not parse the node %s \n", node1->name);

                        node1 = node1->next;
                    }
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

    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}



int
nmp_create_pu_get_joint_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetJointPara *req_info;
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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->alarm_type);
    node = xmlNewChild(root_node,
                      NULL,
                      BAD_CAST "alarmType",
                      BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_set_joint_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_joint_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL, node3 = NULL;
    NmpSetJointPara *req_info = NULL;

    char str[INT_TO_CHAR_LEN] = {0};

    req_info = nmp_get_msginfo_data(sys_msg);
    if (!req_info)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->alarm_type);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "alarmType",
                       BAD_CAST str);

    node1 = xmlNewNode(NULL, BAD_CAST "jointAction");
    xmlAddChild(root_node, node1);

    node2 = xmlNewNode(NULL, BAD_CAST "jointRecord");
    xmlAddChild(node1, node2);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->record_channel);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "recordEnableChannel",
                      BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->record_second);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "recordSecond",
                      BAD_CAST str);

    node2 = xmlNewNode(NULL, BAD_CAST "jointIO");
    xmlAddChild(node1, node2);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->beep_enable);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "beepEnable",
                      BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->beep_second);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "beepSecond",
                      BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->output_channel);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "outputEnableChannel",
                      BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->output_second);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "outputTimes",
                      BAD_CAST str);
    node2 = xmlNewNode(NULL, BAD_CAST "jointSnap");
    xmlAddChild(node1, node2);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->snap_channel);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "snapEnableChannel",
                      BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->snap_interval);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "snapInterval",
                      BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->snap_times);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "snapTimes",
                      BAD_CAST str);
    node2 = xmlNewNode(NULL, BAD_CAST "jointEmail");
    xmlAddChild(node1, node2);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->email_enable);
    node3 = xmlNewChild(node2,
                      NULL,
                      BAD_CAST "emailEnable",
                      BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_ptz_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetPtzParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else
                nmp_parse_ptz_para(doc, cur, &res_info.ptz_para);

            cur = cur->next;
        }
    }

	 xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


/**
 * nmp_create_pu_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_pu_get_ptz_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_ptz_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_ptz_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetPtzPara *req_info;

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
   nmp_create_ptz_para(root_node, req_info->ptz_para);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_control_ptz_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_control_ptz(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpControlPtz *req_info;
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

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->direction);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "direction",
                           BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->param);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "param",
                           BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_preset_point_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    xmlNodePtr node, node1;
    NmpGetPresetPointRes tmp;
    NmpGetPresetPointRes *res_info = NULL;
    NmpMsgInfo *sys_msg = NULL;
    int i, j = 0, size;
    char *count;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if((!xmlStrcmp(cur->name, (const xmlChar *)"preset")))
           {
                count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (count)
                {
                    tmp.preset_num = atoi(count);
                    xmlFree(count);
                }
		  else
		  	tmp.preset_num = 0;

                size = sizeof(NmpGetPresetPointRes) + tmp.preset_num*sizeof(NmpPresetInfo);
                res_info = nmp_mem_kalloc(size);
                if (!res_info)
                    return NULL;
                memset(res_info, 0, sizeof(res_info));
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (tmp.preset_num <= 0)
		           break;
                    if  ((!xmlStrcmp(node->name, (const xmlChar *)"point")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"presetNo")))
                                nmp_deal_value(doc, node1, &res_info->preset_info[j].preset_no);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"presetName")))
                                nmp_deal_text(doc, node1, res_info->preset_info[j].preset_name, PRESET_NAME_LEN);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < tmp.preset_num)
                            j++;
                    }
                    node = node->next;
                }
            }

            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

     if (res_info)
        memcpy(res_info, &tmp, sizeof(tmp));
    else
    {
        size = sizeof(NmpGetPresetPointRes);
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
nmp_create_pu_get_preset_point(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_preset_point_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_preset_point(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetPresetPoint *req_info;
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
        BAD_CAST req_info->session
    );
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "guId",
        BAD_CAST req_info->guid
    );
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "domainId",
        BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->preset_action);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "presetAction",
        BAD_CAST str);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "presetName",
        BAD_CAST req_info->preset_name);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->preset_no);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "presetNo",
        BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_cruise_way_set_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    xmlNodePtr node, node1;
    NmpGetCruiseWaySetRes tmp;
    NmpGetCruiseWaySetRes *res_info = NULL;
    NmpMsgInfo *sys_msg = NULL;
    int i, j = 0, size;
    char *count;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
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
                nmp_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseSet")))
           {
                count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (count)
                {
                    tmp.cruise_num = atoi(count);
                    xmlFree(count);
                }
		  else
		  	tmp.cruise_num = 0;

                size = sizeof(NmpGetCruiseWaySetRes) + tmp.cruise_num*sizeof(NmpCruiseInfo);
                res_info = nmp_mem_kalloc(size);
                if (!res_info)
                    return NULL;
                memset(res_info, 0, sizeof(res_info));
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (tmp.cruise_num <= 0)
		           break;
                    if  ((!xmlStrcmp(node->name, (const xmlChar *)"cruiseWay")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"cruiseNo")))
                                nmp_deal_value(doc, node1, &res_info->cruise_info[j].cruise_no);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"cruiseName")))
                                nmp_deal_text(doc, node1, res_info->cruise_info[j].cruise_name, CRUISE_NAME_LEN);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < tmp.cruise_num)
                            j++;
                    }
                    node = node->next;
                }
            }

            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if (res_info)
        memcpy(res_info, &tmp, sizeof(tmp));
    else
    {
        size = sizeof(NmpGetCruiseWaySetRes);
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
nmp_create_pu_get_cruise_way_set(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_get_cruise_way_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    xmlNodePtr node, node1;
    NmpGetCruiseWayRes tmp;
    NmpGetCruiseWayRes *res_info = NULL;
    NmpMsgInfo *sys_msg = NULL;
    int i, j = 0, size, code;
    char *count;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                nmp_deal_value(doc, cur, &code);
                SET_CODE(&tmp, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseName")))
                nmp_deal_text(doc, cur, tmp.cruise_name, CRUISE_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseNo")))
                nmp_deal_value(doc, cur, &tmp.cruise_no);
            else if((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseWay")))
           {
                count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (count)
                {
                    tmp.cruise_num = atoi(count);
                    xmlFree(count);
                }
		  else
		  	tmp.cruise_num = 0;

                size = sizeof(NmpGetCruiseWayRes) + tmp.cruise_num*sizeof(NmpCruiseWayInfo);
                res_info = nmp_mem_kalloc(size);
                if (!res_info)
                    return NULL;
                memset(res_info, 0, sizeof(res_info));
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (tmp.cruise_num <= 0)
		           break;
                    if  ((!xmlStrcmp(node->name, (const xmlChar *)"point")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"presetNo")))
                                nmp_deal_value(doc, node1, &res_info->cruise_way[j].preset_no);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"speed")))
                                nmp_deal_value(doc, node1, &res_info->cruise_way[j].speed);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"dwellTime")))
                                nmp_deal_value(doc, node1, &res_info->cruise_way[j].step);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < tmp.cruise_num)
                            j++;
                    }
                    node = node->next;
                }
            }

            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if (res_info)
        memcpy(res_info, &tmp, sizeof(tmp));
    else
    {
        size = sizeof(NmpGetCruiseWayRes);
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
nmp_create_pu_get_cruise_way(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetCruiseWay *req_info;
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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", req_info->cruise_no);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "cruiseNo",
                       BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_add_cruise_way_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpAddCruiseWayRes res_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseNo")))
                nmp_deal_value(doc, cur, &res_info.cruise_no);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_add_cruise_way(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpAddCruiseWay *res_info;
	char str[INT_TO_CHAR_LEN] = {0};
	int i = 0;

	res_info = nmp_get_msginfo_data(sys_msg);
	if (!res_info)
	    return -E_NOMEM;//-ENOMEM;

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "sessionId",
	            BAD_CAST res_info->session);
	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "domainId",
	            BAD_CAST res_info->domain_id);
	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "guId",
	            BAD_CAST res_info->guid);
	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "cruiseName",
	            BAD_CAST res_info->cruise_name);

	snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_num);
	node = xmlNewNode(NULL, BAD_CAST "cruiseWay");
	xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
	xmlAddChild(root_node, node);

	while ((res_info->cruise_num > 0)&&(i < res_info->cruise_num))
	{
		node1 = xmlNewNode(NULL, BAD_CAST "point");
		xmlAddChild(node, node1);
		snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_way[i].preset_no);
		xmlNewChild(node1,
		        NULL,
		        BAD_CAST "presetNo",
		        BAD_CAST str);
		snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_way[i].speed);
		xmlNewChild(node1,
		        NULL,
		        BAD_CAST "speed",
		        BAD_CAST str);
		snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_way[i].step);
		xmlNewChild(node1,
		        NULL,
		        BAD_CAST "dwellTime",
		        BAD_CAST str);
		i++;
	}


    return 0;
}


NmpMsgInfo *
nmp_parse_pu_modify_cruise_way_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_modify_cruise_way(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpModifyCruiseWay *res_info;
	char str[INT_TO_CHAR_LEN] = {0};
	int i = 0;

	res_info = nmp_get_msginfo_data(sys_msg);
	if (!res_info)
	    return -E_NOMEM;//-ENOMEM;

	snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "resultCode",
	            BAD_CAST str);
	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "sessionId",
	            BAD_CAST res_info->session);
	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "guId",
	            BAD_CAST res_info->guid);
	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "cruiseName",
	            BAD_CAST res_info->cruise_name);
	snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_no);
	xmlNewChild(node1,
	                NULL,
	                BAD_CAST "cruiseNo",
	                BAD_CAST str);

	snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_num);
	node = xmlNewNode(NULL, BAD_CAST "cruiseWay");
	xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
	xmlAddChild(root_node, node);

	while ((res_info->cruise_num > 0)&&(i < res_info->cruise_num))
	{
		node1 = xmlNewNode(NULL, BAD_CAST "point");
		xmlAddChild(node, node1);
		snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_way[i].preset_no);
		xmlNewChild(node1,
		        NULL,
		        BAD_CAST "presetNo",
		        BAD_CAST str);
		snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_way[i].speed);
		xmlNewChild(node1,
		        NULL,
		        BAD_CAST "speed",
		        BAD_CAST str);
		snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_way[i].step);
		xmlNewChild(node1,
		        NULL,
		        BAD_CAST "dwellTime",
		        BAD_CAST str);
		i++;
	}


    return 0;
}


NmpMsgInfo *
nmp_parse_pu_set_cruise_way_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_cruise_way(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetCruiseWay *req_info;
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
        BAD_CAST req_info->session
    );
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "guId",
        BAD_CAST req_info->guid
    );
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "domainId",
        BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->cruise_action);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "cruiseAction",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->cruise_no);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "cruiseNo",
        BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_3D_control_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_3D_control(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    Nmp3DControl *req_info;
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
        BAD_CAST req_info->session
    );
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "guId",
        BAD_CAST req_info->guid
    );
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "domainId",
        BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->x_offset);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "xOffset",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->y_offset);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "yOffset",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->amplify);
    node = xmlNewChild(root_node,
        NULL,
        BAD_CAST "amplify",
        BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_3D_goback_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_3D_goback(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_get_device_time_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetDeviceTimeRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"syncEnable")))
                nmp_deal_value(doc, cur, &res_info.sync_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"time")))
                nmp_deal_text(doc, cur, res_info.server_time, TIME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeZone")))
                nmp_deal_value(doc, cur, &res_info.time_zone);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

	 xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_device_time(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_device_time_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_device_time(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetDeviceTime *req_info;
    char str[INT_TO_CHAR_LEN] = {0};

    printf("-------------nmp_create_pu_set_device_time\n");
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->sync_enable);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "syncEnable",
                           BAD_CAST str);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "time",
                           BAD_CAST req_info->server_time);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_zone);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "timeZone",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->set_flag);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "setFlag",
                           BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_ntp_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetNTPInfoRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ntpServerIp")))
                nmp_deal_text(doc, cur, res_info.ntp_server_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeZone")))
                nmp_deal_value(doc, cur, &res_info.time_zone);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeInterval")))
                nmp_deal_value(doc, cur, &res_info.time_interval);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ntpEnable")))
                nmp_deal_value(doc, cur, &res_info.ntp_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dstEnable")))
                nmp_deal_value(doc, cur, &res_info.dst_enable);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

	 xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_ntp_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_ntp_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_ntp_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetNTPInfo *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "ntpServerIp",
                           BAD_CAST req_info->ntp_server_ip);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_zone);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "timeZone",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->time_interval);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "timeInterval",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->ntp_enable);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "ntpEnable",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->dst_enable);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "dstEnable",
                           BAD_CAST str);
	return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_ftp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetFtpParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpIp")))
                nmp_deal_text(doc, cur, res_info.ftp_server_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpPort")))
                nmp_deal_value(doc, cur, &res_info.ftp_server_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpUsr")))
                nmp_deal_text(doc, cur, res_info.ftp_usr, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpPwd")))
                nmp_deal_text(doc, cur, res_info.ftp_pwd, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpPath")))
                nmp_deal_text(doc, cur, res_info.ftp_path, PATH_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_ftp_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_ftp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_ftp_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetFtpPara *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ftpIp",
                       BAD_CAST req_info->ftp_server_ip);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->ftp_server_port);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ftpPort",
                       BAD_CAST str);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ftpUsr",
                       BAD_CAST req_info->ftp_usr);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ftpPwd",
                       BAD_CAST req_info->ftp_pwd);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ftpPath",
                       BAD_CAST req_info->ftp_path);
	return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_smtp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetSmtpParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailIp")))
                nmp_deal_text(doc, cur, res_info.mail_server_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailPort")))
                nmp_deal_value(doc, cur, &res_info.mail_server_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailAddr")))
                nmp_deal_text(doc, cur, res_info.mail_addr, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailUsr")))
                nmp_deal_text(doc, cur, res_info.mail_usr, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailPwd")))
                nmp_deal_text(doc, cur, res_info.mail_pwd, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailRctp1")))
                nmp_deal_text(doc, cur, res_info.mail_rctp1, MAIL_ADDR_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailRctp2")))
                nmp_deal_text(doc, cur, res_info.mail_rctp2, MAIL_ADDR_LEN);
	        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailRctp3")))
                nmp_deal_text(doc, cur, res_info.mail_rctp3, MAIL_ADDR_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"SSLEnable")))
                nmp_deal_value(doc, cur, &res_info.ssl_enable);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_smtp_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_smtp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_smtp_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetSmtpPara *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailIp",
                           BAD_CAST req_info->mail_server_ip);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->mail_server_port);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailPort",
                           BAD_CAST str);

    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailAddr",
                           BAD_CAST req_info->mail_addr);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailUsr",
                           BAD_CAST req_info->mail_usr);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailPwd",
                           BAD_CAST req_info->mail_pwd);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailRctp1",
                           BAD_CAST req_info->mail_rctp1);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailRctp2",
                           BAD_CAST req_info->mail_rctp2);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailRctp3",
                           BAD_CAST req_info->mail_rctp3);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->ssl_enable);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "SSLEnable",
                           BAD_CAST str);

	return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_upnp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetUpnpParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpIp")))
                nmp_deal_text(doc, cur, res_info.upnp_server_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpEnable")))
                nmp_deal_value(doc, cur, &res_info.upnp_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpEthNo")))
                nmp_deal_value(doc, cur, &res_info.eth_no);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpModel")))
                nmp_deal_value(doc, cur, &res_info.model);
	         else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpRefreshTime")))
                nmp_deal_value(doc, cur, &res_info.ref_time);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpDataPort")))
                nmp_deal_value(doc, cur, &res_info.data_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpWebPort")))
                nmp_deal_value(doc, cur, &res_info.web_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpDataPortResult")))
                nmp_deal_value(doc, cur, &res_info.data_port_result);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpWebPortResult")))
                nmp_deal_value(doc, cur, &res_info.web_port_result);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}



int
nmp_create_pu_get_upnp_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_upnp_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_upnp_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetUpnpPara *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpIp",
                           BAD_CAST req_info->upnp_server_ip);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->upnp_enable);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpEnable",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->eth_no);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpEthNo",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->model);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpModel",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->ref_time);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpRefreshTime",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->data_port);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpDataPort",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->web_port);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpWebPort",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->data_port_result);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpDataPortResult",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->web_port_result);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "upnpWebPortResult",
                           BAD_CAST str);

	return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_transparent_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetTransparentParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &res_info.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"channel")))
                nmp_deal_value(doc, cur, &res_info.channel);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"length")))
                nmp_deal_value(doc, cur, &res_info.length);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"data")))
                nmp_deal_text(doc, cur, res_info.data, STRING_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}



int
nmp_create_pu_get_transparent_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetTransparentPara *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->type);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "type",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->channel);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "channel",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->length);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "length",
                           BAD_CAST str);

    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "data",
                       BAD_CAST req_info->data);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_set_transparent_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_transparent_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetTransparentPara *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->type);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "type",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->channel);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "channel",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->length);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "length",
                           BAD_CAST str);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "data",
                           BAD_CAST req_info->data);
	return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_ddns_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetDdnsParaRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"open")))
                nmp_deal_value(doc, cur, &res_info.open);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &res_info.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
                nmp_deal_value(doc, cur, &res_info.port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"times")))
                nmp_deal_text(doc, cur, res_info.times, TIME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"account")))
                nmp_deal_text(doc, cur, res_info.account, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"userName")))
                nmp_deal_text(doc, cur, res_info.username, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"password")))
                nmp_deal_text(doc, cur, res_info.password, USER_PASSWD_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}



int
nmp_create_pu_get_ddns_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_ddns_para_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_ddns_para(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetDdnsPara *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->open);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "open",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->type);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "type",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->port);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "port",
                           BAD_CAST str);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "times",
                           BAD_CAST req_info->times);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "account",
                           BAD_CAST req_info->account);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "userName",
                           BAD_CAST req_info->username);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "password",
                           BAD_CAST req_info->password);
	return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_disk_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetDiskInfoRes *res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i,num = 0;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node = NULL;
    char *xpath = "/message";
    char *diskpath = "/message/diskInfo";
    char *disk_num = NULL;
    char *disk_no = NULL;
    int j = 0, size;

    app_result =  nmp_get_node(doc, (const xmlChar *)diskpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;

    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];

        if ((!xmlStrcmp(cur->name, (const xmlChar *)"diskInfo")))
        {
            disk_num = (char *)xmlGetProp(cur, (const xmlChar *)"diskNum");
            if (disk_num)
            {
                num = atoi(disk_num);
                xmlFree(disk_num);
            }

            size = sizeof(NmpGetDiskInfoRes) + num*sizeof(NmpDiskInfo);
            res_info	= nmp_mem_kalloc(size);
	     if (!res_info)
		  return NULL;
            memset(res_info, 0, sizeof(res_info));
            res_info->disk_num = num;
            cur = cur->xmlChildrenNode;
            while (cur != NULL)
            {
                if ((!xmlStrcmp(cur->name, (const xmlChar *)"disk")))
                {
                    disk_no = (char *)xmlGetProp(cur, (const xmlChar *)"diskNo");
                    if (disk_no)
                    {
                        res_info->disk_info[j].disk_no = atoi(disk_no);
                        xmlFree(disk_no);
                    }
                    node = cur->xmlChildrenNode;
                    while (node != NULL)
                    {
                        if ((!xmlStrcmp(node->name, (const xmlChar *)"totalSize")))
                            nmp_deal_value(doc, node, &res_info->disk_info[j].total_size);
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"freeSize")))
                            nmp_deal_value(doc, node, &res_info->disk_info[j].free_size);
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"isBackup")))
                            nmp_deal_value(doc, node, &res_info->disk_info[j].is_backup);
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"diskStatus")))
                            nmp_deal_value(doc, node, &res_info->disk_info[j].status);
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"diskType")))
                            nmp_deal_value(doc, node, &res_info->disk_info[j].disk_type)	;
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"sysFileType")))
                            nmp_deal_value(doc, node, &res_info->disk_info[j].sys_file_type);
                        else
                            xml_warning("Warning, not parse the node %s \n", cur->name);

                        node = node->next;
                    }
                    j++;
                }

                cur = cur->next;
            }
        }
    }

    xmlXPathFreeObject(app_result);

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    nodeset = app_result->nodesetval;

    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
            {
                nmp_deal_value(doc, cur, &code);
                SET_CODE(res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info->puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info->session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info->domain_id, DOMAIN_ID_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    return sys_msg;
}


int
nmp_create_pu_get_disk_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_configure_info(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_get_resolution_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetResolutionInfoRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, res_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"resolution")))
                nmp_deal_value(doc, cur, &res_info.resolution);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_get_resolution_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_resolution_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_resolution_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetResolutionInfo *req_info;
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
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->resolution);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "resolution",
                           BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_ircut_control_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetIrcutControlInfoRes tmp;
    NmpGetIrcutControlInfoRes *res_info;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr cur_node = NULL,node = NULL, node1 = NULL, node2 = NULL, node3 = NULL;
    gint  code, i, j = 0, k = 0, size;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *count = NULL;
    char *channel = NULL;
    char *day = NULL;

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&tmp, 0, sizeof(tmp));
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ircutControl")))
            {
                count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (count)
                {
                    tmp.channel_count = atoi(count);
                    xmlFree(count);
                }

                size = sizeof(NmpGetIrcutControlInfoRes) + tmp.channel_count*sizeof(NmpIrcutControlInfo);
                res_info = nmp_mem_kalloc(size);
    	          if (!res_info)
    		      return NULL;
                memset(res_info, 0, size);
                cur_node = cur->xmlChildrenNode;
                while (cur_node != NULL)
                {
                    if ((!xmlStrcmp(cur_node->name, (const xmlChar *)"ircut")))
                    {
                        channel = (char *)xmlGetProp(cur_node, (const xmlChar *)"channel");
                        if (channel)
                        {
                            res_info->ircut_control_info[j].channel = atoi(channel);
                            xmlFree(channel);
                        }
                        node = cur_node->xmlChildrenNode;
                        while (node != NULL)
                        {
                            if ((!xmlStrcmp(node->name, (const xmlChar *)"switchMode")))
                                nmp_deal_value(doc, node, &res_info->ircut_control_info[j].switch_mode);
                            else if ((!xmlStrcmp(node->name, (const xmlChar *)"autoC2B")))
                                nmp_deal_value(doc, node, &res_info->ircut_control_info[j].auto_c2b);
                            else if ((!xmlStrcmp(node->name, (const xmlChar *)"autoSwitch")))
                            {
                                node1 = node->xmlChildrenNode;
                                while (node1 != NULL)
                               {
                                    if ((!xmlStrcmp(node1->name, (const xmlChar *)"sensitive")))
                                        nmp_deal_value(doc, node1, &res_info->ircut_control_info[j].sensitive);
                                    else
                                        xml_warning("Warning, not parse the node %s \n", node1->name);
                                    node1 = node1->next;
                                }
                            }
                            else if ((!xmlStrcmp(node->name, (const xmlChar *)"manuSwitch")))
                            {
                                node1 = node->xmlChildrenNode;
                                while (node1 != NULL)
                               {
                                    if ((!xmlStrcmp(node1->name, (const xmlChar *)"open")))
                                        nmp_deal_value(doc, node1, &res_info->ircut_control_info[j].open);
                                    else
                                        xml_warning("Warning, not parse the node %s \n", node1->name);
                                    node1 = node1->next;
                                }
                            }
                            else if ((!xmlStrcmp(node->name, (const xmlChar *)"rtcSwitch")))
                            {
                                node1 = node->xmlChildrenNode;
                                while (node1 != NULL)
                               {
                                    if ((!xmlStrcmp(node1->name, (const xmlChar *)"rtc")))
                                        nmp_deal_value(doc, node1, &res_info->ircut_control_info[j].rtc);
                                    else
                                        xml_warning("Warning, not parse the node %s \n", node1->name);
                                    node1 = node1->next;
                                }
                            }
                            else if ((!xmlStrcmp(node->name, (const xmlChar *)"timerSwitch")))
                            {
                                node1 = node->xmlChildrenNode;
                                while (node1 != NULL)
                               {
                                    if ((!xmlStrcmp(node1->name, (const xmlChar *)"day")))
                                    {
                                        day = (char *)xmlGetProp(node1, (const xmlChar *)"id");
                                        if (day)
                                        {
                                            res_info->ircut_control_info[j].timer_switch.weekday = atoi(day);
                                            xmlFree(day);
                                        }
                                        node2 = node1->xmlChildrenNode;

                                        k = 0;
                                        while (node2 != NULL)
                                        {
                                            if ((!xmlStrcmp(node2->name, (const xmlChar *)"segment")))
                                            {
                                                node3 = node2->xmlChildrenNode;
                                                while (node3 != NULL)
                                                {
                                                     if ((!xmlStrcmp(node3->name, (const xmlChar *)"open")))
                                                     {
                                                         nmp_deal_value(doc, node3, &res_info->ircut_control_info[j].timer_switch.seg_time[k].open);
                                                     }
                                                     else if ((!xmlStrcmp(node3->name, (const xmlChar *)"begin")))
                                                     {
                                                         nmp_deal_value(doc, node3, &res_info->ircut_control_info[j].timer_switch.seg_time[k].begin_sec);
                                                     }
                                                     else if ((!xmlStrcmp(node3->name, (const xmlChar *)"end")))
                                                     {
                                                         nmp_deal_value(doc, node3, &res_info->ircut_control_info[j].timer_switch.seg_time[k].end_sec);
                                                     }
                                                     else
                                                         xml_warning("Warning, not parse the node %s \n", node3->name);

                                                     node3 = node3->next;
                                                }
                                                k++;
                                                res_info->ircut_control_info[j].timer_switch.time_seg_num = k;
                                                if (k > 4)
                                                {
                                                    res_info->ircut_control_info[j].timer_switch.time_seg_num = 0;
                                                    break;
                                                }
                                            }
                                            else
                                                xml_warning("Warning, not parse the node %s \n", node2->name);

                                            node2 = node2->next;
                                        }


                                    }
                                    else
                                        xml_warning("Warning, not parse the node %s \n", node1->name);

                                    node1 = node1->next;
                                }
                            }
                            else
                                xml_warning("Warning, not parse the node %s \n", cur->name);

                            node = node->next;
                        }
                        j++;
                    }

                    cur_node = cur_node->next;
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
        size = sizeof(NmpGetIrcutControlInfoRes);
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
nmp_create_pu_get_ircut_control_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_pu_get_device_para(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_pu_set_ircut_control_info_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_para_resp(doc, cur, cmd);
}


int
nmp_create_pu_set_ircut_control_info(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL,
        node2 = NULL, node3 = NULL, node4 = NULL;
    NmpSetIrcutControlInfo *res_info;
    char str[INT_TO_CHAR_LEN] = {0};
    int i = 0, j = 0;

    res_info = nmp_get_msginfo_data(sys_msg);
    if (!res_info)
        return -E_NOMEM;//-ENOMEM;
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "sessionId",
                BAD_CAST res_info->session);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "domainId",
                BAD_CAST res_info->domain_id);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "guId",
                BAD_CAST res_info->guid);

    node = xmlNewNode(NULL, BAD_CAST "ircutControl");
    snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->channel_count);
    xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
    xmlAddChild(root_node, node);
    while (i < res_info->channel_count)
    {
        node1 = xmlNewNode(NULL, BAD_CAST "ircut");
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].channel);
        xmlNewProp(node1, BAD_CAST "channel", BAD_CAST str);
        xmlAddChild(node, node1);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].switch_mode);
        xmlNewChild(node1,
          NULL,
          BAD_CAST "switchMode",
          BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].auto_c2b);
        xmlNewChild(node1,
          NULL,
          BAD_CAST "autoC2B",
          BAD_CAST str);

        switch (res_info->ircut_control_info[i].switch_mode)
        {
        case 0:
        {
            node2 = xmlNewNode(NULL, BAD_CAST "autoSwitch");
            xmlAddChild(node1, node2);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].sensitive);
            xmlNewChild(node2,
              NULL,
              BAD_CAST "sensitive",
              BAD_CAST str);
            break;
        }
        /*case 1:
            node2 = xmlNewNode(NULL, BAD_CAST "manuSwitch");
            xmlAddChild(node1, node2);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].open);
            xmlNewChild(node2,
              NULL,
              BAD_CAST "open",
              BAD_CAST str);
            break;*/
        case 1:
            node2 = xmlNewNode(NULL, BAD_CAST "rtcSwitch");
            xmlAddChild(node1, node2);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].rtc);
            xmlNewChild(node2,
              NULL,
              BAD_CAST "rtc",
              BAD_CAST str);
            break;
       case 2:
           node2 = xmlNewNode(NULL, BAD_CAST "timerSwitch");
           xmlAddChild(node1, node2);
           node3 = xmlNewNode(NULL, BAD_CAST "day");
           snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].timer_switch.weekday);
           xmlNewProp(node3, BAD_CAST "id", BAD_CAST str);
           xmlAddChild(node2, node3);
           while (j < res_info->ircut_control_info[i].timer_switch.time_seg_num)
           {
               node4 = xmlNewNode(NULL, BAD_CAST "segment");
               xmlAddChild(node3, node4);

               snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].timer_switch.seg_time[j].open);
               xmlNewChild(node4,
                 NULL,
                 BAD_CAST "open",
                 BAD_CAST str);
               snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].timer_switch.seg_time[j].begin_sec);
               xmlNewChild(node4,
                 NULL,
                 BAD_CAST "begin",
                 BAD_CAST str);
               snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].timer_switch.seg_time[j].end_sec);
               xmlNewChild(node4,
                 NULL,
                 BAD_CAST "end",
                 BAD_CAST str);
               j++;
            }
            break;
        default:
            break;
        }
        i++;
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_format_disk_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_format_disk(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpFormatDisk *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->disk_no);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "diskNo",
                           BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_submit_format_pro(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSubmitFormatPos res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"diskNo")))
                nmp_deal_value(doc, cur, &res_info.disk_no);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"fmtProgress")))
                nmp_deal_value(doc, cur, &res_info.format_pro);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

	 xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


NmpMsgInfo *
nmp_parse_pu_submit_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSubmitAlarm res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmTime")))
                nmp_deal_text(doc, cur, res_info.submit_time, TIME_INFO_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &res_info.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"channel")))
                nmp_deal_value(doc, cur, &res_info.channel);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"actionType")))
                nmp_deal_value(doc, cur, &res_info.action_type);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"data")))
                nmp_deal_text(doc, cur, res_info.alarm_info, ALARM_INFO_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

	 xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


NmpMsgInfo *
nmp_parse_pu_get_store_log_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
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

    return sys_msg;
}


int
nmp_create_pu_get_store_log(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetStoreLog *req_info;
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
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->store_path);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "storePath",
                       BAD_CAST str);
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


NmpMsgInfo *
nmp_parse_pu_firmware_upgrade_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpPuUpgradeRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
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
                SET_CODE(&res_info, code);
    	     }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, res_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, res_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, res_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ip")))
                nmp_deal_text(doc, cur, res_info.ip, MAX_IP_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
                nmp_deal_value(doc, cur, &res_info.port);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));

    return sys_msg;
}


int
nmp_create_pu_firmware_upgrade(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpPuUpgrade *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "data",
                       BAD_CAST req_info->data);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->file_len);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "fileLen",
                       BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_control_device_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_pu_set_device_info_resp(doc, cur, cmd);
}


int
nmp_create_pu_control_device(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpControlDevice *req_info;
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
                       BAD_CAST "puId",
                       BAD_CAST req_info->puid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST req_info->domain_id);

    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->command);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "command",
                           BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_div_mode(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpPuGetDivMode req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, req_info.puid, MAX_ID_LEN );
            else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}


int
nmp_create_pu_get_div_mode_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, nnode = NULL;
    NmpGetDivModeRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};
    int count;

    res_info = nmp_get_msginfo_data(sys_msg);
    if(!res_info)
        return -E_NOMSGINFO;

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(res_info));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
        count = res_info->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "divisionInfo");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);
        }

	  int i;
	  for (i = 0; i < res_info->back_num; i++)
      	  {
      		NmpDivMode *div_to_dec = &res_info->scr_div_info[i];

      		nnode = xmlNewNode(NULL, BAD_CAST "divisionMode");
      		xmlAddChild(node, nnode);

      		snprintf(str, INT_TO_CHAR_LEN, "%d", div_to_dec->div_id);
      		xmlNewChild(nnode,
      			NULL,
      			BAD_CAST "divisionId",
      			BAD_CAST str);

      		xmlNewChild(nnode,
      			NULL,
      			BAD_CAST "divisionName",
      			BAD_CAST div_to_dec->div_name);

      		xmlNewChild(nnode,
      			NULL,
      			BAD_CAST "description",
      			BAD_CAST div_to_dec->description);
      	}
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_get_scr_state_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetScrStateRes *res_info = NULL;
    NmpGetScrStateRes	tmp;
    NmpMsgInfo *sys_msg = NULL;
    int  code, i;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionId")))
                nmp_deal_value(doc, cur, &tmp.div_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenLockState")))
                nmp_deal_value(doc, cur, &tmp.scr_lock_state);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"fullScreenState")))
            {
                back_count = (char *)xmlGetProp(cur, (const xmlChar *)"mode");
                if (back_count)
                {
                    tmp.full_scr_state.mode = atoi(back_count);
                    xmlFree(back_count);
                }
		   else
		  	tmp.full_scr_state.mode = 0;
                if (tmp.full_scr_state.mode)
                {
    		       node = cur->xmlChildrenNode;
                    while (node != NULL)
                    {
                         if ((!xmlStrcmp(node->name, (const xmlChar *)"encoderName")))
                            nmp_deal_text(doc, node, tmp.full_scr_state.enc_name, GU_NAME_LEN);
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"encoderChannel")))
                            nmp_deal_value(doc, node, &tmp.full_scr_state.enc_channel);
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"level")))
                            nmp_deal_value(doc, node, &tmp.full_scr_state.level);
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"actionType")))
                            nmp_deal_value(doc, node, &tmp.full_scr_state.action_type);
                        else if ((!xmlStrcmp(node->name, (const xmlChar *)"actionResult")))
                            nmp_deal_value(doc, node, &tmp.full_scr_state.action_result);
                        else
                            xml_warning("Warning, not parse the node %s \n", node->name);
                        node = node->next;
                    }
                }
		}
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenInfo")))
            {
                back_count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (back_count)
                {
                    tmp.back_num = atoi(back_count);
                    xmlFree(back_count);
                }
		  else
		  	tmp.back_num = 0;
                size = sizeof(NmpGetScrStateRes) + tmp.back_num * sizeof(NmpScrStateInfo);
                res_info = nmp_mem_kalloc(size);
                if (!res_info)
                    return NULL;
                memset(res_info, 0, sizeof(res_info));
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (tmp.back_num <= 0)
		           break;
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"division")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"divisionNum")))
                                nmp_deal_value(doc, node1, &res_info->scr_state_info[j].div_num);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"encoderName")))
                                nmp_deal_text(doc, node1, res_info->scr_state_info[j].enc_name, GU_NAME_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"encoderChannel")))
                                nmp_deal_value(doc, node1, &res_info->scr_state_info[j].enc_channel);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"level")))
                                nmp_deal_value(doc, node1, &res_info->scr_state_info[j].level);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"actionType")))
                                nmp_deal_value(doc, node1, &res_info->scr_state_info[j].action_type);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"actionResult")))
                                nmp_deal_value(doc, node1, &res_info->scr_state_info[j].action_result);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < tmp.back_num)
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
        size = sizeof(NmpGetScrStateRes);
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
nmp_create_pu_get_scr_state(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetScrState *req_info;
    char str[INT_TO_CHAR_LEN] = {0};
    gint channel_id = 0;

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
    nmp_get_chan_from_guid(req_info->guid.guid, &channel_id);
    snprintf(str, INT_TO_CHAR_LEN, "%d", channel_id);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "disChannel",
                           BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_pu_change_div_mode_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpCuExecuteRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    xmlXPathObjectPtr app_result;
    char *path = "/message";
    int i = 0, code = 0;

    memset(&res_info, 0, sizeof(res_info));
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
                SET_CODE(&res_info, code);
            }
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));
    return sys_msg;
}


int
nmp_create_pu_change_div_mode(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    tw_operate_to_decoder *req_info;
    char str[INT_TO_CHAR_LEN] = {0};
    int channel;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    nmp_get_chan_from_guid(req_info->dis_guid, &channel);
    snprintf(str, INT_TO_CHAR_LEN, "%d", channel);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "disChannel",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->division_id);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "divisionId",
                           BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_tw_play_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    tw_decoder_rsp res_info;
    NmpMsgInfo *sys_msg = NULL;
    gint  code, i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node = NULL, node1 = NULL;
    char *path = "/message";
    char *back_count = NULL;
    int j = 0;

    memset(&res_info, 0, sizeof(res_info));
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
                res_info.result = code;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionId")))
            {
                nmp_deal_value(doc, cur, &code);
                res_info.division_id = code;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisions")))
            {
                back_count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (back_count)
                {
                    res_info.div_sum = atoi(back_count);
                    xmlFree(back_count);
                }
		  else
		  	res_info.div_sum = 0;

                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (res_info.div_sum <= 0)
		           break;
                    if  ((!xmlStrcmp(node->name, (const xmlChar *)"division")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"divisionNum")))
                                nmp_deal_value(doc, node1, &res_info.divisions[j].division_num);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"encoderName")))
                                nmp_deal_text(doc, node1, res_info.divisions[j].ec_name, TW_MAX_VALUE_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"encoderChannel")))
                                nmp_deal_value(doc, node1, &res_info.divisions[j].ec_channel);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"level")))
                                nmp_deal_value(doc, node1, &res_info.divisions[j].level);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"actionResult")))
                                nmp_deal_value(doc, node1, &res_info.divisions[j].result);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < res_info.div_sum)
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
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));
    return sys_msg;
}


int
nmp_create_pu_tw_play(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL, node = NULL, nnode = NULL;
	tw_screen_to_decoder *screen_to_dec = NULL;
	char temp[INT_TO_CHAR_LEN] = {0};
	int channel;

	screen_to_dec = nmp_get_msginfo_data(sys_msg);

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

	snprintf(temp, INT_TO_CHAR_LEN, "%d", screen_to_dec->gp_type);
	xmlNewChild(root_node,
		NULL,
		BAD_CAST "actionType",
		BAD_CAST temp);

	xmlNewChild(root_node,
		NULL,
		BAD_CAST "name",
		BAD_CAST screen_to_dec->gp_name);

	snprintf(temp, INT_TO_CHAR_LEN, "%d", screen_to_dec->step_num);
	xmlNewChild(root_node,
		NULL,
		BAD_CAST "stepNum",
		BAD_CAST temp);

	nmp_get_chan_from_guid(screen_to_dec->dis_guid, &channel);
	snprintf(temp, INT_TO_CHAR_LEN, "%d", channel);
	xmlNewChild(root_node,
		NULL,
		BAD_CAST "disChannel",
		BAD_CAST temp);

	snprintf(temp, INT_TO_CHAR_LEN, "%d", screen_to_dec->division_id);
	xmlNewChild(root_node,
		NULL,
		BAD_CAST "divisionId",
		BAD_CAST temp);

	snprintf(temp, INT_TO_CHAR_LEN, "%d", screen_to_dec->keep_other);
	xmlNewChild(root_node,
		NULL,
		BAD_CAST "keepOther",
		BAD_CAST temp);

	node = xmlNewNode(NULL, BAD_CAST "divisions");
	snprintf(temp, INT_TO_CHAR_LEN, "%d", screen_to_dec->div_sum);
	xmlNewProp(node, BAD_CAST "count", BAD_CAST temp);
	xmlAddChild(root_node, node);

	int i;
	for (i = 0; i < screen_to_dec->div_sum; i++)
	{
		tw_division_to_decoder *div_to_dec = &screen_to_dec->divisions[i];

            nnode = xmlNewNode(NULL, BAD_CAST "division");
            xmlAddChild(node, nnode);

            snprintf(temp, INT_TO_CHAR_LEN, "%d", div_to_dec->division_num);
            xmlNewChild(nnode,
                NULL,
                BAD_CAST "divisionNum",
                BAD_CAST temp);

            xmlNewTextChild(nnode,
                NULL,
                BAD_CAST "encoderName",
                BAD_CAST div_to_dec->ec_name);

            xmlNewChild(nnode,
                NULL,
                BAD_CAST "decoderPlug",
                BAD_CAST div_to_dec->ec_dec_plug);

            xmlNewTextChild(nnode,
                NULL,
                BAD_CAST "encoderUrl",
                BAD_CAST div_to_dec->ec_url);

		if (div_to_dec->ec_url != NULL)
			snprintf(temp, INT_TO_CHAR_LEN, "%d", 0);
		else
			snprintf(temp, INT_TO_CHAR_LEN, "%d", 1);
		xmlNewChild(nnode,
			NULL,
			BAD_CAST "clearFlag",
			BAD_CAST temp);
	}

	return 0;
}


NmpMsgInfo *
nmp_parse_pu_full_screen_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpCuExecuteRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    xmlXPathObjectPtr app_result;
    char *path = "/message";
    int i = 0, code = 0;

    memset(&res_info, 0, sizeof(res_info));
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
                SET_CODE(&res_info, code);
            }
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));
    return sys_msg;
}


int
nmp_create_pu_full_screen(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    tw_operate_to_decoder *req_info;
    char str[INT_TO_CHAR_LEN] = {0};
    int channel;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    nmp_get_chan_from_guid(req_info->dis_guid, &channel);
    snprintf(str, INT_TO_CHAR_LEN, "%d", channel);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "disChannel",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->division_num);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "divisionNum",
                           BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_exit_full_screen_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpCuExecuteRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    xmlXPathObjectPtr app_result;
    char *path = "/message";
    int i = 0, code = 0;

    memset(&res_info, 0, sizeof(res_info));
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
                SET_CODE(&res_info, code);
            }
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));
    return sys_msg;
}


int
nmp_create_pu_exit_full_screen(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    tw_operate_to_decoder *req_info;
    char str[INT_TO_CHAR_LEN] = {0};
    int channel;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    nmp_get_chan_from_guid(req_info->dis_guid, &channel);
    snprintf(str, INT_TO_CHAR_LEN, "%d", channel);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "disChannel",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->division_num);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "divisionNum",
                           BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_clear_division_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpCuExecuteRes res_info;
    NmpMsgInfo *sys_msg = NULL;
    xmlXPathObjectPtr app_result;
    char *path = "/message";
    int i = 0, code = 0;

    memset(&res_info, 0, sizeof(res_info));
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
                SET_CODE(&res_info, code);
            }
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &res_info, sizeof(res_info));
    return sys_msg;
}


int
nmp_create_pu_clear_division(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    tw_operate_to_decoder *req_info;
    char str[INT_TO_CHAR_LEN] = {0};
    int channel;

    req_info = nmp_get_msginfo_data(sys_msg);
    if(!req_info)
        return -E_NOMSGINFO;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    if(!root_node)
        return -E_NEWNODE;

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

    nmp_get_chan_from_guid(req_info->dis_guid, &channel);
    snprintf(str, INT_TO_CHAR_LEN, "%d", channel);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "disChannel",
                           BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", req_info->division_num);
    node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "divisionNum",
                           BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_pu_alarm_link_io_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpAmsActionIORes res_info;
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


int
nmp_create_pu_alarm_link_io(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpAmsActionIO *res_info;
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
                       BAD_CAST "sessionId",
                       BAD_CAST res_info->session);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "guId",
                       BAD_CAST res_info->action_guid.guid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST res_info->action_guid.domain_id);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->time_len);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeLen",
                       BAD_CAST str);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "data",
                       BAD_CAST res_info->io_value);

    return 0;
}


int
nmp_create_pu_alarm_link_preset(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpAmsActionPreset *res_info;
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
                       BAD_CAST "sessionId",
                       BAD_CAST res_info->session);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "guId",
                       BAD_CAST res_info->action_guid.guid);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST res_info->action_guid.domain_id);
    snprintf(str, INT_TO_CHAR_LEN, "%d", res_info->preset_num);
    node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "presetNo",
                       BAD_CAST str);

    return 0;
}


