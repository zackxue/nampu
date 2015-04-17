/********************************************************************
 * nmp_bss.c  - deal xml of bss, parse and create xml
 * Function£ºparse or create xml relate to bss.
 * Author:yangy
 * Description:users can add parse or create message of bss,define
 *             struct about bss information
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
#include "nmp_bss_xml.h"
#include "nmp_msg_struct.h"
#include "nmp_share_errno.h"
#include "nmp_share_debug.h"
#include "nmp_memory.h"

#include "nmp_xml.h"
#include "nmp_xml_shared.h"

/**
 * nmp_parse_bss_login: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */

#define LOCAL_HOST_IP "127.0.0.1"
NmpMsgInfo *
nmp_parse_bss_login(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpBssLoginInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/admin";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.admin_name, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"password")))
                nmp_deal_text(doc, cur, tmp.password, USER_PASSWD_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode ( cur ) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    return sys_msg;
}


/**
 * nmp_create_bss_login_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_bss_login_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL,node = NULL;
    NmpBssLoginRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "admin");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        xmlNewChild(node,
                NULL,
                BAD_CAST "domainName",
                BAD_CAST tmp->domain_name);
        xmlNewChild(node,
                NULL,
                BAD_CAST "domainID",
                BAD_CAST tmp->domain_id);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->module_sets);
        xmlNewChild(node,
                NULL,
                BAD_CAST "moduleSets",
                BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpBssHeart tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/admin";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.admin_name, USER_NAME_LEN);
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
nmp_create_bss_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpBssHeartResp *tmp = NULL;
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
nmp_parse_validata_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAdminInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/admin";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.admin_name, USER_NAME_LEN);
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
 * nmp_create_validate_admin_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_validata_admin_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "admin");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_validata_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpValidateUserGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/group";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&tmp, 0, sizeof(tmp));

    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        memset(&tmp,0, sizeof(tmp));

        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.group_name, GROUP_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_value(doc, cur, &tmp.group_id);
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
 * nmp_create_validata_user_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_validata_user_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "group");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_validata_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpValidateUser tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/user";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.username, USER_NAME_LEN);
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
nmp_create_validata_user_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "user");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_validata_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpValidateArea tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/area";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.area_name, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    return sys_msg;
}


 int
nmp_create_validata_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "area");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_validata_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpValidatePu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/pu";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puid")))
                nmp_deal_text(doc, cur, tmp.puid, MAX_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    return sys_msg;
}


 int
nmp_create_validata_pu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "pu");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}

NmpMsgInfo *
nmp_parse_add_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddAdmin tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/admin";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.admin_name, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"password")))
                nmp_deal_text(doc, cur, tmp.password, USER_PASSWD_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_admin_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_admin_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "admin");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddUserGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/group";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.group_name, GROUP_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rank")))
                nmp_deal_value(doc, cur, &tmp.group_rank);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"permissions")))
                nmp_deal_value(doc, cur, &tmp.group_permissions);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_user_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_user_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "group");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddUser tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/user";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.username, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sex")))
                nmp_deal_value(doc, cur, &tmp.sex);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"password")))
                nmp_deal_text(doc, cur, tmp.password, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"phone")))
                nmp_deal_text(doc, cur, tmp.user_phone, PHONE_NUM_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"description")))
                nmp_deal_text(doc, cur, tmp.user_description, DESCRIPTION_INFO_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_user_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_user_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "user");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddArea tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/area";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.area_name, AREA_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"areaId")))
                nmp_deal_value(doc, cur, &tmp.area_parent);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "area");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddPu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message/device";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puid")))
                nmp_deal_text(doc, cur, tmp.puid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domain")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.domain_id, DOMAIN_ID_LEN);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puName")))
                nmp_deal_text(doc, cur, tmp.pu_info, PU_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
                nmp_deal_value(doc, cur, &tmp.keep_alive_time);
                if (tmp.keep_alive_time < 3)
                    tmp.keep_alive_time = 3;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puType")))
                nmp_deal_value(doc, cur, &tmp.pu_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puMinorType")))
                nmp_deal_value(doc, cur, &tmp.pu_minor_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"manufacturer")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.manufacturer, MF_ID_LEN);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdu")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.mds_id, MDS_ID_LEN);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"area")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_value(doc, node, &tmp.area_id);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puCount")))
                nmp_deal_value(doc, cur, &tmp.pu_count);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"avCount")))
                nmp_deal_value(doc, cur, &tmp.av_count);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"aiCount")))
                nmp_deal_value(doc, cur, &tmp.ai_count);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"aoCount")))
                nmp_deal_value(doc, cur, &tmp.ao_count);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dsCount")))
                nmp_deal_value(doc, cur, &tmp.ds_count);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
                nmp_deal_text(doc, cur, tmp.ivs_id, IVS_ID_LEN);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"amsId")))
                nmp_deal_text(doc, cur, tmp.ams_id, AMS_ID_LEN);
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
 * nmp_create_add_pu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_pu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpAddPuRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "device");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->success_count);
    xmlNewChild(node,
                NULL,
                BAD_CAST "successCount",
                BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_add_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddGu tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i, j = 0, enable;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node, node1;
    char *xpath = "/message/gu";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guid")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domain")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.domain_id, DOMAIN_ID_LEN);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"device")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"puid")))
                        nmp_deal_text(doc, node, tmp.puid, MAX_ID_LEN);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guName")))
                nmp_deal_text(doc, cur, tmp.gu_name, GU_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
                nmp_deal_text(doc, cur, tmp.ivs_id, IVS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guHasDome")))
            {
                nmp_deal_value(doc, cur, &enable);
                tmp.gu_attributes = tmp.gu_attributes | (enable&0x1);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guAlarmBypass")))
            {
                nmp_deal_value(doc, cur, &enable);
                tmp.gu_attributes = tmp.gu_attributes | ((enable&0x1)<<1);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guType")))
                nmp_deal_value(doc, cur, &tmp.gu_type);
	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssList")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"mss")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"id")))
                            {
                                nmp_deal_text(doc, node1, tmp.mss[j].mss_id, MSS_ID_LEN);
                                j++;
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
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_gu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "gu");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddMds tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/mdu";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.mds_name, MDS_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_text(doc, cur, tmp.mds_id, MDS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mduType")))
                nmp_deal_value(doc, cur, &tmp.mds_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
                nmp_deal_value(doc, cur, &tmp.keep_alive_freq);
                if (tmp.keep_alive_freq < 3)
                    tmp.keep_alive_freq = 3;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puPort")))
                nmp_deal_value(doc, cur, &tmp.mds_pu_port);
	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rtspPort")))
                nmp_deal_value(doc, cur, &tmp.mds_rtsp_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"getIpEnable")))
                nmp_deal_value(doc, cur, &tmp.get_ip_enable);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    return sys_msg;
}


/**
 * nmp_create_add_mds_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_mds_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "mdu");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddMdsIp tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message/dispatchList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdu")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.mds_id, MDS_ID_LEN);
                    else
                        xml_error("0000not parse the node %s\n",node->name);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dispatch")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"cmsIp")))
                        nmp_deal_text(doc, node, tmp.cms_ip, MAX_IP_LEN);
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"dispatchIp")))
                        nmp_deal_text(doc, node, tmp.mds_ip, MAX_IP_LEN);
                    else
                        xml_error("1111not parse the node %s\n",node->name);

                    node = node->next;
                }
            }
            else
                xml_error("222not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_mds_ip_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_mds_ip_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "dispatchList");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddMss tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/mss";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.mss_name, MSS_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
                nmp_deal_value(doc, cur, &tmp.keep_alive_freq);
		  if (tmp.keep_alive_freq < 3)
			tmp.keep_alive_freq = 3;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"storageType")))
            {
                nmp_deal_value(doc, cur, &tmp.storage_type);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mode")))
            {
                nmp_deal_value(doc, cur, &tmp.mode);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    return sys_msg;
}


/**
 * nmp_create_add_mss_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_mss_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "mss");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_gu_to_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddGuToUser user;
    NmpAddGuToUser *tmp = NULL;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node, node1;
    char *xpath = "/message/guList";
    gint count = 0, size;

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    memset(&user, 0, sizeof(user));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"count")))
            {
                nmp_deal_value(doc, cur, &count);
                if (count < 0)
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }

                size = sizeof(NmpAddGuToUser)  + sizeof(NmpGuToUserInfo) * count ;
                tmp = nmp_mem_kalloc(size);
                if (G_UNLIKELY(!tmp))
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }
                memset(tmp, 0, size);
                user.total_num = count;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"user")))
            {
                if (G_UNLIKELY(!tmp))
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"name")))
                        nmp_deal_text(doc, node, user.username, USER_NAME_LEN);

                    node = node->next;
                }
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if (G_UNLIKELY(!tmp))
        return NULL;

    memcpy(tmp, &user, sizeof(user));
    if (G_UNLIKELY(count <= 0))
    {
        sys_msg = nmp_msginfo_new_2(cmd, tmp, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
        return sys_msg;
    }

    app_result =  nmp_get_node(doc, (const xmlChar *)"/message/guList/list/gu");
    if (G_UNLIKELY(!app_result))
        return NULL;

    nodeset = app_result->nodesetval;
    i=0;
    for (i=0; i< nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        node = cur->xmlChildrenNode;
        while (node != NULL)
        {
            if ((!xmlStrcmp(node->name, (const xmlChar *)"guid")))
                nmp_deal_text(doc, node, tmp->gu_to_user_info[i].user_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(node->name, (const xmlChar *)"domain")))
            {
                node1 = node->xmlChildrenNode;
                while (node1 != NULL)
                {
                    if ((!xmlStrcmp(node1->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node1, tmp->gu_to_user_info[i].user_guid_domain, DOMAIN_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node1->name);
                    node1 = node1->next;
                }
            }
            else
                xml_error("not parse the node %s\n",node->name);

            node = node->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new_2(cmd, tmp, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);

    return sys_msg;
}


int
nmp_create_add_gu_to_user_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "guList");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_add_tw_to_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddTwToUser user;
    NmpAddTwToUser *tmp = NULL;
    NmpMsgInfo *sys_msg = NULL;
    gint i, j;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message";
    gint count = 0, size;

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    memset(&user, 0, sizeof(user));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"count")))
            {
                nmp_deal_value(doc, cur, &count);
                if (count < 0)
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }

                size = sizeof(NmpAddTwToUser)  + sizeof(NmpTwToUserInfo) * count ;
                tmp = nmp_mem_kalloc(size);
                if (G_UNLIKELY(!tmp))
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }
                memset(tmp, 0, size);
                user.total_num = count;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"username")))
            {
                if (G_UNLIKELY(!tmp))
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }
                nmp_deal_text(doc, cur, user.username, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if (G_UNLIKELY(!tmp))
        return NULL;

    memcpy(tmp, &user, sizeof(user));
    if (G_UNLIKELY(count <= 0))
    {
        sys_msg = nmp_msginfo_new_2(cmd, tmp, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
        return sys_msg;
    }

    app_result =  nmp_get_node(doc, (const xmlChar *)"/message/list");
    if (G_UNLIKELY(!app_result))
        return NULL;

    nodeset = app_result->nodesetval;
    i=0;
    j = 0;
    for (i=0; i< nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        node = cur->xmlChildrenNode;
        while (node != NULL)
        {
            if ((!xmlStrcmp(node->name, (const xmlChar *)"twId")))
            {
                nmp_deal_value(doc, node, &tmp->tw_to_user_info[j].tw_id);
                j++;
            }
            else
                xml_error("not parse the node %s\n",node->name);

            node = node->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new_2(cmd, tmp, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);

    return sys_msg;
}


int
nmp_create_add_tw_to_user_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_tour_to_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddTourToUser user;
    NmpAddTourToUser *tmp = NULL;
    NmpMsgInfo *sys_msg = NULL;
    gint i, j;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message";
    gint count = 0, size;

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    memset(&user, 0, sizeof(user));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"count")))
            {
                nmp_deal_value(doc, cur, &count);
                if (count < 0)
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }

                size = sizeof(NmpAddTourToUser)  + sizeof(NmpTourToUserInfo) * count ;
                tmp = nmp_mem_kalloc(size);
                if (G_UNLIKELY(!tmp))
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }
                memset(tmp, 0, size);
                user.total_num = count;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"username")))
            {
                if (G_UNLIKELY(!tmp))
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }
                nmp_deal_text(doc, cur, user.username, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if (G_UNLIKELY(!tmp))
        return NULL;

    memcpy(tmp, &user, sizeof(user));
    if (G_UNLIKELY(count <= 0))
    {
        sys_msg = nmp_msginfo_new_2(cmd, tmp, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
        return sys_msg;
    }

    app_result =  nmp_get_node(doc, (const xmlChar *)"/message/list");
    if (G_UNLIKELY(!app_result))
        return NULL;

    nodeset = app_result->nodesetval;
    i = 0;
    j = 0;
    for (i=0; i< nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        node = cur->xmlChildrenNode;
        while (node != NULL)
        {
            if ((!xmlStrcmp(node->name, (const xmlChar *)"tourId")))
            {
                nmp_deal_value(doc, node, &tmp->tour_to_user_info[j].tour_id);
				j++;
            }
            else
                xml_error("not parse the node %s\n",node->name);

            node = node->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new_2(cmd, tmp, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);

    return sys_msg;
}


int
nmp_create_add_tour_to_user_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddDefenceArea tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"defenceAreaId")))
                nmp_deal_value(doc, cur, &tmp.defence_area_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"enable")))
                nmp_deal_value(doc, cur, &tmp.enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"policy")))
                nmp_deal_text(doc, cur, tmp.policy, POLICY_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_defence_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_defence_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_defence_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddDefenceMap tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"defenceAreaId")))
                nmp_deal_value(doc, cur, &tmp.defence_area_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapName")))
                nmp_deal_text(doc, cur, tmp.map_name, MAP_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapLocation")))
                nmp_deal_text(doc, cur, tmp.map_location, MAP_LOCATION_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_defence_map_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_defence_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddDefenceGu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapId")))
                nmp_deal_value(doc, cur, &tmp.map_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guDomain")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateX")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_x);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateY")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_y);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_defence_map_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_defence_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_set_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpSetMapHref tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"sourceMapId")))
                nmp_deal_value(doc, cur, &tmp.src_map_id);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"destinationMapId")))
                nmp_deal_value(doc, cur, &tmp.dst_map_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateX")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_x);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateY")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_y);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_add_defence_map_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_set_map_href_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_create_general_cmd_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddTw tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"twName")))
                nmp_deal_text(doc, cur, tmp.tw_name, TW_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_add_tw_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddScreen tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &tmp.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayDomain")))
                nmp_deal_text(doc, cur, tmp.dis_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayGuId")))
                nmp_deal_text(doc, cur, tmp.dis_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateX")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_x);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateY")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_y);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"length")))
                nmp_deal_float_value(doc, cur, &tmp.length);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"width")))
                nmp_deal_float_value(doc, cur, &tmp.width);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_add_screen_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddTour tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourName")))
                nmp_deal_text(doc, cur, tmp.tour_name, TOUR_NAME_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"autoJump")))
                nmp_deal_value(doc, cur, &tmp.auto_jump);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_add_tour_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_tour_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddTourStep tmp;
    NmpAddTourStep *tour_steps = NULL;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message";
    gint count = 0, size;

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    memset(&tmp, 0, sizeof(tmp));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"count")))
            {
                nmp_deal_value(doc, cur, &count);
                if (count < 0)
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }

                size = sizeof(NmpAddTourStep)  + sizeof(NmpTourStep) * count ;
                tour_steps = nmp_mem_kalloc(size);
                if (G_UNLIKELY(!tour_steps))
                {
                    xmlXPathFreeObject(app_result);
                    return NULL;
                }
                memset(tour_steps, 0, size);
                tmp.total_num = count;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourId")))
            {
                nmp_deal_value(doc, cur, &tmp.tour_id);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    if (G_UNLIKELY(!tour_steps))
        return NULL;

    memcpy(tour_steps, &tmp, sizeof(tmp));
    if (G_UNLIKELY(count <= 0))
    {
        sys_msg = nmp_msginfo_new_2(cmd, tour_steps, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
        return sys_msg;
    }

    app_result =  nmp_get_node(doc, (const xmlChar *)"/message/tourStepList/tourStep");
    if (G_UNLIKELY(!app_result))
        return NULL;

    nodeset = app_result->nodesetval;
    i=0;
    for (i=0; i< nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        node = cur->xmlChildrenNode;
        while (node != NULL)
        {
            if ((!xmlStrcmp(node->name, (const xmlChar *)"stepNo")))
                nmp_deal_value(doc, node, &tour_steps->tour_step[i].step_no);
            else if ((!xmlStrcmp(node->name, (const xmlChar *)"interval")))
                nmp_deal_value(doc, node, &tour_steps->tour_step[i].interval);
            else if ((!xmlStrcmp(node->name, (const xmlChar *)"encoderGuId")))
                nmp_deal_text(doc, node, tour_steps->tour_step[i].encoder_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(node->name, (const xmlChar *)"encoderDomain")))
                nmp_deal_text(doc, node, tour_steps->tour_step[i].encoder_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(node->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, node, &tour_steps->tour_step[i].level);
            else
                xml_error("not parse the node %s\n",node->name);

            node = node->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new_2(cmd, tour_steps, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);

    return sys_msg;
}


int
nmp_create_add_tour_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_add_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupName")))
                nmp_deal_text(doc, cur, tmp.group_name, GROUP_NAME_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &tmp.tw_id);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_add_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_add_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddGroupStep tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stepNo")))
                nmp_deal_value(doc, cur, &tmp.step_no);
           else if ((!xmlStrcmp(cur->name, (const xmlChar *)"interval")))
                nmp_deal_value(doc, cur, &tmp.interval);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_add_group_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_add_ivs(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddIvs tmp;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsName")))
                nmp_deal_text(doc, cur, tmp.ivs_name, IVS_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
                nmp_deal_text(doc, cur, tmp.ivs_id, IVS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
                nmp_deal_value(doc, cur, &tmp.keep_alive_freq);
                if (tmp.keep_alive_freq < 3)
                    tmp.keep_alive_freq = 3;
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    return sys_msg;
}


int
nmp_create_add_ivs_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_validate_gu_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpValidateGuMap tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_validate_gu_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_config_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    xmlNodePtr node, node1;
    NmpConfigGroupStep tmp;
    NmpConfigGroupStep *req_info = NULL;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stepNo")))
                nmp_deal_value(doc, cur, &tmp.step_no);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"scrId")))
                nmp_deal_value(doc, cur, &tmp.scr_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divId")))
                nmp_deal_value(doc, cur, &tmp.div_id);
           else if((!xmlStrcmp(cur->name, (const xmlChar *)"groupStepInfoList")))
           {
                count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (count)
                {
                    tmp.total_num = atoi(count);
                    xmlFree(count);
                }
		  else
		  	tmp.total_num = 0;

                size = sizeof(NmpConfigGroupStep) + tmp.total_num*sizeof(NmpGroupStep);
                req_info = nmp_mem_kalloc(size);
                if (!req_info)
                    return NULL;
                memset(req_info, 0, sizeof(req_info));
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (tmp.total_num <= 0)
		           break;
                    if  ((!xmlStrcmp(node->name, (const xmlChar *)"groupStepInfo")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"divNo")))
                                nmp_deal_value(doc, node1, &req_info->group_step[j].div_no);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"encoderDomain")))
                                nmp_deal_text(doc, node1, req_info->group_step[j].encoder_domain, DOMAIN_ID_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"encoderGuId")))
                                nmp_deal_text(doc, node1, req_info->group_step[j].encoder_guid, MAX_ID_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"level")))
                                nmp_deal_value(doc, node1, &req_info->group_step[j].level);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < tmp.total_num)
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
        size = sizeof(NmpConfigGroupStep);
        req_info	= nmp_mem_kalloc(size);
        if (req_info)
            memcpy(req_info, &tmp, sizeof(tmp));
        else
            return NULL;
    }

    sys_msg = nmp_msginfo_new_2(cmd, req_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    //sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_config_group_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_set_del_alarm_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelAlarmPolicy tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"enable")))
                nmp_deal_value(doc, cur, &tmp.enable);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"maxCapacity")))
                nmp_deal_value(doc, cur, &tmp.max_capacity);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"delAlarmNum")))
                nmp_deal_value(doc, cur, &tmp.del_alarm_num);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


NmpMsgInfo *
nmp_parse_link_time_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpLinkTimePolicyConfig tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i = 0;
    xmlXPathObjectPtr app_result;
    xmlNodePtr new_node;
    xmlBufferPtr buff;
    char *xpath = "/message";
    xmlNodeSetPtr nodeset;

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timePolicy")))
            {
                new_node = xmlDocCopyNode(cur, doc, 1);
                buff = xmlBufferCreate();
                if (xmlNodeDump(buff, doc, new_node, 0, 0) == -1)
                {
                    xmlFreeNode(new_node);
                    xmlBufferFree(buff);
                    return NULL;
                }
                strncpy(tmp.time_policy, (char *)buff->content,  POLICY_LEN - 1);
                xmlFreeNode(new_node);
                xmlBufferFree(buff);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
   sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    printf("-------------&&&cmd=%s, tmp->time_policy=%s\n",cmd, tmp.time_policy);
    return sys_msg;
}


int
nmp_create_link_time_policy_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpLinkRecord tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i, j = 0;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeLen")))
                nmp_deal_value(doc, cur, &tmp.time_len);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                    {
                        nmp_deal_text(doc, node, tmp.mss[j].mss_id, MSS_ID_LEN);
                        j++;
                    }
                    node = node->next;
                }
            }
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
nmp_create_link_record_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_link_io(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpLinkIO tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeLen")))
                nmp_deal_value(doc, cur, &tmp.time_len);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"IOValue")))
                nmp_deal_text(doc, cur, tmp.io_value, IO_VALUE_LEN);
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
nmp_create_link_io_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_link_snapshot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpLinkSnapshot tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i, j = 0;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pictureNum")))
                nmp_deal_value(doc, cur, &tmp.picture_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                    {
                        nmp_deal_text(doc, node, tmp.mss[j].mss_id, MSS_ID_LEN);
                        j++;
                    }
                    node = node->next;
                }
            }
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
nmp_create_link_snapshot_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_link_preset(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpLinkPreset tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"presetNo")))
                nmp_deal_value(doc, cur, &tmp.preset_no);
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
nmp_create_link_preset_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_link_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpLinkStepConfig tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"encoderGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"encoderDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &tmp.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &tmp.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionNum")))
                nmp_deal_value(doc, cur, &tmp.div_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionId")))
                nmp_deal_value(doc, cur, &tmp.div_id);
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
nmp_create_link_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_link_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpLinkMap tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
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
nmp_create_link_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_create_set_del_alarm_policy_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_modify_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddAdmin tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/admin";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    printf("login$$$$$$$$$$nodeset->nodeNr=%d\n",nodeset->nodeNr);

    memset(&tmp, 0, sizeof(tmp));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        printf("==================cur->name=%s\n",cur->name);
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.admin_name, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"password")))
                nmp_deal_text(doc, cur, tmp.password, USER_PASSWD_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));
    return sys_msg;
}

/**
 * nmp_create_modify_admin_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_modify_admin_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "admin");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_modify_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpUserGroupInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/group";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    printf("login$$$$$$$$$$nodeset->nodeNr=%d\n",nodeset->nodeNr);

    memset(&tmp, 0, sizeof(tmp));
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        printf("==================cur->name=%s\n",cur->name);
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.group_name, GROUP_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_value(doc, cur, &tmp.group_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rank")))
                nmp_deal_value(doc, cur, &tmp.group_rank);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"permissions")))
                nmp_deal_value(doc, cur, &tmp.group_permissions);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_modify_user_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_modify_user_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "group");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_modify_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpUserInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/user";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.username, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"userId")))
                nmp_deal_value(doc, cur, &tmp.user_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sex")))
                nmp_deal_value(doc, cur, &tmp.sex);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"password")))
                nmp_deal_text(doc, cur, tmp.password, USER_PASSWD_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"phone")))
                nmp_deal_text(doc, cur, tmp.user_phone, PHONE_NUM_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"description")))
                nmp_deal_text(doc, cur, tmp.user_description, DESCRIPTION_INFO_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_modify_user_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_modify_user_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "user");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


 NmpMsgInfo *
nmp_parse_modify_domain(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDomainInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/domain";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.domain_name, DOMAIN_NAME_LEN);
            //else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainType")))
              //  nmp_deal_value(doc, cur, &tmp.domain_type);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_modify_domain_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_modify_domain_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", -RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "domain");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


 NmpMsgInfo *
nmp_parse_add_modify_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAreaInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/area";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    memset(&tmp, 0, sizeof(tmp));
    tmp.area_id = -1;

    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.area_name, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_value(doc, cur, &tmp.area_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"parentId")))
                nmp_deal_value(doc, cur, &tmp.area_parent);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"userName")))
                nmp_deal_text(doc, cur, tmp.user_name, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"userPhone")))
                nmp_deal_text(doc, cur, tmp.user_phone, PHONE_NUM_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"userAddress")))
                nmp_deal_text(doc, cur, tmp.user_address, DESCRIPTION_INFO_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"description")))
                nmp_deal_text(doc, cur, tmp.description, DESCRIPTION_INFO_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_modify_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_modify_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpAddAreaRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "area");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->area_id);
    xmlNewChild(node,
                NULL,
                BAD_CAST "id",
                BAD_CAST str);
    return 0;
}


NmpMsgInfo *
nmp_parse_modify_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpPuInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int len;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr  node;
    char *xpath = "/message/device";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"puName")))
                nmp_deal_text(doc, cur, tmp.pu_info, PU_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"amsId")))
                nmp_deal_text(doc, cur, tmp.ams_id, AMS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puid")))
                nmp_deal_text(doc, cur, tmp.puid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domain")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.domain_id, DOMAIN_ID_LEN);

                    node = node->next;
                    while ( node && xmlIsBlankNode (node) )
                        node = node->next;
                }

            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"manufacturer")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.mf_id, MF_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);

                    node = node->next;
                    while ( node && xmlIsBlankNode (node) )
                        node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdu")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.mds_id, MDS_ID_LEN);

                    node = node->next;
                    while ( node && xmlIsBlankNode (node) )
                        node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"area")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_value(doc, node, &tmp.area_id);

                    node = node->next;
                    while ( node && xmlIsBlankNode (node) )
                        node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
                nmp_deal_value(doc, cur, &tmp.keep_alive_time);
				  if (tmp.keep_alive_time < 3)
					  tmp.keep_alive_time = 3;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puMinorType")))
            {
                nmp_deal_value(doc, cur, &tmp.pu_minor_type);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpPuInfo);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}


/**
 * nmp_create_modify_pu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_modify_pu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "device");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_modify_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGuInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i, j = 0, enable;
    xmlXPathObjectPtr app_result;
    xmlNodePtr  node, node1;
    char *xpath = "/message/gu";
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guName")))
                nmp_deal_text(doc, cur, tmp.gu_name, GU_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guid")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
                nmp_deal_text(doc, cur, tmp.ivs_id, IVS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domain")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.domain_id, DOMAIN_ID_LEN);

                    node = node->next;
                    while ( node && xmlIsBlankNode (node) )
                        node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guName")))
                nmp_deal_text(doc, cur, tmp.gu_name, GU_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guHasDome")))
            {
                nmp_deal_value(doc, cur, &enable);
                tmp.gu_attributes = tmp.gu_attributes | (enable&0x1);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guAlarmBypass")))
            {
                nmp_deal_value(doc, cur, &enable);
                tmp.gu_attributes = tmp.gu_attributes | ((enable&0x1)<<1);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guType")))
                nmp_deal_value(doc, cur, &tmp.gu_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssList")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {

					  if ((!xmlStrcmp(node->name, (const xmlChar *)"mss")))
					  	{
					    	node1 = node->xmlChildrenNode;
    					  while (node1 != NULL)
                       {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"id")))
                            {
                                nmp_deal_text(doc, node1, tmp.mss[j].mss_id, MSS_ID_LEN);
        							j++;
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

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


/**
 * nmp_create_modify_gu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_modify_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "gu");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_modify_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMdsInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/mdu";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.mds_name, MDS_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_text(doc, cur, tmp.mds_id, MDS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mduType")))
                nmp_deal_value(doc, cur, &tmp.mds_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
                nmp_deal_value(doc, cur, &tmp.keep_alive_freq);
		   if (tmp.keep_alive_freq < 3)
			tmp.keep_alive_freq = 3;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puPort")))
                nmp_deal_value(doc, cur, &tmp.mds_pu_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rtspPort")))
                nmp_deal_value(doc, cur, &tmp.mds_rtsp_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"getIpEnable")))
                nmp_deal_value(doc, cur, &tmp.get_ip_enable);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_modify_mds_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_modify_mds_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "mdu");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_modify_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMssInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/mss";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.mss_name, MSS_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
                nmp_deal_value(doc, cur, &tmp.keep_alive_freq);
		  if (tmp.keep_alive_freq < 3)
			tmp.keep_alive_freq = 3;
	     }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"storageType")))
            {
                nmp_deal_value(doc, cur, &tmp.storage_type);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mode")))
            {
                nmp_deal_value(doc, cur, &tmp.mode);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_modify_mss_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_modify_mss_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "mss");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


 NmpMsgInfo *
nmp_parse_modify_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDefenceAreaInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr new_node = NULL;
    xmlBufferPtr buff;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";
    int i;

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"defenceAreaId")))
                nmp_deal_value(doc, cur, &tmp.defence_area_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"enable")))
                nmp_deal_value(doc, cur, &tmp.defence_enable);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"policy")))
            {
                new_node = xmlDocCopyNode(cur, doc, 1);
                buff = xmlBufferCreate();
                if (xmlNodeDump(buff, doc, new_node, 0, 0) == -1)
                {
                    xmlFreeNode(new_node);
                    xmlBufferFree(buff);
                    return NULL;
                }
                //   strncpy(tmp->time_policy, buff->content,  strlen(buff->content));
                strncpy(tmp.policy, (char *)buff->content,  POLICY_LEN - 1);
                xmlFreeNode(new_node);
                xmlBufferFree(buff);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_defence_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_modify_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyDefenceGu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapId")))
                nmp_deal_value(doc, cur, &tmp.map_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guDomain")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateX")))
            	{
                nmp_deal_float_value(doc, cur, &tmp.coordinate_x);
		  printf("-----tmp.coordinate_x=%.4f\n",tmp.coordinate_x);
            	}
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateY")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_y);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_defence_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_modify_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyMapHref tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"sourceMapId")))
                nmp_deal_value(doc, cur, &tmp.src_map_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"destinationMapId")))
                nmp_deal_value(doc, cur, &tmp.dst_map_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateX")))
            {
                nmp_deal_float_value(doc, cur, &tmp.coordinate_x);
		  printf("-----tmp.coordinate_x=%.4f\n",tmp.coordinate_x);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateY")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_y);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_map_href_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_modify_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyTw tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &tmp.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twName")))
                nmp_deal_text(doc, cur, tmp.tw_name, TW_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"line")))
                nmp_deal_value(doc, cur, &tmp.line_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"column")))
            	{
                nmp_deal_value(doc, cur, &tmp.column_num);  printf("--------tmp.column_num=%d\n",tmp.column_num);
            	}
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_tw_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_modify_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyScreen tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &tmp.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateX")))
            {
                nmp_deal_float_value(doc, cur, &tmp.coordinate_x);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"coordinateY")))
                nmp_deal_float_value(doc, cur, &tmp.coordinate_y);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"length")))
            {
                nmp_deal_float_value(doc, cur, &tmp.length);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"width")))
                nmp_deal_float_value(doc, cur, &tmp.width);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayDomain")))
                nmp_deal_text(doc, cur, tmp.dis_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayGuId")))
                nmp_deal_text(doc, cur, tmp.dis_guid, MAX_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_screen_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_modify_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyTour tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourId")))
                nmp_deal_value(doc, cur, &tmp.tour_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourName")))
                nmp_deal_text(doc, cur, tmp.tour_name, TOUR_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"autoJump")))
                nmp_deal_value(doc, cur, &tmp.auto_jump);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_tour_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_modify_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupName")))
                nmp_deal_text(doc, cur, tmp.group_name, GROUP_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyGroupStep tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stepNo")))
                nmp_deal_value(doc, cur, &tmp.step_no);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"interval")))
                nmp_deal_value(doc, cur, &tmp.interval);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_group_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_group_step_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyGroupStepInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stepNo")))
                nmp_deal_value(doc, cur, &tmp.step_no);
           else if ((!xmlStrcmp(cur->name, (const xmlChar *)"scrId")))
                nmp_deal_value(doc, cur, &tmp.scr_id);
           else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
           else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divNo")))
                nmp_deal_value(doc, cur, &tmp.div_no);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_group_step_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


 NmpMsgInfo *
nmp_parse_add_modify_manufacturer(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddModifyManufacturer tmp;
    NmpMsgInfo *sys_msg = NULL;
    int len;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/manufacturer";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.mf_name, MF_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_text(doc, cur, tmp.mf_id, MF_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpAddModifyManufacturer);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);
    printf("&&&&&&&&&&&&&&&tmp.mf_id=%s,tmp.mf_name=%s\n",tmp.mf_id,tmp.mf_name);
    return sys_msg;
}

/**
 * nmp_create_add_modify_manufacturer_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_add_modify_manufacturer_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "manufacturer");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_modify_link_time_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyLinkTimePolicy tmp;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr new_node;
    xmlBufferPtr buff;
    int i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timePolicy")))
            {
                new_node = xmlDocCopyNode(cur, doc, 1);
                buff = xmlBufferCreate();
                if (xmlNodeDump(buff, doc, new_node, 0, 0) == -1)
                {
                    xmlFreeNode(new_node);
                    xmlBufferFree(buff);
                    return NULL;
                }
                strncpy(tmp.time_policy, (char *)buff->content,  POLICY_LEN - 1);
                xmlFreeNode(new_node);
                xmlBufferFree(buff);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_link_time_policy_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyLinkRecord tmp;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL;
    int i, j = 0;
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
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeLen")))
                nmp_deal_value(doc, cur, &tmp.time_len);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                    {
                        nmp_deal_text(doc, node, tmp.mss[j].mss_id, MSS_ID_LEN);
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
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_link_record_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_link_io(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyLinkIO tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeLen")))
                nmp_deal_value(doc, cur, &tmp.time_len);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"IOValue")))
                nmp_deal_text(doc, cur, tmp.io_value, IO_VALUE_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_link_io_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_link_snapshot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyLinkSnapshot tmp;
    NmpMsgInfo *sys_msg = NULL;
    xmlNodePtr node = NULL;
    int i, j = 0;
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
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pictureNum")))
                nmp_deal_value(doc, cur, &tmp.picture_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
	      else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                    {
                        nmp_deal_text(doc, node, tmp.mss[j].mss_id, MSS_ID_LEN);
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
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_link_snapshot_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_link_preset(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyLinkPreset tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"presetNo")))
                nmp_deal_value(doc, cur, &tmp.preset_no);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_link_preset_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_link_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyLinkStep tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"encoderGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"encoderDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &tmp.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &tmp.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionNum")))
                nmp_deal_value(doc, cur, &tmp.div_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionId")))
                nmp_deal_value(doc, cur, &tmp.div_id);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_link_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_link_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpModifyLinkMap tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
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
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp.level);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_link_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_modify_ivs(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpIvsInfo tmp;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsName")))
                nmp_deal_text(doc, cur, tmp.ivs_name, IVS_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
                nmp_deal_text(doc, cur, tmp.ivs_id, IVS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
				nmp_deal_value(doc, cur, &tmp.keep_alive_freq);
				if (tmp.keep_alive_freq < 3)
					tmp.keep_alive_freq = 3;
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_modify_ivs_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_del_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelAdmin tmp;
    NmpMsgInfo *sys_msg = NULL;
    gchar del_name[USER_NAME_LEN] = {0};
    gint len;
    gint i, j= 0 ;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/adminList/list/admin";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    memset(&tmp, 0, sizeof(tmp));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
            {
                nmp_deal_text(doc, cur, del_name, USER_NAME_LEN);
                if ((strlen(tmp.admin_name) + strlen(del_name) + 3) < MULTI_NAME_LEN)
                {
                    if (j == 0)
                    {
                        strncpy(tmp.admin_name, "'", MULTI_NAME_LEN-1);

                        strcat(tmp.admin_name, del_name);
                        strcat(tmp.admin_name,  "'");
                        //strncpy(tmp.admin_name, del_name, MULTI_NAME_LEN-1);
                    }
                    else
                    {
                        strcat(tmp.admin_name, ",'");
                        strcat(tmp.admin_name,del_name);
                        strcat(tmp.admin_name, "'");
                    }

                    j++;
                }
                else
                {
                    memset(tmp.admin_name, 0 , MULTI_NAME_LEN);
                    strcpy(tmp.admin_name, "StringTooLong");
                    nmp_warning("del admin name len is too long");
                }

           }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpDelAdmin);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}

/**
 * nmp_create_del_admin_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_admin_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "adminList");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelUserGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    gchar del_id[USER_NAME_LEN] = {0};
    gint len;
    gint i, j= 0 ;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/groupList/list/group";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
            {
                nmp_deal_text(doc, cur, del_id, USER_NAME_LEN);

                if ((strlen(tmp.group_id) + strlen(del_id) + 3) < MULTI_NAME_LEN)
                {
                    if (j == 0)
                    {
                        strncpy(tmp.group_id, del_id, MULTI_NAME_LEN-1);
                    }
                    else
                    {
                        strcat(tmp.group_id, ",");
                        strcat(tmp.group_id,del_id);
                    }

                    j++;
                }
                else
                {
                    memset(tmp.group_id, 0 , MULTI_NAME_LEN);
                    strcpy(tmp.group_id, "StringTooLong");
                    nmp_warning("del admin name len is too long");
                }
           }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpDelUserGroup);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}

/**
 * nmp_create_del_user_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_user_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "groupList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelUser tmp;
    NmpMsgInfo *sys_msg = NULL;
    gchar del_id[USER_NAME_LEN] = {0};
    gint len;
    gint i, j= 0 ;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/userList/list/user";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
            {
                nmp_deal_text(doc, cur, del_id, USER_NAME_LEN);

                if ((strlen(tmp.username) + strlen(del_id) + 3) < MULTI_NAME_LEN)
                {
                    if (j == 0)
                    {
                        strncpy(tmp.username, "'", MULTI_NAME_LEN-1);
						   strcat(tmp.username,del_id);
						   strcat(tmp.username, "'");
                        //strncpy(tmp.username, del_id, MULTI_NAME_LEN-1);
                    }
                    else
                    {
                        strcat(tmp.username, ",'");
                        strcat(tmp.username,del_id);
						   strcat(tmp.username, "'");
                    }

                    j++;
                }
                else
                {
                    memset(tmp.username, 0 , MULTI_NAME_LEN);
                    strcpy(tmp.username, "StringTooLong");
                    nmp_warning("del admin name len is too long");
                }
           }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpDelUser);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}

/**
 * nmp_create_del_user_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_user_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "userList");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelMds tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint len;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/mdu";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_text(doc, cur, tmp.mds_id, MDS_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpDelMds);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}

/**
 * nmp_create_del_mds_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_mds_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "mdu");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelMdsIp tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/dispatchList/mdu";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
            {
                nmp_deal_text(doc, cur, tmp.mds_id, MDS_ID_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);

    app_result =
        nmp_get_node(
            doc,
            (const xmlChar *)"/message/dispatchList/list/dispatch"
            );
    if (!app_result)
        return NULL;

    nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
           if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
           {
                nmp_deal_text(doc, cur, tmp.cms_ip, MAX_IP_LEN);
           }
           else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_del_mds_ip_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_mds_ip_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "dispatchList");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelMss tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint len;
    gint i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/mss";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpDelMss);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}

/**
 * nmp_create_del_mss_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_mss_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "mss");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelArea tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint len;
    gint i ;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/area";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
            {
                nmp_deal_value(doc, cur, &tmp.area_id);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpDelArea);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}

/**
 * nmp_create_del_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "area");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelPu *req_info;
    NmpDelPu tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i, j = 0, size;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node, node1, node2;
    char *xpath = "/message";
    char *count;
    gint del_count;

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    memset(&tmp, 0, sizeof(tmp));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
             if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                 nmp_deal_value(doc, cur, &tmp.type);
             else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"deviceList")))
            {
                count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (count)
                {
                    del_count = atoi(count);
                    xmlFree(count);
                }
		  else
		  	del_count = 0;

                size = sizeof(NmpDelPu) + del_count*sizeof(NmpPuPoint);
                req_info = nmp_mem_kalloc(size);
                if (!req_info)
                    return NULL;
                memset(req_info, 0, size);
                req_info->count = del_count;
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (del_count <= 0)
		           break;
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"device")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"puid")))
                                nmp_deal_text(doc, node1, req_info->pu_list[j].puid, MAX_ID_LEN);
                             else if ((!xmlStrcmp(node1->name, (const xmlChar *)"domain")))
                             {
                                 node2 = node1->xmlChildrenNode;
                                 while(node2 != NULL)
                                 {
                                     if ((!xmlStrcmp(node2->name, (const xmlChar *)"id")))
                                         nmp_deal_text(doc, node2, req_info->pu_list[j].domain_id, DOMAIN_ID_LEN);

                                     node2 = node2->next;
                                     while ( node2 && xmlIsBlankNode (node2) )
                                         node2 = node2->next;
                                 }
                             }
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < del_count)
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
    {
        strcpy(req_info->key,tmp.key);
        req_info->type = tmp.type;
        sys_msg = nmp_msginfo_new_2(cmd, req_info, size, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    }
    else
        sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));;

    return sys_msg;
}

/**
 * nmp_create_del_pu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_pu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "device");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelGu tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message/gu";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guid")))
            {
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domain")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.domain_id, DOMAIN_ID_LEN);

                    node = node->next;
                    while ( node && xmlIsBlankNode (node) )
                        node = node->next;
                }
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_del_gu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "gu");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_manufacturer(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelManufacturer tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint len;
    gint i ;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/manufacturer";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"id")))
            {
                nmp_deal_text(doc, cur, tmp.mf_id, MF_ID_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpDelManufacturer);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}

/**
 * nmp_create_del_manufacturer_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_manufacturer_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "manufacturer");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelDefenceArea tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"defenceAreaId")))
            {
                nmp_deal_value(doc, cur, &tmp.defence_area_id);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}

/**
 * nmp_create_del_defence_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_del_defence_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_del_defence_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelDefenceMap tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"defenceAreaId")))
            {
                nmp_deal_value(doc, cur, &tmp.defence_area_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapId")))
            {
                nmp_deal_value(doc, cur, &tmp.map_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapLocation")))
            {
                nmp_deal_text(doc, cur, tmp.map_location, MAP_LOCATION_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_defence_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_del_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelDefenceGu tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapId")))
            {
                nmp_deal_value(doc, cur, &tmp.map_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
            {
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guDomain")))
            {
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
            {
                nmp_deal_value(doc, cur, &tmp.type);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_defence_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_del_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelMapHref tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"sourceMapId")))
            {
                nmp_deal_value(doc, cur, &tmp.src_map_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"destinationMapId")))
            {
                nmp_deal_value(doc, cur, &tmp.dst_map_id);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_map_href_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_del_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelTw tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
            {
                nmp_deal_value(doc, cur, &tmp.tw_id);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_tw_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_del_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelScreen tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
            {
                nmp_deal_value(doc, cur, &tmp.type);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
            {
                nmp_deal_value(doc, cur, &tmp.tw_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayGuId")))
            {
                nmp_deal_text(doc, cur, tmp.dis_guid, MAX_ID_LEN);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"displayDomain")))
            {
                nmp_deal_text(doc, cur, tmp.dis_domain, DOMAIN_ID_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_screen_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_del_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelTour tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourId")))
            {
                nmp_deal_value(doc, cur, &tmp.tour_id);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_tour_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_del_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
            {
                nmp_deal_value(doc, cur, &tmp.group_id);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelGroupStep tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
            {
                nmp_deal_value(doc, cur, &tmp.group_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stepNo")))
            {
                nmp_deal_value(doc, cur, &tmp.step_no);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_group_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_group_step_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelGroupStepInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
            {
                nmp_deal_value(doc, cur, &tmp.group_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stepNo")))
            {
                nmp_deal_value(doc, cur, &tmp.step_no);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"scrId")))
            {
                nmp_deal_value(doc, cur, &tmp.scr_id);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divNo")))
            {
                nmp_deal_value(doc, cur, &tmp.div_no);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
            {
                nmp_deal_value(doc, cur, &tmp.type);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_group_step_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_alarm(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelAlarm tmp;
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
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
            {
                nmp_deal_value(doc, cur, &tmp.type);
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmId")))
                nmp_deal_text(doc, cur, tmp.alarm_ids, MULTI_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmState")))
                nmp_deal_value(doc, cur, &tmp.alarm_state);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmStartTime")))
                nmp_deal_text(doc, cur, tmp.start_time, TIME_INFO_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmEndTime")))
                nmp_deal_text(doc, cur, tmp.end_time, TIME_INFO_LEN);
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
nmp_create_del_alarm_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgCode *tmp = NULL;
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

    snprintf(str, INT_TO_CHAR_LEN,  "%ld", tmp->affect_num);
    xmlNewChild(root_node,
            NULL,
            BAD_CAST "delRecordNum",
            BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_del_link_time_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelLinkTimePolicy tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_link_time_policy_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelLinkRecord tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_link_record_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_link_io(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelLinkIO tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_link_io_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_link_snapshot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelLinkSnapshot tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_link_snapshot_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_link_preset(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelLinkPreset tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_link_preset_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_link_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelLinkStep tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &tmp.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &tmp.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionNum")))
                nmp_deal_value(doc, cur, &tmp.div_num);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_link_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_link_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelLinkMap tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i ;
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
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkGuId")))
                nmp_deal_text(doc, cur, tmp.link_guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"linkDomainId")))
                nmp_deal_text(doc, cur, tmp.link_domain, DOMAIN_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_del_link_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    return nmp_create_general_cmd_resp(doc, sys_msg);
}


NmpMsgInfo *
nmp_parse_del_ivs(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelIvs tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint len;
    gint i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
                nmp_deal_text(doc, cur, tmp.ivs_id, IVS_ID_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    len = sizeof(NmpDelIvs);
    sys_msg = nmp_msginfo_new(cmd, &tmp, len);

    return sys_msg;
}


int
nmp_create_del_ivs_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
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
nmp_parse_query_admin(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryAdmin tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/adminList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryAdmin));

    return sys_msg;
}

/**
 * nmp_create_query_admin_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_admin_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL;
    NmpQueryAdminRes *tmp = NULL;
    gint count = 0, total_num = 0, i;
    gchar str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "adminList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        count = tmp->req_num;
        total_num = tmp->total_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", total_num);
        xmlNewChild(node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);
        //if (total_num > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "list");
            xmlAddChild(node, node1);
        }
        i = 0;
        while ((count > 0)&&(total_num > 0))
        {
            node2 = xmlNewNode(NULL, BAD_CAST "admin");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                    NULL,
                    BAD_CAST "name",
                    BAD_CAST tmp->admin_info[i].admin_name);
            xmlNewChild(node2,
                    NULL,
                    BAD_CAST "password",
                    BAD_CAST tmp->admin_info[i].password);


	     i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_user_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryUserGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/groupList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryAdmin));

    return sys_msg;
}

/**
 * nmp_create_query_user_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_user_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL;
    NmpQueryUserGroupRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "groupList");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
            NULL,
            BAD_CAST "resultCode",
            BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        count = tmp->req_num;

        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->total_num);
        xmlNewChild(node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;
        while ((count > 0)&&(tmp->total_num > 0))
        {
            node2 = xmlNewNode(NULL, BAD_CAST "group");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                NULL,
                BAD_CAST "name",
                BAD_CAST tmp->group_info[i].group_name);
            printf("tmp->group_info[i].group_id=%d\n",tmp->group_info[i].group_id);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_info[i].group_id);
            xmlNewChild(node2,
                    NULL,
                    BAD_CAST "id",
                    BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_info[i].group_permissions);
            xmlNewChild(node2,
                    NULL,
                    BAD_CAST "permissions",
                    BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_info[i].group_rank);
            xmlNewChild(node2,
                    NULL,
                    BAD_CAST "rank",
                    BAD_CAST str);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_user(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryUser tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/userList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryUser));

    return sys_msg;
}

/**
 * nmp_create_query_user_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_user_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL, node3 = NULL;
    NmpQueryUserRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "userList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        count = tmp->req_num;

        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->total_num);
        xmlNewChild(node,
                    NULL,
                    BAD_CAST "count",
                    BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;
        while ((count > 0)&&(tmp->total_num > 0))
        {
            node2 = xmlNewNode(NULL, BAD_CAST "user");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->user_info[i].username);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->user_info[i].sex);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "sex",
                        BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->user_info[i].user_id);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "userId",
                        BAD_CAST str);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "password",
                        BAD_CAST  tmp->user_info[i].password);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "phone",
                        BAD_CAST  tmp->user_info[i].user_phone);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "description",
                        BAD_CAST  tmp->user_info[i].user_description);
            node3 = xmlNewNode(NULL, BAD_CAST "group");
            xmlAddChild(node2, node3);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->user_info[i].group_id);
            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);

            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->user_info[i].group_name);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_domain(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryDomain tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/domainList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryDomain));

    return sys_msg;
}


/**
 * nmp_create_query_domain_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_domain_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL;
    NmpQueryDomainRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "domainList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        count = tmp->total_num;

        /*snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->total_num);
        xmlNewChild(node,
                    NULL,
                    BAD_CAST "count",
                    BAD_CAST str);  */
       // if (tmp->total_num > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "list");
            xmlAddChild(node, node1);
        }
        i = 0;
        while ((count > 0)&&(tmp->total_num > 0))
        {
            node2 = xmlNewNode(NULL, BAD_CAST "domain");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->domain_info[i].domain_name);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->domain_info[i].domain_id);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "ip",
                        BAD_CAST  tmp->domain_info[i].domain_ip);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->domain_info[i].domain_port);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "port",
                        BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->domain_info[i].domain_type);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "type",
                        BAD_CAST str);

            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryArea tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/areaList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
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
 * nmp_create_query_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL;
    NmpQueryAreaRes *tmp = NULL;
    int count, req_count, i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "areaList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        count = tmp->total_num;
        req_count =  tmp->req_num;
	printf("-----------req area count=%d\n",req_count);
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        xmlNewChild(node,
                    NULL,
                    BAD_CAST "count",
                    BAD_CAST str);


         node1 = xmlNewNode(NULL, BAD_CAST "list");
         xmlAddChild(node, node1);

        i = 0;
        while (req_count > 0)
        {
            node2 = xmlNewNode(NULL, BAD_CAST "area");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->area_info[i].area_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->area_info[i].area_id);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->area_info[i].area_parent);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "parentId",
                        BAD_CAST str);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "userName",
                        BAD_CAST tmp->area_info[i].user_name);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "userPhone",
                        BAD_CAST tmp->area_info[i].user_phone);
            printf("---------bss  tmp->area_info[i].user_phone=%s\n", tmp->area_info[i].user_phone);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "userAddress",
                        BAD_CAST tmp->area_info[i].user_address);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "description",
                        BAD_CAST tmp->area_info[i].description);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryPu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/deviceList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryPu));

    return sys_msg;
}

