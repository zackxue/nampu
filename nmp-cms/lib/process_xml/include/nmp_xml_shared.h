#ifndef __XML_SHARED_H__
#define __XML_SHARED_H__

#include "nmp_msg_struct.h"
#include "nmp_msg_ams.h"


/*
 * define use take care (zyt)
 *
 * parse function parameters must be: xmlDocPtr doc ,xmlNodePtr cur, char *cmd
 * create function parameters must be: xmlDocPtr doc, JpfMsgInfo *sys_msg
 */

#define JPF_XML_DEF_PARSE_BEGIN(tmp)	\
	JpfMsgInfo *sys_msg = NULL;	\
	xmlXPathObjectPtr app_result;	\
	char *xpath = "/message";	\
	int i;	\
	app_result =  jpf_get_node(doc, (const xmlChar *)xpath);	\
	if (!app_result)	\
		return NULL;	\
	xmlNodeSetPtr nodeset = app_result->nodesetval;	\
	memset(&tmp, 0, sizeof(tmp));	\
	for (i=0; i < nodeset->nodeNr; i++) {	\
		cur = nodeset->nodeTab[i];	\
		cur = cur->xmlChildrenNode;	\
		while (cur != NULL) {	\
			if (0 == 1);


#define JPF_XML_DEF_PARSE_GET_VALUE(dst_p, src_str)	\
	else if ((!xmlStrcmp(cur->name, (const xmlChar *)src_str)))	\
				jpf_deal_value(doc, cur, dst_p);


#define JPF_XML_DEF_PARSE_GET_TEXT(dst_p, src_str, dst_max_len)	\
	else if ((!xmlStrcmp(cur->name, (const xmlChar *)src_str)))	\
				jpf_deal_text(doc, cur, dst_p, dst_max_len);


#define JPF_XML_DEF_PARSE_END(tmp)	\
	cur = cur->next;	\
			while (cur && xmlIsBlankNode(cur))	\
				cur = cur->next;	\
		}	\
	}	\
	xmlXPathFreeObject(app_result);	\
	sys_msg = jpf_msginfo_new(cmd, &tmp, sizeof(tmp));	\
	return sys_msg;



#define JPF_XML_DEF_CREATE_BEGIN(tmp, str_tmp, root_node, end) do {	\
	tmp = jpf_get_msginfo_data(sys_msg);	\
	if (!tmp)	\
		return -E_NOMEM;	\
	root_node = xmlNewNode(NULL, BAD_CAST ROOTNODE);	\
	jpf_create_xml_type(doc, root_node, ATTRIBUTE_TYPE,sys_msg->msg_id);	\
	snprintf(str_tmp, INT_TO_CHAR_LEN,  "%d", RES_CODE(tmp));	\
	xmlNewChild(root_node, NULL, BAD_CAST "resultCode", BAD_CAST str_tmp);	\
	if (RES_CODE(tmp))	\
		goto end;	\
} while (0)


#define JPF_XML_DEF_VAL_CREATE_CHILD(dst_str, src_val, node_parent, str_tmp) do {	\
	snprintf(str_tmp, INT_TO_CHAR_LEN, "%d", src_val);	\
		xmlNewChild(node_parent, NULL, BAD_CAST dst_str, BAD_CAST str_tmp);	\
} while (0)


#define JPF_XML_DEF_STR_CREATE_CHILD(dst_str, src_p, node_parent) do {	\
	xmlNewChild(node_parent, NULL, BAD_CAST dst_str, BAD_CAST src_p);	\
} while (0)



#define JPF_XML_DEF_CREATE_BEGIN_SINGLE(tmp)	\
	ASSERT(sys_msg != NULL);	\
	xmlNodePtr root_node = NULL;	\
	char str[INT_TO_CHAR_LEN] = {0};	\
	JPF_XML_DEF_CREATE_BEGIN(tmp, str, root_node, end);


#define JPF_XML_DEF_VAL_CREATE_CHILD_SINGLE(dst_str, src_val) do {	\
	snprintf(str, INT_TO_CHAR_LEN, "%d", src_val);	\
		xmlNewChild(root_node, NULL, BAD_CAST dst_str, BAD_CAST str);	\
} while (0)


#define JPF_XML_DEF_STR_CREATE_CHILD_SINGLE(dst_str, src_p) do {	\
	xmlNewChild(root_node, NULL, BAD_CAST dst_str, BAD_CAST src_p);	\
} while (0)


#define JPF_XML_DEF_CREATE_END_SINGLE()	 \
	end:	\
	return 0;



#define JPF_XML_DEF_CREATE_JPF_RESULT() do {	\
	JpfResult *tmp = NULL;	\
	JPF_XML_DEF_CREATE_BEGIN_SINGLE(tmp);	\
	JPF_XML_DEF_CREATE_END_SINGLE();	\
} while (0)



void
jpf_get_weekday(xmlDocPtr doc, char *weekpath,
    JpfWeekday *weekdays,int *weekday_num);


void
jpf_set_weekday(xmlNodePtr node, JpfWeekday *weekdays, int weekday_num);

void
jpf_get_rectarea(xmlDocPtr doc, char *rectpath, JpfRectangle *detect_area, int *rect_num);

void
jpf_set_rectarea(xmlNodePtr node, JpfRectangle *detect_area, int rect_num);

void jpf_create_ptz_para(xmlNodePtr parent_node, JpfPtzPara para);

void jpf_parse_ptz_para(xmlDocPtr doc,xmlNodePtr cur, JpfPtzPara *para);


#endif

