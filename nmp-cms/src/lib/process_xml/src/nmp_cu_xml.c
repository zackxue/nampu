/********************************************************************
 * nmp_pu_xml.c  - deal xml of cu, parse and create xml
 * Function：parse or create xml relate to cu.
 * Author:yangy
 * Description:users can add parse or create message of cu
 * History:
 * 2011.06.01 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 * 2011.06.14 - Yang Ying add request device list,modify request area
 *              list,area list and device list make use of var len
 *              struct
 ********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>
#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_cu_xml.h"
#include "nmp_msg_struct.h"
#include "nmp_share_errno.h"
#include "nmp_xml.h"
#include "nmp_xml_shared.h"
#include "nmp_tw_interface.h"
#include "nmp_memory.h"
/**
 * nmp_parse_cu_login: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_cu_login(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpCuLoginInfo req_info;
    NmpMsgInfo *sys_msg = NULL;;
    int len;

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"username")))
            {
                nmp_deal_text(doc, cur, req_info.username, USER_NAME_LEN);
                xml_error("cu_info.username=%s\n",req_info.username);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"password")))
                nmp_deal_text(doc, cur, req_info.password, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cuVersion")))
                nmp_deal_value(doc, cur, &req_info.cu_version);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    printf("cu login user name=%s\n",req_info.username);
    len = sizeof(NmpCuLoginInfo);
    sys_msg = nmp_msginfo_new(cmd, &req_info, len);
    return sys_msg;
}


NmpMsgInfo *
nmp_parse_cu_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpCuHeart req_info;
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
            {
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
                xml_error("cu_heart.session=%s\n",req_info.session);
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
nmp_create_cu_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpCuHeartResp *tmp = NULL;
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


/**
 * nmp_create_cu_login_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_cu_login_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpCuLoginResp *tmp = NULL;
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
                    BAD_CAST "sessionId",
                    BAD_CAST tmp->session);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "domainName",
                    BAD_CAST tmp->domain_name);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "domainId",
                    BAD_CAST tmp->domain_id);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->root_area_id);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "rootAreaId",
                    BAD_CAST str);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "rootAreaName",
                    BAD_CAST tmp->root_area_name);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->usr_permissions);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "usrPermissions",
                    BAD_CAST str);
    }

    xmlNewChild(root_node,
                NULL,
                BAD_CAST "cmsVersion",
                BAD_CAST tmp->cms_version);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "cuMinVersion",
                BAD_CAST tmp->cu_min_version);
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->module_sets);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "moduleSets",
                BAD_CAST str);

    return 0;
}


/**
 * nmp_parse_get_all_area: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_get_all_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetArea req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"areaId")))
                nmp_deal_value(doc, cur, &req_info.area_id);
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
 * nmp_create_cu_req_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_get_all_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetAreaRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {

        count = tmp->total_num;
        snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);
        count = tmp->req_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "areaList");
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "area");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->area_info[i].area_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->area_info[i].area_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);

            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_area_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetAreaInfo req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"areaId")))
                nmp_deal_value(doc, cur, &req_info.area_id);
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


int
nmp_create_get_area_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetAreaInfoRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->gu_count);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "areaName",
                BAD_CAST tmp->area_name);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "userName",
                BAD_CAST tmp->user_name);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "userPhone",
                BAD_CAST tmp->user_phone);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "userAddress",
                BAD_CAST tmp->user_address);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "description",
                BAD_CAST tmp->description);
    }

    return 0;
}

/**
 * nmp_parse_get_device_list: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_get_device_list(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetDevice req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"areaId")))
                nmp_deal_value(doc, cur, &req_info.area_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
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
 * nmp_create_get_device_list_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_get_device_list_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL, node3 = NULL;
    NmpGetDeviceRes *tmp =NULL;
    NmpDevice *cur_device = NULL;
    NmpGu *cur_gu = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int count = 0;

    tmp = nmp_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    node = nmp_create_xml_type(
            doc, root_node,
            ATTRIBUTE_TYPE,
            sys_msg->msg_id
            );

    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        count = tmp->gu_count;
        snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
        xmlNewChild(root_node,
            NULL,
            BAD_CAST "count",
            BAD_CAST str
        );

        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "deviceList");
            xmlAddChild(root_node, node);
        }
        count = tmp->device_count;
        printf("========before list_for_each_entry  count=%d\n",count);
        if (tmp->device_list)
        {
             list_for_each_entry(cur_device, &tmp->device_list->list, list)
            {
                node1 = xmlNewNode(NULL, BAD_CAST "device");
                xmlAddChild(node, node1);

                xmlNewChild(
                    node1,
                    NULL,
                    BAD_CAST "puName",
                    BAD_CAST cur_device->pu_name
                );
                xmlNewChild(
                    node1,
                    NULL,
                    BAD_CAST "puId",
                    BAD_CAST cur_device->puid
                );

                xmlNewChild(
                    node1,
                    NULL,
                    BAD_CAST "domainId",
                    BAD_CAST cur_device->domain_id
                );

                snprintf(str, INT_TO_CHAR_LEN,  "%d", cur_device->pu_type);
                xmlNewChild(
                    node1,
                    NULL,
                    BAD_CAST "puType",
                    BAD_CAST str
                );

                snprintf(str, INT_TO_CHAR_LEN,  "%d", cur_device->pu_state);
                xmlNewChild(
                    node1,
                    NULL,
                    BAD_CAST "puState",
                    BAD_CAST str
                );
                node2 = xmlNewNode(NULL, BAD_CAST "guList");
                xmlAddChild(node1, node2);
                if (!list_empty(&cur_device->gu_list->list))
                    list_for_each_entry(cur_gu, &cur_device->gu_list->list, list)
                    {
                        node3 = xmlNewNode(NULL, BAD_CAST "gu");
                        xmlAddChild(node2, node3);
                        xmlNewChild(node3,
                            NULL,
                            BAD_CAST "guName",
                            BAD_CAST cur_gu->gu_name
                        );
                        xmlNewChild(node3,
                            NULL,
                            BAD_CAST "guid",
                            BAD_CAST cur_gu->guid
                        );
                        snprintf(str, INT_TO_CHAR_LEN,  "%d", cur_gu->gu_num);
                        xmlNewChild(node3,
                            NULL,
                            BAD_CAST "guNum",
                            BAD_CAST str
                        );
                        snprintf(str, INT_TO_CHAR_LEN,  "%d", cur_gu->gu_type);
                        xmlNewChild(node3,
                            NULL,
                            BAD_CAST "guType",
                            BAD_CAST str
                        );
                        snprintf(str, INT_TO_CHAR_LEN,  "%d", cur_gu->gu_attribute);
                        xmlNewChild(node3,
                            NULL,
                            BAD_CAST "guAttributes",
                            BAD_CAST str
                        );
                    }

                count--;
            }
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_get_area_device(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetAreaDevice req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"areaId")))
                nmp_deal_value(doc, cur, &req_info.area_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
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


int
nmp_create_get_area_device_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetAreaDeviceRes *tmp =NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int count = 0, i;

    tmp = nmp_get_msginfo_data(sys_msg);
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    node = nmp_create_xml_type(
            doc, root_node,
            ATTRIBUTE_TYPE,
            sys_msg->msg_id
            );

    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {

        count = tmp->device_count;
        snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "puCount",
                BAD_CAST str);
        count = tmp->req_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "deviceList");
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "device");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puName",
                        BAD_CAST tmp->device_list[i].pu_name);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puId",
                        BAD_CAST tmp->device_list[i].puid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "domainId",
                        BAD_CAST tmp->device_list[i].domain_id);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->device_list[i].pu_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->device_list[i].pu_state);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puState",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puIp",
                        BAD_CAST tmp->device_list[i].pu_ip);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puLastAliveTime",
                        BAD_CAST tmp->device_list[i].pu_last_alive_time);
            i++;
            count--;
        }
    }
    return 0;
}



/**
 * nmp_parse_get_media_url: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_get_media_url(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetMediaUrl req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
                nmp_deal_text(doc, cur, req_info.ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"media")))
                nmp_deal_value(doc, cur, &req_info.media);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"connectMode")))
                nmp_deal_value(doc, cur, &req_info.connect_mode);
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
 * nmp_create_get_media_url_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_get_media_url_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
   ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetMediaUrlRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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
    printf("nmp_create_request_media_resp \n");
    if (!RES_CODE(tmp))
    {
         xmlNewTextChild(root_node,
                    NULL,
                    BAD_CAST "url",
                    BAD_CAST tmp->url);
         xmlNewTextChild(root_node,
                    NULL,
                    BAD_CAST "decodePath",
                    BAD_CAST tmp->decode_path);
         xmlNewTextChild(root_node,
                    NULL,
                    BAD_CAST "decodeName",
                    BAD_CAST tmp->decode_name);
    }

    return 0;
}


int
nmp_create_change_pu_online_state(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    //fPuOnlineStatusChange *tmp = NULL;
    NmpPuOwnToAllCu *tmp = NULL;
    printf("---------enter nmp_create_change_pu_online_state");
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    printf("nmp_create_request_media_resp tmp->puid=%s\n", tmp->pu_state.puid);

    xmlNewChild(root_node,
            NULL,
            BAD_CAST "puid",
            BAD_CAST tmp->pu_state.puid);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "domainId",
            BAD_CAST tmp->pu_state.domain_id);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->pu_state.new_status);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "puState",
            BAD_CAST str);

    return 0;
}


int
nmp_create_broadcast_general_msg(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
   ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpNotifyMessage *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->msg_id);
    xmlNewChild(root_node,
          NULL,
          BAD_CAST "messageId",
          BAD_CAST str);
    xmlNewTextChild(root_node,
            NULL,
            BAD_CAST "parameter1",
            BAD_CAST tmp->param1);
    xmlNewTextChild(root_node,
            NULL,
            BAD_CAST "parameter2",
            BAD_CAST tmp->param2);
    xmlNewTextChild(root_node,
            NULL,
            BAD_CAST "parameter3",
            BAD_CAST tmp->param3);
    xmlNewTextChild(root_node,
            NULL,
            BAD_CAST "content",
            BAD_CAST tmp->content);
    return 0;
}

int
nmp_create_notify_modify_domain(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpModifyDomain *tmp = NULL;

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    printf("nmp_create_request_media_resp tmp->domain=%s\n", tmp->dm_name);

    xmlNewChild(root_node,
            NULL,
            BAD_CAST "domainName",
            BAD_CAST tmp->dm_name);
    return 0;
}


int
nmp_create_force_usr_offline(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
   ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpForceUserOffline *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->reason);
    xmlNewChild(root_node,
          NULL,
          BAD_CAST "reason",
          BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_cu_get_configure_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetDeviceInfo req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}


NmpMsgInfo *
nmp_cu_get_device_channel_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetDeviceChannelInfo req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
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
nmp_cu_set_configure_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL;
    NmpSetDeviceParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_get_platform_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
       return nmp_cu_get_configure_info(doc, cur, cmd);
}


/**
 * nmp_create_cu_get_platform_info_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_cu_get_platform_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetPlatformInfoRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "cmsIp",
                       BAD_CAST res_info->cms_ip);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cms_port);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "cmsPort",
                       BAD_CAST str);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "mdsIp",
                       BAD_CAST res_info->mds_ip);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->mds_port);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "mdsPort",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->protocol);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "protocol",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->is_conn_cms);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "isConCms",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_platform_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetPlatformInfo req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
    	         nmp_deal_text(doc, cur, req_info.cms_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsPort")))
    	         nmp_deal_value(doc, cur, &req_info.cms_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsIp")))
    	         nmp_deal_text(doc, cur, req_info.mds_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsPort")))
    	         nmp_deal_value(doc, cur, &req_info.mds_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"protocol")))
    	         nmp_deal_value(doc, cur, &req_info.protocol);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"isConCms")))
    	         nmp_deal_value(doc, cur, &req_info.is_conn_cms);
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
nmp_create_cu_set_platform_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_device_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_device_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetDeviceInfoRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "manuInfo",
                       BAD_CAST res_info->manu_info);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "releaseDate",
                       BAD_CAST res_info->release_date);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "devVersion",
                       BAD_CAST res_info->dev_version);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "hwVersion",
                       BAD_CAST res_info->hardware_version);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->pu_type);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "puType",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->pu_sub_type);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "puSubType",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->di_num);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "diNum",
                       BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->do_num);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "doNum",
                       BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->channel_num);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "channelNum",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->rs232_num);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "RS232Num",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->rs485_num);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "RS485Num",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_get_network_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_network_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetNetworkInfoRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};
    gint type = 0, i = 0;

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

    if (!RES_CODE(res_info))
    {
        node = xmlNewTextChild(root_node,
                       NULL,
                       BAD_CAST "mainDns",
                       BAD_CAST res_info->main_dns);
        node = xmlNewTextChild(root_node,
                       NULL,
                       BAD_CAST "backupDns",
                       BAD_CAST res_info->sub_dns);
        node = xmlNewTextChild(root_node,
                       NULL,
                       BAD_CAST "puId",
                       BAD_CAST res_info->puid);
        node = xmlNewTextChild(root_node,
                       NULL,
                       BAD_CAST "domainId",
                       BAD_CAST res_info->domain_id);

        while (i < 3)
        {
            type = res_info->network[i].network_type;
            if ((type >= 0) &&(type < 3))
            {
                snprintf(str, INT_TO_CHAR_LEN,  "%d", type);
                node = xmlNewNode(NULL, BAD_CAST "network");
                xmlNewProp(node, BAD_CAST "type", BAD_CAST str);
                xmlAddChild(root_node, node);
                node1 = xmlNewTextChild(node,
                                NULL,
                                BAD_CAST "ip",
                                BAD_CAST res_info->network[i].ip);
                node1 = xmlNewTextChild(node,
                                NULL,
                                BAD_CAST "netmask",
                                BAD_CAST res_info->network[i].netmask);
                node1 = xmlNewTextChild(node,
                                NULL,
                                BAD_CAST "gateway",
                                BAD_CAST res_info->network[i].gateway);
                node1 = xmlNewTextChild(node,
                                NULL,
                                BAD_CAST "mac",
                                BAD_CAST res_info->network[i].mac);
                snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->network[i].dhcp_enable);
                node1 = xmlNewChild(node,
                                NULL,
                                BAD_CAST "dhcpEnable",
                                BAD_CAST str);

            }
            i++;
         }

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->auto_dns_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "autoDnsEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cmd_port);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "cmdPort",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->data_port);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "dataPort",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->web_port);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "webPort",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_network_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetNetworkInfo req_info;
    NmpMsgInfo *sys_msg = NULL;
	xmlNodePtr node = NULL;
    gint i, j = 0;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *type = NULL;
    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&req_info, -1, sizeof(req_info));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, req_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                 nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mainDns")))
                 nmp_deal_text(doc, cur, req_info.main_dns, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"backupDns")))
                 nmp_deal_text(doc, cur, req_info.sub_dns, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"autoDnsEnable")))
                 nmp_deal_value(doc, cur, &req_info.auto_dns_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmdPort")))
                 nmp_deal_value(doc, cur, &req_info.cmd_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dataPort")))
                 nmp_deal_value(doc, cur, &req_info.data_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"webPort")))
                 nmp_deal_value(doc, cur, &req_info.web_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"network")))
            {
                type = (char *)xmlGetProp(cur, (const xmlChar *)"type");
                if (!type)
                {
                    req_info.network[j].network_type = -1;
                }
                else
                {
                    req_info.network[j].network_type = atoi(type);
                    xmlFree(type);
                }

                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"ip")))
                        nmp_deal_text(doc, node, req_info.network[j].ip, MAX_IP_LEN );
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"netmask")))
                        nmp_deal_text(doc, node, req_info.network[j].netmask, MAX_IP_LEN );
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"gateway")))
                        nmp_deal_text(doc, node, req_info.network[j].gateway, MAX_IP_LEN );
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"mac")))
                        nmp_deal_text(doc, node, req_info.network[j].mac, MAX_IP_LEN);
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"dhcpEnable")))
                        nmp_deal_value(doc, node, &req_info.network[j].dhcp_enable);
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
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}


int
nmp_create_cu_set_network_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}

NmpMsgInfo *
nmp_parse_cu_get_pppoe_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
       return nmp_cu_get_configure_info(doc, cur, cmd);
}


/**
 * nmp_create_cu_get_pppoe_para_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_cu_get_pppoe_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetPppoeInfoRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        node = xmlNewTextChild(root_node,
                       NULL,
                       BAD_CAST "pppoeAccount",
                       BAD_CAST res_info->account);
        node = xmlNewTextChild(root_node,
                       NULL,
                       BAD_CAST "pppoePasswd",
                       BAD_CAST res_info->passwd);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->interfaces);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "pppoeInterface",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->pppoeEnable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "pppoeEnable",
                       BAD_CAST str);
        node = xmlNewTextChild(root_node,
                       NULL,
                       BAD_CAST "pppoeIp",
                       BAD_CAST res_info->pppoeIp);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_pppoe_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetPppoeInfo req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoeAccount")))
    	         nmp_deal_text(doc, cur, req_info.account, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoePasswd")))
    	         nmp_deal_text(doc, cur, req_info.passwd, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoeInterface")))
    	         nmp_deal_value(doc, cur, &req_info.interfaces);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoeEnable")))
    	         nmp_deal_value(doc, cur, &req_info.pppoeEnable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pppoeIp")))
    	         nmp_deal_text(doc, cur, req_info.pppoeIp, MAX_IP_LEN);
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
nmp_create_cu_set_pppoe_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_encode_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_encode_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetEncodeParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->video_format);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "videoFormat",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->frame_rate);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "frameRate",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->i_frame_interval);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "iFrameInterval",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->video_type);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "videoType",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->resolution);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "resolution",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->bit_rate_type);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "bitRateType",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->Qp_value);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "QpValue",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->code_rate);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "codeRate",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->frame_priority);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "framePriority",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->audio_type);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "audioType",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->audio_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "audioEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->audio_input_mode);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "audioInputMode",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->encodeLevel);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "encodeLevel",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_encode_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetEncodePara req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"videoFormat")))
    	         nmp_deal_value(doc, cur, &req_info.video_format);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"frameRate")))
    	         nmp_deal_value(doc, cur, &req_info.frame_rate);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"iFrameInterval")))
    	         nmp_deal_value(doc, cur, &req_info.i_frame_interval);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"videoType")))
    	         nmp_deal_value(doc, cur, &req_info.video_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"resolution")))
    	         nmp_deal_value(doc, cur, &req_info.resolution);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bitRateType")))
    	         nmp_deal_value(doc, cur, &req_info.bit_rate_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"QpValue")))
    	         nmp_deal_value(doc, cur, &req_info.Qp_value);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"codeRate")))
    	         nmp_deal_value(doc, cur, &req_info.code_rate);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"framePriority")))
    	         nmp_deal_value(doc, cur, &req_info.frame_priority);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"audioType")))
    	         nmp_deal_value(doc, cur, &req_info.audio_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"audioEnable")))
    	         nmp_deal_value(doc, cur, &req_info.audio_enable);
           else if ((!xmlStrcmp(cur->name, (const xmlChar *)"audioInputMode")))
    	         nmp_deal_value(doc, cur, &req_info.audio_input_mode);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"encodeLevel")))
    	         nmp_deal_value(doc, cur, &req_info.encodeLevel);
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
nmp_create_cu_set_encode_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_display_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_display_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetDisplayParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->contrast);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "contrast",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->bright);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "bright",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->hue);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "hue",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->saturation);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "saturation",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->sharpness);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sharpness",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_get_def_display_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
	return nmp_parse_cu_get_display_para(doc, cur, cmd);
}


int
nmp_create_cu_get_def_display_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	return nmp_create_cu_get_display_para_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_set_display_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetDisplayPara req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"contrast")))
    	         nmp_deal_value(doc, cur, &req_info.contrast);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bright")))
    	         nmp_deal_value(doc, cur, &req_info.bright);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hue")))
    	         nmp_deal_value(doc, cur, &req_info.hue);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"saturation")))
    	         nmp_deal_value(doc, cur, &req_info.saturation);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sharpness")))
    	         nmp_deal_value(doc, cur, &req_info.sharpness);

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
nmp_create_cu_set_display_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_OSD_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_OSD_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetOSDParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_display_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "displayTimeEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_display_x);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeDisplayX",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_display_y);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeDisplayY",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_display_color);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeDisplayColor",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->text_display_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "displayTextEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->text_display_x);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "textDisplayX",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->text_display_y);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "textDisplayY",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->text_display_color);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "textDisplayColor",
                       BAD_CAST str);
        node = xmlNewTextChild(root_node,
                       NULL,
                       BAD_CAST "textDisplayData",
                       BAD_CAST res_info->text_display_data);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->max_width);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxWidth",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->max_height);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxHeight",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->stream_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "displayStreamEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_display_w);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeDisplayW",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_display_h);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeDisplayH",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->text_display_w);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "textDisplayW",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->text_display_h);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "textDisplayH",
                       BAD_CAST str);
   }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_OSD_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetOSDPara req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayTimeEnable")))
    	         nmp_deal_value(doc, cur, &req_info.time_display_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayX")))
    	         nmp_deal_value(doc, cur, &req_info.time_display_x);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayY")))
    	         nmp_deal_value(doc, cur, &req_info.time_display_y);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayColor")))
    	         nmp_deal_value(doc, cur, &req_info.time_display_color);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayTextEnable")))
    	         nmp_deal_value(doc, cur, &req_info.text_display_enable);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayX")))
    	         nmp_deal_value(doc, cur, &req_info.text_display_x);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayY")))
    	         nmp_deal_value(doc, cur, &req_info.text_display_y);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayColor")))
    	         nmp_deal_value(doc, cur, &req_info.text_display_color);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayData")))
    	         nmp_deal_text(doc, cur, req_info.text_display_data, TEXT_DATA_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxWidth")))
    	         nmp_deal_value(doc, cur, &req_info.max_width);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxHeight")))
    	         nmp_deal_value(doc, cur, &req_info.max_height);
    	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayStreamEnable")))
    	         nmp_deal_value(doc, cur, &req_info.stream_enable);
    	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayW")))
    	         nmp_deal_value(doc, cur, &req_info.time_display_w);
	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeDisplayH")))
    	         nmp_deal_value(doc, cur, &req_info.time_display_h);
    	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayW")))
    	         nmp_deal_value(doc, cur, &req_info.text_display_w);
	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textDisplayH")))
    	         nmp_deal_value(doc, cur, &req_info.text_display_h);
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
nmp_create_cu_set_OSD_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}





NmpMsgInfo *
nmp_parse_cu_get_record_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_record_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetRecordParaRes *res_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->auto_cover);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "autoCover",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->pre_record);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "preRecord",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->all_day_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "allDayEnable",
                       BAD_CAST str);
        node = xmlNewNode(NULL, BAD_CAST "weekDays");
        xmlAddChild(root_node, node);
        nmp_set_weekday(node, &res_info->weekdays[0], res_info->weekday_num);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_record_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetRecordPara req_info;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *weekpath = "/message/weekDays/weekDay";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"autoCover")))
                nmp_deal_value(doc, cur, &req_info.auto_cover);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"preRecord")))
                nmp_deal_value(doc, cur, &req_info.pre_record);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"allDayEnable")))
                nmp_deal_value(doc, cur, &req_info.all_day_enable);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    nmp_get_weekday( doc, weekpath,  &req_info.weekdays[0], &req_info.weekday_num);

    return nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
}


int
nmp_create_cu_set_record_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_hide_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_hide_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetHideParaRes *res_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->hide_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "hideEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->hide_color);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "hideColor",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->max_width);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxWidth",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->max_height);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxHeight",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->detect_num);
        node = xmlNewNode(NULL, BAD_CAST "hideArea");
        xmlNewProp(node, BAD_CAST "hideNum", BAD_CAST str);
        xmlAddChild(root_node, node);
        nmp_set_rectarea(node, &res_info->detect_area[0], res_info->detect_num);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_hide_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetHidePara req_info;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *rectpath = "/message/hideArea/rect";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hideEnable")))
                nmp_deal_value(doc, cur, &req_info.hide_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hideColor")))
                nmp_deal_value(doc, cur, &req_info.hide_color);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxHeight")))
                nmp_deal_value(doc, cur, &req_info.max_height);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxWidth")))
                nmp_deal_value(doc, cur, &req_info.max_width);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    nmp_get_rectarea(doc, rectpath, &req_info.detect_area[0], &req_info.detect_num);

    return nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
}


int
nmp_create_cu_set_hide_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_serial_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetSerialPara req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"serialNo")))
    	         nmp_deal_value(doc, cur, &req_info.serial_no);
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
nmp_create_cu_get_serial_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetSerialParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->serial_no);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "serialNo",
                       BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->baud_rate);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "baudRate",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->data_bit);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "dataBit",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->stop_bit);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "stopBit",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->verify);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "verify",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_serial_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetSerialPara req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"serialNo")))
    	         nmp_deal_value(doc, cur, &req_info.serial_no);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"baudRate")))
    	         nmp_deal_value(doc, cur, &req_info.baud_rate);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dataBit")))
    	         nmp_deal_value(doc, cur, &req_info.data_bit);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stopBit")))
    	         nmp_deal_value(doc, cur, &req_info.stop_bit);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"verify")))
    	         nmp_deal_value(doc, cur, &req_info.verify);
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
nmp_create_cu_set_serial_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_move_detection(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_move_detection_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetMoveAlarmParaRes *res_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->move_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "moveEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->sensitive_level);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "sensitiveLevel",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->detect_interval);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "detectInterval",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->max_width);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxWidth",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->max_height);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxHeight",
                       BAD_CAST str);

        node = xmlNewNode(NULL, BAD_CAST "detectArea");
        xmlAddChild(root_node, node);
        nmp_set_rectarea(node, &res_info->detect_area[0], res_info->detect_num);

        node = xmlNewNode(NULL, BAD_CAST "weekDays");
        xmlAddChild(root_node, node);
        nmp_set_weekday(node, &res_info->weekdays[0], res_info->weekday_num);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_move_detection(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetMoveAlarmPara req_info;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *rectpath = "/message/detectArea/rect";
    char *weekpath = "/message/weekDays/weekDay";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"moveEnable")))
    	         nmp_deal_value(doc, cur, &req_info.move_enable);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sensitiveLevel")))
    	         nmp_deal_value(doc, cur, &req_info.sensitive_level);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"detectInterval")))
    	         nmp_deal_value(doc, cur, &req_info.detect_interval);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxWidth")))
    	         nmp_deal_value(doc, cur, &req_info.max_width);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxHeight")))
    	         nmp_deal_value(doc, cur, &req_info.max_height);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    nmp_get_rectarea(doc, rectpath, &req_info.detect_area[0], &req_info.detect_num);
    nmp_get_weekday(doc, weekpath, &req_info.weekdays[0], &req_info.weekday_num);

    return nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
}


int
nmp_create_cu_set_move_detection_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}



NmpMsgInfo *
nmp_parse_cu_get_video_lost(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_video_lost_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetVideoLostParaRes *res_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->lost_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "lostEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->detect_interval);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "detectInterval",
                       BAD_CAST str);

        node = xmlNewNode(NULL, BAD_CAST "weekDays");
        xmlAddChild(root_node, node);
        nmp_set_weekday(node, &res_info->weekdays[0], res_info->weekday_num);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_video_lost(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetVideoLostPara req_info;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *weekpath = "/message/weekDays/weekDay";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"lostEnable")))
                nmp_deal_value(doc, cur, &req_info.lost_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"detectInterval")))
                nmp_deal_value(doc, cur, &req_info.detect_interval);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    nmp_get_weekday( doc, weekpath,  &req_info.weekdays[0], &req_info.weekday_num);

    return nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
}


int
nmp_create_cu_set_video_lost_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_hide_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_hide_alarm_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetHideAlarmParaRes *res_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->hide_enable);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "hideEnable",
                           BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->detect_interval);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "detectInterval",
                           BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->max_width);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxWidth",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->max_height);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "maxHeight",
                       BAD_CAST str);
        node = xmlNewNode(NULL, BAD_CAST "detectArea");
        xmlAddChild(root_node, node);
        nmp_set_rectarea(node, &res_info->detect_area[0], res_info->detect_num);

        node = xmlNewNode(NULL, BAD_CAST "weekDays");
        xmlAddChild(root_node, node);
        nmp_set_weekday(node, &res_info->weekdays[0], res_info->weekday_num);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_hide_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetHideAlarmPara req_info;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *rectpath = "/message/detectArea/rect";
    char *weekpath = "/message/weekDays/weekDay";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hideEnable")))
    	         nmp_deal_value(doc, cur, &req_info.hide_enable);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"detectInterval")))
    	         nmp_deal_value(doc, cur, &req_info.detect_interval);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxHeight")))
                nmp_deal_value(doc, cur, &req_info.max_height);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxWidth")))
                nmp_deal_value(doc, cur, &req_info.max_width);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
   	}

    xmlXPathFreeObject(app_result);

    nmp_get_rectarea(doc, rectpath, &req_info.detect_area[0], &req_info.detect_num);
    nmp_get_weekday(doc, weekpath, &req_info.weekdays[0], &req_info.weekday_num);

    return nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
}


int
nmp_create_cu_set_hide_alarm_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_io_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_io_alarm_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetIOAlarmParaRes *res_info = NULL;
    gint weekday_num;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->io_enable);
        node = xmlNewChild(root_node,
                         NULL,
                         BAD_CAST "ioEnable",
                         BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->io_type);
        node = xmlNewChild(root_node,
                         NULL,
                         BAD_CAST "ioType",
                         BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->detect_interval);
        node = xmlNewChild(root_node,
                         NULL,
                         BAD_CAST "detectInterval",
                         BAD_CAST str);

        node = xmlNewNode(NULL, BAD_CAST "weekDays");
        xmlAddChild(root_node, node);
        weekday_num = res_info->weekday_num;
        nmp_set_weekday(node, &res_info->weekdays[0], weekday_num);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_io_alarm(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetIOAlarmPara req_info;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    char *weekpath = "/message/weekDays/weekDay";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ioEnable")))
                nmp_deal_value(doc, cur, &req_info.io_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ioType")))
                nmp_deal_value(doc, cur, &req_info.io_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"detectInterval")))
                nmp_deal_value(doc, cur, &req_info.detect_interval);
            else
                xml_warning("Warning, not parse the node %s \n", cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    nmp_get_weekday(doc, weekpath, &req_info.weekdays[0], &req_info.weekday_num);

    return nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
}


int
nmp_create_cu_set_io_alarm_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_joint_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetJointPara req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
    	         nmp_deal_value(doc, cur, &req_info.alarm_type);
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
nmp_create_cu_get_joint_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr  root_node = NULL, node = NULL,
			node1 = NULL, node2 = NULL, node3 = NULL;
    NmpGetJointParaRes *res_info = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->alarm_type);
        node = xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "alarmType",
                   BAD_CAST str);
        node1 = xmlNewNode(NULL, BAD_CAST "jointAction");
        xmlAddChild(root_node, node1);
        node2 = xmlNewNode(NULL, BAD_CAST "jointRecord");
        xmlAddChild(node1, node2);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->record_channel);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "recordEnableChannel",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->record_second);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "recordSecond",
                       BAD_CAST str);
        node2 = xmlNewNode(NULL, BAD_CAST "jointIO");
        xmlAddChild(node1, node2);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->beep_enable);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "beepEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->beep_second);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "beepSecond",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->output_channel);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "outputEnableChannel",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->output_second);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "outputTimes",
                       BAD_CAST str);
        node2 = xmlNewNode(NULL, BAD_CAST "jointSnap");
        xmlAddChild(node1, node2);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->snap_channel);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "snapEnableChannel",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->snap_interval);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "snapInterval",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->snap_times);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "snapTimes",
                       BAD_CAST str);
        node2 = xmlNewNode(NULL, BAD_CAST "jointEmail");
        xmlAddChild(node1, node2);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->email_enable);
        node3 = xmlNewChild(node2,
                       NULL,
                       BAD_CAST "emailEnable",
                       BAD_CAST str);

    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_joint_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetJointPara req_info;
    gint i = 0;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node, node1;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &req_info.alarm_type);
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
                                nmp_deal_value(doc, node1, &req_info.record_channel);
                            else if((!xmlStrcmp(node1->name, (const xmlChar *)"recordSecond")))
                                nmp_deal_value(doc, node1, &req_info.record_second);
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
                                nmp_deal_value(doc, node1, &req_info.beep_enable);
                            else if((!xmlStrcmp(node1->name, (const xmlChar *)"beepSecond")))
                                nmp_deal_value(doc, node1, &req_info.beep_second);
                            else if((!xmlStrcmp(node1->name, (const xmlChar *)"outputEnableChannel")))
                                nmp_deal_value(doc, node1, &req_info.output_channel);
                            else if((!xmlStrcmp(node1->name, (const xmlChar *)"outputTimes")))
                                nmp_deal_value(doc, node1, &req_info.output_second);
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
                                nmp_deal_value(doc, node1, &req_info.snap_channel);
                            else if((!xmlStrcmp(node1->name, (const xmlChar *)"snapInterval")))
                                nmp_deal_value(doc, node1, &req_info.snap_interval);
                            else if((!xmlStrcmp(node1->name, (const xmlChar *)"snapTimes")))
                                nmp_deal_value(doc, node1, &req_info.snap_times);
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
                                nmp_deal_value(doc, node1, &req_info.email_enable);
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

    return nmp_msginfo_new(cmd, &req_info, sizeof(req_info));
}


int
nmp_create_cu_set_joint_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_ptz_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


/**
 * nmp_create_cu_get_ptz_para_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_cu_get_ptz_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL;
    NmpGetPtzParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        nmp_create_ptz_para(root_node, res_info->ptz_para);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_ptz_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetPtzPara req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else
                nmp_parse_ptz_para(doc, cur, &req_info.ptz_para);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &req_info, sizeof(req_info));

    return sys_msg;
}


int
nmp_create_cu_set_ptz_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_control_ptz(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpControlPtz req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"direction")))
    	         nmp_deal_value(doc, cur, &req_info.direction);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"param")))
    	         nmp_deal_value(doc, cur, &req_info.param);
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
nmp_create_cu_control_ptz_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_preset_point(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_preset_point_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetPresetPointRes *res_info;
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

    if (!RES_CODE(res_info))
    {
        if (res_info->preset_num >= 0)
        {
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->preset_num);
            node = xmlNewNode(NULL, BAD_CAST "preset");
            xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);
        }
        while ((res_info->preset_num > 0)&&(i < res_info->preset_num))
        {
            node1 = xmlNewNode(NULL, BAD_CAST "point");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "presetName",
                    BAD_CAST res_info->preset_info[i].preset_name);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->preset_info[i].preset_no);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "presetNo",
                    BAD_CAST str);
            i++;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_preset_point(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetPresetPoint req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"presetAction")))
    	         nmp_deal_value(doc, cur, &req_info.preset_action);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"presetName")))
    	         nmp_deal_text(doc, cur, req_info.preset_name, PRESET_NAME_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"presetNo")))
    	         nmp_deal_value(doc, cur, &req_info.preset_no);
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
nmp_create_cu_set_preset_point_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_cruise_way_set(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_cruise_way_set_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetCruiseWaySetRes *res_info;
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

    if (!RES_CODE(res_info))
    {
        if (res_info->cruise_num >= 0)
        {
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_num);
            node = xmlNewNode(NULL, BAD_CAST "cruiseSet");
            xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);
        }
        while ((res_info->cruise_num > 0)&&(i < res_info->cruise_num))
        {
            node1 = xmlNewNode(NULL, BAD_CAST "cruiseWay");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "cruiseName",
                    BAD_CAST res_info->cruise_info[i].cruise_name);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_info[i].cruise_no);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "cruiseNo",
                    BAD_CAST str);
            i++;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_get_cruise_way(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetCruiseWay req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	          nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
    	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseNo")))
    	          nmp_deal_value(doc, cur, &req_info.cruise_no);
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
nmp_create_cu_get_cruise_way_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetCruiseWayRes *res_info;
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
                BAD_CAST "cruiseName",
                BAD_CAST res_info->cruise_name);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_no);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "cruiseNo",
                BAD_CAST str);
    if (!RES_CODE(res_info))
    {
        if (res_info->cruise_num >= 0)
        {
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_num);
            node = xmlNewNode(NULL, BAD_CAST "cruiseWay");
            xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);
        }
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
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_add_cruise_way(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    xmlNodePtr node, node1;
    NmpAddCruiseWay tmp;
    NmpAddCruiseWay *req_info = NULL;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseName")))
                nmp_deal_text(doc, cur, tmp.cruise_name, CRUISE_NAME_LEN);
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

                size = sizeof(NmpAddCruiseWay) + tmp.cruise_num*sizeof(NmpCruiseWayInfo);
                req_info = nmp_mem_kalloc(size);
                if (!req_info)
                    return NULL;
                memset(req_info, 0, sizeof(req_info));
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
                                nmp_deal_value(doc, node1, &req_info->cruise_way[j].preset_no);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"speed")))
                                nmp_deal_value(doc, node1, &req_info->cruise_way[j].speed);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"dwellTime")))
                                nmp_deal_value(doc, node1, &req_info->cruise_way[j].step);
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

     if (req_info)
        memcpy(req_info, &tmp, sizeof(tmp));
    else
    {
        size = sizeof(NmpAddCruiseWay);
        req_info	= nmp_mem_kalloc(size);
        if (req_info)
            memcpy(req_info, &tmp, sizeof(tmp));
        else
            return NULL;
    }

    sys_msg = nmp_msginfo_new_2(cmd, req_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);

    return sys_msg;
}


int
nmp_create_cu_add_cruise_way_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL;
    NmpAddCruiseWayRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->cruise_no);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "cruiseNo",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_modify_cruise_way(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    xmlNodePtr node, node1;
    NmpModifyCruiseWay tmp;
    NmpModifyCruiseWay *req_info = NULL;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseName")))
                nmp_deal_text(doc, cur, tmp.cruise_name, CRUISE_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseName")))
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

                size = sizeof(NmpModifyCruiseWay) + tmp.cruise_num*sizeof(NmpCruiseWayInfo);
                req_info = nmp_mem_kalloc(size);
                if (!req_info)
                    return NULL;
                memset(req_info, 0, sizeof(req_info));
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
                                nmp_deal_value(doc, node1, &req_info->cruise_way[j].preset_no);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"speed")))
                                nmp_deal_value(doc, node1, &req_info->cruise_way[j].speed);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"dwellTime")))
                                nmp_deal_value(doc, node1, &req_info->cruise_way[j].step);
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

     if (req_info)
        memcpy(req_info, &tmp, sizeof(tmp));
    else
    {
        size = sizeof(NmpModifyCruiseWay);
        req_info	= nmp_mem_kalloc(size);
        if (req_info)
            memcpy(req_info, &tmp, sizeof(tmp));
        else
            return NULL;
    }

    sys_msg = nmp_msginfo_new_2(cmd, req_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);

    return sys_msg;
}


int
nmp_create_cu_modify_cruise_way_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_set_cruise_way(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetCruiseWay req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseAction")))
    	         nmp_deal_value(doc, cur, &req_info.cruise_action);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cruiseNo")))
    	         nmp_deal_value(doc, cur, &req_info.cruise_no);
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
nmp_create_cu_set_cruise_way_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_3D_control(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    Nmp3DControl req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"xOffset")))
    	         nmp_deal_value(doc, cur, &req_info.x_offset);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"yOffset")))
    	         nmp_deal_value(doc, cur, &req_info.y_offset);
    	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"amplify")))
    	         nmp_deal_value(doc, cur, &req_info.amplify);
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
nmp_create_cu_3D_control_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_3D_goback(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_3D_goback_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}

NmpMsgInfo *
nmp_parse_cu_get_device_time(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_device_time_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetDeviceTimeRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->sync_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "syncEnable",
                       BAD_CAST str);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "time",
                       BAD_CAST res_info->server_time);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_zone);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeZone",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_device_time(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetDeviceTime req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"syncEnable")))
    	         nmp_deal_value(doc, cur, &req_info.sync_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"time")))
    	         nmp_deal_text(doc, cur, req_info.server_time, TIME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeZone")))
    	         nmp_deal_value(doc, cur, &req_info.time_zone);
    	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"setFlag")))
    	         nmp_deal_value(doc, cur, &req_info.set_flag);
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
nmp_create_cu_set_device_time_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_ntp_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_ntp_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetNTPInfoRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {

        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ntpServerIp",
                       BAD_CAST res_info->ntp_server_ip);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_zone);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeZone",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->time_interval);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "timeInterval",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ntp_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "ntpEnable",
                       BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->dst_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "dstEnable",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_ntp_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetNTPInfo req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ntpServerIp")))
                nmp_deal_text(doc, cur, req_info.ntp_server_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeZone")))
                nmp_deal_value(doc, cur, &req_info.time_zone);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeInterval")))
                nmp_deal_value(doc, cur, &req_info.time_interval);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ntpEnable")))
                nmp_deal_value(doc, cur, &req_info.ntp_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dstEnable")))
                nmp_deal_value(doc, cur, &req_info.dst_enable);
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
nmp_create_cu_set_ntp_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_ftp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_ftp_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetFtpParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "ftpIp",
                           BAD_CAST res_info->ftp_server_ip);
	 snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ftp_server_port);
	 node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "ftpPort",
                           BAD_CAST str);

        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "ftpUsr",
                           BAD_CAST res_info->ftp_usr);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "ftpPwd",
                           BAD_CAST res_info->ftp_pwd);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "ftpPath",
                           BAD_CAST res_info->ftp_path);

   	}

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_ftp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetFtpPara req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpIp")))
    	         nmp_deal_text(doc, cur, req_info.ftp_server_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpPort")))
    	         nmp_deal_value(doc, cur, &req_info.ftp_server_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpUsr")))
    	         nmp_deal_text(doc, cur, req_info.ftp_usr, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpPwd")))
    	         nmp_deal_text(doc, cur, req_info.ftp_pwd, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ftpPath")))
    	         nmp_deal_text(doc, cur, req_info.ftp_path, PATH_LEN);
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
nmp_create_cu_set_ftp_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_smtp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_smtp_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetSmtpParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {

        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailIp",
                           BAD_CAST res_info->mail_server_ip);
	 snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->mail_server_port);
	 node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailPort",
                           BAD_CAST str);

        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailAddr",
                           BAD_CAST res_info->mail_addr);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailUsr",
                           BAD_CAST res_info->mail_usr);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailPwd",
                           BAD_CAST res_info->mail_pwd);

        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailRctp1",
                           BAD_CAST res_info->mail_rctp1);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailRctp2",
                           BAD_CAST res_info->mail_rctp2);
        node = xmlNewChild(root_node,
                           NULL,
                           BAD_CAST "mailRctp3",
                           BAD_CAST res_info->mail_rctp3);
	 snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ssl_enable);
	 node = xmlNewChild(root_node,
                          NULL,
                          BAD_CAST "SSLEnable",
                          BAD_CAST str);
   	}

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_smtp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetSmtpPara req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailIp")))
    	         nmp_deal_text(doc, cur, req_info.mail_server_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailPort")))
    	         nmp_deal_value(doc, cur, &req_info.mail_server_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailAddr")))
    	         nmp_deal_text(doc, cur, req_info.mail_addr, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailUsr")))
    	         nmp_deal_text(doc, cur, req_info.mail_usr, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailPwd")))
    	         nmp_deal_text(doc, cur, req_info.mail_pwd, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailRctp1")))
    	         nmp_deal_text(doc, cur, req_info.mail_rctp1, MAIL_ADDR_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailRctp2")))
    	         nmp_deal_text(doc, cur, req_info.mail_rctp2, MAIL_ADDR_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mailRctp3")))
    	         nmp_deal_text(doc, cur, req_info.mail_rctp3, MAIL_ADDR_LEN);
			 else if ((!xmlStrcmp(cur->name, (const xmlChar *)"SSLEnable")))
    	         nmp_deal_value(doc, cur, &req_info.ssl_enable);
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
nmp_create_cu_set_smtp_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_upnp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_upnp_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetUpnpParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpIp",
                       BAD_CAST res_info->upnp_server_ip);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->upnp_enable);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpEnable",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->eth_no);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpEthNo",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->model);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpModel",
                       BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ref_time);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpRefreshTime",
                       BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->data_port);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpDataPort",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->web_port);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpWebPort",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->data_port_result);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpDataPortResult",
                       BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->web_port_result);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "upnpWebPortResult",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_upnp_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetUpnpPara req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpIp")))
    	         nmp_deal_text(doc, cur, req_info.upnp_server_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpEnable")))
    	         nmp_deal_value(doc, cur, &req_info.upnp_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpEthNo")))
    	         nmp_deal_value(doc, cur, &req_info.eth_no);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpModel")))
    	         nmp_deal_value(doc, cur, &req_info.model);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpRefreshTime")))
    	         nmp_deal_value(doc, cur, &req_info.ref_time);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpDataPort")))
    	         nmp_deal_value(doc, cur, &req_info.data_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpWebPort")))
    	         nmp_deal_value(doc, cur, &req_info.web_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpDataPortResult")))
    	         nmp_deal_value(doc, cur, &req_info.data_port_result);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"upnpWebPortResult")))
    	         nmp_deal_value(doc, cur, &req_info.web_port_result);
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
nmp_create_cu_set_upnp_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_transparent_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpGetTransparentPara req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
    	         nmp_deal_value(doc, cur, &req_info.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"channel")))
    	         nmp_deal_value(doc, cur, &req_info.channel);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"length")))
    	         nmp_deal_value(doc, cur, &req_info.length);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"data")))
    	         nmp_deal_text(doc, cur, req_info.data, STRING_LEN);
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
nmp_create_cu_get_transparent_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetTransparentParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->type);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "type",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->channel);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "channel",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->length);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "length",
                       BAD_CAST str);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "data",
                       BAD_CAST res_info->data);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_transparent_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetTransparentPara req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
    	         nmp_deal_value(doc, cur, &req_info.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"channel")))
    	         nmp_deal_value(doc, cur, &req_info.channel);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"length")))
    	         nmp_deal_value(doc, cur, &req_info.length);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"data")))
    	         nmp_deal_text(doc, cur, req_info.data, STRING_LEN);
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
nmp_create_cu_set_transparent_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_ddns_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_ddns_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetDdnsParaRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->open);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "open",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->type);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "type",
                       BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->port);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "port",
                       BAD_CAST str);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "times",
                       BAD_CAST res_info->times);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "account",
                       BAD_CAST res_info->account);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "userName",
                       BAD_CAST res_info->username);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "password",
                       BAD_CAST res_info->password);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_ddns_para(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetDdnsPara req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"open")))
    	         nmp_deal_value(doc, cur, &req_info.open);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
    	         nmp_deal_value(doc, cur, &req_info.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
    	         nmp_deal_value(doc, cur, &req_info.port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"times")))
    	         nmp_deal_text(doc, cur, req_info.times, TIME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"account")))
    	         nmp_deal_text(doc, cur, req_info.account, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"userName")))
    	         nmp_deal_text(doc, cur, req_info.username, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"password")))
    	         nmp_deal_text(doc, cur, req_info.password, USER_PASSWD_LEN);
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
nmp_create_cu_set_ddns_para_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_disk_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_configure_info(doc, cur, cmd);
}


int
nmp_create_cu_get_disk_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetDiskInfoRes *res_info;
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
    printf("--------------disknum=%d\n",res_info->disk_num);
    if (!RES_CODE(res_info))
    {
        if (res_info->disk_num >= 0)
        {
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->disk_num);
            node = xmlNewNode(NULL, BAD_CAST "diskInfo");
            xmlNewProp(node, BAD_CAST "diskNum", BAD_CAST str);
            xmlAddChild(root_node, node);
        }
        while ((res_info->disk_num > 0)&&(i < res_info->disk_num))
        {
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->disk_info[i].disk_no);
            node1 = xmlNewNode(NULL, BAD_CAST "disk");
            xmlNewProp(node1, BAD_CAST "diskNo", BAD_CAST str);
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->disk_info[i].total_size);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "totalSize",
                    BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->disk_info[i].free_size);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "freeSize",
                    BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->disk_info[i].is_backup);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "isBackup",
                    BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->disk_info[i].status);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "diskStatus",
                    BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->disk_info[i].disk_type);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "diskType",
                    BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->disk_info[i].sys_file_type);
            xmlNewChild(node1,
                    NULL,
                    BAD_CAST "sysFileType",
                    BAD_CAST str);
            i++;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_get_resolution_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_resolution_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpGetResolutionInfoRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(res_info))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->resolution);
        node = xmlNewChild(root_node,
                       NULL,
                       BAD_CAST "resolution",
                       BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_resolution_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetResolutionInfo req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"resolution")))
    	         nmp_deal_value(doc, cur, &req_info.resolution);
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
nmp_create_cu_set_resolution_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_cu_get_ircut_control_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    return nmp_cu_get_device_channel_para(doc, cur, cmd);
}


int
nmp_create_cu_get_ircut_control_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL,
        node2 = NULL, node3 = NULL, node4 = NULL;
    NmpGetIrcutControlInfoRes *res_info;
    char str[INT_TO_CHAR_LEN] = {0};
    int i = 0, j = 0;

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

    if (!RES_CODE(res_info))
    {
        node = xmlNewNode(NULL, BAD_CAST "ircutControl");
        snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->channel_count);
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);
        while (i < res_info->channel_count)
        {
            j = 0;
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
                node2 = xmlNewNode(NULL, BAD_CAST "autoSwitch");
                xmlAddChild(node1, node2);
                snprintf(str, INT_TO_CHAR_LEN,  "%d", res_info->ircut_control_info[i].sensitive);
                xmlNewChild(node2,
                  NULL,
                  BAD_CAST "sensitive",
                  BAD_CAST str);
                break;
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
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_ircut_control_info(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpSetIrcutControlInfo tmp;
    NmpSetIrcutControlInfo *res_info;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr cur_node = NULL, node = NULL, node1 = NULL, node2 = NULL, node3 = NULL;
    gint  i, j = 0, k = 0, size;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
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

                size = sizeof(NmpSetIrcutControlInfo) + tmp.channel_count*sizeof(NmpIrcutControlInfo);
                res_info = nmp_mem_kalloc(size);
    	        if (!res_info)
    		      return NULL;

                memset(res_info, 0, sizeof(res_info));
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
                                xml_warning("Warning, not parse the node %s \n", node->name);

                            node = node->next;
                        }
                        j++;
                    }
                    else
                       xml_warning("Warning, not parse the node %s \n", cur_node->name);
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
        size = sizeof(NmpSetIrcutControlInfo);
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
nmp_create_cu_set_ircut_control_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}

NmpMsgInfo *
nmp_parse_cu_format_disk(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpFormatDisk req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"diskNo")))
    	         nmp_deal_value(doc, cur, &req_info.disk_no);
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
nmp_create_cu_format_disk_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


int
nmp_create_cu_submit_format_pro(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpSubmitFormatPosRes *tmp = NULL;
    printf("---------enter nmp_create_change_pu_online_state");
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    printf("nmp_create_request_media_resp tmp->puid=%s\n", tmp->pro.puid);

    xmlNewChild(root_node,
              NULL,
              BAD_CAST "puId",
              BAD_CAST tmp->pro.puid);
    xmlNewChild(root_node,
              NULL,
              BAD_CAST "domainId",
              BAD_CAST tmp->pro.domain_id);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->pro.disk_no);
    xmlNewChild(root_node,
              NULL,
              BAD_CAST "diskNo",
              BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->pro.format_pro);
    xmlNewChild(root_node,
              NULL,
              BAD_CAST "fmtProgress",
              BAD_CAST str);

    return 0;
}


int
nmp_create_cu_submit_alarm(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpSubmitAlarmRes *tmp = NULL;
    printf("---------enter nmp_create_change_pu_online_state");
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    printf("nmp_create_request_media_resp tmp->puid=%s\n", tmp->alarm.puid);

    xmlNewChild(root_node,
            NULL,
            BAD_CAST "puId",
            BAD_CAST tmp->alarm.puid);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "domainId",
            BAD_CAST tmp->alarm.domain_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "guId",
            BAD_CAST tmp->alarm.guid);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "puName",
            BAD_CAST tmp->alarm.pu_name);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "guName",
            BAD_CAST tmp->alarm.gu_name);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->alarm.alarm_type);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "alarmType",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->alarm.action_type);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "actionType",
            BAD_CAST str);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "alarmTime",
            BAD_CAST tmp->alarm.alarm_time);
    xmlNewTextChild(root_node,
            NULL,
            BAD_CAST "data",
            BAD_CAST tmp->alarm.alarm_info);
    if (tmp->alarm.action_type == 0)
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->alarm.alarm_id);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "alarmId",
                BAD_CAST str);
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_cu_get_alarm(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetAlarm req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmState")))
                nmp_deal_value(doc, cur, &req_info.alarm_state);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &req_info.alarm_type);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmStartTime")))
                nmp_deal_text(doc, cur, req_info.start_time, TIME_INFO_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmEndTime")))
                nmp_deal_text(doc, cur, req_info.end_time, TIME_INFO_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
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
 * nmp_create_cu_get_alarm_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_cu_get_alarm_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetAlarmRes *tmp = NULL;
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
    if (!RES_CODE(tmp))
    {
        count = tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);

        node = xmlNewNode(NULL, BAD_CAST "alarmInfo");
        xmlAddChild(root_node, node);
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "alarmNode");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->alarm_list[i].alarm_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmId",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puId",
                        BAD_CAST tmp->alarm_list[i].puid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guId",
                        BAD_CAST tmp->alarm_list[i].guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puName",
                        BAD_CAST tmp->alarm_list[i].pu_name);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->alarm_list[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->alarm_list[i].alarm_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->alarm_list[i].state);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "state",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmTime",
                        BAD_CAST tmp->alarm_list[i].alarm_time);
	     xmlNewTextChild(node1,
                       NULL,
                       BAD_CAST "data",
                       BAD_CAST tmp->alarm_list[i].alarm_info);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_get_alarm_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetAlarmState req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmId")))
                nmp_deal_value(doc, cur, &req_info.alarm_id);
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


int
nmp_create_cu_get_alarm_state_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetAlarmStateRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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
            BAD_CAST "operator",
            BAD_CAST tmp->operator);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "dealTime",
            BAD_CAST tmp->deal_time);
    xmlNewTextChild(root_node,
            NULL,
            BAD_CAST "description",
            BAD_CAST tmp->description);
    return 0;
}


NmpMsgInfo *
nmp_parse_cu_deal_alarm(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDealAlarm req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmId")))
                nmp_deal_value(doc, cur, &req_info.alarm_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"description")))
                nmp_deal_text(doc, cur, req_info.description, DESCRIPTION_INFO_LEN);
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


int
nmp_create_cu_deal_alarm_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpDealAlarmRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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
    return 0;
}


NmpMsgInfo *
nmp_parse_cu_get_store_log(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetStoreLog req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"storePath")))
                nmp_deal_value(doc, cur, &req_info.store_path);
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
 * nmp_create_cu_get_store_log_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_cu_get_store_log_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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
nmp_parse_cu_get_mss_store_log(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
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
 * nmp_create_cu_get_mss_store_log_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_cu_get_mss_store_log_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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
nmp_parse_cu_get_gu_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetGuMss req_info;
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
 * nmp_create_cu_get_gu_mss_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_cu_get_gu_mss_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL, node3 = NULL;
    NmpGetGuMssRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    gint count, i,j;

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
    if (!RES_CODE(tmp))
    {
        count = tmp->total_num;
        node = xmlNewNode(NULL, BAD_CAST "mssList");
        xmlAddChild(root_node, node);
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "mss");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST tmp->mss_list[i].mss_id);
             xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->mss_list[i].mss_name);
	      node2 = xmlNewNode(NULL, BAD_CAST "hdGroupList");
             xmlAddChild(node1, node2);
	      for (j = 0; j < HD_GROUP_NUM; j++)
	      {
                  if (strlen(tmp->mss_list[i].hd_group[j].hd_group_id) > 0)
                  {
                      node3 = xmlNewNode(NULL, BAD_CAST "hdGroup");
                      xmlAddChild(node2, node3);
                      xmlNewChild(node3,
                                  NULL,
                                  BAD_CAST "id",
                                  BAD_CAST tmp->mss_list[i].hd_group[j].hd_group_id);
		    }
	      }

            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_firmware_upgrade(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpPuUpgrade req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"data")))
    	         nmp_deal_text(doc, cur, req_info.data, FIRMWARE_HEAD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"fileLen")))
    	         nmp_deal_value(doc, cur, &req_info.file_len);
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
nmp_create_cu_firmware_upgrade_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpPuUpgradeRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);


    xmlNewChild(root_node,
              NULL,
              BAD_CAST "ip",
              BAD_CAST tmp->ip);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->port);
    xmlNewChild(root_node,
              NULL,
              BAD_CAST "port",
              BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_cu_control_device(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpControlDevice req_info;
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
        {  printf("----------------------nmp_parse_cu_control_device-\n");
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, req_info.puid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"command")))
    	         nmp_deal_value(doc, cur, &req_info.command);
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
nmp_create_cu_control_device_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_cu_set_configure_info_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_get_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetDefenceArea req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"areaId")))
                nmp_deal_value(doc, cur, &req_info.area_id);
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


int
nmp_create_get_defence_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL, node3 = NULL, new_node= NULL;
    xmlDocPtr doc_str;
    NmpGetDefenceAreaRes *tmp = NULL;
    int count,i, xml_len;
    char str[INT_TO_CHAR_LEN] = {0};

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


    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "areaList");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "area");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->area_info[i].area_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->area_info[i].area_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->area_info[i].is_defence_area);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "isActive",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->area_info[i].defence_enable);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "defenceEnable",
                        BAD_CAST str);
	     node2 = xmlNewNode(NULL, BAD_CAST "policy");
            xmlAddChild(node1, node2);
            xml_len = strlen(tmp->area_info[i].policy);
            if (xml_len != 0)
            {
                doc_str =xmlParseMemory(tmp->area_info[i].policy, xml_len);
                if (doc_str == NULL )
                {
                    printf("Document not parsed successfully. \n");
                    return -1;
                }

                node3 = xmlDocGetRootElement(doc_str); //确定文档根元素
                if (node3 == NULL)
                {
                    xml_error("empty document\n");
                    xmlFreeDoc(doc_str);
                    return -1;
                }
                new_node = xmlDocCopyNode(node3, doc, 1);
                xmlAddChild(node2, new_node);
                xmlFreeDoc(doc_str);
            }

            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_defence_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetDefenceMap req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"defenceId")))
                nmp_deal_value(doc, cur, &req_info.defence_area_id);
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


int
nmp_create_get_defence_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetDefenceMapRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);
	 count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "maps");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "map");
            xmlAddChild(node, node1);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->map_info[i].map_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->map_info[i].map_name);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "location",
                        BAD_CAST tmp->map_info[i].map_location);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetDefenceGu req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapId")))
                nmp_deal_value(doc, cur, &req_info.map_id);
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


int
nmp_create_get_defence_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetDefenceGuRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "guList");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "gu");
            xmlAddChild(node, node1);

            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guId",
                        BAD_CAST tmp->defence_gu[i].guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guDomain",
                        BAD_CAST tmp->defence_gu[i].domain_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->defence_gu[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->defence_gu[i].gu_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->defence_gu[i].coordinate_x);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateX",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->defence_gu[i].coordinate_y);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateY",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->defence_gu[i].pu_online_state);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puOnlineState",
                        BAD_CAST str);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetMapHref req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapId")))
                nmp_deal_value(doc, cur, &req_info.map_id);
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


int
nmp_create_get_map_href_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetMapHrefRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "hrefMaps");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "map");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->map_href[i].dst_map_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->map_href[i].map_name);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "location",
                        BAD_CAST tmp->map_href[i].map_location);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->map_href[i].coordinate_x);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateX",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->map_href[i].coordinate_y);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateY",
                        BAD_CAST str);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_gu_map_location(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetGuMapLocation req_info;
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


int
nmp_create_get_gu_map_location_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetGuMapLocationRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->defence_area_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "defenceId",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->map_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "mapId",
            BAD_CAST str);
    /*xmlNewChild(root_node,
            NULL,
            BAD_CAST "mapName",
            BAD_CAST tmp->map_name);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "mapLocation",
            BAD_CAST tmp->map_location);      */
    snprintf(str, INT_TO_CHAR_LEN,  "%.4f", tmp->coordinate_x);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "coordinateX",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%.4f", tmp->coordinate_y);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "coordinateY",
            BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_get_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetTw req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
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