/**
 * nmp_create_query_pu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_pu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL, node3 = NULL;
    NmpQueryPuRes *tmp = NULL;
    int count,req_count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "deviceList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        count = tmp->total_num;
        req_count =  tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->total_num);
        xmlNewChild(node,
                    NULL,
                    BAD_CAST "count",
                    BAD_CAST str);
         printf("---------=========req_count=%d\n",req_count);

        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;
        while (req_count > 0)
        {
            node2 = xmlNewNode(NULL, BAD_CAST "device");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "puName",
                        BAD_CAST tmp->pu_info[i].pu_info);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "puid",
                        BAD_CAST tmp->pu_info[i].puid);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "amsId",
                        BAD_CAST tmp->pu_info[i].ams_id);

            node3 = xmlNewNode(NULL, BAD_CAST "domain");
            xmlAddChild(node2, node3);

            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->pu_info[i].domain_id);

            node3 = xmlNewNode(NULL, BAD_CAST "area");
            xmlAddChild(node2, node3);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->pu_info[i].area_id);
            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
			  xmlNewChild(node3,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST  tmp->pu_info[i].area_name);

          //  if (tmp->type != 0)
            {
                snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->pu_info[i].keep_alive_time);
                xmlNewChild(node2,
                            NULL,
                            BAD_CAST "keepAliveTime",
                            BAD_CAST str);

                snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->pu_info[i].pu_type);
                xmlNewChild(node2,
                            NULL,
                            BAD_CAST "puType",
                            BAD_CAST str);
                snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->pu_info[i].pu_minor_type);
                xmlNewChild(node2,
                            NULL,
                            BAD_CAST "puMinorType",
                            BAD_CAST str);
                snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->pu_info[i].pu_state);
                xmlNewChild(node2,
                            NULL,
                            BAD_CAST "puState",
                            BAD_CAST str);
                xmlNewChild(node2,
                            NULL,
                            BAD_CAST "puLastAlive",
                            BAD_CAST tmp->pu_info[i].pu_last_alive);
                xmlNewChild(node2,
                            NULL,
                            BAD_CAST "puLastIp",
                            BAD_CAST tmp->pu_info[i].pu_last_ip);
                node3 = xmlNewNode(NULL, BAD_CAST "manufacturer");
                xmlAddChild(node2, node3);
                xmlNewChild(node3,
                            NULL,
                            BAD_CAST "id",
                            BAD_CAST  tmp->pu_info[i].mf_id);
                xmlNewChild(node3,
                            NULL,
                            BAD_CAST "name",
                            BAD_CAST  tmp->pu_info[i].mf_name);

                node3 = xmlNewNode(NULL, BAD_CAST "mdu");
                xmlAddChild(node2, node3);
                xmlNewChild(node3,
                            NULL,
                            BAD_CAST "id",
                            BAD_CAST  tmp->pu_info[i].mds_id);
		        xmlNewChild(node3,
                            NULL,
                            BAD_CAST "name",
                            BAD_CAST  tmp->pu_info[i].mds_name);
            }

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryGu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message/guList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guid")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domain")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.domain_id, DOMAIN_ID_LEN);
                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"device")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"puid")))
                        nmp_deal_text(doc, node, tmp.puid, MAX_ID_LEN);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"gu")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.guid, MAX_ID_LEN);
                    node = node->next;
                }
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryGu));

    return sys_msg;
}

/**
 * nmp_create_query_gu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL, node3 = NULL, node4 = NULL;
    NmpQueryGuRes *tmp = NULL;
    int count, req_count, i, j;
    char str[INT_TO_CHAR_LEN] = {0};
    gint gu_attributes;

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "guList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        count = tmp->total_num;
        req_count = tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        xmlNewChild(node,
                    NULL,
                    BAD_CAST "count",
                    BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;

        while (req_count > 0)
        {
            node2 = xmlNewNode(NULL, BAD_CAST "gu");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->gu_info[i].gu_name);


            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "guid",
                        BAD_CAST tmp->gu_info[i].guid);

		xmlNewChild(node2,
                        NULL,
                        BAD_CAST "ivsId",
                        BAD_CAST  tmp->gu_info[i].ivs_id);

		xmlNewChild(node2,
                        NULL,
                        BAD_CAST "ivsName",
                        BAD_CAST  tmp->gu_info[i].ivs_name);

		xmlNewChild(node2,
                        NULL,
                        BAD_CAST "amsName",
                        BAD_CAST  tmp->gu_info[i].ams_name);

            node3 = xmlNewNode(NULL, BAD_CAST "domain");
            xmlAddChild(node2, node3);

            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->gu_info[i].domain_id);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->gu_info[i].gu_type);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "guType",
                        BAD_CAST str);

            node3 = xmlNewNode(NULL, BAD_CAST "device");
            xmlAddChild(node2, node3);
            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "puid",
                        BAD_CAST  tmp->gu_info[i].puid);

            if ((tmp->type == 3)||(tmp->type == 4) || (tmp->type == 5))
            {
                 xmlNewChild(node3,
                 NULL,
                 BAD_CAST "puName",
                 BAD_CAST  tmp->gu_info[i].pu_name);
	     }

	     if (tmp->type == 4)
	     {
                 xmlNewChild(node3,
                     NULL,
                     BAD_CAST "puLastIp",
                     BAD_CAST  tmp->gu_info[i].pu_ip);

	             snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->gu_info[i].pu_state);
                 xmlNewChild(node3,
                     NULL,
                     BAD_CAST "puState",
                     BAD_CAST  str);
                 snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->gu_info[i].pu_minor_type);
                 xmlNewChild(node3,
                     NULL,
                     BAD_CAST "puMinorType",
                     BAD_CAST  str);
	     }

	     if (tmp->type == 5)
	     {
                xmlNewChild(node3,
                 NULL,
                 BAD_CAST "areaName",
                 BAD_CAST  tmp->gu_info[i].area_name);
	     }

            if ((tmp->type != 0)&&(tmp->type != 4))
            {
                gu_attributes = tmp->gu_info[i].gu_attributes;
                snprintf(str, INT_TO_CHAR_LEN, "%d", (gu_attributes&0x01));
                xmlNewChild(node2,
                        NULL,
                        BAD_CAST "guHasDome",
                        BAD_CAST str);

                snprintf(str, INT_TO_CHAR_LEN, "%d", (gu_attributes>>1)&0x01);
                xmlNewChild(node2,
                        NULL,
                        BAD_CAST "guAlarmBypass",
                        BAD_CAST str);
            }

            if ((tmp->type == 2) || (tmp->type == 1))
            {
                node3 = xmlNewNode(NULL, BAD_CAST "mssList");
                xmlAddChild(node2, node3);
                for (j = 0; j < MSS_NUM; j++)
                {
                    if (strlen(tmp->gu_info[i].mss[j].mss_id) != 0)
                    {
                        node4 = xmlNewNode(NULL, BAD_CAST "mss");
                        xmlAddChild(node3, node4);
                        xmlNewChild(node4,
                            NULL,
                            BAD_CAST "id",
                            BAD_CAST  tmp->gu_info[i].mss[j].mss_id);
			   xmlNewChild(node4,
                            NULL,
                            BAD_CAST "name",
                            BAD_CAST  tmp->gu_info[i].mss[j].mss_name);
                    }
                }
            }
            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_mds(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryMds tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/mduList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryMds));

    return sys_msg;
}


/**
 * nmp_create_query_mds_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_mds_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL,  node2 = NULL;
    NmpQueryMdsRes *tmp = NULL;
    int count, req_count, i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "mduList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        count = tmp->total_num;
        req_count = tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        xmlNewChild(node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);
        i = 0;
        while (req_count > 0)
        {

            node2 = xmlNewNode(NULL, BAD_CAST "mdu");
            xmlAddChild(node1, node2);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->mds_info[i].mds_id);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->mds_info[i].mds_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mds_info[i].mds_type);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "mduType",
                        BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mds_info[i].keep_alive_freq);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "keepAliveTime",
                         BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mds_info[i].mds_pu_port);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "puPort",
                         BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mds_info[i].mds_rtsp_port);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "rtspPort",
                         BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mds_info[i].mds_state);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "mdsState",
                         BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mds_info[i].get_ip_enable);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "getIpEnable",
                         BAD_CAST str);

	     i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_mds_ip(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryMdsIp tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/dispatchList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
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
 * nmp_create_query_mds_ip_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_mds_ip_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL,  node2 = NULL;
    NmpQueryMdsIpRes *tmp = NULL;
    int count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "dispatchList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        count = tmp->total_num;
        if (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "mdu");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->mds_id);

            node1 = xmlNewNode(NULL, BAD_CAST "list");
            xmlAddChild(node, node1);
        }

        i = 0;
        while (count > 0)
        {

            node2 = xmlNewNode(NULL, BAD_CAST "dispatch");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "cmsIp",
                        BAD_CAST tmp->mds_ip_info[i].cms_ip);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "dispatchIp",
                        BAD_CAST tmp->mds_ip_info[i].mds_ip);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryMss tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/mssList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryMss));

    return sys_msg;
}


/**
 * nmp_create_query_mss_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_mss_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL,  node2 = NULL;
    NmpQueryMssRes *tmp = NULL;
    int count, total_num, i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "mssList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        count = tmp->req_num;
        total_num = tmp->total_num;

        snprintf(str, INT_TO_CHAR_LEN, "%d", total_num);
        xmlNewChild(node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;
        while ((count > 0)&&(total_num > 0))
        {
            node2 = xmlNewNode(NULL, BAD_CAST "mss");
            xmlAddChild(node1, node2);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->mss_info[i].mss_id);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->mss_info[i].mss_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mss_info[i].keep_alive_freq);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "keepAliveTime",
                         BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mss_info[i].mss_state);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "mssState",
                         BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mss_info[i].storage_type);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "storageType",
                         BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->mss_info[i].mode);
            xmlNewChild(node2,
                         NULL,
                         BAD_CAST "mode",
                         BAD_CAST str);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "mssLastIp",
                        BAD_CAST tmp->mss_info[i].mss_last_ip);
            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_record_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryRecordPolicy tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node, node1;
    char *xpath = "/message/policy";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"gu")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"guid")))
                        nmp_deal_text(doc, node, tmp.guid, MAX_ID_LEN);
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"domain")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"id")))
                                nmp_deal_text(doc, node1, tmp.domain_id, DOMAIN_ID_LEN);
                            node1 = node1->next;
                        }
                    }
                    else if ((!xmlStrcmp(node->name, (const xmlChar *)"mss")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"id")))
                                nmp_deal_text(doc, node1, tmp.mss_id, MSS_ID_LEN);
                            node1 = node1->next;
                        }
                    }
                    node = node->next;
                }
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryRecordPolicy));

    return sys_msg;
}


int
nmp_create_query_record_policy_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL,
    		node3 = NULL, new_node= NULL;
    xmlDocPtr doc_str;
    NmpQueryRecordPolicyRes *tmp = NULL;
    int count, total_num, i, j, xml_len;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "policy");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        count = tmp->req_num;
        total_num = tmp->total_num;

        snprintf(str, INT_TO_CHAR_LEN, "%d", total_num);
        xmlNewChild(node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "guList");
        xmlAddChild(node, node1);

        i = 0;
        while ((count > 0)&&(total_num > 0))
        {
            node2 = xmlNewNode(NULL, BAD_CAST "gu");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "areaName",
                        BAD_CAST  tmp->record_policy[i].area_name);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "puid",
                        BAD_CAST  tmp->record_policy[i].puid);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "puName",
                        BAD_CAST  tmp->record_policy[i].pu_name);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "guid",
                        BAD_CAST  tmp->record_policy[i].guid);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->record_policy[i].level);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "level",
                        BAD_CAST  str);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST  tmp->record_policy[i].gu_name);
            node3 = xmlNewNode(NULL, BAD_CAST "domain");
            xmlAddChild(node2, node3);

            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->record_policy[i].domain_id);
            node3 = xmlNewNode(NULL, BAD_CAST "mss");
            xmlAddChild(node2, node3);

            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->record_policy[i].mss_id);
            xmlNewChild(node3,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST  tmp->record_policy[i].mss_name);
            i++;
            count--;
        }

        if (tmp->type == 1)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "hdGroupList");
            xmlAddChild(node, node1);
	     for (j = 0; j < HD_GROUP_NUM; j++)
	     {
                 if (strlen(tmp->hd_group[j].hd_group_id) != 0)
                 {
                      node2 = xmlNewNode(NULL, BAD_CAST "hdGroup");
                      xmlAddChild(node1, node2);
                      xmlNewChild(node2,
                           NULL,
                           BAD_CAST "hdGroupId",
                           BAD_CAST  tmp->hd_group[j].hd_group_id);
		    }

	     }
            xml_len = strlen(tmp->time_policy);
            if (xml_len != 0)
            {
                doc_str =xmlParseMemory(tmp->time_policy,xml_len);
                if (doc_str == NULL )
                {
                    printf("Document not parsed successfully. \n");
                    return -1;
                }

                node1 = xmlDocGetRootElement(doc_str); //È·¶¨ÎÄµµ¸ùÔªËØ
                if (node1 == NULL)
                {
                    xml_error("empty document\n");
                    xmlFreeDoc(doc_str);
                    return -1;
                }
                new_node = xmlDocCopyNode(node1, doc, 1);
                xmlAddChild(node, new_node);
                xmlFreeDoc(doc_str);
            }
            else
            {
                node1 = xmlNewNode(NULL, BAD_CAST "timePolicy");
                xmlAddChild(node, node1);
            }
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_record_policy_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpRecordPolicyConfig *tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i = 0, j = 0, count, len;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node, node1, node2,new_node;
    xmlBufferPtr buff;
    char *count_path = "/message/policy/count";
    char *xpath = "/message/policy";

    app_result =  nmp_get_node(doc, (const xmlChar *)count_path);
    if (app_result == NULL)
        return NULL;

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"count")))
        {
            nmp_deal_value(doc, cur, &count);
            len = sizeof(NmpRecordPolicyConfig) + count * (sizeof(NmpRecordGu));
            tmp = nmp_mem_kalloc(len);
	     if (!tmp)
	         return NULL;

            memset(tmp, 0, len);
            tmp->gu_count = count;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"guList")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"gu")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if (tmp->gu_count == 0)
                                break;
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"guid")))
                                nmp_deal_text(doc, node1, tmp->record_policy[j].guid, MAX_ID_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"domain")))
                            {
                                node2 = node1->xmlChildrenNode;
                                while (node2 != NULL)
                                {
                                    if ((!xmlStrcmp(node2->name, (const xmlChar *)"id")))
                                    nmp_deal_text(doc, node2, tmp->record_policy[j].domain_id, DOMAIN_ID_LEN);
                                    node2 = node2->next;
                                }
                            }
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"level")))
                                nmp_deal_value(doc, node1, &tmp->record_policy[j].level);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"mss")))
                            {
                                node2 = node1->xmlChildrenNode;
                                while (node2 != NULL)
                                {
                                    if ((!xmlStrcmp(node2->name, (const xmlChar *)"id")))
                                    nmp_deal_text(doc, node2, tmp->record_policy[j].mss_id, MSS_ID_LEN);
                                    node2 = node2->next;
                                }
                            }
                            node1 = node1->next;
                        }
                        j++;
                    }
                    node = node->next;
               }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
                nmp_deal_text(doc, cur, tmp->mss_id, MSS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"level")))
                nmp_deal_value(doc, cur, &tmp->level);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp->type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"timePolicy")))
            {
                new_node = xmlDocCopyNode(cur, doc, 1);
                buff = xmlBufferCreate();
                if (xmlNodeDump(buff, doc, new_node, 0, 0) == -1)
                {
                    xmlFreeNode(new_node);
                    xmlBufferFree(buff);
                    return NULL;
                }
                //   strncpy(tmp->time_policy, buff->content,  strlen(buff->content));
                strncpy(tmp->time_policy, (char *)buff->content,  POLICY_LEN - 1);
                xmlFreeNode(new_node);
                xmlBufferFree(buff);
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
                               nmp_deal_text(doc, node1, tmp->hd_group[0].hd_group_id, HD_GROUP_ID_LEN);
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
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
//  sys_msg = nmp_msginfo_new(cmd, tmp, len);
      sys_msg = nmp_msginfo_new_2(cmd, tmp, len, (NmpMsgInfoPrivDes)nmp_mem_kfree);
    printf("-------------&&&cmd=%s, tmp->time_policy=%s\n",cmd, tmp->time_policy);
    return sys_msg;
}


/**
 * nmp_create_record_policy_config_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_record_policy_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
	NmpQueryRecordPolicyRes *tmp = NULL;

    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "policy");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_query_manufacturer(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryManufacturer tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/manufacturerList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
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
 * nmp_create_query_manufacturer_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_manufacturer_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL;
    NmpQueryManufacturerRes *tmp = NULL;
    int count, req_count, i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "manufacturerList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        count = tmp->total_num;
        req_count =  tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        xmlNewChild(node,
                    NULL,
                    BAD_CAST "count",
                    BAD_CAST str);


        node1 = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(node, node1);

        i = 0;
        while (req_count > 0)
        {
            node2 = xmlNewNode(NULL, BAD_CAST "manufacturer");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->manufacturer_info[i].mf_name);

            xmlNewChild(node2,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST tmp->manufacturer_info[i].mf_id);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_user_own_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryUserOwnGu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message/guList";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domain")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                        nmp_deal_text(doc, node, tmp.domain_id, DOMAIN_ID_LEN);

                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"user")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"name")))
                        nmp_deal_text(doc, node, tmp.user, USER_NAME_LEN);

                    node = node->next;
                }
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryUserOwnGu));

    return sys_msg;
}


/**
 * nmp_create_query_user_own_gu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_user_own_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL;
    NmpQueryUserOwnGuRes *tmp = NULL;
    int count,total_count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "guList");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        total_count = tmp->total_num;
        count = tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", total_count);
        xmlNewChild(node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "user");
        xmlAddChild(node, node1);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "name",
                    BAD_CAST tmp->user);
        node1 = xmlNewNode(NULL, BAD_CAST "domain");
        xmlAddChild(node, node1);
        xmlNewChild(node1,
                     NULL,
                     BAD_CAST "id",
                     BAD_CAST tmp->domain_id);

        //if (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "list");
            xmlAddChild(node, node1);
        }
        i = 0;
        while (count > 0)
        {
            node2 = xmlNewNode(NULL, BAD_CAST "gu");
            xmlAddChild(node1, node2);
            xmlNewChild(node2,
                     NULL,
                     BAD_CAST "guid",
                     BAD_CAST tmp->user_own_gu_info[i].user_guid);

            xmlNewChild(node2,
                     NULL,
                     BAD_CAST "guName",
                     BAD_CAST tmp->user_own_gu_info[i].guid_name);

            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_user_own_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryUserOwnTw tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"username")))
            {
            	nmp_deal_text(doc, cur, tmp.user, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryUserOwnTw));

    return sys_msg;
}


int
nmp_create_query_user_own_tw_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL;
    NmpQueryUserOwnTwRes *tmp = NULL;
    int count,total_count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, node, ATTRIBUTE_TYPE,sys_msg->msg_id);
   // node = xmlNewNode(NULL, BAD_CAST "twList");
   // xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        total_count = tmp->total_num;
        count = tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", total_count);
        xmlNewChild(node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "user");
        xmlAddChild(node, node1);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "name",
                    BAD_CAST tmp->user);

        //if (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "list");
            xmlAddChild(node, node1);
        }
        i = 0;
        while (count > 0)
        {
            node2 = xmlNewNode(NULL, BAD_CAST "tw");
            xmlAddChild(node1, node2);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->user_own_tw_info[i].tw_id);
            xmlNewChild(node2,
                     NULL,
                     BAD_CAST "twId",
                     BAD_CAST str);

            xmlNewChild(node2,
                     NULL,
                     BAD_CAST "twName",
                     BAD_CAST tmp->user_own_tw_info[i].tw_name);

            i++;
            count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_user_own_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryUserOwnTour tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"username")))
            {
            	nmp_deal_text(doc, cur, tmp.user, USER_NAME_LEN);
            }
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryUserOwnTour));

    return sys_msg;
}


int
nmp_create_query_user_own_tour_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL;
    NmpQueryUserOwnTourRes *tmp = NULL;
    int count,total_count,i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    //node = xmlNewNode(NULL, BAD_CAST "tourList");
   // xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        total_count = tmp->total_num;
        count = tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", total_count);
        xmlNewChild(node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);

        node1 = xmlNewNode(NULL, BAD_CAST "user");
        xmlAddChild(node, node1);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "name",
                    BAD_CAST tmp->user);

        //if (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "list");
            xmlAddChild(node, node1);
        }
        i = 0;
        while (count > 0)
        {
            node2 = xmlNewNode(NULL, BAD_CAST "tour");
            xmlAddChild(node1, node2);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->user_own_tour_info[i].tour_id);
            xmlNewChild(node2,
                     NULL,
                     BAD_CAST "tourId",
                     BAD_CAST str);

            xmlNewChild(node2,
                     NULL,
                     BAD_CAST "tourName",
                     BAD_CAST tmp->user_own_tour_info[i].tour_name);

            i++;
            count--;
        }
    }

    return 0;
}


/*NmpMsgInfo *
nmp_parse_query_system_time(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryServerTime tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message/calendar";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeZone")))
                nmp_deal_text(doc, cur, tmp.time_zone, TIME_ZONE_LEN);
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
}*/


