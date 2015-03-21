/********************************************************************
 * nmp_ams_xml.c  - deal xml of ams, parse and create xml
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
#include "nmp_xml_fun.h"
#include "nmp_msg_struct.h"
#include "nmp_errno.h"
#include "nmp_xml_shared.h"
#include "nmp_memory.h"

#include "nmp_xml_shared.h"

/**
 * nmp_parse_ams_register: used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @cur:            input, a pointer to the tree's node
 * @cmd:            input, string, indicate command id
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *
nmp_parse_ams_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd)
{
    NmpAmsRegister req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"amsId")))
                nmp_deal_text(doc, cur, req_info.ams_id, AMS_ID_LEN );
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
 * nmp_create_ams_register_resp: used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 * @return:         succeed 0, else -1
 */
int
nmp_create_ams_register_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    xmlNodePtr root_node = NULL, node = NULL;
    NmpAmsRegisterRes *res_info;
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
    }

    return 0;
}


NmpMsgInfo *
nmp_parse_ams_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
    NmpAmsHeart req_info;
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
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"amsId")))
            {
                nmp_deal_text(doc, cur, req_info.ams_id, AMS_ID_LEN);
                xml_error("ams_heart.ams_id=%s\n",req_info.ams_id);
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
nmp_create_ams_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);
    xmlNodePtr root_node = NULL;
    NmpAmsHeartRes *tmp = NULL;
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
nmp_create_ams_device_info_change(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL;
	NmpAmsId *tmp = NULL;

	tmp = nmp_get_msginfo_data(sys_msg);
	if (!tmp)
		return -E_NOMEM;

	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);
	nmp_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);

	NMP_XML_DEF_STR_CREATE_CHILD("amsId", tmp->ams_id, root_node);

	return 0;
}


NmpMsgInfo *
nmp_parse_ams_get_device_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd)
{
	NmpAmsGetDeviceInfo tmp;

	NMP_XML_DEF_PARSE_BEGIN(tmp)

	NMP_XML_DEF_PARSE_GET_TEXT(tmp.ams_id, "amsId", AMS_ID_LEN)
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.req_num, "reqNum")
	NMP_XML_DEF_PARSE_GET_VALUE(&tmp.start_num, "startRow")

	NMP_XML_DEF_PARSE_END(tmp)
}


int
nmp_create_ams_get_device_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
	NmpAmsGetDeviceInfoRes *tmp = NULL;
	int back_count, i;
	char str[INT_TO_CHAR_LEN] = {0};

	NMP_XML_DEF_CREATE_BEGIN(tmp, str, root_node, end);

	NMP_XML_DEF_VAL_CREATE_CHILD("totalCount", tmp->total_count, root_node, str);

	back_count = tmp->back_count;
	snprintf(str, INT_TO_CHAR_LEN, "%d", back_count);
	node = xmlNewNode(NULL, BAD_CAST "deviceList");
	xmlNewProp(node, BAD_CAST "count", BAD_CAST str);
	xmlAddChild(root_node, node);

	i = 0;
	while (back_count--)
	{
		NmpAmsDeviceInfo *dev = &tmp->dev_info[i];
		node1 = xmlNewNode(NULL, BAD_CAST "device");
		xmlAddChild(node, node1);

		NMP_XML_DEF_STR_CREATE_CHILD("puid", dev->puid, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("domain", dev->domain, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("devName", dev->dev_name, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("devPasswd", dev->dev_passwd, node1);
		NMP_XML_DEF_STR_CREATE_CHILD("devIP", dev->dev_ip, node1);
		NMP_XML_DEF_VAL_CREATE_CHILD("devPort", dev->dev_port, node1, str);

		i++;
	}

end:
	return 0;
}