int
nmp_create_get_tw_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetTwRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "twList");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "tw");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tw_info[i].tw_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->tw_info[i].tw_name);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetScreen req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &req_info.tw_id);
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


int
nmp_create_get_screen_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetScreenRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "screens");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "screen");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->screen_info[i].scr_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->screen_info[i].screen_num);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "screenNum",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "screenName",
                        BAD_CAST tmp->screen_info[i].screen_name);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->screen_info[i].coordinate_x);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateX",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->screen_info[i].coordinate_y);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateY",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->screen_info[i].length);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "length",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->screen_info[i].width);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "width",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->screen_info[i].online_state);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "onlineState",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->screen_info[i].pu_minor_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puMinorType",
                        BAD_CAST str);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_div_mode(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetDivMode req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
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


int
nmp_create_get_div_mode_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetDivModeRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "divisionInfo");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);
        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "divisionMode");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_div_info[i].div_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divisionId",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divisionName",
                        BAD_CAST tmp->scr_div_info[i].div_name);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "description",
                        BAD_CAST tmp->scr_div_info[i].description);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_scr_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetScrState req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &req_info.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &req_info.screen_id);
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


int
nmp_create_get_scr_state_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetScrStateRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->div_id);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "divisionId",
                BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->scr_lock_state);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "screenLockState",
                BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->full_scr_state.mode);
        node = xmlNewNode(NULL, BAD_CAST "fullScreenState");
        xmlNewProp(node, BAD_CAST "mode", BAD_CAST str);
        xmlAddChild(root_node, node);
        if (tmp->full_scr_state.mode)
        {
            xmlNewChild(node,
                        NULL,
                        BAD_CAST "encoderName",
                        BAD_CAST tmp->full_scr_state.enc_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->full_scr_state.enc_channel);
            xmlNewChild(node,
                        NULL,
                        BAD_CAST "encoderChannel",
                        BAD_CAST str);
             snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->full_scr_state.level);
             xmlNewChild(node,
                        NULL,
                        BAD_CAST "level",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->full_scr_state.action_type);
            xmlNewChild(node,
                        NULL,
                        BAD_CAST "actionType",
                        BAD_CAST str);
             snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->full_scr_state.action_result);
             xmlNewChild(node,
                        NULL,
                        BAD_CAST "actionResult",
                        BAD_CAST str);
        }
        count = tmp->back_num;
        node = xmlNewNode(NULL, BAD_CAST "screenInfo");
        xmlAddChild(root_node, node);

        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "division");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_state_info[i].div_num);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divisionNum",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "encoderName",
                        BAD_CAST tmp->scr_state_info[i].enc_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_state_info[i].enc_channel);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "encoderChannel",
                        BAD_CAST str);
             snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_state_info[i].level);
             xmlNewChild(node1,
                        NULL,
                        BAD_CAST "level",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_state_info[i].action_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "actionType",
                        BAD_CAST str);
             snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_state_info[i].action_result);
             xmlNewChild(node1,
                        NULL,
                        BAD_CAST "actionResult",
                        BAD_CAST str);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_change_div_mode(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_operate req_info;
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
                nmp_deal_text(doc, cur, req_info.session_id, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &req_info.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &req_info.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionId")))
                nmp_deal_value(doc, cur, &req_info.division_id);
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


int
nmp_create_change_div_mode_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


NmpMsgInfo *
nmp_parse_run_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_run_step_request req_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
                nmp_deal_text(doc, cur, req_info.session_id, TW_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &req_info.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &req_info.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionId")))
                nmp_deal_value(doc, cur, &req_info.division_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionNum")))
                nmp_deal_value(doc, cur, &req_info.division_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"encoderName")))
                nmp_deal_text(doc, cur, req_info.ec_name, TW_MAX_VALUE_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, req_info.ec_domain_id, TW_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.ec_guid, TW_ID_LEN);
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


int
nmp_create_run_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


int
nmp_create_tw_play_notify(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    tw_screen_to_cu *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->gp_type);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "actionType",
                BAD_CAST str);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "groupName",
                BAD_CAST tmp->gp_name);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->tw_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "twId",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->screen_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "screenId",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->division_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "divisionId",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->keep_other);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "keepOther",
            BAD_CAST str);
    count = tmp->div_sum;
    node = xmlNewNode(NULL, BAD_CAST "screenInfo");
    xmlAddChild(root_node, node);

    i = 0;
    while (count > 0)
    {
        node1 = xmlNewNode(NULL, BAD_CAST "division");
        xmlAddChild(node, node1);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->divisions[i].division_num);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "divisionNum",
                    BAD_CAST str);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "encoderName",
                    BAD_CAST tmp->divisions[i].ec_name);
         snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->divisions[i].ec_channel);
         xmlNewChild(node1,
                    NULL,
                    BAD_CAST "encoderChannel",
                    BAD_CAST str);
         snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->divisions[i].level);
         xmlNewChild(node1,
                    NULL,
                    BAD_CAST "level",
                    BAD_CAST str);
         snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->divisions[i].result);
         xmlNewChild(node1,
                    NULL,
                    BAD_CAST "actionResult",
                    BAD_CAST str);
        i++;
        count--;
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_full_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_operate req_info;
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
                nmp_deal_text(doc, cur, req_info.session_id, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &req_info.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &req_info.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionId")))
                nmp_deal_value(doc, cur, &req_info.division_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionNum")))
                nmp_deal_value(doc, cur, &req_info.division_num);
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


int
nmp_create_full_screen_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


NmpMsgInfo *
nmp_parse_exit_full_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_operate req_info;
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
                nmp_deal_text(doc, cur, req_info.session_id, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &req_info.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &req_info.screen_id);
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


int
nmp_create_exit_full_screen_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


NmpMsgInfo *
nmp_parse_clear_division(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_operate req_info;
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
                nmp_deal_text(doc, cur, req_info.session_id, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &req_info.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &req_info.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionNum")))
                nmp_deal_value(doc, cur, &req_info.division_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"operateMode")))
                nmp_deal_value(doc, cur, &req_info.operate_mode);
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


int
nmp_create_clear_division_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


int
nmp_create_tw_operate_notify(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    tw_operate_result_to_cu *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->operate_type);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "actionType",
                BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->tw_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "twId",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->screen_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "screenId",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->division_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "divisionId",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->division_num);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "divisionNum",
            BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->operate_mode);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "operateMode",
            BAD_CAST str);
    return 0;
}