NmpMsgInfo *
nmp_parse_query_system_time(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpMsgInfo *sys_msg = NULL;

    sys_msg = nmp_msginfo_new(cmd, NULL, 0);

    return sys_msg;
}


/**
 * nmp_create_query_system_time_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_system_time_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpQueryServerTimeRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "calendar");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        xmlNewChild(node,
                 NULL,
                 BAD_CAST "serverTime",
                 BAD_CAST tmp->system_time);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_link_time_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryLinkTimePolicy tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
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
nmp_create_query_link_time_policy_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node1 = NULL, new_node= NULL;
    xmlDocPtr doc_str;
    NmpQueryLinkTimePolicyRes *tmp = NULL;
    int xml_len;
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
        xml_len = strlen(tmp->time_policy);
        if (xml_len != 0)
        {
            doc_str =xmlParseMemory(tmp->time_policy,xml_len);
            if (doc_str == NULL )
            {
                printf("Document not parsed successfully. \n");
                return -1;
            }

            node1 = xmlDocGetRootElement(doc_str); //È·¶¨ÎÄµµ¸ùÔªËØ
            if (node1 == NULL)
            {
                xml_error("empty document\n");
                xmlFreeDoc(doc_str);
                return -1;
            }
            new_node = xmlDocCopyNode(node1, doc, 1);
            xmlAddChild(root_node, new_node);
            xmlFreeDoc(doc_str);
        }
        else
        {
            node1 = xmlNewNode(NULL, BAD_CAST "timePolicy");
            xmlAddChild(root_node, node1);
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_link_record(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryLinkRecord tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
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
nmp_create_query_link_record_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL, node3 = NULL;
    NmpQueryLinkRecordRes *tmp = NULL;
    int count, req_count, i, j;
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
        req_count = tmp->back_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        node = xmlNewNode(NULL, BAD_CAST "linkRecord");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;

        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "recordInfo");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkGuId",
                        BAD_CAST tmp->link_record_info[i].link_guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkDomainId",
                        BAD_CAST tmp->link_record_info[i].link_domain);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->link_record_info[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_record_info[i].alarm_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_record_info[i].time_len);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "timeLen",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_record_info[i].level);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "level",
                        BAD_CAST str);
            node2 = xmlNewNode(NULL, BAD_CAST "mssList");
            xmlAddChild(node1, node2);
            for (j = 0; j < MSS_NUM; j++)
            {
                if (strlen(tmp->link_record_info[i].mss[j].mss_id) != 0)
                {
                    node3 = xmlNewNode(NULL, BAD_CAST "mss");
                    xmlAddChild(node2, node3);
                    xmlNewChild(node3,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->link_record_info[i].mss[j].mss_id);
		   xmlNewChild(node3,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST  tmp->link_record_info[i].mss[j].mss_name);
                }
            }

            i++;
            req_count--;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_query_link_io(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryLinkIO tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
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
nmp_create_query_link_io_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryLinkIORes *tmp = NULL;
    int count, req_count, i;
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
        req_count = tmp->back_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        node = xmlNewNode(NULL, BAD_CAST "linkIO");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;

        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "IOInfo");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkGuId",
                        BAD_CAST tmp->link_io_info[i].link_guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkDomainId",
                        BAD_CAST tmp->link_io_info[i].link_domain);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->link_io_info[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_io_info[i].alarm_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_io_info[i].time_len);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "timeLen",
                        BAD_CAST str);

            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "IOValue",
                        BAD_CAST tmp->link_io_info[i].io_value);

            i++;
            req_count--;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_query_link_snapshot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryLinkSnapshot tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
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
nmp_create_query_link_snapshot_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    xmlNodePtr node2 = NULL, node3 = NULL;
    NmpQueryLinkSnapshotRes *tmp = NULL;
    int count, req_count, i, j;
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
        req_count = tmp->back_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        node = xmlNewNode(NULL, BAD_CAST "linkSnapshot");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;

        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "snapshotInfo");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkGuId",
                        BAD_CAST tmp->link_snapshot_info[i].link_guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkDomainId",
                        BAD_CAST tmp->link_snapshot_info[i].link_domain);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->link_snapshot_info[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_snapshot_info[i].alarm_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_snapshot_info[i].picture_num);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "pictureNum",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_snapshot_info[i].level);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "level",
                        BAD_CAST str);
            node2 = xmlNewNode(NULL, BAD_CAST "mssList");
            xmlAddChild(node1, node2);
            for (j = 0; j < MSS_NUM; j++)
            {
                if (strlen(tmp->link_snapshot_info[i].mss[j].mss_id) != 0)
                {
                    node3 = xmlNewNode(NULL, BAD_CAST "mss");
                    xmlAddChild(node2, node3);
                    xmlNewChild(node3,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST  tmp->link_snapshot_info[i].mss[j].mss_id);
		       xmlNewChild(node3,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST  tmp->link_snapshot_info[i].mss[j].mss_name);
                }
            }

            i++;
            req_count--;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_query_link_preset(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryLinkPreset tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
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
nmp_create_query_link_preset_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryLinkPresetRes *tmp = NULL;
    int count, req_count, i;
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
        req_count = tmp->back_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        node = xmlNewNode(NULL, BAD_CAST "linkPreset");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;

        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "presetInfo");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkGuId",
                        BAD_CAST tmp->link_preset_info[i].link_guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkDomainId",
                        BAD_CAST tmp->link_preset_info[i].link_domain);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->link_preset_info[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_preset_info[i].alarm_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_preset_info[i].preset_no);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "presetNo",
                        BAD_CAST str);
            i++;
            req_count--;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_query_link_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryLinkStep tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"twId")))
                nmp_deal_value(doc, cur, &tmp.tw_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"screenId")))
                nmp_deal_value(doc, cur, &tmp.screen_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"divisionNum")))
                nmp_deal_value(doc, cur, &tmp.div_num);
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
nmp_create_query_link_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryLinkStepRes *tmp = NULL;
    int count, req_count, i;
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
        if (tmp->type == 2)
            count = tmp->back_num;
        else
            count = tmp->total_num;
        req_count = tmp->back_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        node = xmlNewNode(NULL, BAD_CAST "linkStep");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        if (tmp->type == 2)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "stepInfo");
            xmlAddChild(node, node1);
            if (req_count == 0)
                snprintf(str, INT_TO_CHAR_LEN, "%d", 0);
            else
                snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_step_info[0].div_id);

            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divisionId",
                        BAD_CAST str);
            return 0;
        }
        i = 0;

        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "stepInfo");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "encoderGuId",
                        BAD_CAST tmp->link_step_info[i].link_guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "encoderDomainId",
                        BAD_CAST tmp->link_step_info[i].link_domain);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->link_step_info[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_step_info[i].alarm_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_step_info[i].level);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "level",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_step_info[i].tw_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "twId",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_step_info[i].screen_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "screenId",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "screenName",
                        BAD_CAST tmp->link_step_info[i].screen_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_step_info[i].div_num);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divisionNum",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_step_info[i].div_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divisionId",
                        BAD_CAST str);
            i++;
            req_count--;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_query_link_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryLinkMap tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain, DOMAIN_ID_LEN);
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
nmp_create_query_link_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryLinkMapRes *tmp = NULL;
    int count, req_count, i;
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
        req_count = tmp->back_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        node = xmlNewNode(NULL, BAD_CAST "linkMap");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;

        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "linkInfo");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkGuId",
                        BAD_CAST tmp->link_map_info[i].link_guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "linkDomainId",
                        BAD_CAST tmp->link_map_info[i].link_domain);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->link_map_info[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_map_info[i].alarm_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->link_map_info[i].level);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "level",
                        BAD_CAST str);

            i++;
            req_count--;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_set_system_time(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpSetServerTime tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/calendar";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"timeZone")))
                nmp_deal_text(doc, cur, tmp.time_zone, TIME_ZONE_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"serverTime")))
                nmp_deal_text(doc, cur, tmp.system_time, TIME_LEN);
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
 * nmp_create_set_system_time_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_set_system_time_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpSetServerTimeRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);
    node = xmlNewNode(NULL, BAD_CAST "calendar");
    xmlAddChild(root_node, node);
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}



NmpMsgInfo *
nmp_parse_database_backup(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDbBackup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/database";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"fileName")))
                nmp_deal_text(doc, cur, tmp.filename, FILENAME_LEN);
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
nmp_create_database_backup_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "database");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_database_import(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDbImport tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message/database";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"fileName")))
                nmp_deal_text(doc, cur, tmp.filename, FILENAME_LEN);
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
nmp_create_database_import_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpDbImportRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "database");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    xmlNewChild(node,
             NULL,
             BAD_CAST "description",
             BAD_CAST tmp->error_desc);

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_add_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddHdGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
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
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                         nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupName")))
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
nmp_create_bss_add_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_group_id);
    xmlNewChild(node,
                NULL,
                BAD_CAST "hdGroupId",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_add_hd_to_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddHdToGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message/hdGroup";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    memset(&tmp, 0, sizeof(tmp));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                         nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hardDisk")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"hdId")))
                         nmp_deal_value(doc, node, &tmp.hd_id);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))
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
 * nmp_create_add_hd_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_bss_add_hd_to_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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

    return 0;
}



NmpMsgInfo *
nmp_parse_bss_del_hd_from_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelHdFromGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
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
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                         nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hardDisk")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"hdId")))
                         nmp_deal_value(doc, node, &tmp.hd_id);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))
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
 * nmp_create_bss_del_hd_from_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_bss_del_hd_from_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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

    return 0;
}


int
nmp_create_bss_reboot_mss_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpRebootMssRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_reboot_mss(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpRebootMss tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

    app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
    if (!app_result)
        return NULL;

    memset(&tmp, 0, sizeof(tmp));
    xmlNodeSetPtr nodeset = app_result->nodesetval;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
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


NmpMsgInfo *
nmp_parse_bss_query_all_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryAllHdGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
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
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                         nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
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
 * nmp_create_query_all_hd_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_bss_query_all_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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
                        BAD_CAST  tmp->group_info[i].hd_group_name);
            i++;
            total_num--;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_bss_query_hd_group_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryHdGroupInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
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
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                         nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"hdGroupId")))
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
 * nmp_create_query_all_hd_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_bss_query_hd_group_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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
nmp_parse_bss_query_all_hd(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryAllHd tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
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
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                         nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
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
 * nmp_create_query_all_hd_group_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_bss_query_all_hd_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
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
nmp_parse_bss_del_hd_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDelHdGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
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
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                         nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
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
 * nmp_create_del_hd_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_bss_del_hd_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpDelHdGroupRes *tmp = NULL;
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

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_get_hd_format_progress(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetHdFormatProgress  tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    xmlNodePtr node;
    char *xpath = "/message/hdFormatInfo";

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
	     if ((!xmlStrcmp(cur->name, (const xmlChar *)"mss")))
            {
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if ((!xmlStrcmp(node->name, (const xmlChar *)"id")))
                         nmp_deal_text(doc, node, tmp.mss_id, MSS_ID_LEN);
                    else
                        xml_error("not parse the node %s\n",node->name);
                    node = node->next;
                }
            }
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


int
nmp_create_bss_get_hd_format_progress_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetHdFormatProgressRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    node = xmlNewNode(NULL, BAD_CAST "hdFormatInfo");
    xmlAddChild(root_node, node);

    xmlNewChild(node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        node1 = xmlNewNode(NULL, BAD_CAST "hdGroup");
        xmlAddChild(node, node1);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_group_id);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "hdGroupId",
                    BAD_CAST str);
        node1 = xmlNewNode(NULL, BAD_CAST "hardDisk");
        xmlAddChild(node, node1);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "hdName",
                    BAD_CAST tmp->hd_name);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->hd_id);
        xmlNewChild(node1,
                    NULL,
                    BAD_CAST "hdId",
                    BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->percent);
        xmlNewChild(node,
                    NULL,
                    BAD_CAST "percent",
                    BAD_CAST str);
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_bss_query_gu_record_status(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpQueryGuRecordStatus tmp;
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
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"guId")))
                nmp_deal_text(doc, cur, tmp.guid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
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


int
nmp_create_bss_query_gu_record_status_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL;
    NmpQueryGuRecordStatusRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    if (!RES_CODE(tmp))
    {
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->status);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "status",
                    BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->status_code);
        xmlNewChild(root_node,
                    NULL,
                    BAD_CAST "statusCode",
                    BAD_CAST str);
    }
    return 0;
}

NmpMsgInfo *
nmp_parse_query_defence_area(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryDefenceArea tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, MAX_ID_LEN);
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
 * nmp_create_query_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_defence_area_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryDefenceAreaRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->req_num;
	printf("-----------req area count=%d\n",req_count);
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);


         node = xmlNewNode(NULL, BAD_CAST "defenceAreaList");
            xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);


        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "defenceArea");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->defence_area_info[i].defence_area_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->defence_area_info[i].defence_area_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->defence_area_info[i].defence_enable);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "enable",
                        BAD_CAST str);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_defence_map(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryDefenceMap tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"defenceAreaId")))
                nmp_deal_value(doc, cur, &tmp.defence_area_id);
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
 * nmp_create_query_area_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_query_defence_map_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryDefenceMapRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	printf("-----------req area count=%d\n",req_count);
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);


         node = xmlNewNode(NULL, BAD_CAST "mapList");
            xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
            xmlAddChild(root_node, node);


        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "map");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->defence_map_info[i].map_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->defence_map_info[i].map_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "location",
                        BAD_CAST tmp->defence_map_info[i].map_location);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_defence_gu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryDefenceGu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapId")))
                nmp_deal_value(doc, cur, &tmp.map_id);
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
nmp_create_query_defence_gu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryDefenceGuRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	 printf("-----------req area count=%d\n",req_count);
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);

         node = xmlNewNode(NULL, BAD_CAST "guList");
         xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
         xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "gu");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "domainId",
                        BAD_CAST tmp->defence_gu_info[i].domain_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guId",
                        BAD_CAST tmp->defence_gu_info[i].guid);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guName",
                        BAD_CAST tmp->defence_gu_info[i].gu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->defence_gu_info[i].gu_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "guType",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puName",
                        BAD_CAST tmp->defence_gu_info[i].pu_name);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->defence_gu_info[i].coordinate_x);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateX",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->defence_gu_info[i].coordinate_y);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateY",
                        BAD_CAST str);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_map_href(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryMapHref tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mapId")))
                nmp_deal_value(doc, cur, &tmp.map_id);
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
nmp_create_query_map_href_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryMapHrefRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	 printf("-----------req area count=%d\n",req_count);
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);


         node = xmlNewNode(NULL, BAD_CAST "mapList");
         xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
         xmlAddChild(root_node, node);


        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "map");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->map_href_info[i].dst_map_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->map_href_info[i].map_name);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "location",
                        BAD_CAST tmp->map_href_info[i].map_location);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->map_href_info[i].coordinate_x);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateX",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%.4f", tmp->map_href_info[i].coordinate_y);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "coordinateY",
                        BAD_CAST str);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_tw(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryTw tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
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
nmp_create_query_tw_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryTwRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	printf("-----------req area count=%d\n",req_count);
       snprintf(str, INT_TO_CHAR_LEN, "%d", count);
       xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "totalCount",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);

        node = xmlNewNode(NULL, BAD_CAST "twList");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "tw");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "name",
                        BAD_CAST tmp->tw_info[i].tw_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tw_info[i].tw_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
                        BAD_CAST str);
            if (tmp->type == 1)
            {
                snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tw_info[i].line_num);
                xmlNewChild(node1,
                            NULL,
                            BAD_CAST "line",
                            BAD_CAST str);
		  snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tw_info[i].column_num);
                xmlNewChild(node1,
                        NULL,
                        BAD_CAST "column",
                        BAD_CAST str);
	     }
            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_screen(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryScreen tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
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
nmp_create_query_screen_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryScreenRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	printf("-----------req area count=%d\n",req_count);
	snprintf(str, INT_TO_CHAR_LEN, "%d", count);
       xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "totalCount",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);

        node = xmlNewNode(NULL, BAD_CAST "screenList");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {

            node1 = xmlNewNode(NULL, BAD_CAST "screen");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->screen_info[i].screen_id);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "id",
                BAD_CAST str);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "displayDomain",
                BAD_CAST tmp->screen_info[i].dis_domain);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "displayGuId",
                BAD_CAST tmp->screen_info[i].dis_guid);

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

            xmlNewChild(node1,
                NULL,
                BAD_CAST "puName",
                BAD_CAST  tmp->screen_info[i].pu_name);

            xmlNewChild(node1,
                NULL,
                BAD_CAST "puLastIp",
                BAD_CAST  tmp->screen_info[i].pu_ip);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->screen_info[i].pu_state);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "puState",
                BAD_CAST  str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->screen_info[i].pu_minor_type);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "puMinorType",
                BAD_CAST  str);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "guName",
                BAD_CAST  tmp->screen_info[i].gu_name);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_scr_div(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryScrDiv tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
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
nmp_create_query_scr_div_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryScrDivRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	printf("-----------req area count=%d\n",req_count);
	snprintf(str, INT_TO_CHAR_LEN, "%d", count);
       xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "totalCount",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);

        node = xmlNewNode(NULL, BAD_CAST "divScreenList");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "divScreen");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_div_info[i].div_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divId",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divName",
                        BAD_CAST tmp->scr_div_info[i].div_name);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "description",
                        BAD_CAST tmp->scr_div_info[i].description);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_tour(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryTour tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
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
nmp_create_query_tour_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryTourRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	printf("-----------req area count=%d\n",req_count);
       snprintf(str, INT_TO_CHAR_LEN, "%d", count);
       xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "totalCount",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);

        node = xmlNewNode(NULL, BAD_CAST "tourList");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "tour");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tour_info[i].tour_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
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
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_tour_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryTourStep tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"tourId")))
                nmp_deal_value(doc, cur, &tmp.tour_id);
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
nmp_create_query_tour_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryTourStepRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
        printf("-----------req area count=%d\n",req_count);
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "totalCount",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tour_id);
        xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "tourId",
                   BAD_CAST str);
         snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);
         node = xmlNewNode(NULL, BAD_CAST "tourStepList");
         xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
         xmlAddChild(root_node, node);

         i = 0;
         while (req_count > 0)
         {
             node1 = xmlNewNode(NULL, BAD_CAST "tourStep");
             xmlAddChild(node, node1);
             snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tour_step[i].step_no);
             xmlNewChild(node1,
                         NULL,
                         BAD_CAST "stepNo",
                         BAD_CAST str);
             snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tour_step[i].interval);
             xmlNewChild(node1,
                         NULL,
                         BAD_CAST "interval",
                         BAD_CAST str);
             xmlNewChild(node1,
                         NULL,
                         BAD_CAST "encoderDomain",
                         BAD_CAST tmp->tour_step[i].encoder_domain);
             xmlNewChild(node1,
                         NULL,
                         BAD_CAST "encoderGuId",
                         BAD_CAST tmp->tour_step[i].encoder_guid);
             xmlNewChild(node1,
                         NULL,
                         BAD_CAST "encoderName",
                         BAD_CAST tmp->tour_step[i].gu_name);
             snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->tour_step[i].level);
             xmlNewChild(node1,
                         NULL,
                         BAD_CAST "level",
                         BAD_CAST str);
             i++;
             req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_group(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryGroup tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
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
nmp_create_query_group_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryGroupRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	printf("-----------req area count=%d\n",req_count);
       snprintf(str, INT_TO_CHAR_LEN, "%d", count);
       xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "totalCount",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);

        node = xmlNewNode(NULL, BAD_CAST "groupList");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "group");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_info[i].group_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "id",
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
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "twName",
                        BAD_CAST tmp->group_info[i].tw_name);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_group_step(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryGroupStep tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
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
nmp_create_query_group_step_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryGroupStepRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
	printf("-----------req area count=%d\n",req_count);
       snprintf(str, INT_TO_CHAR_LEN, "%d", count);
       xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "totalCount",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);

        node = xmlNewNode(NULL, BAD_CAST "groupStepList");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "groupStep");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_step[i].step_no);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "stepNo",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_step[i].interval);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "interval",
                        BAD_CAST str);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_group_step_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryGroupStepInfo tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stepNo")))
                nmp_deal_value(doc, cur, &tmp.step_no);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"scrId")))
                nmp_deal_value(doc, cur, &tmp.scr_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
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
nmp_create_query_group_step_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryGroupStepInfoRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->back_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "totalCount",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_id);
        xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "groupId",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->scr_id);
        xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "scrId",
                   BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->div_id);
        xmlNewChild(root_node,
                   NULL,
                   BAD_CAST "divId",
                   BAD_CAST str);

        snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);
        node = xmlNewNode(NULL, BAD_CAST "groupStepInfoList");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "groupStepInfo");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_step[i].div_no);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "divNo",
                        BAD_CAST str);
            xmlNewChild(node1,
                         NULL,
                         BAD_CAST "encoderDomain",
                         BAD_CAST tmp->group_step[i].encoder_domain);
             xmlNewChild(node1,
                         NULL,
                         BAD_CAST "encoderGuId",
                         BAD_CAST tmp->group_step[i].encoder_guid);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_step[i].level);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "level",
                        BAD_CAST str);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "puName",
                BAD_CAST  tmp->group_step[i].pu_name);

            xmlNewChild(node1,
                NULL,
                BAD_CAST "puLastIp",
                BAD_CAST  tmp->group_step[i].pu_ip);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->group_step[i].pu_state);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "puState",
                BAD_CAST  str);
            xmlNewChild(node1,
                NULL,
                BAD_CAST "guName",
                BAD_CAST  tmp->group_step[i].gu_name);
            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_group_step_div(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryGroupStepDiv tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"groupId")))
                nmp_deal_value(doc, cur, &tmp.group_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"scrId")))
                nmp_deal_value(doc, cur, &tmp.scr_id);
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
nmp_create_query_group_step_div_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpQueryGroupStepDivRes *tmp = NULL;
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
                 BAD_CAST "divId",
                 BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_alarm(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryAlarm tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"order")))
                nmp_deal_value(doc, cur, &tmp.order_by);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmType")))
                nmp_deal_value(doc, cur, &tmp.alarm_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmState")))
                nmp_deal_value(doc, cur, &tmp.alarm_state);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmStartTime")))
                nmp_deal_text(doc, cur, tmp.start_time, TIME_INFO_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"alarmEndTime")))
                nmp_deal_text(doc, cur, tmp.end_time, TIME_INFO_LEN);
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
nmp_create_query_alarm_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryAlarmRes *tmp = NULL;
    int count, req_count, i;
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
        req_count =  tmp->req_num;
	 printf("-----------req area count=%d\n",req_count);
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);


         node = xmlNewNode(NULL, BAD_CAST "alarmList");
         xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
         xmlAddChild(root_node, node);

        i = 0;
        while (req_count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "alarm");
            xmlAddChild(node, node1);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->alarm_list[i].alarm_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmId",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "domainId",
                        BAD_CAST tmp->alarm_list[i].domain_id);
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
                        BAD_CAST "alarmState",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "alarmTime",
                        BAD_CAST tmp->alarm_list[i].alarm_time);
	     xmlNewTextChild(node1,
                       NULL,
                       BAD_CAST "alarmInfo",
                       BAD_CAST tmp->alarm_list[i].alarm_info);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "operator",
                        BAD_CAST  tmp->alarm_list[i].operator);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "dealTime",
                        BAD_CAST tmp->alarm_list[i].deal_time);
	     xmlNewTextChild(node1,
                       NULL,
                       BAD_CAST "description",
                       BAD_CAST tmp->alarm_list[i].description);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "submitTime",
                        BAD_CAST tmp->alarm_list[i].submit_time);

            i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_del_alarm_policy(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpMsgInfo *sys_msg = NULL;

    sys_msg = nmp_msginfo_new(cmd, NULL, 0);

    return sys_msg;
}


int
nmp_create_query_del_alarm_policy_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpQueryDelAlarmPolicyRes *tmp = NULL;
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
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->enable);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "enable",
                BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->max_capacity);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "maxCapacity",
                BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->del_alarm_num);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "delAlarmNum",
                BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_platform_upgrade(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpPlatformUpgrade tmp;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
                nmp_deal_text(doc, cur, tmp.admin_name, USER_NAME_LEN);
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
nmp_create_bss_platform_upgrade_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpPlatformUpgradeResp *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_get_net_interface_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMsgInfo *sys_msg = NULL;

    sys_msg = nmp_msginfo_new(cmd, NULL, 0);
    return sys_msg;
}


int
nmp_create_get_net_interface_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpGetNetInterfaceConfigRes *tmp = NULL;
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
                    BAD_CAST "networkInterface",
                    BAD_CAST tmp->network_interface);
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_get_network_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMsgInfo *sys_msg = NULL;
    NmpGetNetworkConfig tmp;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"networkInterface")))
                nmp_deal_text(doc, cur, tmp.network_interface, NET_INTERFACE_LEN);
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
nmp_create_get_network_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpGetNetworkConfigRes *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};
	int count = 0, i = 0;

	tmp = nmp_get_msginfo_data(sys_msg);
	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE, sys_msg->msg_id);

	xmlNewChild(root_node,
	            NULL,
	            BAD_CAST "resultCode",
	            BAD_CAST str);

	if (!RES_CODE(tmp))
	{
		xmlNewChild(root_node,
			NULL,
			BAD_CAST "dns",
			BAD_CAST tmp->dns);

		count = tmp->count;
		snprintf(str, INT_TO_CHAR_LEN, "%d", count);
		node = xmlNewNode(NULL, BAD_CAST "ipList");
		xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
		xmlAddChild(root_node, node);

		while (count > 0)
		{
			node1 = xmlNewNode(NULL, BAD_CAST "ipInfo");
			xmlAddChild(node, node1);
			xmlNewChild(node1,
				NULL,
				BAD_CAST "ip",
				BAD_CAST tmp->ip_list[i].ip);
			xmlNewChild(node1,
				NULL,
				BAD_CAST "netmask",
				BAD_CAST tmp->ip_list[i].netmask);
			xmlNewChild(node1,
				NULL,
				BAD_CAST "gateway",
				BAD_CAST tmp->ip_list[i].gateway);

			i++;
			count--;
		}
	}

    return 0;
}


NmpMsgInfo *
nmp_parse_set_network_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpSetNetworkConfig *res_info = NULL;
    NmpSetNetworkConfig	tmp;
    NmpMsgInfo *sys_msg = NULL;
    gint i;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"networkInterface")))
                nmp_deal_text(doc, cur, tmp.network_interface, NET_INTERFACE_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dns")))
                nmp_deal_text(doc, cur, tmp.dns, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ipList")))
            {
                count = (char *)xmlGetProp(cur, (const xmlChar *)"count");
                if (count)
                {
                    tmp.count = atoi(count);
                    xmlFree(count);
                }
		  else
		  	tmp.count = 0;

                size = sizeof(NmpSetNetworkConfig) + tmp.count * sizeof(IpInfo);
                res_info = nmp_mem_kalloc(size);
                if (!res_info)
                    return NULL;

                memset(res_info, 0, sizeof(res_info));
                node = cur->xmlChildrenNode;
                while (node != NULL)
                {
                    if (tmp.count <= 0)
		           break;
                    if  ((!xmlStrcmp(node->name, (const xmlChar *)"ipInfo")))
                    {
                        node1 = node->xmlChildrenNode;
                        while (node1 != NULL)
                        {
                            if ((!xmlStrcmp(node1->name, (const xmlChar *)"ip")))
                                nmp_deal_text(doc, node1, res_info->ip_list[j].ip, MAX_IP_LEN);
                            else if ((!xmlStrcmp(node1->name, (const xmlChar *)"netmask")))
                                nmp_deal_text(doc, node1, res_info->ip_list[j].netmask, MAX_IP_LEN);
			         else if ((!xmlStrcmp(node1->name, (const xmlChar *)"gateway")))
                                nmp_deal_text(doc, node1, res_info->ip_list[j].gateway, MAX_IP_LEN);
                            else
                                xml_warning("Warning, not parse the node %s \n", node1->name);
                            node1 = node1->next;
                        }

                        if (j < tmp.count)
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
        size = sizeof(NmpSetNetworkConfig);
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
nmp_create_set_network_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpSetResult *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_query_server_resource_info(xmlDocPtr doc ,
	xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpMsgInfo *sys_msg = NULL;
    NmpQueryServerResourceInfo tmp;

    memset(&tmp, 0, sizeof(tmp));
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_query_server_resource_info_resp(xmlDocPtr doc,
	NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpQueryServerResourceInfoRes *tmp = NULL;
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
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->enc_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "encoderNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->enc_online_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "encoderOnlineNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->dec_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "decoderNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->dec_online_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "decoderOnlineNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->av_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "avNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->av_limited_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "avLimitedNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->ds_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "dsNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->ds_limited_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "dsLimitedNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->ai_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "aiNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->ai_limited_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "aiLimitedNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->ao_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "aoNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->ao_limited_num);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "aoLimitedNum",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->expired_time.type);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "expiredType",
                 BAD_CAST str);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "expiredTime",
                 BAD_CAST tmp->expired_time.expired_time);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->system_version);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "systemVersion",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->manufactor_type);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "manufacturerType",
                 BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->support_keyboard);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "supportKeyboard",
                 BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_cms_all_ips(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMsgInfo *sys_msg = NULL;

    sys_msg = nmp_msginfo_new(cmd, NULL, 0);
    return sys_msg;
}


int
nmp_create_query_cms_all_ips_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL;
    NmpQueryCmsAllIpRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int count, i = 0;

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
        node = xmlNewNode(NULL, BAD_CAST "cms");
        xmlAddChild(root_node, node);
        count = tmp->count;
        while (count > 0)
        {
            if (strcmp(tmp->ips[i].ip, LOCAL_HOST_IP))
            {
                xmlNewChild(node,
                        NULL,
                        BAD_CAST "ip",
                        BAD_CAST tmp->ips[i].ip);
            }
            count--;
            i++;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_search_device(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpMsgInfo *sys_msg = NULL;

    sys_msg = nmp_msginfo_new(cmd, NULL, 0);
    return sys_msg;
}


int
nmp_create_search_device_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpSearchPuRes *tmp = NULL;
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
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->pu_count);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_get_searched_device(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpGetSearchedPu tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
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
nmp_create_get_searched_device_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetSearchedPuRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int count, i = 0;

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
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->pu_count);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "totalCount",
                BAD_CAST str);
        snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->res_count);
        node = xmlNewNode(NULL, BAD_CAST "deviceList");
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);
        count = tmp->res_count;
        printf("--------dev count=%d\n",count);
        while (count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "device");
            xmlAddChild(node, node1);

            printf("--------dev dst_id=%s\n",tmp->search_pu[i].dst_id);
            printf("--------dev dst_ip=%s\n",tmp->search_pu[i].nmp_srch.dev_info.dev_ip);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "manufacturer",
                        BAD_CAST tmp->search_pu[i].nmp_srch.dev_info.mnfct);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->search_pu[i].nmp_srch.dev_info.pu_type);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puType",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->search_pu[i].nmp_srch.dev_info.av_mun);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "avNum",
                        BAD_CAST str);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "devId",
                        BAD_CAST tmp->search_pu[i].dst_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "devIp",
                        BAD_CAST tmp->search_pu[i].nmp_srch.dev_info.dev_ip);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "puId",
                        BAD_CAST tmp->search_pu[i].nmp_srch.dev_info.pu_id);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "cmsIp",
                        BAD_CAST tmp->search_pu[i].nmp_srch.nmp_plt.cms_ip);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "mdsIp",
                        BAD_CAST tmp->search_pu[i].nmp_srch.nmp_plt.mds_ip);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->search_pu[i].nmp_srch.nmp_plt.cms_port);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "cmsPort",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->search_pu[i].nmp_srch.nmp_plt.mds_port);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "mdsPort",
                        BAD_CAST str);
            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->search_pu[i].nmp_srch.nmp_plt.conn_cms);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "connectCmsEnable",
                        BAD_CAST str);
            count--;
            i++;
        }
    }
    return 0;
}


NmpMsgInfo *
nmp_parse_query_tw_auth_info(xmlDocPtr doc ,
	xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpMsgInfo *sys_msg = NULL;
    NmpQueryTwAuthInfo tmp;

    memset(&tmp, 0, sizeof(tmp));
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_query_tw_auth_info_resp(xmlDocPtr doc,
	NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpQueryTwAuthInfoRes *tmp = NULL;
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
nmp_parse_query_alarm_link_auth_info(xmlDocPtr doc ,
	xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpMsgInfo *sys_msg = NULL;
    NmpQueryAlarmLinkAuthInfo tmp;

    memset(&tmp, 0, sizeof(tmp));
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_query_alarm_link_auth_info_resp(xmlDocPtr doc,
	NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpQueryAlarmLinkAuthInfoRes *tmp = NULL;
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
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->alarm_link_auth_type);
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "alarmLinkAuthType",
                 BAD_CAST str);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_query_ivs(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryIvs tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
                nmp_deal_value(doc, cur, &tmp.req_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
                nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, USER_NAME_LEN);
            else
                xml_error("not parse the node %s\n",cur->name);

            cur = cur->next;
            while ( cur && xmlIsBlankNode (cur) )
                cur = cur->next;
        }
    }

    xmlXPathFreeObject(app_result);
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(NmpQueryIvs));

    return sys_msg;
}


int
nmp_create_query_ivs_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryIvsRes *tmp = NULL;
    int count, req_count, i;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;

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
        req_count = tmp->req_num;
        snprintf(str, INT_TO_CHAR_LEN, "%d", count);
        xmlNewChild(root_node,
                NULL,
                BAD_CAST "count",
                BAD_CAST str);

        node = xmlNewNode(NULL, BAD_CAST "list");
        xmlAddChild(root_node, node);
        i = 0;
        while (req_count > 0)
        {

            node1 = xmlNewNode(NULL, BAD_CAST "ivs");
            xmlAddChild(node, node1);

            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "ivsId",
                        BAD_CAST  tmp->ivs_info[i].ivs_id);

            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "ivsName",
                        BAD_CAST tmp->ivs_info[i].ivs_name);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->ivs_info[i].keep_alive_freq);
            xmlNewChild(node1,
                         NULL,
                         BAD_CAST "keepAliveTime",
                         BAD_CAST str);

            snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->ivs_info[i].ivs_state);
            xmlNewChild(node1,
                         NULL,
                         BAD_CAST "ivsState",
                         BAD_CAST str);

		xmlNewChild(node1,
                        NULL,
                        BAD_CAST "ivsIP",
                        BAD_CAST tmp->ivs_info[i].ivs_last_ip);

	     	i++;
            req_count--;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_auto_add_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAutoAddPu tmp;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"devId")))
                nmp_deal_text(doc, cur, tmp.dev_id, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"domainId")))
                nmp_deal_text(doc, cur, tmp.domain_id, DOMAIN_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"manufacturer")))
                nmp_deal_text(doc, cur, tmp.manufacturer, MF_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puName")))
                nmp_deal_text(doc, cur, tmp.dev_name, PU_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puId")))
                nmp_deal_text(doc, cur, tmp.puid, MAX_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"keepAliveTime")))
            {
                nmp_deal_value(doc, cur, &tmp.keep_alive_time);
                if (tmp.keep_alive_time < 3)
                    tmp.keep_alive_time = 3;
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"puType")))
                nmp_deal_value(doc, cur, &tmp.pu_type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mduId")))
                nmp_deal_text(doc, cur, tmp.mds_id, MDS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"areaId")))
                nmp_deal_value(doc, cur, &tmp.area_id);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"avNum")))
                nmp_deal_value(doc, cur, &tmp.av_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsIp")))
                nmp_deal_text(doc, cur, tmp.cms_ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmsPort")))
                nmp_deal_value(doc, cur, &tmp.cms_port);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"connectCmsEnable")))
                nmp_deal_value(doc, cur, &tmp.connect_cms_enable);
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
 * nmp_create_auto_add_pu_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_auto_add_pu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpMsgErrCode *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);
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
nmp_parse_get_next_puno(xmlDocPtr doc ,
	xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpMsgInfo *sys_msg = NULL;
    NmpGetNextPuNo tmp;

    memset(&tmp, 0, sizeof(tmp));
    sys_msg = nmp_msginfo_new(cmd, &tmp, sizeof(tmp));

    return sys_msg;
}


int
nmp_create_get_next_puno_resp(xmlDocPtr doc,
	NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpGetNextPuNoRes *tmp = NULL;
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
        xmlNewChild(root_node,
                 NULL,
                 BAD_CAST "puNo",
                 BAD_CAST tmp->pu_no);
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_get_initiator_name(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetInitName tmp;
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
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
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


int
nmp_create_bss_get_initiator_name_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpGetInitNameRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "initiatorName",
                BAD_CAST tmp->init_name);

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_set_initiator_name(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpSetInitName tmp;
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
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"initiatorName")))
                nmp_deal_text(doc, cur, tmp.init_name, USER_NAME_LEN);
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
nmp_create_bss_set_initiator_name_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpSetInitNameRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_get_ipsan_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetIpsanInfo tmp;
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
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
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


int
nmp_create_bss_get_ipsan_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpGetIpsanInfoRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
    int count, i = 0;

    tmp = nmp_get_msginfo_data(sys_msg);

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    if (!RES_CODE(tmp))
    {
        node = xmlNewNode(NULL, BAD_CAST "ipsans");
        snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->count);
        xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
        xmlAddChild(root_node, node);
        count = tmp->count;
        while(count > 0)
        {
            node1 = xmlNewNode(NULL, BAD_CAST "ipsan");
            xmlAddChild(node, node1);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "ip",
                        BAD_CAST tmp->ipsan_info[i].ip);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->ipsan_info[i].port);
            xmlNewChild(node1,
                        NULL,
                        BAD_CAST "port",
                        BAD_CAST str);
            count--;
            i++;
        }
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_add_one_ipsan(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAddOneIpsan tmp;
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
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ip")))
                nmp_deal_text(doc, cur, tmp.ipsan_info.ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
                nmp_deal_value(doc, cur, &tmp.ipsan_info.port);
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
nmp_create_bss_add_one_ipsan_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpAddOneIpsanRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_delete_one_ipsan(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpDeleteOneIpsan tmp;
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
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ip")))
                nmp_deal_text(doc, cur, tmp.ipsan_info.ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
                nmp_deal_value(doc, cur, &tmp.ipsan_info.port);
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
nmp_create_bss_delete_one_ipsan_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpDeleteOneIpsanRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_bss_get_one_ipsan_detail(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpGetOneIpsanDetail tmp;
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
                nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
	     else if ((!xmlStrcmp(cur->name, (const xmlChar *)"sessionId")))
                nmp_deal_text(doc, cur, tmp.session, USER_NAME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ip")))
                nmp_deal_text(doc, cur, tmp.ipsan_info.ip, MAX_IP_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
                nmp_deal_value(doc, cur, &tmp.ipsan_info.port);
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
nmp_create_bss_get_one_ipsan_detail_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpGetOneIpsanDetailRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};

    tmp = nmp_get_msginfo_data(sys_msg);

    root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
    nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "resultCode",
                BAD_CAST str);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "target",
                BAD_CAST tmp->target);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", tmp->connect_state);
    xmlNewChild(root_node,
                NULL,
                BAD_CAST "connectState",
                BAD_CAST str);

    return 0;
}


NmpMsgInfo *
nmp_parse_query_log(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	ASSERT(doc != NULL && cur != NULL && cmd != NULL);

	NmpQueryLog tmp;
	NmpMsgInfo *sys_msg = NULL;
	int i;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";

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
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"orderBy")))
				nmp_deal_value(doc, cur, &tmp.order_by);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
				nmp_deal_value(doc, cur, &tmp.req_count);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
				nmp_deal_value(doc, cur, &tmp.start_num);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"logLevel")))
				nmp_deal_value(doc, cur, &tmp.log_level);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"logStartTime")))
				nmp_deal_text(doc, cur, tmp.start_time, TIME_INFO_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"logEndTime")))
				nmp_deal_text(doc, cur, tmp.end_time, TIME_INFO_LEN);
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
nmp_create_query_log_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpQueryLogRes *tmp = NULL;
	int count, req_count, i;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
	xmlNewChild(root_node,
		NULL,
		BAD_CAST "resultCode",
		BAD_CAST str);
	if (RES_CODE(tmp))
		goto end;

	count = tmp->total_num;
	req_count = tmp->req_total;
	snprintf(str, INT_TO_CHAR_LEN, "%d", count);
	node = xmlNewNode(NULL, BAD_CAST "logList");
	xmlNewProp(node, BAD_CAST "totalCount", BAD_CAST str);
	xmlAddChild(root_node, node);

	i = 0;
	while (req_count--)
	{
		NmpBssLog *log = &tmp->log_list[i];
		node1 = xmlNewNode(NULL, BAD_CAST "log");
		xmlAddChild(node, node1);

		snprintf(str, INT_TO_CHAR_LEN, "%d", log->order_num);
		xmlNewChild(node1,
			NULL,
			BAD_CAST "orderNum",
			BAD_CAST str);

		snprintf(str, INT_TO_CHAR_LEN, "%d", log->log_level);
		xmlNewChild(node1,
			NULL,
			BAD_CAST "logLevel",
			BAD_CAST str);

		snprintf(str, INT_TO_CHAR_LEN, "%d", log->log_id);
		xmlNewChild(node1,
			NULL,
			BAD_CAST "logId",
			BAD_CAST str);

		snprintf(str, INT_TO_CHAR_LEN, "%d", log->result_code);
		xmlNewChild(node1,
			NULL,
			BAD_CAST "exeCode",
			BAD_CAST str);

		xmlNewChild(node1,
			NULL,
			BAD_CAST "logTime",
			BAD_CAST log->log_time);

		xmlNewChild(node1,
			NULL,
			BAD_CAST "userName",
			BAD_CAST log->user_name);

		xmlNewChild(node1,
			NULL,
			BAD_CAST "childData1",
			BAD_CAST log->child_data1);

		xmlNewChild(node1,
			NULL,
			BAD_CAST "childData2",
			BAD_CAST log->child_data2);

		xmlNewChild(node1,
			NULL,
			BAD_CAST "childData3",
			BAD_CAST log->child_data3);

		i++;
	}

end:
	return 0;
}


#if ONLINE_RATE_FLAG
NmpMsgInfo *
nmp_parse_query_area_dev_online_rate(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    ASSERT(doc != NULL && cur != NULL && cmd != NULL);

    NmpQueryAreaDevRate tmp;
    NmpMsgInfo *sys_msg = NULL;
    int i;
    xmlXPathObjectPtr app_result;
    char *xpath = "/message";

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
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"pageSize")))
				nmp_deal_value(doc, cur, &tmp.req_num);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"startRow")))
				nmp_deal_value(doc, cur, &tmp.start_num);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
                nmp_deal_value(doc, cur, &tmp.type);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
                nmp_deal_text(doc, cur, tmp.key, TIME_LEN);
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"areaId")))
                nmp_deal_value(doc, cur, &tmp.area_id);
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
nmp_create_query_area_dev_online_rate_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    NmpQueryAreaDevRateRes *tmp = NULL;
    char str[INT_TO_CHAR_LEN] = {0};
	int count, req_count, i;

    tmp = nmp_get_msginfo_data(sys_msg);
    if (!tmp)
        return -E_NOMEM;//-ENOMEM;

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	snprintf(str, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));
	xmlNewChild(root_node,
		NULL,
		BAD_CAST "resultCode",
		BAD_CAST str);
	if (RES_CODE(tmp))
		goto end;

	count = tmp->total_num;
	req_count = tmp->back_num;
	snprintf(str, INT_TO_CHAR_LEN,  "%d", count);
	xmlNewChild(root_node,
		NULL,
		BAD_CAST "totalCount",
		BAD_CAST str);
	snprintf(str, INT_TO_CHAR_LEN, "%d", req_count);
	node = xmlNewNode(NULL, BAD_CAST "areaList");
	xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
	xmlAddChild(root_node, node);

	i = 0;
	while (req_count--)
	{
		NmpAreaDevRate *area_list = &tmp->area_dev_rate[i];
		node1 = xmlNewNode(NULL, BAD_CAST "area");
		xmlAddChild(node, node1);

		snprintf(str, INT_TO_CHAR_LEN, "%d", area_list->area_id);
		xmlNewChild(node1,
			NULL,
			BAD_CAST "areaId",
			BAD_CAST str);

		xmlNewChild(node1,
			NULL,
			BAD_CAST "areaName",
			BAD_CAST area_list->area_name);
		snprintf(str, INT_TO_CHAR_LEN, "%.0lf", area_list->total_count);
		xmlNewChild(node1,
			NULL,
			BAD_CAST "totalCount",
			BAD_CAST str);
		snprintf(str, INT_TO_CHAR_LEN, "%.0lf", area_list->online_count);
		xmlNewChild(node1,
			NULL,
			BAD_CAST "onlineCount",
			BAD_CAST str);
		snprintf(str, INT_TO_CHAR_LEN, "%.2lf", area_list->rate*100);
		xmlNewChild(node1,
			NULL,
			BAD_CAST "onlineRate",
			BAD_CAST str);

		i++;
	}

end:
	return 0;
}
#endif


NmpMsgInfo *
nmp_parse_get_server_flag(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMsgInfo *sys_msg = NULL;

	sys_msg = nmp_msginfo_new(cmd, NULL, 0);
	return sys_msg;
}


int
nmp_create_get_server_flag_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpGetServerFlagRes *tmp = NULL;
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
		snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->server_flag);
		xmlNewChild(root_node,
			NULL,
			BAD_CAST "serverFlag",
			BAD_CAST str);
	}
	return 0;
}


NmpMsgInfo *
nmp_parse_get_mds_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMsgInfo *sys_msg = NULL;

	sys_msg = nmp_msginfo_new(cmd, NULL, 0);
	return sys_msg;
}


int
nmp_create_get_mds_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpGetMdsConfigRes *tmp = NULL;
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
			BAD_CAST "mdsId",
			BAD_CAST tmp->mds_id);

		snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->start_port);
		xmlNewChild(root_node,
			NULL,
			BAD_CAST "beginPort",
			BAD_CAST str);

		snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->end_port);
		xmlNewChild(root_node,
			NULL,
			BAD_CAST "endPort",
			BAD_CAST str);
	}
	return 0;
}


NmpMsgInfo *
nmp_parse_set_mds_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpSetMdsConfig tmp;
	NmpMsgInfo *sys_msg = NULL;
	int i;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	memset(&tmp, 0, sizeof(tmp));

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (!app_result)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;

	for (i=0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;

		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsId")))
				nmp_deal_text(doc, cur, tmp.mds_id, MDS_ID_LEN);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"beginPort")))
				nmp_deal_value(doc, cur, &tmp.start_port);
			else if ((!xmlStrcmp(cur->name, (const xmlChar *)"endPort")))
				nmp_deal_value(doc, cur, &tmp.end_port);
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
nmp_create_set_mds_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpSetResult *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	xmlNewChild(root_node,
		NULL,
		BAD_CAST "resultCode",
		BAD_CAST str);

	return 0;
}


NmpMsgInfo *
nmp_parse_get_mds_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpGetMdsState tmp;
	NmpMsgInfo *sys_msg = NULL;
	int i;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	memset(&tmp, 0, sizeof(tmp));

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (!app_result)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;

	for (i=0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;

		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"mdsId")))
				nmp_deal_text(doc, cur, tmp.mds_id, MDS_ID_LEN);
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
nmp_create_get_mds_state_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpGetMdsStateRes *tmp = NULL;
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
		snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->state);
		xmlNewChild(root_node,
			NULL,
			BAD_CAST "state",
			BAD_CAST str);
	}
	return 0;
}


NmpMsgInfo *
nmp_parse_get_mss_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMsgInfo *sys_msg = NULL;

	sys_msg = nmp_msginfo_new(cmd, NULL, 0);
	return sys_msg;
}


int
nmp_create_get_mss_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpGetMssConfigRes *tmp = NULL;
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
			BAD_CAST "mssId",
			BAD_CAST tmp->mss_id);
	}
	return 0;
}


NmpMsgInfo *
nmp_parse_set_mss_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpSetMssConfig tmp;
	NmpMsgInfo *sys_msg = NULL;
	int i;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	memset(&tmp, 0, sizeof(tmp));

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (!app_result)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;

	for (i=0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;

		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
				nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
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
nmp_create_set_mss_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpSetResult *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	xmlNewChild(root_node,
		NULL,
		BAD_CAST "resultCode",
		BAD_CAST str);

	return 0;
}


NmpMsgInfo *
nmp_parse_get_mss_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpGetMssState tmp;
	NmpMsgInfo *sys_msg = NULL;
	int i;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	memset(&tmp, 0, sizeof(tmp));

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (!app_result)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;

	for (i=0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;

		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"mssId")))
				nmp_deal_text(doc, cur, tmp.mss_id, MSS_ID_LEN);
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
nmp_create_get_mss_state_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpGetMssStateRes *tmp = NULL;
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
		snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->state);
		xmlNewChild(root_node,
			NULL,
			BAD_CAST "state",
			BAD_CAST str);
	}
	return 0;
}


NmpMsgInfo *
nmp_parse_get_ivs_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMsgInfo *sys_msg = NULL;

	sys_msg = nmp_msginfo_new(cmd, NULL, 0);
	return sys_msg;
}


int
nmp_create_get_ivs_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpGetIvsConfigRes *tmp = NULL;
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
			BAD_CAST "ivsId",
			BAD_CAST tmp->ivs_id);
	}
	return 0;
}


NmpMsgInfo *
nmp_parse_set_ivs_config(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpSetIvsConfig tmp;
	NmpMsgInfo *sys_msg = NULL;
	int i;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	memset(&tmp, 0, sizeof(tmp));

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (!app_result)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;

	for (i=0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;

		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
				nmp_deal_text(doc, cur, tmp.ivs_id, IVS_ID_LEN);
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
nmp_create_set_ivs_config_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpSetResult *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	xmlNewChild(root_node,
		NULL,
		BAD_CAST "resultCode",
		BAD_CAST str);

	return 0;
}


NmpMsgInfo *
nmp_parse_get_ivs_state(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpGetIvsState tmp;
	NmpMsgInfo *sys_msg = NULL;
	int i;
	xmlXPathObjectPtr app_result;
	char *xpath = "/message";
	memset(&tmp, 0, sizeof(tmp));

	app_result =  nmp_get_node(doc, (const xmlChar *)xpath);
	if (!app_result)
		return NULL;

	xmlNodeSetPtr nodeset = app_result->nodesetval;

	for (i=0; i < nodeset->nodeNr; i++)
	{
		cur = nodeset->nodeTab[i];
		cur = cur->xmlChildrenNode;

		while (cur != NULL)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *)"ivsId")))
				nmp_deal_text(doc, cur, tmp.ivs_id, IVS_ID_LEN);
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
nmp_create_get_ivs_state_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpGetIvsStateRes *tmp = NULL;
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
		snprintf(str, INT_TO_CHAR_LEN, "%d", tmp->state);
		xmlNewChild(root_node,
			NULL,
			BAD_CAST "state",
			BAD_CAST str);
	}
	return 0;
}


NmpMsgInfo *
nmp_parse_cms_shutdown(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMsgInfo *sys_msg = NULL;

	sys_msg = nmp_msginfo_new(cmd, NULL, 0);
	return sys_msg;
}


int
nmp_create_cms_shutdown_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpSetResult *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	xmlNewChild(root_node,
		NULL,
		BAD_CAST "resultCode",
		BAD_CAST str);

	return 0;
}


NmpMsgInfo *
nmp_parse_cms_reboot(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpMsgInfo *sys_msg = NULL;

	sys_msg = nmp_msginfo_new(cmd, NULL, 0);
	return sys_msg;
}


int
nmp_create_cms_reboot_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpSetResult *tmp = NULL;
	char str[INT_TO_CHAR_LEN] = {0};

	tmp = nmp_get_msginfo_data(sys_msg);
	snprintf(str, INT_TO_CHAR_LEN, "%d", RES_CODE(tmp));

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);

	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	xmlNewChild(root_node,
		NULL,
		BAD_CAST "resultCode",
		BAD_CAST str);

	return 0;
}



NmpMsgInfo *
nmp_parse_add_ams(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpAddAms tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.ams_id, "amsId", AMS_ID_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.ams_name, "amsName", AMS_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.keep_alive_freq, "keepAliveTime")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_add_ams_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NMP_XML_DEF_CREATE_NMP_RESULT();
}


NmpMsgInfo *
nmp_parse_modify_ams(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpModifyAms tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.ams_id, "amsId", AMS_ID_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.ams_name, "amsName", AMS_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.keep_alive_freq, "keepAliveTime")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_modify_ams_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NMP_XML_DEF_CREATE_NMP_RESULT();
}


NmpMsgInfo *
nmp_parse_del_ams(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpDelAms tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.ams_id, "amsId", AMS_ID_LEN)

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_del_ams_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NMP_XML_DEF_CREATE_NMP_RESULT();
}


NmpMsgInfo *
nmp_parse_query_ams(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpQueryAms tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.ams_id, "amsId", AMS_ID_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.type, "type")
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.req_num, "pageSize")
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.start_num, "startRow")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_query_ams_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpQueryAmsRes *tmp = NULL;
	int back_count, i;
	char str[INT_TO_CHAR_LEN] = {0};

	NMP_XML_DEF_CREATE_BEGIN(tmp, str, root_node, end);

	NMP_XML_DEF_VAL_CREATE_CHILD("count", tmp->total_count, root_node, str);

	back_count = tmp->back_count;
	node = xmlNewNode(NULL, BAD_CAST "list");
	xmlAddChild(root_node, node);

	i = 0;
	while (back_count--)
	{
		NmpAmsInfo *ams = &tmp->ams_info[i];
		node1 = xmlNewNode(NULL, BAD_CAST "ams");
		xmlAddChild(node, node1);

		NMP_XML_DEF_STR_CREATE_CHILD("amsId", ams->ams_id, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("amsName", ams->ams_name, node1);
		NMP_XML_DEF_VAL_CREATE_CHILD("keepAliveTime", ams->keep_alive_freq, node1, str);
		NMP_XML_DEF_VAL_CREATE_CHILD("amsState", ams->ams_state, node1, str);
		NMP_XML_DEF_STR_CREATE_CHILD("amsIP", ams->ams_ip, node1);

		i++;
	}

end:
	return 0;
}


NmpMsgInfo *
nmp_parse_query_ams_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpQueryAmsPu tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.ams_id, "amsId", AMS_ID_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.req_num, "pageSize")
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.start_num, "startRow")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_query_ams_pu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpQueryAmsPuRes *tmp = NULL;
	int back_count, i;
	char str[INT_TO_CHAR_LEN] = {0};

	NMP_XML_DEF_CREATE_BEGIN(tmp, str, root_node, end);

	NMP_XML_DEF_VAL_CREATE_CHILD("count", tmp->total_count, root_node, str);

	back_count = tmp->back_count;
	node = xmlNewNode(NULL, BAD_CAST "list");
	xmlAddChild(root_node, node);

	i = 0;
	while (back_count--)
	{
		NmpAmsPuInfo *pu = &tmp->pu_info[i];
		node1 = xmlNewNode(NULL, BAD_CAST "device");
		xmlAddChild(node, node1);

		NMP_XML_DEF_STR_CREATE_CHILD("puid", pu->puid, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("domain", pu->domain, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("puName", pu->pu_name, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("areaName", pu->area_name, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("devName", pu->dev_name, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("devPasswd", pu->dev_passwd, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("devIP", pu->dev_ip, node1);
		NMP_XML_DEF_VAL_CREATE_CHILD("devPort", pu->dev_port, node1, str);
		NMP_XML_DEF_VAL_CREATE_CHILD("devState", pu->dev_state, node1, str);

		i++;
	}

end:
	return 0;
}


NmpMsgInfo *
nmp_parse_modify_ams_pu(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpModifyAmsPu tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.puid, "puid", MAX_ID_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.domain, "domain", DOMAIN_ID_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.dev_name, "devName", AMS_DEV_NAME_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.dev_passwd, "devPasswd", AMS_DEV_PASSWD_LEN)
	NMP_XML_DEF_PARSE_GET_TEXT(tmp.dev_ip, "devIP", MAX_IP_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.dev_port, "devPort")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_modify_ams_pu_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	NMP_XML_DEF_CREATE_NMP_RESULT();
}