int
nmp_create_tw_decoder_online_state_notify(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpPuTwScr *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->state);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "onlineState",
                BAD_CAST str);
    count = tmp->total_num;
    node = xmlNewNode(NULL, BAD_CAST "screenInfo");
    snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
    xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
    xmlAddChild(root_node, node);

    i = 0;
    while (count > 0)
    {
        node1 = xmlNewNode(NULL, BAD_CAST "screen");
        xmlAddChild(node, node1);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_list[i].tw_id);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "twId",
                    BAD_CAST str);
         snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_list[i].scr_id);
         xmlNewChild(node1,
                    NULL,
                    BAD_CAST "screenId",
                    BAD_CAST str);
        i++;
        count--;
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetTour req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
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


int
nmp_create_get_tour_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetTourRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "tourList");
	     	snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     	xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "tour");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tour_info[i].tour_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tour_info[i].tour_num);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "tourNum",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->tour_info[i].tour_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tour_info[i].auto_jump);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "autoJump",
                        BAD_CAST str);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_get_tour_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetTourStep req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourId")))
                nmp_deal_value(doc, cur, &req_info.tour_id);
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


int
nmp_create_get_tour_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetTourStepRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "tourStepList");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "tourStep");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->step_info[i].step_no);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "stepNo",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->step_info[i].interval);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "interval",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "encoderDomain",
                        BAD_CAST tmp->step_info[i].encoder_domain);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "encoderGuId",
                        BAD_CAST tmp->step_info[i].encoder_guid);

            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_run_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_run_tour_request req_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
                nmp_deal_text(doc, cur, req_info.session_id, TW_SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &req_info.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &req_info.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionId")))
                nmp_deal_value(doc, cur, &req_info.division_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionNum")))
                nmp_deal_value(doc, cur, &req_info.division_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourId")))
               nmp_deal_value(doc, cur, &req_info.tour_id);

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


int
nmp_create_run_tour_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


NmpMsgInfo *
nmp_parse_stop_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_stop_tour_request req_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
                nmp_deal_text(doc, cur, req_info.session_id, TW_SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourId")))
                nmp_deal_value(doc, cur, &req_info.tour_id);
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


int
nmp_create_stop_tour_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


NmpMsgInfo *
nmp_parse_get_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetGroup req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &req_info.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &req_info.start_num);
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


int
nmp_create_get_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetGroupRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

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

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->total_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
        count = tmp->back_num;
        if (count > 0)
        {
            node = xmlNewNode(NULL, BAD_CAST "groupList");
	     snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	     xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);

        }
        i = 0;
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "group");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_info[i].group_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_info[i].group_num);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "groupNum",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->group_info[i].group_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_info[i].tw_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "twId",
                        BAD_CAST str);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_run_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_run_group_request req_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
                nmp_deal_text(doc, cur, req_info.session_id, TW_SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &req_info.group_id);
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


int
nmp_create_run_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


NmpMsgInfo *
nmp_parse_stop_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    tw_stop_group_request req_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
                nmp_deal_text(doc, cur, req_info.session_id, TW_SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &req_info.group_id);
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


int
nmp_create_stop_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuExecuteRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


NmpMsgInfo *
nmp_parse_get_license_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetLicenseInfo req_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
                nmp_deal_text(doc, cur, req_info.session, TW_SESSION_ID_LEN);
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


int
nmp_create_get_license_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpGetLicenseInfoRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->expired_time.type);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "type",
            BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->version);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "version",
            BAD_CAST str);

    xmlNewChild(root_node,
            NULL,
            BAD_CAST "licenseTime",
            BAD_CAST tmp->expired_time.expired_time);
    return 0;
}


NmpMsgInfo *
nmp_parse_get_tw_auth_info(xmlDocPtr doc ,
	xmlNodePtr cur, char *cmd)
{
    NmpGetTwLicenseInfo req_info;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
                nmp_deal_text(doc, cur, req_info.session, TW_SESSION_ID_LEN);
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


int
nmp_create_get_tw_auth_info_resp(xmlDocPtr doc,
	NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpGetTwLicenseInfoRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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
    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->tw_auth_type);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "twAuthType",
                 BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_alarm_link_io(xmlDocPtr doc ,
	xmlNodePtr cur, char *cmd)
{
    NmpAmsActionIO req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, req_info.action_guid.guid, MAX_ID_LEN );
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, req_info.session, SESSION_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
    	         nmp_deal_text(doc, cur, req_info.action_guid.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeLen")))
    	         nmp_deal_value(doc, cur, (int *)(&req_info.time_len));
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"data")))
    	         nmp_deal_text(doc, cur, req_info.io_value, IO_VALUE_LEN);
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
nmp_create_cu_alarm_link_io_resp(xmlDocPtr doc,
	NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpAmsActionIORes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

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

    return 0;
}


int
nmp_create_cu_alarm_link_map(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1;
    NmpAmsActionMap *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int count, i = 0;

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node,
            NULL,
            BAD_CAST "guId",
            BAD_CAST tmp->action_guid.guid);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "domainId",
            BAD_CAST tmp->action_guid.domain_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "guName",
            BAD_CAST tmp->gu_name);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->defence_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "defenceId",
            BAD_CAST str);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "defenceName",
            BAD_CAST tmp->defence_name);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->map_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "mapId",
            BAD_CAST str);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "mapName",
            BAD_CAST tmp->map_name);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->alarm_type);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "alarmType",
            BAD_CAST str);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "alarmTime",
            BAD_CAST tmp->alarm_time);
    count = tmp->action_gu_count;
    node = xmlNewNode(NULL, BAD_CAST "linkage");
 	snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
 	xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
    xmlAddChild(root_node, node);
    while (count > 0)
    {
		node1 = xmlNewNode(NULL, BAD_CAST "gu");
        xmlAddChild(node, node1);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "guId",
                    BAD_CAST tmp->action_gu[i].action_guid.guid);
		xmlNewChild(node1,
                    NULL,
                    BAD_CAST "domainId",
                    BAD_CAST tmp->action_gu[i].action_guid.domain_id);
		xmlNewChild(node1,
                    NULL,
                    BAD_CAST "guName",
                    BAD_CAST tmp->action_gu[i].gu_name);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->action_gu[i].map_id);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "mapId",
                    BAD_CAST str);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "mapName",
                    BAD_CAST tmp->action_gu[i].map_name);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->action_gu[i].level);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "level",
                    BAD_CAST str);

		i++;
		count--;
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_cu_modify_user_pwd(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpCuModifyUserPwd req_info;
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
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"oldPassword")))
                nmp_deal_text(doc, cur, req_info.old_password, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"newPassword")))
                nmp_deal_text(doc, cur, req_info.new_password, USER_PASSWD_LEN);
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


int
nmp_create_cu_modify_user_pwd_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpCuModifyUserPwdRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(&tmp->code));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_cu_query_guid(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuQueryGuid tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.user, "user", USER_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.gu_num, "guNum")
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.session, "sessionId", SESSION_ID_LEN)

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_cu_query_guid_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NmpCuQueryGuidRes *tmp = NULL;

	NMP_XML_DEF_CREATE_BEGIN_SINGLE(tmp);

	NMP_XML_DEF_STR_CREATE_CHILD_SINGLE("guid", tmp->guid);
	NMP_XML_DEF_STR_CREATE_CHILD_SINGLE("domain", tmp->domain);
	NMP_XML_DEF_STR_CREATE_CHILD_SINGLE("guName", tmp->gu_name);
	NMP_XML_DEF_VAL_CREATE_CHILD_SINGLE("level", tmp->level);
	NMP_XML_DEF_VAL_CREATE_CHILD_SINGLE("guAttributes", tmp->gu_attributes);

	NMP_XML_DEF_CREATE_END_SINGLE();
}


NmpMsgInfo *
nmp_parse_cu_query_screen_id(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuQueryScreenID tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.user, "user", USER_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.screen_num, "screenNum")
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.session, "sessionId", SESSION_ID_LEN)

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_cu_query_screen_id_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NmpCuQueryScreenIDRes *tmp = NULL;

	NMP_XML_DEF_CREATE_BEGIN_SINGLE(tmp);

	NMP_XML_DEF_VAL_CREATE_CHILD_SINGLE("screenID", tmp->screen_id);
	NMP_XML_DEF_VAL_CREATE_CHILD_SINGLE("twID", tmp->tw_id);

	NMP_XML_DEF_CREATE_END_SINGLE();
}



NmpMsgInfo *
nmp_parse_cu_query_user_guids(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuQueryUserGuids tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.user, "user", USER_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.req_num, "reqNum")
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.start_num, "startRow")
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.session, "sessionId", SESSION_ID_LEN)

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_cu_query_user_guids_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpCuQueryUserGuidsRes *tmp = NULL;
	int back_count, i;
	char str[INT_TO_CHAR_LEN] = {0};

	NMP_XML_DEF_CREATE_BEGIN(tmp, str, root_node, end);

	NMP_XML_DEF_VAL_CREATE_CHILD("totalCount", tmp->total_count, root_node, str);

	back_count = tmp->back_count;
	snprintf(str, INT_TO_CHAR_LEN, "%d", back_count);
	node = xmlNewNode(NULL, BAD_CAST "guList");
	xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
	xmlAddChild(root_node, node);

	i = 0;
	while (back_count--)
	{
		NmpUserGuidInfo *gu = &tmp->guid_info[i];
		node1 = xmlNewNode(NULL, BAD_CAST "device");
		xmlAddChild(node, node1);

		NMP_XML_DEF_STR_CREATE_CHILD("guid", gu->guid, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("domain", gu->domain, node1);
		NMP_XML_DEF_VAL_CREATE_CHILD("level", gu->level, node1, str);
		NMP_XML_DEF_STR_CREATE_CHILD("guName", gu->gu_name, node1);
		NMP_XML_DEF_VAL_CREATE_CHILD("guNum", gu->gu_num, node1, str);

		i++;
	}

end:
	return 0;
}


NmpMsgInfo *
nmp_parse_cu_set_user_guids(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuSetUserGuids tmp, *res_info = NULL;
	NmpUserGuidInfo *guid_info = NULL;
	NmpMsgInfo *sys_msg = NULL;
	xmlXPathObjectPtr app_result;
	xmlNodePtr node;
	char *xpath = "/message";
	char *count_p = NULL;
	gint count = 0;
	gint i, size, ret = 0;
	memset(&tmp, 0, sizeof(tmp));

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (!app_result)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;
		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"resultCode")))
			{
				nmp_deal_value(doc, cur, &ret);
				if (ret)
					break;
			}
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
			{
				nmp_deal_text(doc, cur, tmp.session, SESSION_ID_LEN);
			}
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"user")))
			{
				nmp_deal_text(doc, cur, tmp.user, USER_NAME_LEN);
			}
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"firstReq")))
			{
				nmp_deal_value(doc, cur, &tmp.first_req);
			}
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guList")))
			{
				count_p = (char *)xmlGetProp(cur, (const xmlChar *)"count");
				if (count_p)
				{
					count = atoi(count_p);
					xmlFree(count_p);
				}
				tmp.count = count;

				size = sizeof(NmpCuSetUserGuids) + sizeof(NmpUserGuidInfo) * count;
				res_info = nmp_mem_kalloc(size);
				if (!res_info)
					return NULL;
			}
			else
				xml_error("not parse the node %s\n", cur->name);

			cur = cur->next;
			while ( cur && xmlIsBlankNode (cur) )
				cur = cur->next;
		}
		if (ret)
			break;
	}

	xmlXPathFreeObject(app_result);

	if (!res_info)
	{
		return NULL;
	}

	memcpy(res_info, &tmp, sizeof(tmp));
	if (res_info->count == 0)
		goto end;

	app_result =  nmp_get_node(doc, (const xmlChar *)"/message/guList/gu");
	if (G_UNLIKELY(!app_result))
		return NULL;

	nodeset = app_result->nodesetval;
	for (i = 0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		node = cur->xmlChildrenNode;
		guid_info = &res_info->guid_info[i];
		while (node != NULL)
		{
			if ((!xmlStrcmp(node->name, (const xmlChar *)"guid")))
				nmp_deal_text(doc, node, guid_info->guid, MAX_ID_LEN);
			else if ((!xmlStrcmp(node->name, (const xmlChar *)"domain")))
				nmp_deal_text(doc, node, guid_info->domain, DOMAIN_ID_LEN);
			else if ((!xmlStrcmp(node->name, (const xmlChar *)"level")))
				nmp_deal_value(doc, node, &guid_info->level);
			else if ((!xmlStrcmp(node->name, (const xmlChar *)"guNum")))
				nmp_deal_value(doc, node, &guid_info->gu_num);
			else
				xml_error("not parse the node %s\n", node->name);

			node = node->next;
		}
	}

	xmlXPathFreeObject(app_result);

end:
	sys_msg = nmp_msginfo_new_2(cmd, res_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);

	return sys_msg;
}


int
nmp_create_cu_set_user_guids_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NMP_XML_DEF_CREATE_NMP_RESULT();
}


NmpMsgInfo *
nmp_parse_cu_set_screen_num(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuSetScreenNum tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.user, "user", USER_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.screen_id, "screenID")
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.screen_num, "screenNum")
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.session, "sessionId", SESSION_ID_LEN)

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_cu_set_screen_num_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NMP_XML_DEF_CREATE_NMP_RESULT();
}


NmpMsgInfo *
nmp_parse_cu_query_tour_id(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuQueryTourID tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.session, "sessionId", SESSION_ID_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.user, "user", USER_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.tour_num, "tourNum")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_cu_query_tour_id_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NmpCuQueryTourIDRes *tmp = NULL;

	NMP_XML_DEF_CREATE_BEGIN_SINGLE(tmp);

	NMP_XML_DEF_VAL_CREATE_CHILD_SINGLE("tourID", tmp->tour_id);

	NMP_XML_DEF_CREATE_END_SINGLE();
}


NmpMsgInfo *
nmp_parse_cu_set_tour_num(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuSetTourNum tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.session, "sessionId", SESSION_ID_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.user, "user", USER_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.tour_id, "tourID")
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.tour_num, "tourNum")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_cu_set_tour_num_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NMP_XML_DEF_CREATE_NMP_RESULT();
}


NmpMsgInfo *
nmp_parse_cu_query_group_id(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuQueryGroupID tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.session, "sessionId", SESSION_ID_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.user, "user", USER_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.group_num, "groupNum")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_cu_query_group_id_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NmpCuQueryGroupIDRes *tmp = NULL;

	NMP_XML_DEF_CREATE_BEGIN_SINGLE(tmp);

	NMP_XML_DEF_VAL_CREATE_CHILD_SINGLE("groupID", tmp->group_id);

	NMP_XML_DEF_CREATE_END_SINGLE();
}


NmpMsgInfo *
nmp_parse_cu_set_group_num(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpCuSetGroupNum tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.session, "sessionId", SESSION_ID_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.user, "user", USER_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.group_id, "groupID")
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.group_num, "groupNum")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_cu_set_group_num_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NMP_XML_DEF_CREATE_NMP_RESULT();
}

