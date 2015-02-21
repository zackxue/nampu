
#include "config.h"
#include "xml-tree.h"
#include "nmp_packet.h"
#include "nmp_xmlinfo.h"
#include "xml-api.h"



#define PU_ID_SPACING			2

extern int merge_time_string(char *buffer, size_t size, JTime *time);
extern int merge_time_part_string(char *buffer, size_t size, 
										JTime *time_start, JTime *time_end);
extern void init_xml_tree_attr(XmlTreeAttr **new_node_attr);
extern void init_xml_tree_node(XmlTreeNode **new_tree_node);
extern int xml_tree_delete(XmlTreeNode *xml_tree);
extern int xml_tree_merge_by_mxml(XmlTreeNode *xml_tree, 
								 		char *buffer, size_t size);


static __inline__ XmlTreeNode *
create_xml_head(XmlTreeNode *xml_tree, const char *message_type)
{
	XmlTreeNode *tree_node = NULL;
	XmlTreeAttr *node_attr = NULL;
	
	//xml head
	snprintf(xml_tree->element.name, sizeof(xml_tree->element.name),
											"%s", DEF_MXML_HEAD_STR);
	
	//xml root
	tree_node = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&tree_node);
	
	xml_tree->child = tree_node;
	tree_node->parent = xml_tree->child;
	
	snprintf(tree_node->element.name, sizeof(tree_node->element.name),
											"%s", MESSAGE_STR);
	
	//xml root attr
	node_attr = (XmlTreeAttr*)j_xml_alloc(sizeof(XmlTreeAttr));
	init_xml_tree_attr(&node_attr);
	
	tree_node->element.attr = node_attr;
	
	snprintf(node_attr->name, sizeof(node_attr->name),
							"%s", MESSAGE_TYPE_STR);
	snprintf(node_attr->value, sizeof(node_attr->value),
							"%s", message_type);

	return tree_node;
}

static __inline__ XmlTreeNode *
create_child(XmlTreeNode *tree_node, const char *element, 
	void *contact, ContactType type)
{
    int len, size;
	XmlTreeNode *child_node = NULL;
	
	if (NULL == tree_node)
	{
		printf("tree_node NULL.\n");
	}
	else if (NULL == element)
	{
		printf("element NULL.\n");
	}
	else
	{
		child_node = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
		init_xml_tree_node(&child_node);
	
		child_node->parent = tree_node;
		tree_node->child= child_node;
		tree_node = child_node;
		
		snprintf(tree_node->element.name, sizeof(tree_node->element.name),
				"%s", element);

		switch (type)
		{
			case CONTACT_INT:
				snprintf(tree_node->element.contact, 
						sizeof(tree_node->element.contact),
						"%d", (int)contact);
				break;
			case CONTACT_CHAR:
				snprintf(tree_node->element.contact, 
						sizeof(tree_node->element.contact),
						"%d", (int)contact);
				break;
			case CONTACT_STRING:
                len = strlen(contact);
                if (MAX_MXML_CONTACT_LEN > len)
                {
    				snprintf(tree_node->element.contact, 
    						sizeof(tree_node->element.contact),
    						"%s", (char*)contact);
                }
                else
                {
                    size = sizeof(int)+len+1;/*head+body+end*/
    			    tree_node->element.extend_contact = j_xml_alloc(size);
                    tree_node->element.extend_contact[size-1] = '\0';
                    memcpy(tree_node->element.extend_contact, &len, sizeof(int));
                    tree_node->element.extend_contact += sizeof(int);
    				memcpy(tree_node->element.extend_contact, contact, len);
                }
				break;
			default:
				break;
		}
		
		return tree_node;
	}

	return NULL;
}

static __inline__ void 
create_brother(XmlTreeNode **tree_node, const char *element, 
	void *contact, ContactType type)
{
    int len, size;
	XmlTreeNode *brother_node = NULL;
	
	if (NULL == *tree_node)
	{
		printf("*tree_node NULL.\n");
	}
	else if (NULL == element)
	{
		printf("element NULL.\n");
	}
	else
	{
		brother_node = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
		init_xml_tree_node(&brother_node);
		
		brother_node->prev = *tree_node;
		(*tree_node)->next = brother_node;
		*tree_node = brother_node;
		
		snprintf(brother_node->element.name, sizeof(brother_node->element.name),
				"%s", element);

		switch (type)
		{
			case CONTACT_INT:
				snprintf(brother_node->element.contact, 
						sizeof(brother_node->element.contact),
						"%d", (int)contact);
				break;
			case CONTACT_CHAR:
				snprintf(brother_node->element.contact, 
						sizeof(brother_node->element.contact),
						"%d", (int)contact);
				break;
			case CONTACT_STRING:
                len = strlen(contact);
                if (MAX_MXML_CONTACT_LEN > len)
                {
    				snprintf(brother_node->element.contact, 
    						sizeof(brother_node->element.contact),
    						"%s", (char*)contact);
                }
                else
                {
                    size = sizeof(int)+len+1;/*head+body+end*/
    			    brother_node->element.extend_contact = j_xml_alloc(size);
                    brother_node->element.extend_contact[size-1] = '\0';
                    memcpy(brother_node->element.extend_contact, &len, sizeof(int));
                    brother_node->element.extend_contact += sizeof(int);
                    memcpy(brother_node->element.extend_contact, contact, len);
                }
				break;
			default:
				break;
		}
	}
}

static __inline__ void 
create_attr(XmlTreeNode **tree_node, const char *attr_name, 
	void *contact, ContactType type)
{
	XmlTreeAttr *node_attr = NULL;
	
	if (NULL == *tree_node)
	{
		printf("*tree_node NULL.\n");
	}
	else if (NULL == attr_name)
	{
		printf("attr_name NULL.\n");
	}
	else
	{
		node_attr = (XmlTreeAttr*)j_xml_alloc(sizeof(XmlTreeAttr));
		init_xml_tree_attr(&node_attr);
		
		(*tree_node)->element.attr = node_attr;
		
		snprintf(node_attr->name, sizeof(node_attr->name),
								"%s", attr_name);

		switch (type)
		{
			case CONTACT_INT: 
				snprintf(node_attr->value, sizeof(node_attr->value),
										"%d", (int)contact);
				break;
			case CONTACT_CHAR: 
				snprintf(node_attr->value, sizeof(node_attr->value),
										"%d", (int)contact);
				break;
			case CONTACT_STRING:
				snprintf(node_attr->value, sizeof(node_attr->value),
										"%s", (char*)contact);
				break;
			default:
				break;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////
static __inline__ int judge_pu_or_gu(const char *pu_or_gu)
{
	int index = 0;
	char *p, *pvalue;
	
	p = pvalue = strchr(pu_or_gu, '-');
	while (p)
	{
		pvalue = p+1;
		p = strchr(pvalue, '-');
		index++;
	}
	
	if (index > PU_ID_SPACING)
		return 1;
	
	return 0;
}

static int merge_request_action_xml(Request *request, 
			char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == request)
	{
		printf("request NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
							 (void*)request->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)request->domain_id, CONTACT_STRING);
		
		if (!judge_pu_or_gu(request->pu_or_gu_id))
			create_brother(&tree_node, PU_ID_STR, 
				(void*)request->pu_or_gu_id, CONTACT_STRING);
		else
			create_brother(&tree_node, GU_ID_STR, 
				(void*)request->pu_or_gu_id, CONTACT_STRING);

		if (!strcmp(GET_SERIAL_PARAMETER_CMD, command))
			create_brother(&tree_node, SERIAL_NO_STR, 
				(void*)request->reserve, CONTACT_INT);
		else if (!strcmp(GET_SERIAL_PARAMETER_CMD, command))
			create_brother(&tree_node, ALARM_TYPE_STR, 
				(void*)request->reserve, CONTACT_INT);
		else if (!strcmp(GET_CRUISE_WAY_REQUEST_CMD, command))
			create_brother(&tree_node, CRUISE_NO_STR, 
				(void*)request->reserve, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

static int merge_result_action_xml(Result *result, 
			char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == result)
	{
		printf("result NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
							 (void*)result->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)result->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)result->domain_id, CONTACT_STRING);
		
		if (!judge_pu_or_gu(result->pu_or_gu_id))
			create_brother(&tree_node, PU_ID_STR, 
				(void*)result->pu_or_gu_id, CONTACT_STRING);
		else
			create_brother(&tree_node, GU_ID_STR, 
				(void*)result->pu_or_gu_id, CONTACT_STRING);
		
		if (!strcmp(ADD_CRUISE_WAY_RESULT_CMD, command))
			create_brother(&tree_node, CRUISE_NO_STR, 
				(void*)result->reserve, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

//##########################################################################
int merge_get_css_request_xml(void *pvalue, const char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	PuGetCssPacket *get_css = (PuGetCssPacket*)pvalue;
	
	if (NULL == get_css)
	{
		printf("get_css NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, PU_GET_CSS_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, DEVICE_CODE_STR, 
        (void*)get_css->dev_code, CONTACT_STRING)))
	{
		create_brother(&tree_node, PU_VERSION_STR, 
						(void*)get_css->software_ver, CONTACT_STRING);
	}
	else
	{
		printf("create_child(DEVICE_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, (char*)buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_css_response_xml(void *pvalue, const char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	PuGetCssPacket *get_css = (PuGetCssPacket*)pvalue;
	
	if (NULL == get_css)
	{
		printf("get_css NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, PU_GET_CSS_RESPONSE_CMD);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
        (void*)get_css->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, CSS_IP_STR, 
						(void*)get_css->css_ip, CONTACT_STRING);
		create_brother(&tree_node, CSS_PORT_STR, 
						(void*)get_css->css_port, CONTACT_INT);
	}
	else
	{
		printf("create_child(DEVICE_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, (char*)buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_pu_register_css_xml(void *pvalue, const char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	PuRegisterCssPacket *reg_css = (PuRegisterCssPacket*)pvalue;
	
	if (NULL == reg_css)
	{
		printf("reg_css NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, REGISTER_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, DEVICE_CODE_STR, 
        (void*)reg_css->dev_code, CONTACT_STRING)))
	{
		create_brother(&tree_node, DEVICE_IP_STR, 
						(void*)reg_css->dev_ip, CONTACT_STRING);
		create_brother(&tree_node, CSS_IP_STR, 
						(void*)reg_css->css_ip, CONTACT_STRING);
		create_brother(&tree_node, PU_VERSION_STR, 
						(void*)reg_css->software_ver, CONTACT_STRING);
	}
	else
	{
		printf("create_child(DEVICE_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, (char*)buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_register_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	RegisterRequestPacket *reg_request = (RegisterRequestPacket*)pvalue;
	
	if (NULL == reg_request)
	{
		printf("reg_request NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, REGISTER_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
										 (void*)reg_request->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, PU_TYPE_STR, 
						(void*)reg_request->pu_type, CONTACT_INT);
		create_brother(&tree_node, DEVICE_IP_STR, 
						(void*)reg_request->dev_ip, CONTACT_STRING);
		create_brother(&tree_node, CMS_IP_STR, 
						(void*)reg_request->cms_ip, CONTACT_STRING);
	}
	else
	{
		printf("create_child(PU_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

//##########################################################################
int merge_register_response_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	RegisterResponsePacket *reg_response = (RegisterResponsePacket*)pvalue;
	
	if (NULL == reg_response)
	{
		printf("reg_response NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, REGISTER_RESPONSE_CMD);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
										 (void*)reg_response->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, RESULT_CODE_STR, 
						(void*)reg_response->result.code, CONTACT_INT);
		create_brother(&tree_node, KEEP_ALIVE_STR, 
						(void*)reg_response->keep_alive, CONTACT_INT);
		create_brother(&tree_node, MDS_IP_STR, 
						(void*)reg_response->mds_ip, CONTACT_STRING);
		create_brother(&tree_node, MDS_PORT_STR, 
						(void*)reg_response->mds_port, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_heart_beat_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	HeartBeatRequestPacket *heart_beat = (HeartBeatRequestPacket*)pvalue;
	
	if (NULL == heart_beat)
	{
		printf("heart_beat NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, HEART_BEAT_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
										 (void*)heart_beat->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DEVICE_IP_STR, 
						(void*)heart_beat->dev_ip, CONTACT_STRING);
	}
	else
	{
		printf("create_child(PU_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_heart_beat_response_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	char time_buffer[J_SDK_MAX_TIME_LEN];
	
	HeartBeatResponsePacket *heart_beat_resp = (HeartBeatResponsePacket*)pvalue;
	
	if (NULL == heart_beat_resp)
	{
		printf("heart_beat_resp NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, HEART_BEAT_RESPONSE_CMD);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)heart_beat_resp->result.code, 
										 CONTACT_INT)))
	{
		memset(time_buffer, 0, sizeof(time_buffer));
		if (0 == merge_time_string(time_buffer, sizeof(time_buffer), 
									&(heart_beat_resp->server_time)))
		{
			create_brother(&tree_node, SERVER_TIME_STR, (void*)time_buffer, CONTACT_STRING);
		}
		else
		{
			printf("merge_time_string(SERVER_TIME_STR) failed.\n");
			xml_tree_delete(xml_tree);
			return -1;
		}
	}	
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

//##########################################################################
int merge_get_mds_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	MdsInfoPacket *mds_info = (MdsInfoPacket*)pvalue;
	
	if (NULL == mds_info)
	{
		printf("mds_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, GET_MDS_INFO_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
								(void*)mds_info->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, CMS_IP_STR, 
			(void*)mds_info->cms_ip, CONTACT_STRING);
	}
	else
	{
		printf("create_child(PU_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_mds_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	MdsInfoPacket *mds_info = (MdsInfoPacket*)pvalue;
	
	if (NULL == mds_info)
	{
		printf("mds_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, GET_MDS_INFO_RESPONSE_CMD);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)mds_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, PU_ID_STR, 
			(void*)mds_info->pu_id, CONTACT_STRING);
		create_brother(&tree_node, CMS_IP_STR, 
			(void*)mds_info->cms_ip, CONTACT_STRING);
		create_brother(&tree_node, IP_STR, 
			(void*)mds_info->mds_ip, CONTACT_STRING);
		create_brother(&tree_node, PORT_STR, 
			(void*)mds_info->mds_port, CONTACT_INT);
	}	
	else
	{
		printf("create_child(PU_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_change_dispatch_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	ChangeDispatchPacket *change_disp = (ChangeDispatchPacket*)pvalue;
	
	if (NULL == change_disp)
	{
		printf("change_disp NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, CHANGE_DISPATCH_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
										 (void*)change_disp->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)change_disp->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
						(void*)change_disp->pu_id, CONTACT_STRING);
		create_brother(&tree_node, MDS_IP_STR, 
						(void*)change_disp->mds_ip, CONTACT_STRING);
		create_brother(&tree_node, MDS_PORT_STR, 
						(void*)change_disp->mds_port, CONTACT_INT);
	}
	else
	{
		printf("create_child(PU_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_change_dispatch_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, CHANGE_DISPATCH_RESULT_CMD);
}

//##########################################################################
int merge_get_device_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_DEVICE_INFO_CMD);
}

int merge_device_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	DeviceInfoPacket *dev_info = (DeviceInfoPacket*)pvalue;
	
	if (NULL == dev_info)
	{
		printf("dev_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, DEVICE_INFO_RESPONSE_CMD);
	
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
										 (void*)dev_info->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)dev_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
						(void*)dev_info->pu_id, CONTACT_STRING);
		create_brother(&tree_node, RESULT_CODE_STR, 
						(void*)dev_info->result.code, CONTACT_INT);
		
		if (0 == dev_info->result.code)
		{
			create_brother(&tree_node, PU_TYPE_STR, 
							(void*)dev_info->pu_type, CONTACT_INT);
			create_brother(&tree_node, PU_SUB_TYPE_STR, 
							(void*)dev_info->sub_type, CONTACT_INT);
			create_brother(&tree_node, MANU_INFO_STR, 
							(void*)dev_info->manu_info, CONTACT_STRING);
			create_brother(&tree_node, RELEASE_DATE_STR, 
							(void*)dev_info->release_date, CONTACT_STRING);
			create_brother(&tree_node, DEV_VERSION_STR, 
							(void*)dev_info->dev_version, CONTACT_STRING);
			create_brother(&tree_node, HW_VERSION_STR, 
							(void*)dev_info->hw_version, CONTACT_STRING);
			create_brother(&tree_node, DI_NUM_STR, 
							(void*)dev_info->di_num, CONTACT_INT);
			create_brother(&tree_node, DO_NUM_STR, 
							(void*)dev_info->do_num, CONTACT_INT);
			create_brother(&tree_node, CHANNEL_NUM_STR, 
							(void*)dev_info->channel_num, CONTACT_INT);
			create_brother(&tree_node, RS485_NUM_STR, 
							(void*)dev_info->rs485_num, CONTACT_INT);
			create_brother(&tree_node, RS232_NUM_STR, 
							(void*)dev_info->rs232_num, CONTACT_INT);
		}
	}
	else
	{
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

//##########################################################################
static int merge_device_ntp_info_xml(DeviceNTPInfoPacket *ntp_info, 
												char *buffer, size_t size, 
												const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == ntp_info)
	{
		printf("ntp_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)ntp_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, (void*)ntp_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, (void*)ntp_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, (void*)ntp_info->pu_id, CONTACT_STRING);
		create_brother(&tree_node, NTP_SERVER_IP_STR, (void*)ntp_info->ntp_server_ip, CONTACT_STRING);
		create_brother(&tree_node, TIME_ZONE_STR, (void*)ntp_info->time_zone, CONTACT_INT);
		create_brother(&tree_node, TIME_INTERVAL_STR, (void*)ntp_info->time_interval, CONTACT_INT);
		create_brother(&tree_node, NTP_ENABLE_STR, (void*)ntp_info->ntp_enable, CONTACT_INT);
		create_brother(&tree_node, DST_ENABLE_STR, (void*)ntp_info->dst_enable, CONTACT_INT);
		create_brother(&tree_node, RESERVE_STR, (void*)ntp_info->reserve, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_device_ntp_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_DEVICE_NTP_INFO_CMD);
}

int merge_device_ntp_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	DeviceNTPInfoPacket *ntp_info = (DeviceNTPInfoPacket*)pvalue;
	
	return merge_device_ntp_info_xml(ntp_info, buffer, size, DEVICE_NTP_INFO_RESPONSE_CMD);
}

int merge_set_device_ntp_info_xml(void *pvalue, char *buffer, size_t size)
{
	DeviceNTPInfoPacket *ntp_info = (DeviceNTPInfoPacket*)pvalue;
	
	return merge_device_ntp_info_xml(ntp_info, buffer, size, SET_DEVICE_NTP_INFO_CMD);
}

int merge_device_ntp_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, DEVICE_NTP_INFO_RESULT_CMD);
}

//##########################################################################
int merge_device_time_xml(DeviceTimePacket *dev_time, 
	char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	char time_buffer[J_SDK_MAX_TIME_LEN];
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
										 (void*)dev_time->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, (void*)dev_time->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, (void*)dev_time->pu_id, CONTACT_STRING);
		create_brother(&tree_node, RESULT_CODE_STR, (void*)dev_time->result.code, CONTACT_INT);
		
		memset(time_buffer, 0, sizeof(time_buffer));
		if (0 == merge_time_string(time_buffer, sizeof(time_buffer), &(dev_time->time)))
			create_brother(&tree_node, TIME_STR, (void*)time_buffer, CONTACT_STRING);
		else
		{
			printf("merge_time_string(TIME_STR) failed.\n");
			xml_tree_delete(xml_tree);
			return -1;
		}
		create_brother(&tree_node, TIME_ZONE_STR, (void*)dev_time->zone, CONTACT_INT);
		create_brother(&tree_node, SYNC_ENABLE_STR, (void*)dev_time->sync_enable, CONTACT_INT);
		create_brother(&tree_node, SET_FLAG_STR, (void*)dev_time->set_flag, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_device_time_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_DEVICE_TIME_CMD);
}
int merge_device_time_response_xml(void *pvalue, char *buffer, size_t size)
{
	DeviceTimePacket *dev_time = (DeviceTimePacket*)pvalue;
	
	return merge_device_time_xml(dev_time, buffer, size, DEVICE_TIME_RESPONSE_CMD);
}
int merge_set_device_time_xml(void *pvalue, char *buffer, size_t size)
{
	DeviceTimePacket *dev_time = (DeviceTimePacket*)pvalue;
	
	return merge_device_time_xml(dev_time, buffer, size, SET_DEVICE_TIME_CMD);
}
int merge_device_time_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, DEVICE_TIME_RESULT_CMD);
}

//##########################################################################
static int merge_platform_info_xml(PlatformInfoPacket *pltf_info, 
												char *buffer, size_t size, 
												const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pltf_info)
	{
		printf("pltf_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)pltf_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, (void*)pltf_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, (void*)pltf_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, (void*)pltf_info->pu_id, CONTACT_STRING);
		create_brother(&tree_node, CMS_IP_STR, (void*)pltf_info->cms_ip, CONTACT_STRING);
		create_brother(&tree_node, CMS_PORT_STR, (void*)pltf_info->cms_port, CONTACT_INT);
		create_brother(&tree_node, MDS_IP_STR, (void*)pltf_info->mds_ip, CONTACT_STRING);
		create_brother(&tree_node, MDS_PORT_STR, (void*)pltf_info->mds_port, CONTACT_INT);
		create_brother(&tree_node, PROTOCOL_STR, (void*)pltf_info->protocol, CONTACT_INT);
		create_brother(&tree_node, IS_CON_CMS_STR, (void*)pltf_info->is_con_cms, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_platform_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_PLATFORM_INFO_CMD);
}

int merge_platform_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	PlatformInfoPacket *pltf_info = (PlatformInfoPacket*)pvalue;
	
	return merge_platform_info_xml(pltf_info, buffer, size, PLATFORM_INFO_RESPONSE_CMD);
}

int merge_set_platform_info_xml(void *pvalue, char *buffer, size_t size)
{
	PlatformInfoPacket *pltf_info = (PlatformInfoPacket*)pvalue;
	
	return merge_platform_info_xml(pltf_info, buffer, size, SET_PLATFORM_INFO_CMD);
}

int merge_platform_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, PLATFORM_INFO_RESULT_CMD);
}

//##########################################################################
static int merge_network_info_xml(NetworkInfoPacket *net_info, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	int index = 0;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	if (NULL == net_info)
	{
		printf("net_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)net_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, (void*)net_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, (void*)net_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, (void*)net_info->pu_id, CONTACT_STRING);

		for (; index<J_SDK_MAX_NETWORK_TYPE; index++)
		{
			create_brother(&tree_node, NETWORK_STR, NULL, CONTACT_NULL);
			switch (index)
			{
				case J_SDK_ETH0:
					create_attr(&tree_node, NETWORK_TYPE_STR, 
						(void*)J_SDK_ETH0, CONTACT_INT);
					break;
				case J_SDK_WIFI:
					create_attr(&tree_node, NETWORK_TYPE_STR, 
						(void*)J_SDK_WIFI, CONTACT_INT);
					break;
				case J_SDK_3G:
					create_attr(&tree_node, NETWORK_TYPE_STR, 
						(void*)J_SDK_3G, CONTACT_INT);
					break;
				default:
					break;
			}
			tree_child = create_child(tree_node, DHCP_ENABLE_STR, 
						(void*)net_info->network[index].dhcp_enable, CONTACT_INT);
			if (tree_child)
			{
				create_brother(&tree_child, MAC_STR, 
								(void*)net_info->network[index].mac, CONTACT_STRING);
				create_brother(&tree_child, IP_STR, 
								(void*)net_info->network[index].ip, CONTACT_STRING);
				create_brother(&tree_child, NETMASK_STR, 
								(void*)net_info->network[index].netmask, CONTACT_STRING);
				create_brother(&tree_child, GATEWAY_STR, 
								(void*)net_info->network[index].gateway, CONTACT_STRING);
			}
		}
		
		create_brother(&tree_node, MAIN_DNS_STR, 
						(void*)net_info->main_dns, CONTACT_STRING);
		create_brother(&tree_node, BACKUP_DNS_STR, 
						(void*)net_info->backup_dns, CONTACT_STRING);
		create_brother(&tree_node, AUTO_DNS_ENABLE_STR, 
						(void*)net_info->auto_dns_enable, CONTACT_INT);
		create_brother(&tree_node, CMD_PORT_STR, 
						(void*)net_info->cmd_port, CONTACT_INT);
		create_brother(&tree_node, DATA_PORT_STR, 
						(void*)net_info->data_port, CONTACT_INT);
		create_brother(&tree_node, WEB_PORT_STR, 
						(void*)net_info->web_port, CONTACT_INT);
        create_brother(&tree_node, TALK_PORT_STR, 
						(void*)net_info->talk_port, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_network_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_NETWORK_INFO_CMD);
}

int merge_network_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	NetworkInfoPacket *net_info = (NetworkInfoPacket*)pvalue;
	
	return merge_network_info_xml(net_info, buffer, size, NETWORK_INFO_RESPONSE_CMD);
}
int merge_set_network_info_xml(void *pvalue, char *buffer, size_t size)
{
	NetworkInfoPacket *net_info = (NetworkInfoPacket*)pvalue;
	
	return merge_network_info_xml(net_info, buffer, size, SET_NETWORK_INFO_CMD);
}

int merge_network_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, NETWORK_INFO_RESULT_CMD);
}

//##########################################################################
static int merge_pppoe_info_xml(PPPOEInfoPacket *pppoe_info, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pppoe_info)
	{
		printf("net_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)pppoe_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)pppoe_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)pppoe_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
						(void*)pppoe_info->pu_id, CONTACT_STRING);
		create_brother(&tree_node, PPPOE_INTERFACE_STR, 
						(void*)pppoe_info->type, CONTACT_INT);
		create_brother(&tree_node, PPPOE_ENABLE_STR, 
						(void*)pppoe_info->enable, CONTACT_INT);
		create_brother(&tree_node, PPPOE_IP_STR, 
						(void*)pppoe_info->ip, CONTACT_STRING);
		create_brother(&tree_node, PPPOE_ACCOUNT_STR, 
						(void*)pppoe_info->account, CONTACT_STRING);
		create_brother(&tree_node, PPPOE_PASSED_STR, 
						(void*)pppoe_info->passwd, CONTACT_STRING);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_pppoe_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_PPPOE_INFO_CMD);
}

int merge_pppoe_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	PPPOEInfoPacket *pppoe_info = (PPPOEInfoPacket*)pvalue;
	
	return merge_pppoe_info_xml(pppoe_info, buffer, size, PPPOE_INFO_RESPONSE_CMD);
}

int merge_set_pppoe_info_xml(void *pvalue, char *buffer, size_t size)
{
	PPPOEInfoPacket *pppoe_info = (PPPOEInfoPacket*)pvalue;
	
	return merge_pppoe_info_xml(pppoe_info, buffer, size, SET_PPPOE_INFO_CMD);
}

int merge_pppoe_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, PPPOE_INFO_RESULT_CMD);
}

//##########################################################################
static int merge_encode_parameter_xml(EncodeParameterPacket *encode_para, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == encode_para)
	{
		printf("encode_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
										 (void*)encode_para->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)encode_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
			(void*)encode_para->gu_id, CONTACT_STRING);
		create_brother(&tree_node, RESULT_CODE_STR, 
			(void*)encode_para->result.code, CONTACT_INT);
		create_brother(&tree_node, FRAME_RATE_STR, 
			(void*)encode_para->frame_rate, CONTACT_INT);
		create_brother(&tree_node, I_FRAME_INTERVAL_STR, 
			(void*)encode_para->i_frame_interval, CONTACT_INT);
		create_brother(&tree_node, VIDEO_TYPE_STR, 
			(void*)encode_para->video_type, CONTACT_INT);
		create_brother(&tree_node, AUDIO_TYPE_STR, 
			(void*)encode_para->audio_type, CONTACT_INT);
		create_brother(&tree_node, AUDIO_INPUT_MODE_STR, 
			(void*)encode_para->au_in_mode, CONTACT_INT);
		create_brother(&tree_node, AUDIO_ENABLE_STR, 
			(void*)encode_para->audio_enble, CONTACT_INT);
		create_brother(&tree_node, RESOLUTION_STR, 
			(void*)encode_para->resolution, CONTACT_INT);
		create_brother(&tree_node, QPVALUE_STR, 
			(void*)encode_para->qp_value, CONTACT_INT);
		create_brother(&tree_node, CODE_RATE_STR, 
			(void*)encode_para->code_rate, CONTACT_INT);
		create_brother(&tree_node, FRAME_PRIORITY_STR, 
			(void*)encode_para->frame_priority, CONTACT_INT);
		create_brother(&tree_node, VIDEO_FORMAT_STR, 
			(void*)encode_para->format, CONTACT_INT);
		create_brother(&tree_node, BIT_RATE_TYPE_STR, 
			(void*)encode_para->bit_rate, CONTACT_INT);
		create_brother(&tree_node, ENCODE_LEVEL_STR, 
			(void*)encode_para->encode_level, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_encode_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, 
								  GET_ENCODE_PARAMETER_CMD);
}

int merge_encode_parameter_response_xml(void *pvalue, 
														char *buffer, size_t size)
{
	EncodeParameterPacket *encode_para = (EncodeParameterPacket*)pvalue;
	

	return merge_encode_parameter_xml(encode_para, buffer, size, 
										ENCODE_PARAMETER_RESPONSE_CMD);
}

int merge_set_encode_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	EncodeParameterPacket *encode_para = (EncodeParameterPacket*)pvalue;
	
	return merge_encode_parameter_xml(encode_para, buffer, size, 
										SET_ENCODE_PARAMETER_CMD);
}

int merge_encode_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, 
								 ENCODE_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_display_parameter_xml(DisplayParameterPacket *display_para, 
															char *buffer, size_t size,
															const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == display_para)
	{
		printf("display_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)display_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)display_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)display_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)display_para->gu_id, CONTACT_STRING);
		create_brother(&tree_node, CONTRAST_STR, 
						(void*)display_para->contrast, CONTACT_INT);
		create_brother(&tree_node, BRIGHT_STR, 
						(void*)display_para->bright, CONTACT_INT);
		create_brother(&tree_node, HUE_STR, 
						(void*)display_para->hue, CONTACT_INT);
		create_brother(&tree_node, SATURATION_STR, 
						(void*)display_para->saturation, CONTACT_INT);
		create_brother(&tree_node, SHARPNESS_STR, 
						(void*)display_para->sharpness, CONTACT_INT);
	}	
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_display_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_DISPLAY_PARAMETER_CMD);
}

int merge_display_parameter_response_xml(void *pvalue, char *buffer, size_t size)
{
	DisplayParameterPacket *display_para = (DisplayParameterPacket*)pvalue;
	
	return merge_display_parameter_xml(display_para, buffer, size, 
										DISPLAY_PARAMETER_RESPONSE_CMD);
}

int merge_set_display_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	DisplayParameterPacket *display_para = (DisplayParameterPacket*)pvalue;
	
	return merge_display_parameter_xml(display_para, buffer, size, 
										SET_DISPLAY_PARAMETER_CMD);
}

int merge_display_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, DISPLAY_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_record_parameter_xml(RecordParameterPacket *record_para, 
														char *buffer, size_t size,
														const char *command)
{
	int ret = -1;
	int flag0, flag1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	
	char time_buffer[J_SDK_MAX_TIME_LEN];
	memset(time_buffer, 0, sizeof(time_buffer));
	
	if (NULL == record_para)
	{
		printf("record_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)record_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)record_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)record_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)record_para->gu_id, CONTACT_STRING);
		create_brother(&tree_node, PRE_RECORD_STR, 
						(void*)record_para->pre_record, CONTACT_INT);
		create_brother(&tree_node, AUTO_COVER_STR, 
						(void*)record_para->auto_cover, CONTACT_INT);

		create_brother(&tree_node, WEEK_DAY_S_STR, NULL, CONTACT_NULL);
		for (flag0=0; flag0<record_para->week.count; flag0++)
		{
			if (!flag0)
				tree_child = create_child(tree_node, WEEK_DAY_STR, 
											NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, WEEK_DAY_STR, 
									NULL, CONTACT_NULL);
			
			create_attr(&tree_child, WEEK_DAY_ID_STR, 
				(void*)record_para->week.days[flag0].day_id, CONTACT_INT);
			
			if (tree_child)
			{
				tree_son = create_child(tree_child, ALL_DAY_ENABLE_STR, 
							(void*)record_para->week.days[flag0].all_day_enable, CONTACT_INT);
				create_brother(&tree_son, TIME_SEG_S_STR, 
										NULL, CONTACT_NULL);
				
				for (flag1=0; flag1<record_para->week.days[flag0].count; flag1++)
				{
					if (0 == merge_time_part_string(time_buffer, sizeof(time_buffer), 
								&(record_para->week.days[flag0].seg[flag1].time_start), 
								&(record_para->week.days[flag0].seg[flag1].time_end)))
					{
						if (!flag1)
							tree_grandson = create_child(tree_son, TIME_SEG_STR, 
											(void*)time_buffer, CONTACT_STRING);
						else
							create_brother(&tree_grandson, TIME_SEG_STR, 
									(void*)time_buffer, CONTACT_STRING);
						
						create_attr(&tree_grandson, TIME_SEG_ENABLE_STR, 
							(void*)record_para->week.days[flag0].seg[flag1].enable, CONTACT_INT);
					}
				}
			}
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_record_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_RECORD_PARAMETER_CMD);
}

int merge_record_parameter_response_xml(void *pvalue, 
														char *buffer, size_t size)
{
	RecordParameterPacket *record_para = (RecordParameterPacket*)pvalue;
	
	return merge_record_parameter_xml(record_para, buffer, size, 
										RECORD_PARAMETER_RESPONSE_CMD);
}

int merge_set_record_parameter_xml(void *pvalue, 
												char *buffer, size_t size)
{
	RecordParameterPacket *record_para = (RecordParameterPacket*)pvalue;
	
	return merge_record_parameter_xml(record_para, buffer, size, 
										SET_RECORD_PARAMETER_CMD);
}

int merge_record_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, RECORD_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_hide_parameter_xml(HideParameterPacket *hide_para, 
													char *buffer, size_t size,
													const char *command)
{
	int ret = -1;
	int index;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	if (NULL == hide_para)
	{
		printf("hide_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)hide_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 			
						(void*)hide_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)hide_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)hide_para->gu_id, CONTACT_STRING);
		
		create_brother(&tree_node, HIDE_AREA_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, HIDE_NUM_STR, 
			(void*)hide_para->hide_area.count, CONTACT_INT);
		for (index=0; index<hide_para->hide_area.count; index++)
		{
			if (!index)
				tree_child = create_child(tree_node, RECTANGLE_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, RECTANGLE_STR, NULL, CONTACT_NULL);

			if (tree_child)
			{
				tree_son = create_child(tree_child, RECT_LEFT_STR, 
							(void*)((int)hide_para->hide_area.rect[index].left), CONTACT_INT);
				if (tree_son)
				{
					create_brother(&tree_son, RECT_TOP_STR, 
						(void*)((int)hide_para->hide_area.rect[index].top), CONTACT_INT);
					create_brother(&tree_son, RECT_RIGHT_STR, 
						(void*)((int)hide_para->hide_area.rect[index].right), CONTACT_INT);
					create_brother(&tree_son, RECT_BOTTOM_STR, 
						(void*)((int)hide_para->hide_area.rect[index].bottom), CONTACT_INT);
				}
			}
		}
		create_brother(&tree_node, HIDE_ENABLE_STR, 
						(void*)hide_para->hide_enable, CONTACT_INT);
		create_brother(&tree_node, HIDE_COLOR_STR, 
						(void*)hide_para->hide_color, CONTACT_INT);
		create_brother(&tree_node, MAX_WIDTH_STR, 
						(void*)hide_para->max_width, CONTACT_INT);
		create_brother(&tree_node, MAX_HEIGHT_STR, 
						(void*)hide_para->max_height, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_hide_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_HIDE_PARAMETER_CMD);
}

int merge_hide_parameter_response_xml(void *pvalue, 
													char *buffer, size_t size)
{
	HideParameterPacket *hide_para = (HideParameterPacket*)pvalue;
	
	return merge_hide_parameter_xml(hide_para, buffer, size, 
									HIDE_PARAMETER_RESPONSE_CMD);
}

int merge_set_hide_parameter_xml(void *pvalue, 
											char *buffer, size_t size)
{
	HideParameterPacket *hide_para = (HideParameterPacket*)pvalue;
	
	return merge_hide_parameter_xml(hide_para, buffer, size, 
									SET_HIDE_PARAMETER_CMD);
}

int merge_hide_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, HIDE_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_serial_parameter_xml(SerialParameterPacket *serial_para, 
														char *buffer, size_t size,
														const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == serial_para)
	{
		printf("serial_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)serial_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)serial_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)serial_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
						(void*)serial_para->pu_id, CONTACT_STRING);
		create_brother(&tree_node, SERIAL_NO_STR, 
						(void*)serial_para->serial_no, CONTACT_INT);
		create_brother(&tree_node, BAUD_RATE_STR, 
						(void*)serial_para->baud_rate, CONTACT_INT);
		create_brother(&tree_node, DATA_BIT_STR, 
						(void*)serial_para->data_bit, CONTACT_INT);
		create_brother(&tree_node, STOP_BIT_STR, 
						(void*)serial_para->stop_bit, CONTACT_INT);
		create_brother(&tree_node, VERIFY_STR, 
						(void*)serial_para->verify, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_serial_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_SERIAL_PARAMETER_CMD);
}

int merge_serial_parameter_response_xml(void *pvalue, 
													char *buffer, size_t size)
{
	SerialParameterPacket *serial_para = (SerialParameterPacket*)pvalue;
	
	return merge_serial_parameter_xml(serial_para, buffer, size, 
										SERIAL_PARAMETER_RESPONSE_CMD);
}

int merge_set_serial_parameter_xml(void *pvalue, 
											char *buffer, size_t size)
{
	SerialParameterPacket *serial_para = (SerialParameterPacket*)pvalue;
	
	return merge_serial_parameter_xml(serial_para, buffer, size, 
										SET_SERIAL_PARAMETER_CMD);
}

int merge_serial_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, 
								 SERIAL_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_osd_parameter_xml(OSDParameterPacket *osd_para, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == osd_para)
	{
		printf("osd_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)osd_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)osd_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)osd_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)osd_para->gu_id, CONTACT_STRING);
		create_brother(&tree_node, DISPLAY_TIME_ENABLE_STR, 
						(void*)osd_para->time_enable, CONTACT_INT);
		create_brother(&tree_node, TIME_DISPLAY_X_STR, 
						(void*)osd_para->time_display_x, CONTACT_INT);
		create_brother(&tree_node, TIME_DISPLAY_Y_STR, 
						(void*)osd_para->time_display_y, CONTACT_INT);
		create_brother(&tree_node, TIME_DISPLAY_COLOR_STR, 
						(void*)osd_para->time_display_color, CONTACT_INT);
		create_brother(&tree_node, DISPLAY_TEXT_ENABLE_STR, 
						(void*)osd_para->text_enable, CONTACT_INT);
		create_brother(&tree_node, TEXT_DISPLAY_X_STR, 
						(void*)osd_para->text_display_x, CONTACT_INT);
		create_brother(&tree_node, TEXT_DISPLAY_Y_STR, 
						(void*)osd_para->text_display_y, CONTACT_INT);
		create_brother(&tree_node, TEXT_DISPLAY_COLOR_STR, 
						(void*)osd_para->text_display_color, CONTACT_INT);
		create_brother(&tree_node, TEXT_DISPLAY_DATA_STR, 
						(void*)osd_para->text_data, CONTACT_STRING);
		create_brother(&tree_node, MAX_WIDTH_STR, 
						(void*)osd_para->max_width, CONTACT_INT);
		create_brother(&tree_node, MAX_HEIGHT_STR, 
						(void*)osd_para->max_height, CONTACT_INT);
		create_brother(&tree_node, DISPLAY_STREAM_ENABLE_STR, 
						(void*)osd_para->stream_enable, CONTACT_INT);
		create_brother(&tree_node, TIME_DISPLAY_W_STR, 
						(void*)osd_para->time_display_w, CONTACT_INT);
		create_brother(&tree_node, TIME_DISPLAY_H_STR, 
						(void*)osd_para->time_display_h, CONTACT_INT);
		create_brother(&tree_node, TEXT_DISPLAY_W_STR, 
						(void*)osd_para->text_display_w, CONTACT_INT);
		create_brother(&tree_node, TEXT_DISPLAY_H_STR, 
						(void*)osd_para->text_display_h, CONTACT_INT);
        
		create_brother(&tree_node, TEXT1_DISPLAY_ENABLE_STR, 
						(void*)osd_para->ext_osd.ext_text1_enable, CONTACT_INT);
		create_brother(&tree_node, TEXT1_DISPLAY_DATA_STR, 
						(void*)osd_para->ext_osd.ext_text1_data, CONTACT_STRING);
		create_brother(&tree_node, TEXT1_DISPLAY_X_STR, 
						(void*)osd_para->ext_osd.ext_text1_display_x, CONTACT_INT);
		create_brother(&tree_node, TEXT1_DISPLAY_Y_STR, 
						(void*)osd_para->ext_osd.ext_text1_display_y, CONTACT_INT);
		create_brother(&tree_node, TEXT1_DISPLAY_W_STR, 
						(void*)osd_para->ext_osd.ext_text1_display_w, CONTACT_INT);
		create_brother(&tree_node, TEXT1_DISPLAY_H_STR, 
						(void*)osd_para->ext_osd.ext_text1_display_h, CONTACT_INT);
		create_brother(&tree_node, TEXT2_DISPLAY_ENABLE_STR, 
						(void*)osd_para->ext_osd.ext_text2_enable, CONTACT_INT);
		create_brother(&tree_node, TEXT2_DISPLAY_DATA_STR, 
						(void*)osd_para->ext_osd.ext_text2_data, CONTACT_STRING);
		create_brother(&tree_node, TEXT2_DISPLAY_X_STR, 
						(void*)osd_para->ext_osd.ext_text2_display_x, CONTACT_INT);
		create_brother(&tree_node, TEXT2_DISPLAY_Y_STR, 
						(void*)osd_para->ext_osd.ext_text2_display_y, CONTACT_INT);
		create_brother(&tree_node, TEXT2_DISPLAY_W_STR, 
						(void*)osd_para->ext_osd.ext_text2_display_w, CONTACT_INT);
		create_brother(&tree_node, TEXT2_DISPLAY_H_STR, 
						(void*)osd_para->ext_osd.ext_text2_display_h, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_osd_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_OSD_PARAMETER_CMD);
}

int merge_osd_parameter_response_xml(void *pvalue, 
													char *buffer, size_t size)
{
	OSDParameterPacket *osd_para = (OSDParameterPacket*)pvalue;
	
	return merge_osd_parameter_xml(osd_para, buffer, size, OSD_PARAMETER_RESPONSE_CMD);
}

int merge_set_osd_parameter_xml(void *pvalue, 
											char *buffer, size_t size)
{
	OSDParameterPacket *osd_para = (OSDParameterPacket*)pvalue;
	
	return merge_osd_parameter_xml(osd_para, buffer, size, SET_OSD_PARAMETER_CMD);
}

int merge_osd_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, OSD_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_ptz_parameter_xml(PTZParameterPacket *ptz_para, 
												char *buffer, size_t size,
												const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == ptz_para)
	{
		printf("ptz_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)ptz_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)ptz_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)ptz_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)ptz_para->gu_id, CONTACT_STRING);
		create_brother(&tree_node, PTZ_SERIAL_NO, 
						(void*)ptz_para->serial_no, CONTACT_INT);
		create_brother(&tree_node, PTZ_ADDR_STR, 
						(void*)ptz_para->ptz_addr, CONTACT_INT);
		create_brother(&tree_node, PTZ_PROTOCOL_STR, 
						(void*)ptz_para->protocol, CONTACT_INT);
		create_brother(&tree_node, BAUD_RATE_STR, 
						(void*)ptz_para->baud_rate, CONTACT_INT);
		create_brother(&tree_node, DATA_BIT_STR, 
						(void*)ptz_para->data_bit, CONTACT_INT);
		create_brother(&tree_node, STOP_BIT_STR, 
						(void*)ptz_para->stop_bit, CONTACT_INT);
		create_brother(&tree_node, VERIFY_STR, 
						(void*)ptz_para->verify, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_ptz_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_PTZ_PARAMETER_CMD);
}

int merge_ptz_parameter_response_xml(void *pvalue, 
												 char *buffer, size_t size)
{
	PTZParameterPacket *ptz_para = (PTZParameterPacket*)pvalue;
	
	return merge_ptz_parameter_xml(ptz_para, buffer, size, PTZ_PARAMETER_RESPONSE_CMD);
}

int merge_set_ptz_parameter_xml(void *pvalue, 
										 char *buffer, size_t size)
{
	PTZParameterPacket *ptz_para = (PTZParameterPacket*)pvalue;
	
	return merge_ptz_parameter_xml(ptz_para, buffer, size, SET_PTZ_PARAMETER_CMD);
}

int merge_ptz_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, PTZ_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_ftp_parameter_xml(FTPParameterPacket *ftp_para, 
											char *buffer, size_t size,
											const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == ftp_para)
	{
		printf("ftp_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)ftp_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)ftp_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)ftp_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
						(void*)ftp_para->pu_id, CONTACT_STRING);
		create_brother(&tree_node, FTP_IP_STR, 
						(void*)ftp_para->ftp_ip, CONTACT_STRING);
		create_brother(&tree_node, FTP_PORT_STR, 
						(void*)ftp_para->ftp_port, CONTACT_INT);
		create_brother(&tree_node, FTP_USR_STR, 
						(void*)ftp_para->ftp_usr, CONTACT_STRING);
		create_brother(&tree_node, FTP_PWD_STR, 
						(void*)ftp_para->ftp_pwd, CONTACT_STRING);
		create_brother(&tree_node, FTP_PATH_STR, 
						(void*)ftp_para->ftp_path, CONTACT_STRING);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_ftp_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_FTP_PARAMETER_CMD);
}

int merge_ftp_parameter_response_xml(void *pvalue, 
												char *buffer, size_t size)
{
	FTPParameterPacket *ftp_para = (FTPParameterPacket*)pvalue;
	
	return merge_ftp_parameter_xml(ftp_para, buffer, size, FTP_PARAMETER_RESPONSE_CMD);
}

int merge_set_ftp_parameter_xml(void *pvalue, 
										 char *buffer, size_t size)
{
	FTPParameterPacket *ftp_para = (FTPParameterPacket*)pvalue;
	
	return merge_ftp_parameter_xml(ftp_para, buffer, size, SET_FTP_PARAMETER_CMD);
}

int merge_ftp_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, FTP_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_smtp_parameter_xml(SMTPParameterPacket *smtp_para, 
													char *buffer, size_t size,
													const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == smtp_para)
	{
		printf("smtp_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)smtp_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)smtp_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)smtp_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
						(void*)smtp_para->pu_id, CONTACT_STRING);
		create_brother(&tree_node, MAIL_IP_STR, 
						(void*)smtp_para->mail_ip, CONTACT_STRING);
		create_brother(&tree_node, MAIL_PORT_STR, 
						(void*)smtp_para->mail_port, CONTACT_INT);
		create_brother(&tree_node, MAIL_ADDR_STR, 
						(void*)smtp_para->mail_addr, CONTACT_STRING);
		create_brother(&tree_node, MAIL_USR_STR, 
						(void*)smtp_para->mail_usr, CONTACT_STRING);
		create_brother(&tree_node, MAIL_PWD_STR, 
						(void*)smtp_para->mail_pwd, CONTACT_STRING);
		create_brother(&tree_node, MAIL_RCTP1_STR, 
						(void*)smtp_para->mail_rctp1, CONTACT_STRING);
		create_brother(&tree_node, MAIL_RCTP2_STR, 
						(void*)smtp_para->mail_rctp2, CONTACT_STRING);
		create_brother(&tree_node, MAIL_RCTP3_STR, 
						(void*)smtp_para->mail_rctp3, CONTACT_STRING);
		create_brother(&tree_node, SSL_ENABLE_STR, 
						(void*)smtp_para->ssl_enable, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_smtp_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_SMTP_PARAMETER_CMD);
}

int merge_smtp_parameter_response_xml(void *pvalue, 
													char *buffer, size_t size)
{
	SMTPParameterPacket *smtp_para = (SMTPParameterPacket*)pvalue;
	
	return merge_smtp_parameter_xml(smtp_para, buffer, size, SMTP_PARAMETER_RESPONSE_CMD);
}

int merge_set_smtp_parameter_xml(void *pvalue, 
											char *buffer, size_t size)
{
	SMTPParameterPacket *smtp_para = (SMTPParameterPacket*)pvalue;
	
	return merge_smtp_parameter_xml(smtp_para, buffer, size, SET_SMTP_PARAMETER_CMD);
}

int merge_smtp_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SMTP_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int merge_upnp_parameter_xml(UPNPParameterPacket *upnp_para, 
													char *buffer, size_t size,
													const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == upnp_para)
	{
		printf("upnp_para NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)upnp_para->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)upnp_para->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)upnp_para->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
						(void*)upnp_para->pu_id, CONTACT_STRING);
		create_brother(&tree_node, UPNP_IP_STR, 
						(void*)upnp_para->upnp_ip, CONTACT_STRING);
		create_brother(&tree_node, UPNP_ENABLE_STR, 
						(void*)upnp_para->upnp_enable, CONTACT_INT);
		create_brother(&tree_node, UPNP_ETH_NO_STR, 
						(void*)upnp_para->upnp_eth_no, CONTACT_INT);
		create_brother(&tree_node, UPNP_MODEL_STR, 
						(void*)upnp_para->upnp_model, CONTACT_INT);
		create_brother(&tree_node, UPNP_REFRESH_TIME_STR, 
						(void*)upnp_para->upnp_refresh_time, CONTACT_INT);
		create_brother(&tree_node, UPNP_DATA_PORT_STR, 
						(void*)upnp_para->upnp_data_port, CONTACT_INT);
		create_brother(&tree_node, UPNP_WEB_PORT_STR, 
						(void*)upnp_para->upnp_web_port, CONTACT_INT);
		create_brother(&tree_node, UPNP_DATA_PORT_RESULT_STR, 
						(void*)upnp_para->upnp_data_port_result, CONTACT_INT);
		create_brother(&tree_node, UPNP_WEB_PORT_RESULT_STR, 
						(void*)upnp_para->upnp_web_port_result, CONTACT_INT);
        create_brother(&tree_node, UPNP_CMD_PORT_STR, 
						(void*)upnp_para->upnp_cmd_port, CONTACT_INT);
        create_brother(&tree_node, UPNP_TALK_PORT_STR, 
						(void*)upnp_para->upnp_talk_port, CONTACT_INT);
        create_brother(&tree_node, UPNP_CMD_PORT_RESULT_STR, 
						(void*)upnp_para->upnp_cmd_port_result, CONTACT_INT);
        create_brother(&tree_node, UPNP_TALK_PORT_RESULT_STR, 
						(void*)upnp_para->upnp_talk_port_result, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_upnp_parameter_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_UPNP_PARAMETER_CMD);
}

int merge_upnp_parameter_response_xml(void *pvalue, 
													char *buffer, size_t size)
{
	UPNPParameterPacket *upnp_para = (UPNPParameterPacket*)pvalue;
	
	return merge_upnp_parameter_xml(upnp_para, buffer, size, UPNP_PARAMETER_RESPONSE_CMD);
}

int merge_set_upnp_parameter_xml(void *pvalue, 
											char *buffer, size_t size)
{
	UPNPParameterPacket *upnp_para = (UPNPParameterPacket*)pvalue;
	
	return merge_upnp_parameter_xml(upnp_para, buffer, size, SET_UPNP_PARAMETER_CMD);
}

int merge_upnp_parameter_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, UPNP_PARAMETER_RESULT_CMD);
}

//##########################################################################
int merge_get_device_disk_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_DEVICE_DISK_INFO_CMD);
}

int merge_device_disk_info_response_xml(void *pvalue, 
											char *buffer, size_t size)
{
	int ret = -1;
	int index =0;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	DeviceDiskInfoPacket *disk_info = (DeviceDiskInfoPacket*)pvalue;
	
	if (NULL == disk_info)
	{
		printf("disk_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, DEVICE_DISK_INFO_RESPONSE_CMD);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR,
										 (void*)disk_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, (void*)disk_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, (void*)disk_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, (void*)disk_info->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, DISK_INFO_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, DISK_NUM_STR, 
			(void*)disk_info->disk_num, CONTACT_INT);
		for (index=0; index<disk_info->disk_num; index++)
		{
			if (!index)
				tree_child = create_child(tree_node, DISK_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, DISK_STR, NULL, CONTACT_NULL);
			
			create_attr(&tree_child, DISK_NO_STR, 
				(void*)disk_info->disk[index].disk_no, CONTACT_INT);

			if (tree_child)
			{
				tree_son = create_child(tree_child, TOTAL_SIZE_STR, 
							(void*)disk_info->disk[index].total_size, CONTACT_INT);
				if (tree_son)
				{
					create_brother(&tree_son, FREE_SIZE_STR, 
						(void*)disk_info->disk[index].free_size, CONTACT_INT);
					create_brother(&tree_son, IS_BACKUP_STR, 
						(void*)disk_info->disk[index].is_backup, CONTACT_INT);
					create_brother(&tree_son, DISK_STATUS_STR, 
						(void*)disk_info->disk[index].status, CONTACT_INT);
					create_brother(&tree_son, DISK_TYPE_STR, 
						(void*)disk_info->disk[index].disk_type, CONTACT_INT);
					create_brother(&tree_son, SYS_FILE_TYPE_STR, 
						(void*)disk_info->disk[index].sys_file_type, CONTACT_INT);
				}
			}
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

//##########################################################################
static int merge_format_disk_info_xml(FormatDiskPacket *format, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == format)
	{
		printf("format NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
										 (void*)format->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DISK_NO_STR, 
						(void*)format->disk_no, CONTACT_INT);
		create_brother(&tree_node, FORMAT_PROGRESS_STR, 
						(void*)format->progress, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_format_disk_request_xml(void *pvalue, char *buffer, size_t size)
{
	FormatDiskPacket *format = (FormatDiskPacket*)pvalue;
	
	return merge_format_disk_info_xml(format, buffer, size, FORMAT_DISK_REQUEST_CMD);
}

int merge_format_disk_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, FORMAT_DISK_RESULT_CMD);
}

int merge_submit_format_progress_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	FormatProgressPacket *format;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	format = (FormatProgressPacket*)pvalue;
		
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, SUBMIT_FORMAT_PROGRESS_CMD);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
										 (void*)format->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DISK_NO_STR, 
						(void*)format->disk_no, CONTACT_INT);
		create_brother(&tree_node, FORMAT_PROGRESS_STR, 
						(void*)format->progress, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}


//##########################################################################
static int merge_move_alarm_info_xml(MoveAlarmPacket *move_alarm, 
												char *buffer, size_t size,
												const char *command)
{
	int ret = -1;
	int flag0, flag1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	
	char time_buffer[J_SDK_MAX_TIME_LEN];
	memset(time_buffer, 0, sizeof(time_buffer));
	
	if (NULL == move_alarm)
	{
		printf("move_alarm NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	tree_node = create_child(tree_root, RESULT_CODE_STR, 
				(void*)move_alarm->result.code, CONTACT_INT);
	if (tree_node)
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)move_alarm->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)move_alarm->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)move_alarm->gu_id, CONTACT_STRING);
		create_brother(&tree_node, MOVE_ENABLE_STR, 
						(void*)move_alarm->move_enable, CONTACT_INT);
		create_brother(&tree_node, SENSITIVE_LEVEL_STR, 
						(void*)move_alarm->sensitive_level, CONTACT_INT);
		create_brother(&tree_node, DETECT_INTERVAL_STR, 
						(void*)move_alarm->detect_interval, CONTACT_INT);
		create_brother(&tree_node, MAX_WIDTH_STR, 
						(void*)move_alarm->max_width, CONTACT_INT);
		create_brother(&tree_node, MAX_HEIGHT_STR, 
						(void*)move_alarm->max_height, CONTACT_INT);
		
		create_brother(&tree_node, DETECT_AREA_STR, NULL, CONTACT_NULL);
		for (flag0=0; flag0<move_alarm->detect_area.count; flag0++)
		{
			if (!flag0)
				tree_child = create_child(tree_node, RECTANGLE_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, RECTANGLE_STR, NULL, CONTACT_NULL);

			if (tree_child)
			{
				tree_son = create_child(tree_child, RECT_LEFT_STR, 
							(void*)((int)move_alarm->detect_area.rect[flag0].left), CONTACT_INT);
				if (tree_son)
				{
					create_brother(&tree_son, RECT_TOP_STR, 
						(void*)((int)move_alarm->detect_area.rect[flag0].top), CONTACT_INT);
					create_brother(&tree_son, RECT_RIGHT_STR, 
						(void*)((int)move_alarm->detect_area.rect[flag0].right), CONTACT_INT);
					create_brother(&tree_son, RECT_BOTTOM_STR, 
						(void*)((int)move_alarm->detect_area.rect[flag0].bottom), CONTACT_INT);
				}
			}
		}

		create_brother(&tree_node, WEEK_DAY_S_STR, NULL, CONTACT_NULL);
		for (flag0=0; flag0<move_alarm->week.count; flag0++)
		{
			if (!flag0)
				tree_child = create_child(tree_node, WEEK_DAY_STR, 
											NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, WEEK_DAY_STR, 
									NULL, CONTACT_NULL);
			
			create_attr(&tree_child, WEEK_DAY_ID_STR, 
				(void*)move_alarm->week.days[flag0].day_id, CONTACT_INT);
			if (tree_child)
			{
				tree_son = create_child(tree_child, TIME_SEG_S_STR, 
										NULL, CONTACT_NULL);
				
				for (flag1=0; flag1<move_alarm->week.days[flag0].count; flag1++)
				{
					if (0 == merge_time_part_string(time_buffer, 
								sizeof(time_buffer), 
								&(move_alarm->week.days[flag0].seg[flag1].time_start), 
								&(move_alarm->week.days[flag0].seg[flag1].time_end)))
					{
						if (!flag1)
							tree_grandson = create_child(tree_son, TIME_SEG_STR, 
											(void*)time_buffer, CONTACT_STRING);
						else
							create_brother(&tree_grandson, TIME_SEG_STR, 
									(void*)time_buffer, CONTACT_STRING);
						
						create_attr(&tree_grandson, TIME_SEG_ENABLE_STR, 
							(void*)move_alarm->week.days[flag0].seg[flag1].enable, CONTACT_INT);
					}
				}
			}
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_move_alarm_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_MOVE_ALARM_INFO_CMD);
}

int merge_move_alarm_info_response_xml(void *pvalue, 
													char *buffer, size_t size)
{
	MoveAlarmPacket *move_alarm = (MoveAlarmPacket*)pvalue;
	
	return merge_move_alarm_info_xml(move_alarm, buffer, size, MOVE_ALARM_INFO_RESPONSE_CMD);
}

int merge_set_move_alarm_info_xml(void *pvalue, 
											char *buffer, size_t size)
{
	MoveAlarmPacket *move_alarm = (MoveAlarmPacket*)pvalue;
	
	return merge_move_alarm_info_xml(move_alarm, buffer, size, SET_MOVE_ALARM_INFO_CMD);
}

int merge_move_alarm_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, MOVE_ALARM_INFO_RESULT_CMD);
}

//##########################################################################
static int merge_lost_alarm_info_xml(LostAlarmPacket *lost_alarm, 
												char *buffer, size_t size,
												const char *command)
{
	int ret = -1;
	int flag0, flag1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	
	char time_buffer[J_SDK_MAX_TIME_LEN];
	memset(time_buffer, 0, sizeof(time_buffer));
	
	if (NULL == lost_alarm)
	{
		printf("lost_alarm NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)lost_alarm->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)lost_alarm->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)lost_alarm->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)lost_alarm->gu_id, CONTACT_STRING);
		//create_brother(&tree_node, CHANNEL_STR, 
			//			(void*)lost_alarm->channel, CONTACT_INT);
		//create_brother(&tree_node, GU_TYPE_STR, 
			//			(void*)lost_alarm->gu_type, CONTACT_INT);
		create_brother(&tree_node, LOST_ENABLE_STR, 
						(void*)lost_alarm->lost_enable, CONTACT_INT);
		create_brother(&tree_node, DETECT_INTERVAL_STR, 
						(void*)lost_alarm->detect_interval, CONTACT_INT);
		
		create_brother(&tree_node, WEEK_DAY_S_STR, NULL, CONTACT_NULL);
		for (flag0=0; flag0<lost_alarm->week.count; flag0++)
		{
			if (!flag0)
				tree_child = create_child(tree_node, WEEK_DAY_STR, 
											NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, WEEK_DAY_STR, 
									NULL, CONTACT_NULL);
			
			create_attr(&tree_child, WEEK_DAY_ID_STR, 
				(void*)lost_alarm->week.days[flag0].day_id, CONTACT_INT);
			if (tree_child)
			{
				tree_son = create_child(tree_child, TIME_SEG_S_STR, 
										NULL, CONTACT_NULL);
				
				for (flag1=0; flag1<lost_alarm->week.days[flag0].count; flag1++)
				{
					if (0 == merge_time_part_string(time_buffer, 
								sizeof(time_buffer), 
								&(lost_alarm->week.days[flag0].seg[flag1].time_start), 
								&(lost_alarm->week.days[flag0].seg[flag1].time_end)))
					{
						if (!flag1)
							tree_grandson = create_child(tree_son, TIME_SEG_STR, 
											(void*)time_buffer, CONTACT_STRING);
						else
							create_brother(&tree_grandson, TIME_SEG_STR, 
									(void*)time_buffer, CONTACT_STRING);
						
						create_attr(&tree_grandson, TIME_SEG_ENABLE_STR, 
							(void*)lost_alarm->week.days[flag0].seg[flag1].enable, CONTACT_INT);
					}
				}
			}
		}
	}	
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_lost_alarm_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_LOST_ALARM_INFO_CMD);
}

int merge_lost_alarm_info_response_xml(void *pvalue, 
													char *buffer, size_t size)
{
	LostAlarmPacket *lost_alarm = (LostAlarmPacket*)pvalue;
	
	return merge_lost_alarm_info_xml(lost_alarm, buffer, size, 
										LOST_ALARM_INFO_RESPONSE_CMD);
}

int merge_set_lost_alarm_info_xml(void *pvalue, char *buffer, size_t size)
{
	LostAlarmPacket *lost_alarm = (LostAlarmPacket*)pvalue;
	
	return merge_lost_alarm_info_xml(lost_alarm, buffer, size, 
										SET_LOST_ALARM_INFO_CMD);
}

int merge_lost_alarm_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, LOST_ALARM_INFO_RESULT_CMD);
}

//##########################################################################
static int merge_hide_alarm_info_xml(HideAlarmPacket *hide_alarm, 
												char *buffer, size_t size,
												const char *command)
{
	int ret = -1;
	int flag0, flag1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	
	char time_buffer[J_SDK_MAX_TIME_LEN];
	memset(time_buffer, 0, sizeof(time_buffer));
	
	if (NULL == hide_alarm)
	{
		printf("hide_alarm NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)hide_alarm->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)hide_alarm->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)hide_alarm->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)hide_alarm->gu_id, CONTACT_STRING);
		create_brother(&tree_node, HIDE_ENABLE_STR, 
						(void*)hide_alarm->hide_enable, CONTACT_INT);
		create_brother(&tree_node, MAX_WIDTH_STR, 
						(void*)hide_alarm->max_width, CONTACT_INT);
		create_brother(&tree_node, MAX_HEIGHT_STR, 
						(void*)hide_alarm->max_height, CONTACT_INT);
		
		create_brother(&tree_node, DETECT_INTERVAL_STR, 
						(void*)hide_alarm->detect_interval, CONTACT_INT);
		create_brother(&tree_node, SENSITIVE_LEVEL_STR, 
						(void*)hide_alarm->sensitive_level, CONTACT_INT);
		
		create_brother(&tree_node, DETECT_AREA_STR, NULL, CONTACT_NULL);
		for (flag0=0; flag0<hide_alarm->detect_area.count; flag0++)
		{
			if (!flag0)
				tree_child = create_child(tree_node, RECTANGLE_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, RECTANGLE_STR, NULL, CONTACT_NULL);

			if (tree_child)
			{
				tree_son = create_child(tree_child, RECT_LEFT_STR, 
							(void*)((int)hide_alarm->detect_area.rect[flag0].left), CONTACT_INT);
				if (tree_son)
				{
					create_brother(&tree_son, RECT_TOP_STR, 
						(void*)((int)hide_alarm->detect_area.rect[flag0].top), CONTACT_INT);
					create_brother(&tree_son, RECT_RIGHT_STR, 
						(void*)((int)hide_alarm->detect_area.rect[flag0].right), CONTACT_INT);
					create_brother(&tree_son, RECT_BOTTOM_STR, 
						(void*)((int)hide_alarm->detect_area.rect[flag0].bottom), CONTACT_INT);
				}
			}
		}
		
		create_brother(&tree_node, WEEK_DAY_S_STR, NULL, CONTACT_NULL);
		for (flag0=0; flag0<hide_alarm->week.count; flag0++)
		{
			if (!flag0)
				tree_child = create_child(tree_node, WEEK_DAY_STR, 
											NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, WEEK_DAY_STR, 
									NULL, CONTACT_NULL);
			
			create_attr(&tree_child, WEEK_DAY_ID_STR, 
				(void*)hide_alarm->week.days[flag0].day_id, CONTACT_INT);
			if (tree_child)
			{
				tree_son = create_child(tree_child, TIME_SEG_S_STR, 
										NULL, CONTACT_NULL);
				
				for (flag1=0; flag1<hide_alarm->week.days[flag0].count; flag1++)
				{
					if (0 == merge_time_part_string(time_buffer, 
								sizeof(time_buffer), 
								&(hide_alarm->week.days[flag0].seg[flag1].time_start), 
								&(hide_alarm->week.days[flag0].seg[flag1].time_end)))
					{
						if (!flag1)
							tree_grandson = create_child(tree_son, TIME_SEG_STR, 
											(void*)time_buffer, CONTACT_STRING);
						else
							create_brother(&tree_grandson, TIME_SEG_STR, 
									(void*)time_buffer, CONTACT_STRING);
						
						create_attr(&tree_grandson, TIME_SEG_ENABLE_STR, 
							(void*)hide_alarm->week.days[flag0].seg[flag1].enable, CONTACT_INT);
					}
				}
			}
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_hide_alarm_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_HIDE_ALARM_INFO_CMD);
}

int merge_hide_alarm_info_response_xml(void *pvalue, 
													char *buffer, size_t size)
{
	HideAlarmPacket *hide_alarm = (HideAlarmPacket*)pvalue;
	
	return merge_hide_alarm_info_xml(hide_alarm, buffer, size, 
										HIDE_ALARM_INFO_RESPONSE_CMD);
}

int merge_set_hide_alarm_info_xml(void *pvalue, 
											char *buffer, size_t size)
{
	HideAlarmPacket *hide_alarm = (HideAlarmPacket*)pvalue;
	
	return merge_hide_alarm_info_xml(hide_alarm, buffer, size, 
										SET_HIDE_ALARM_INFO_CMD);
}

int merge_hide_alarm_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, HIDE_ALARM_INFO_RESULT_CMD);
}

//##########################################################################
static int merge_io_alarm_info_xml(IoAlarmPacket *io_alarm, 
											char *buffer, size_t size,
											const char *command)
{
	int ret = -1;
	int flag0, flag1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	
	char time_buffer[J_SDK_MAX_TIME_LEN];
	memset(time_buffer, 0, sizeof(time_buffer));
	
	if (NULL == io_alarm)
	{
		printf("io_alarm NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)io_alarm->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)io_alarm->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)io_alarm->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)io_alarm->gu_id, CONTACT_STRING);
		create_brother(&tree_node, IO_TYPE_STR, 
						(void*)io_alarm->io_type, CONTACT_INT);
		create_brother(&tree_node, IO_ENABLE_STR, 
						(void*)io_alarm->alarm_enable, CONTACT_INT);
		create_brother(&tree_node, DETECT_INTERVAL_STR, 
						(void*)io_alarm->detect_interval, CONTACT_INT);
		
		create_brother(&tree_node, WEEK_DAY_S_STR, NULL, CONTACT_NULL);
		for (flag0=0; flag0<io_alarm->week.count; flag0++)
		{
			if (!flag0)
				tree_child = create_child(tree_node, WEEK_DAY_STR, 
											NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, WEEK_DAY_STR, 
									NULL, CONTACT_NULL);
			
			create_attr(&tree_child, WEEK_DAY_ID_STR, 
				(void*)io_alarm->week.days[flag0].day_id, CONTACT_INT);
			if (tree_child)
			{
				tree_son = create_child(tree_child, TIME_SEG_S_STR, 
										NULL, CONTACT_NULL);
				
				for (flag1=0; flag1<io_alarm->week.days[flag0].count; flag1++)
				{
					if (0 == merge_time_part_string(time_buffer, 
								sizeof(time_buffer), 
								&(io_alarm->week.days[flag0].seg[flag1].time_start), 
								&(io_alarm->week.days[flag0].seg[flag1].time_end)))
					{
						if (!flag1)
							tree_grandson = create_child(tree_son, TIME_SEG_STR, 
											(void*)time_buffer, CONTACT_STRING);
						else
							create_brother(&tree_grandson, TIME_SEG_STR, 
									(void*)time_buffer, CONTACT_STRING);
						
						create_attr(&tree_grandson, TIME_SEG_ENABLE_STR, 
							(void*)io_alarm->week.days[flag0].seg[flag1].enable, CONTACT_INT);
					}
				}
			}
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_io_alarm_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_IO_ALARM_INFO_CMD);
}

int merge_io_alarm_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	IoAlarmPacket *io_alarm = (IoAlarmPacket*)pvalue;
	
	return merge_io_alarm_info_xml(io_alarm, buffer, size, 
									IO_ALARM_INFO_RESPONSE_CMD);
}

int merge_set_io_alarm_info_xml(void *pvalue, char *buffer, size_t size)
{
	IoAlarmPacket *io_alarm = (IoAlarmPacket*)pvalue;
	
	return merge_io_alarm_info_xml(io_alarm, buffer, size, 
									SET_IO_ALARM_INFO_CMD);
}

int merge_io_alarm_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, IO_ALARM_INFO_RESULT_CMD);
}

//##########################################################################
static int merge_joint_info_xml(JointActionPacket *joint_action, 
											char *buffer, size_t size,
											const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	if (NULL == joint_action)
	{
		printf("joint_action NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)joint_action->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)joint_action->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)joint_action->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)joint_action->gu_id, CONTACT_STRING);
		create_brother(&tree_node, ALARM_TYPE_STR, 
						(void*)joint_action->alarm_type, CONTACT_INT);
		
		create_brother(&tree_node, JOINT_ACTION_STR, NULL, CONTACT_NULL);
		tree_child = create_child(tree_node, JOINT_RECORD_STR, NULL, CONTACT_NULL);			
		if (tree_child)
		{
			tree_son = create_child(tree_child, JOINT_RECORD_ENABLE_CHANNEL_STR, 
					(void*)joint_action->joint.joint_record_enable_channel, CONTACT_INT);
			if (tree_son)
			{
				create_brother(&tree_son, JOINT_RECORD_SECOND_STR, 
					(void*)joint_action->joint.joint_record_second, CONTACT_INT);
			}
			
			create_brother(&tree_child, JOINT_IO_STR, NULL, CONTACT_NULL);
			tree_son = create_child(tree_child, JOINT_BEEP_ENABLE_STR, 
					(void*)joint_action->joint.joint_beep_enable, CONTACT_INT);
			if (tree_son)
			{
				create_brother(&tree_son, JOINT_BEEP_SECOND_STR, 
					(void*)joint_action->joint.joint_beep_second, CONTACT_INT);
				create_brother(&tree_son, JOINT_OUTPUT_TIMES_STR, 
					(void*)joint_action->joint.joint_output_times, CONTACT_INT);
				create_brother(&tree_son, JOINT_OUTPUT_ENABLE_CHANNEL_STR, 
					(void*)joint_action->joint.joint_output_enable_channel, CONTACT_INT);
			}
			create_brother(&tree_child, JOINT_SNAP_STR, NULL, CONTACT_NULL);
			tree_son = create_child(tree_child, JOINT_SNAP_ENABLE_CHANNEL_STR, 
					(void*)joint_action->joint.joint_snap_enable_channel, CONTACT_INT);
			if (tree_son)
			{
				create_brother(&tree_son, JOINT_SNAP_INTERVAL_STR, 
					(void*)joint_action->joint.joint_snap_interval, CONTACT_INT);
				create_brother(&tree_son, JOINT_SNAP_TIMES_STR, 
					(void*)joint_action->joint.joint_snap_times, CONTACT_INT);
			}
			create_brother(&tree_child, JOINT_EMAIL_STR, NULL, CONTACT_NULL);
			tree_son = create_child(tree_child, JOINT_EMAIL_ENABLE_STR, 
					(void*)joint_action->joint.joint_email_enable, CONTACT_INT);
			if (tree_son)
			{
			}
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_joint_action_info_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_JOINT_ACTION_INFO_CMD);
}

int merge_joint_action_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	JointActionPacket *joint_action = (JointActionPacket*)pvalue;
	
	return merge_joint_info_xml(joint_action, buffer, size, 
									JOINT_ACTION_INFO_RESPONSE_CMD);
}

int merge_set_joint_action_info_xml(void *pvalue, char *buffer, size_t size)
{
	JointActionPacket *joint_action = (JointActionPacket*)pvalue;
	
	return merge_joint_info_xml(joint_action, buffer, size, 
									SET_JOINT_ACTION_INFO_CMD);
}

int merge_joint_action_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, JOINT_ACTION_INFO_RESULT_CMD);
}


//##########################################################################
static int merge_control_ptz_xml(PTZControlPacket *ptz_ctrl, 
										char *buffer, size_t size,
										const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == ptz_ctrl)
	{
		printf("ptz_ctrl NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)ptz_ctrl->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)ptz_ctrl->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)ptz_ctrl->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)ptz_ctrl->gu_id, CONTACT_STRING);
		create_brother(&tree_node, PTZ_DIRECTION_STR, 
						(void*)ptz_ctrl->action, CONTACT_INT);
		create_brother(&tree_node, PTZ_PARAM_STR, 
						(void*)ptz_ctrl->param, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_control_ptz_cmd_xml(void *pvalue, char *buffer, size_t size)
{
	PTZControlPacket *ptz_ctrl = (PTZControlPacket*)pvalue;
	
	return merge_control_ptz_xml(ptz_ctrl, buffer, size, CONTROL_PTZ_COMMAND_CMD);
}

int merge_ptz_cmd_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, PTZ_COMMAND_RESULT_CMD);
}

//##########################################################################
int merge_submit_alarm_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	char time_buffer[J_SDK_MAX_TIME_LEN];
	
	SubmitAlarmPacket *sub_alarm = (SubmitAlarmPacket*)pvalue;
	
	if (NULL == sub_alarm)
	{
		printf("sub_alarm NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, SUBMIT_ALARM_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
							 (void*)sub_alarm->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, CHANNEL_STR, 
						(void*)sub_alarm->channel, CONTACT_INT);
		create_brother(&tree_node, ALARM_TYPE_STR, 
						(void*)sub_alarm->alarm_type, CONTACT_INT);
		create_brother(&tree_node, ACTION_TYPE_STR, 
						(void*)sub_alarm->action_type, CONTACT_INT);
		
		memset(time_buffer, 0, sizeof(time_buffer));
		if (0 == merge_time_string(time_buffer, sizeof(time_buffer), 
											&(sub_alarm->alarm_time)))
		{
			create_brother(&tree_node, ALARM_TIME_STR, 
							(void*)time_buffer, CONTACT_STRING);
		}
		else
		{
			printf("merge_time_string(ALARM_TIME_STR) failed.\n");
			xml_tree_delete(xml_tree);
			return -1;
		}
		create_brother(&tree_node, DATA_STR, sub_alarm->data, CONTACT_STRING);
	}
	else
	{
		printf("create_child(PU_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}


//##########################################################################
static int merge_media_url_xml(MediaUrlPacket *media, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == media)
	{
		printf("media NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
										 (void*)media->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)media->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)media->gu_id, CONTACT_STRING);
		if (!strcmp(command, GET_MEDIA_URL_RESPONSE_CMD))
		{
			create_brother(&tree_node, RESULT_CODE_STR, 
						(void*)media->result.code, CONTACT_INT);
			create_brother(&tree_node, URL_STR, 
						(void*)media->url, CONTACT_STRING);
		}
		else if (!strcmp(command, GET_MEDIA_URL_REQUEST_CMD))
		{
			create_brother(&tree_node, MEDIA_STR, 
						(void*)media->media_type, CONTACT_INT);
			create_brother(&tree_node, CMS_IP_STR, 
						(void*)media->ip, CONTACT_STRING);
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_media_url_request_xml(void *pvalue, char *buffer, size_t size)
{
	MediaUrlPacket *media = (MediaUrlPacket*)pvalue;
	
	return merge_media_url_xml(media, buffer, size, GET_MEDIA_URL_REQUEST_CMD);
}
int merge_get_media_url_response_xml(void *pvalue, char *buffer, size_t size)
{
	MediaUrlPacket *media = (MediaUrlPacket*)pvalue;
	
	return merge_media_url_xml(media, buffer, size, GET_MEDIA_URL_RESPONSE_CMD);
}

static __inline__ int 
merge_store_log_info(StoreLogPacket *store_log, char *buffer, 
	size_t size, const char *cmd)
{
	int i, ret = -1;
    
  	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	char time_buffer[J_SDK_MAX_TIME_LEN];

	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);

	tree_root = create_xml_head(xml_tree, cmd);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
								(void *)store_log->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void *)store_log->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
			(void *)store_log->gu_id, CONTACT_STRING);
		if (!strcmp(GET_STORE_LOG_RESPONSE_CMD, cmd))
		{
			create_brother(&tree_node, RESULT_CODE_STR, 
				(void *)store_log->result.code, CONTACT_INT);
			
			create_brother(&tree_node, TOTAL_LOG_COUNT_STR, 
				(void *)store_log->total_count, CONTACT_INT);
			create_brother(&tree_node, SESS_ID_STR, 
				(void *)store_log->sess_id, CONTACT_INT);
			create_brother(&tree_node, REC_INFO_STR, 
				(void *)NULL, CONTACT_NULL);
			create_attr(&tree_node, NODE_COUNT_STR, 
				(void*)store_log->node_count, CONTACT_INT);
			
			for (i=0; i<store_log->node_count; i++)
			{
				if (!i)
				{
					tree_child = create_child(tree_node, REC_NODE_STR, 
									(void*)NULL, CONTACT_NULL);
				}
				else
				{
					create_brother(&tree_child, REC_NODE_STR, 
									(void*)NULL, CONTACT_NULL);
				}
				
				tree_son = create_child(tree_child, REC_TYPE_STR, 
							(void*)store_log->store[i].rec_type, CONTACT_INT);
				create_brother(&tree_son, FILE_SIZE_STR, 
					(void*)store_log->store[i].file_size, CONTACT_INT);
				
				memset(time_buffer, 0, sizeof(time_buffer));
				if (!merge_time_string(time_buffer, sizeof(time_buffer), 
							&(store_log->store[i].beg_time)))
				{
					create_brother(&tree_son, BEG_TIME_STR, 
						(void*)time_buffer, CONTACT_STRING);
				}
				else
				{
					printf("merge_time_string(BEG_TIME_STR) failed.\n");
				}
				memset(time_buffer, 0, sizeof(time_buffer));
				if (0 == merge_time_string(time_buffer, sizeof(time_buffer), 
							&(store_log->store[i].end_time)))
				{
					create_brother(&tree_son, END_TIME_STR, 
						(void*)time_buffer, CONTACT_STRING);
				}
				else
				{
					printf("merge_time_string(END_TIME_STR) failed.\n");
				}
				create_brother(&tree_son, PROPERTY_STR, 
					(void*)store_log->store[i].property, CONTACT_STRING);
			}
		}
		else
		{
			create_brother(&tree_node, SESS_ID_STR, 
				(void *)store_log->sess_id, CONTACT_INT);
			create_brother(&tree_node, REC_TYPE_STR, 
				(void *)store_log->rec_type, CONTACT_INT);
			create_brother(&tree_node, BEG_NODE_STR, 
				(void *)store_log->beg_node, CONTACT_INT);
			create_brother(&tree_node, END_NODE_STR, 
				(void *)store_log->end_node, CONTACT_INT);
			
			memset(time_buffer, 0, sizeof(time_buffer));
			if (!merge_time_string(time_buffer, sizeof(time_buffer), 
						&(store_log->beg_time)))
			{
				create_brother(&tree_node, BEG_TIME_STR, 
					(void*)time_buffer, CONTACT_STRING);
			}
			else
			{
				printf("merge_time_string(BEG_TIME_STR) failed.\n");
			}
			memset(time_buffer, 0, sizeof(time_buffer));
			if (0 == merge_time_string(time_buffer, sizeof(time_buffer), 
						&(store_log->end_time)))
			{
				create_brother(&tree_node, END_TIME_STR, 
					(void*)time_buffer, CONTACT_STRING);
			}
			else
			{
				printf("merge_time_string(END_TIME_STR) failed.\n");
			}
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
 
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
	    //printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
    
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_store_log_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	struct __StoreLogPacket *store_log;
    
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	store_log = (struct __StoreLogPacket*)pvalue;

	return merge_store_log_info(store_log, buffer, size, GET_STORE_LOG_REQUEST_CMD);
}
int merge_get_store_log_response_xml(void *pvalue, 
		char *buffer, size_t size)
{
	struct __StoreLogPacket *store_log;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	store_log = (struct __StoreLogPacket*)pvalue;

	return merge_store_log_info(store_log, buffer, size, GET_STORE_LOG_RESPONSE_CMD);
}

static __inline__ int 
merge_user_info_xml(struct __UserInfo *user_info, 
	char *buffer, size_t size, const char *cmd)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, cmd);
	if (NULL != (tree_node = create_child(tree_root, USERNAME_STR, 
							 (void*)user_info->username, CONTACT_STRING)))
	{
		if ('\0' != user_info->password[0])
			create_brother(&tree_node, PASSWORD_STR, 
						(void*)user_info->password, CONTACT_STRING);
	}
	else
	{
		printf("create_child(USERNAME_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
static __inline__ int
merge_result_info_xml(int *result, 
	char *buffer, size_t size, const char *cmd)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, cmd);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
							 (void*)*((int*)result), CONTACT_INT)))
	{
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_user_login_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_user_info_xml((struct __UserInfo*)pvalue, 
			buffer, size, USER_LONGIN_REQUEST_CMD);
}

int merge_user_login_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	int ret = -1;
	int result, keep_time;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	keep_time = *((int*)pvalue);

	if (-1 == keep_time)
		result = -1;
	else if (0 == keep_time)
		result = 1;
	else //if (0 < keep_time)
		result = 0;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, USER_LONGIN_RESULT_CMD);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
							 (void*)result, CONTACT_INT)))
	{
		create_brother(&tree_node, KEEP_ALIVE_STR, 
					(void*)keep_time, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}


//##########################################################################
int merge_user_heart_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
//	XmlTreeNode *tree_node = NULL;

	struct __UserHeart *heart = (struct __UserHeart*)pvalue;
	
	if (NULL == heart)
	{
		printf("heart_beat NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, USER_HEART_REQUEST_CMD);
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_user_heart_response_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	char time_buffer[J_SDK_MAX_TIME_LEN];
	
	struct __UserHeart *heart = (struct __UserHeart*)pvalue;
	
	if (NULL == heart)
	{
		printf("heart_beat_resp NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, USER_HEART_RESPONSE_CMD);
	memset(time_buffer, 0, sizeof(time_buffer));
	if (!merge_time_string(time_buffer, sizeof(time_buffer), 
			&(heart->server_time)))
	{
		if (NULL != (tree_node = create_child(tree_root, SERVER_TIME_STR, 
									(void*)time_buffer, CONTACT_STRING)))
		{
		}
		else
		{
			printf("create_child(SERVER_TIME_STR) failed.\n");
			xml_tree_delete(xml_tree);
			return -1;
		}
	}
	else
	{
		printf("merge_time_string(SERVER_TIME_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}

//##########################################################################
int merge_firmware_upgrade_request_xml(void *pvalue, 
				char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	FirmwareUpgradePacket *upgrade_info;
	
	if (NULL == pvalue)
	{
		printf("upgrade_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	upgrade_info = (FirmwareUpgradePacket*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, FIRMWARE_UPGRADE_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
								(void*)upgrade_info->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)upgrade_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)upgrade_info->pu_id, CONTACT_STRING);
		create_brother(&tree_node, UPGREADE_ADDR_STR, 
			(void*)upgrade_info->addr, CONTACT_STRING);
		create_brother(&tree_node, UPGREADE_DATA_LEN_STR, 
			(void*)upgrade_info->data_len, CONTACT_INT);
		create_brother(&tree_node, UPGREADE_DATA_STR, 
			(void*)upgrade_info->data, CONTACT_STRING);
		create_brother(&tree_node, FILE_LENGTH_LEN_STR, 
			(void*)upgrade_info->file_len, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_firmware_upgrade_response_xml(void *pvalue, 
				char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	FirmwareUpgradePacket *upgrade_info;
	
	if (NULL == pvalue)
	{
		printf("upgrade_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	upgrade_info = (FirmwareUpgradePacket*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, FIRMWARE_UPGRADE_RESPONSE_CMD);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
										 (void*)upgrade_info->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)upgrade_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)upgrade_info->pu_id, CONTACT_STRING);
		create_brother(&tree_node, RESULT_CODE_STR, 
			(void*)upgrade_info->result.code, CONTACT_INT);
		create_brother(&tree_node, UPGREADE_ADDR_STR, 
			(void*)upgrade_info->addr, CONTACT_STRING);
		create_brother(&tree_node, SESS_ID_STR, 
			(void *)upgrade_info->sess_id, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_submit_upgrade_progress_xml(void *pvalue, 
		char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	UpgradeProgressPacket *upgrade = (UpgradeProgressPacket*)pvalue;
	
	if (NULL == upgrade)
	{
		printf("upgrade NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, SUBMIT_UPGRADE_PROGRESS_CMD);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
								(void*)upgrade->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, UPGREADE_PROGRESS_STR, 
			(void*)upgrade->percent, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

# ifdef HAVE_PROXY_INFO

//##########################################################################
int merge_add_user_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_user_info_xml((struct __UserInfo*)pvalue, 
			buffer, size, ADD_USER_REQUEST_CMD);
}

int merge_add_user_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_result_info_xml((int*)pvalue, 
			buffer, size, ADD_USER_RESULT_CMD);
}

int merge_del_user_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_user_info_xml((struct __UserInfo*)pvalue, 
			buffer, size, DEL_USER_REQUEST_CMD);
}

int merge_del_user_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_result_info_xml((int*)pvalue, 
			buffer, size, DEL_USER_RESULT_CMD);
}

int merge_proxy_user_list_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	int index = 0;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	struct _prx_user_st *user_list;
	struct __UserInfo *user_info;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);

	user_list = (struct _prx_user_st*)pvalue;
	user_info = &user_list->user_info[index++];
	
	tree_root = create_xml_head(xml_tree, USER_LIST_INFO_CMD);
	if (NULL != (tree_node = create_child(tree_root, (void*)user_info->username, 
							 (void*)user_info->password, CONTACT_STRING)))
	{
		while (index < user_list->count)
		{
			user_info = &user_list->user_info[index++];
			
			create_brother(&tree_node, (void*)user_info->username, 
							(void*)user_info->password, CONTACT_STRING);
		}
	}
	else
	{
		printf("create_child() failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_fuzzy_find_user_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_user_info_xml((struct __UserInfo*)pvalue, 
			buffer, size, FUZZY_FIND_USER_REQUEST_CMD);
}

int merge_fuzzy_find_user_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	int ret = -1;
	int index = 0;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	struct _prx_user_st *user_list;
	struct __UserInfo *user_info;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);

	user_list = (struct _prx_user_st*)pvalue;
	
	tree_root = create_xml_head(xml_tree, FUZZY_FIND_USER_RESULT_CMD);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
							 (void*)user_list->result, CONTACT_INT)))
	{
		create_brother(&tree_node, USER_LIST_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)user_list->count, CONTACT_INT);

		while (index < user_list->count)
		{
			if (!index)
			{
				user_info = &user_list->user_info[index++];
				tree_child = create_child(tree_node, USERNAME_STR, 
								(void*)user_info->username, CONTACT_STRING);
			}
			else
			{
				user_info = &user_list->user_info[index++];
				create_brother(&tree_child, USERNAME_STR, 
								(void*)user_info->username, CONTACT_STRING);
			}
				
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
	
}

int merge_modify_password_request_xml(void *pvalue, 
		char *buffer, size_t size)
{	
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	prx_modify_pwd *modify_pwd;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	modify_pwd = (prx_modify_pwd*)pvalue;	
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, MODIFY_PASSWORD_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, USERNAME_STR, 
							 (void*)modify_pwd->username, CONTACT_STRING)))
	{
		create_brother(&tree_node, OLD_PASSWORD_STR, 
						(void*)modify_pwd->old_pwd, CONTACT_STRING);
		create_brother(&tree_node, NEW_PASSWORD_STR, 
						(void*)modify_pwd->new_pwd, CONTACT_STRING);
	}
	else
	{
		printf("create_child(USERNAME_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_modify_password_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_result_info_xml((int*)pvalue, 
			buffer, size, MODIFY_PASSWORD_RESULT_CMD);
}


static __inline__ int 
merge_device_info_xml(prx_device_info *dev_info, 
	char *buffer, size_t size, const char *cmd)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, cmd);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
							 (void*)dev_info->pu_id, CONTACT_STRING)))
	{
		if (!strcmp(ADD_DEVICE_REQUEST_CMD, cmd))
		{
			create_brother(&tree_node, DEVICE_ID_STR, 
						(void*)dev_info->device_id, CONTACT_INT);
		}
		if (!strcmp(GET_DEVICE_INFO_RESULT_CMD, cmd))
		{
			create_brother(&tree_node, RESULT_CODE_STR, 
						(void*)dev_info->result, CONTACT_INT);
		}
		create_brother(&tree_node, DEVICE_TYPE_STR, 
						(void*)dev_info->pu_type, CONTACT_INT);
		create_brother(&tree_node, PROTOCOL_STR, 
						(void*)dev_info->protocol, CONTACT_INT);
		create_brother(&tree_node, FACTORY_NAME_STR, 
						(void*)dev_info->factory, CONTACT_STRING);
		create_brother(&tree_node, SDK_VERSION_STR, 
						(void*)dev_info->sdk_version, CONTACT_STRING);
		create_brother(&tree_node, USERNAME_STR, 
						(void*)dev_info->username, CONTACT_STRING);
		create_brother(&tree_node, PASSWORD_STR, 
						(void*)dev_info->password, CONTACT_STRING);
		create_brother(&tree_node, DEVICE_IP_STR, 
						(void*)dev_info->device_ip, CONTACT_STRING);
		create_brother(&tree_node, DEVICE_PORT_STR, 
						(void*)dev_info->device_port, CONTACT_INT);
		create_brother(&tree_node, PLATFORM_IP_STR, 
						(void*)dev_info->platform_ip, CONTACT_STRING);
		create_brother(&tree_node, PLATFORM_PORT_STR, 
						(void*)dev_info->platform_port, CONTACT_INT);
	}
	else
	{
		printf("create_child(FACTORY_NAME_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
static __inline__ int
merge_device_id_xml(int *device_id, 
	char *buffer, size_t size, const char *cmd)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, cmd);
	if (NULL != (tree_node = create_child(tree_root, DEVICE_ID_STR, 
							 (void*)*((int*)device_id), CONTACT_INT)))
	{
	}
	else
	{
		printf("create_child(DEVICE_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_proxy_device_list_xml(void *pvalue, 
		char *buffer, size_t size)
{
	int index;
	int ret = -1;

	struct _prx_device_st *dev_list;
	struct _prx_device_info *dev_info;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	dev_list = (prx_device_st*)pvalue;
	
	tree_root = create_xml_head(xml_tree, DEVICE_LIST_INFO_CMD);
	for (index=0; index<dev_list->count; index++)
	{
		dev_info = &dev_list->device_info[index];

		if (!index)
		{
			tree_node = create_child(tree_root, DEVICE_STR, 
									 NULL, CONTACT_NULL);
		}
		else
		{
			create_brother(&tree_node, DEVICE_STR, 
							NULL, CONTACT_NULL);
		}
			create_attr(&tree_node, ID_STR, 
				(void*)dev_info->device_id, CONTACT_INT);

		if (tree_node)
		{
			if (NULL != (tree_child = create_child(tree_node, PU_ID_STR, 
									 (void*)dev_info->pu_id, CONTACT_STRING)))
			{
				create_brother(&tree_child, DEVICE_TYPE_STR, 
								(void*)dev_info->pu_type, CONTACT_INT);
				create_brother(&tree_child, PROTOCOL_STR, 
								(void*)dev_info->protocol, CONTACT_INT);
				create_brother(&tree_child, FACTORY_NAME_STR, 
								(void*)dev_info->factory, CONTACT_STRING);
				create_brother(&tree_child, SDK_VERSION_STR, 
								(void*)dev_info->sdk_version, CONTACT_STRING);
				create_brother(&tree_child, USERNAME_STR, 
								(void*)dev_info->username, CONTACT_STRING);
				create_brother(&tree_child, PASSWORD_STR, 
								(void*)dev_info->password, CONTACT_STRING);
				create_brother(&tree_child, DEVICE_IP_STR, 
								(void*)dev_info->device_ip, CONTACT_STRING);
				create_brother(&tree_child, DEVICE_PORT_STR, 
								(void*)dev_info->device_port, CONTACT_INT);
				create_brother(&tree_child, PLATFORM_IP_STR, 
								(void*)dev_info->platform_ip, CONTACT_STRING);
				create_brother(&tree_child, PLATFORM_PORT_STR, 
								(void*)dev_info->platform_port, CONTACT_INT);
			}
			else
			{
				printf("create_child(PU_ID_STR) failed.\n");
				xml_tree_delete(xml_tree);
				return -1;
			}
		}
		else
		{
			printf("create_child(DEVICE_STR) failed.\n");
			xml_tree_delete(xml_tree);
			return -1;
		}
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_add_device_request_xml(void *pvalue, 
		char *buffer, size_t size)
{	
	prx_device_info *dev_info;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	dev_info = (prx_device_info*)pvalue;	
	
	return merge_device_info_xml(dev_info, buffer, 
			size, ADD_DEVICE_REQUEST_CMD);
}

int merge_add_device_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_result_info_xml((int*)pvalue, 
			buffer, size, ADD_DEVICE_RESULT_CMD);
}
int merge_del_device_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_device_id_xml((int*)pvalue, 
			buffer, size, DEL_DEVICE_REQUEST_CMD);
}
int merge_del_device_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_result_info_xml((int*)pvalue, 
			buffer, size, DEL_DEVICE_RESULT_CMD);
}
static __inline__ int 
merge_proxy_page_device_info_xml(struct _prx_page_device *page_dev, 
	char *buffer, size_t size, const char *cmd)
{
	int ret;
	int index;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	struct _prx_device_info *dev_info;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);

	if (page_dev->count)
		ret = 0;
	else
		ret = -1;
	
	tree_root = create_xml_head(xml_tree, cmd);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
							 (void*)ret, CONTACT_INT)))
	{
		if (!ret)
		{
			create_brother(&tree_node, CONDITION_STR, NULL, CONTACT_NULL);
			tree_child = create_child(tree_node, FACTORY_NAME_STR, 
							 (void*)page_dev->factory_name, CONTACT_STRING);

			if (tree_child)
			{
				create_brother(&tree_child, DEVICE_TYPE_STR, 
								(void*)page_dev->machine_type, CONTACT_INT);
				create_brother(&tree_child, SDK_VERSION_STR, 
								(void*)page_dev->sdk_version, CONTACT_STRING);
				create_brother(&tree_child, DEVICE_ID_STR, 
								(void*)page_dev->dev_id, CONTACT_INT);
			}
			
			create_brother(&tree_node, TOTAL_COUNT_STR, 
							(void*)page_dev->total, CONTACT_INT);
			create_brother(&tree_node, OFFSET_STR, 
							(void*)page_dev->offset, CONTACT_INT);
			
			create_brother(&tree_node, DEVICE_LIST_STR, NULL, CONTACT_NULL);
			create_attr(&tree_node, COUNT_STR, 
					(void*)page_dev->count, CONTACT_INT);

			for (index=0; index<page_dev->count; index++)
			{
				dev_info = &page_dev->dev_info[index];
				
				if (!index)
				{
					tree_child = create_child(tree_node, DEVICE_STR, NULL, CONTACT_NULL);
					create_attr(&tree_child, ID_STR, 
							(void*)dev_info->device_id, CONTACT_INT);
				}
				else
				{
					create_brother(&tree_child, DEVICE_STR, NULL, CONTACT_NULL);
					create_attr(&tree_child, ID_STR, 
							(void*)dev_info->device_id, CONTACT_INT);
				}
				
				if (tree_child)
				{
					tree_son = create_child(tree_child, PU_ID_STR, 
								(void*)dev_info->pu_id, CONTACT_STRING);
					create_brother(&tree_son, DEVICE_STATUS_STR, 
									(void*)dev_info->device_st, CONTACT_INT);
					create_brother(&tree_son, ERROR_CODE_STR, 
									(void*)dev_info->device_err, CONTACT_INT);
					create_brother(&tree_son, DEVICE_TYPE_STR, 
									(void*)dev_info->pu_type, CONTACT_INT);
					create_brother(&tree_son, PROTOCOL_STR, 
									(void*)dev_info->protocol, CONTACT_INT);
					create_brother(&tree_son, FACTORY_NAME_STR, 
									(void*)dev_info->factory, CONTACT_STRING);
					create_brother(&tree_son, SDK_VERSION_STR, 
									(void*)dev_info->sdk_version, CONTACT_STRING);
					create_brother(&tree_son, USERNAME_STR, 
									(void*)dev_info->username, CONTACT_STRING);
					create_brother(&tree_son, PASSWORD_STR, 
									(void*)dev_info->password, CONTACT_STRING);
					create_brother(&tree_son, DEVICE_IP_STR, 
									(void*)dev_info->device_ip, CONTACT_STRING);
					create_brother(&tree_son, DEVICE_PORT_STR, 
									(void*)dev_info->device_port, CONTACT_INT);
					create_brother(&tree_son, PLATFORM_IP_STR, 
									(void*)dev_info->platform_ip, CONTACT_STRING);
					create_brother(&tree_son, PLATFORM_PORT_STR, 
									(void*)dev_info->platform_port, CONTACT_INT);
				}
			}
		}
	}
	else
	{
		printf("create_child() failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;	
}

int merge_get_device_info_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	struct _prx_page_device *page_dev;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	page_dev = (struct _prx_page_device*)pvalue;
	
	return merge_proxy_page_device_info_xml(page_dev, 
			buffer, size, GET_DEVICE_INFO_REQUEST_CMD);
}
int merge_get_device_info_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	struct _prx_page_device *page_dev;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	page_dev = (struct _prx_page_device*)pvalue;
	
	return merge_proxy_page_device_info_xml(page_dev, 
			buffer, size, GET_DEVICE_INFO_RESULT_CMD);
}
int merge_set_device_info_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	prx_device_info *dev_info = (prx_device_info*)pvalue;
	
	return merge_device_info_xml(dev_info, buffer, 
			size, SET_DEVICE_INFO_REQUEST_CMD);
}
int merge_set_device_info_result_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_result_info_xml((int*)pvalue, 
			buffer, size, SET_DEVICE_INFO_RESULT_CMD);
}
int merge_get_all_device_id_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
//	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, GET_ALL_DEVICE_ID_REQUEST_CMD);
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_all_device_id_result_xml(void *pvalue, 
		char *buffer, size_t size)
{

  
	int ret = -1;
	int index = 0;
	struct _prx_device_id_st *id_list;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	id_list = (struct _prx_device_id_st*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, GET_ALL_DEVICE_ID_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, DEVICE_ID_LIST_STR, 
							 NULL, CONTACT_NULL)))
	{
		create_attr(&tree_node, COUNT_STR, (void*)id_list->count, CONTACT_INT);

		while (index < id_list->count)
		{
			if (!index)
			{
				tree_child = create_child(tree_node, DEVICE_ID_STR, 
								(void*)id_list->device_id[index++], CONTACT_INT);
			}
			else
			{
				create_brother(&tree_child, DEVICE_ID_STR, 
								(void*)id_list->device_id[index++], CONTACT_INT);
			}
		}
	}
	else
	{
		printf("create_child(DEVICE_ID_LIST_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

static __inline__ int 
merge_proxy_factory_info_xml(prx_factory_list *fct_list, 
	char *buffer, size_t size, const char *cmd)
{
	int index;
	int ret = -1;
	int i, j;
	struct _prx_factory_info *fct_info;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	
	if (NULL == fct_list)
	{
		printf("fct_list NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
		
	tree_root = create_xml_head(xml_tree, cmd);
	for (index=0; index<fct_list->count; index++)
	{
		fct_info = &fct_list->factory[index];

		if (!index)
		{
			tree_node = create_child(tree_root, FACTORY_STR, 
									 NULL, CONTACT_NULL);
		}
		else
		{
			create_brother(&tree_node, FACTORY_STR, 
							NULL, CONTACT_NULL);
		}

		if (tree_node)
		{
			if (NULL != (tree_child = create_child(tree_node, FACTORY_NAME_STR, 
									 (void*)fct_info->factory_name, CONTACT_STRING)))
			{				
				create_brother(&tree_child, MACHINE_LIST_STR, 
								NULL, CONTACT_NULL);
				create_attr(&tree_child, COUNT_STR, 
					(void*)fct_info->type_count, CONTACT_INT);
				
				for (i=0; i<fct_info->type_count; i++)
				{
					if (!i)
					{
						tree_son = create_child(tree_child, DEVICE_TYPE_STR, 
							 (void*)fct_info->type[i], 
							 CONTACT_INT);
					}
					else
					{
						create_brother(&tree_son, DEVICE_TYPE_STR, 
							 (void*)fct_info->type[i], 
							 CONTACT_INT);
					}
					
					create_brother(&tree_son, VERSION_LIST_STR, 
								NULL, CONTACT_NULL);
					create_attr(&tree_son, COUNT_STR, 
						(void*)fct_info->ver_count[i], CONTACT_INT);
					
					for (j=0; j<fct_info->ver_count[i]; j++)
					{
						if (!j)
						{
							tree_grandson = create_child(tree_son, SDK_VERSION_STR, 
								 (void*)fct_info->sdk_version[i][j], 
								 CONTACT_STRING);
						}
						else
						{
							create_brother(&tree_grandson, SDK_VERSION_STR, 
								 (void*)fct_info->sdk_version[i][j], 
								 CONTACT_STRING);
						}
					}
				}

			}
			else
			{
				printf("create_child(FACTORY_NAME_STR) failed.\n");
				xml_tree_delete(xml_tree);
				return -1;
			}
		}
		else
		{
			printf("create_child(FACTORY_STR) failed.\n");
			xml_tree_delete(xml_tree);
			return -1;
		}
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_proxy_factory_list_xml(void *pvalue, 
		char *buffer, size_t size)
{
	prx_factory_list *factory_list;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	factory_list = (prx_factory_list*)pvalue;
	
	return merge_proxy_factory_info_xml(factory_list, 
			buffer, size, FACTORY_LIST_INFO_CMD);
}

int merge_get_factory_info_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	return 0;
}
int merge_set_factory_info_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	return 0;
}

int merge_get_factory_info_response_xml(void *pvalue, 
		char *buffer, size_t size)
{
	prx_factory_list *factory_list;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	factory_list = (prx_factory_list*)pvalue;
	
	return merge_proxy_factory_info_xml(factory_list, 
			buffer, size, GET_FACTORY_RESPONSE_CMD);
}
int merge_set_factory_info_response_xml(void *pvalue, 
		char *buffer, size_t size)
{
	return 0;
}

int merge_proxy_page_user_response_xml(void *pvalue, char *buffer, size_t size)
{
	int ret;
	int index;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	struct _prx_page_user *page_user;
	struct __UserInfo *user_info;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	page_user = (struct _prx_page_user*)pvalue;
	
	if (page_user->count)
		ret = 0;
	else
		ret = -1;
	
	tree_root = create_xml_head(xml_tree, GET_USER_INFO_RESPONSE_CMD);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
							 (void*)ret, CONTACT_INT)))
	{
		if (!ret)
		{
			create_brother(&tree_node, USERNAME_STR, 
							(void*)page_user->username, CONTACT_STRING);
			create_brother(&tree_node, TOTAL_COUNT_STR, 
							(void*)page_user->total, CONTACT_INT);
			create_brother(&tree_node, OFFSET_STR, 
							(void*)page_user->offset, CONTACT_INT);
			
			create_brother(&tree_node, USER_LIST_STR, NULL, CONTACT_NULL);
			create_attr(&tree_node, COUNT_STR, 
					(void*)page_user->count, CONTACT_INT);

			for (index=0; index<page_user->count; index++)
			{
				user_info = &page_user->user_info[index];
				if (!index)
					tree_child = create_child(tree_node, USERNAME_STR, 
							 (void*)user_info->username, CONTACT_STRING);
				else
					create_brother(&tree_child, USERNAME_STR, 
							(void*)user_info->username, CONTACT_STRING);
			}
		}
	}
	else
	{
		printf("create_child() failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

static __inline__ int 
merge_broadcast_user_xml(void *username, 
		char *buffer, size_t size, const char *cmd)
{
	int ret;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
//	XmlTreeNode *tree_child = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, cmd);
	if (NULL != (tree_node = create_child(tree_root, USERNAME_STR, 
							 (void*)username, CONTACT_STRING)))
	{
	}
	else
	{
		printf("create_child() failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_broadcast_add_user_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_broadcast_user_xml(pvalue, buffer, 
			size, BROADCAST_ADD_USER_CMD);
}
int merge_broadcast_del_user_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_broadcast_user_xml(pvalue, buffer, 
			size, BROADCAST_DEL_USER_CMD);
}
int merge_broadcast_add_device_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_device_id_xml((int*)pvalue, 
			buffer, size, BROADCAST_ADD_DEVICE_CMD);
}
int merge_broadcast_del_device_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_device_id_xml((int*)pvalue, 
			buffer, size, BROADCAST_DEL_DEVICE_CMD);
}
int merge_broadcast_modify_device_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_device_id_xml((int*)pvalue, 
			buffer, size, BROADCAST_MODIFY_DEVICE_CMD);
}
int merge_broadcast_device_status_xml(void *pvalue, char *buffer, size_t size)
{
	int ret;
	prx_device_state *dev_state;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	dev_state = (prx_device_state*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, BROADCAST_DEVICE_STATUS_CMD);
	if (NULL != (tree_node = create_child(tree_root, DEVICE_STR, NULL, CONTACT_NULL)))
	{
		create_attr(&tree_node, ID_STR, 
			(void*)dev_state->dev_id, CONTACT_INT);
		tree_child = create_child(tree_node, DEVICE_STATUS_STR, 
						(void*)dev_state->dev_state, CONTACT_INT);
		create_brother(&tree_child, ERROR_CODE_STR, 
			(void*)dev_state->err_code, CONTACT_INT);
	}
	else
	{
		printf("create_child(DEVICE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

//##########################################################################
static int merge_server_config_xml(prx_server_config *srv_cfg, 
		char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	if (NULL == srv_cfg)
	{
		printf("srv_cfg NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (!strcmp(command, GET_SERVER_CONFIG_RESPONSE_CMD))
	{
		if (!srv_cfg->listen_port || !srv_cfg->rtsp_port)
			ret = -1;
		else
			ret = 0;
		tree_node = create_child(tree_root, RESULT_CODE_STR, 
						(void*)ret, CONTACT_INT);
	}
	if (!tree_node)
		tree_node = create_child(tree_root, SERVER_CONFIG_STR, 
						NULL, CONTACT_NULL);
	else
		create_brother(&tree_node, SERVER_CONFIG_STR, 
						NULL, CONTACT_NULL);
	
	if (NULL != tree_node)
	{
		tree_child = create_child(tree_node, SERVER_IP_STR, 
						(void*)srv_cfg->server_ip, CONTACT_STRING);
		if (NULL != tree_child)
		{
			create_brother(&tree_child, LISTEN_PORT_STR, 
				(void*)srv_cfg->listen_port, CONTACT_INT);
			create_brother(&tree_child, RTSP_PORT_STR, 
				(void*)srv_cfg->rtsp_port, CONTACT_INT);
		}
	}
	else
	{
		printf("create_child(SERVER_CONFIG_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_server_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	prx_server_config *srv_cfg;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	srv_cfg = (prx_server_config*)pvalue;
	
	return merge_server_config_xml(srv_cfg, buffer, size, GET_SERVER_CONFIG_REQUEST_CMD);
}
int merge_get_server_config_response_xml(void *pvalue, char *buffer, size_t size)
{
	prx_server_config *srv_cfg;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	srv_cfg = (prx_server_config*)pvalue;
	
	return merge_server_config_xml(srv_cfg, buffer, size, GET_SERVER_CONFIG_RESPONSE_CMD);
}
int merge_set_server_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	prx_server_config *srv_cfg;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	srv_cfg = (prx_server_config*)pvalue;
	
	return merge_server_config_xml(srv_cfg, buffer, size, SET_SERVER_CONFIG_REQUEST_CMD);
}
int merge_set_server_config_result_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_result_info_xml((int*)pvalue, buffer, size, SET_SERVER_CONFIG_RESULT_CMD);
}

int merge_download_request_xml(void *pvalue, char *buffer, size_t size)
{
    int ret;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;

	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, DOWNLOAD_DATA_REQUEST_CMD);
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_download_response_xml(void *pvalue, char *buffer, size_t size)
{
    int ret;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	prx_backup *backup = (prx_backup*)pvalue;

	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, DOWNLOAD_DATA_RESPONSE_CMD);
	tree_node = create_child(tree_root, RESULT_CODE_STR, 
					(void*)backup->result, CONTACT_INT);
	if (tree_node && !backup->result)
	{
		create_brother(&tree_node, BACKUP_MAGIC_STR, 
			(void*)backup->magic, CONTACT_INT);
		create_brother(&tree_node, BACKUP_PORT_STR, 
			(void*)backup->port, CONTACT_INT);
		create_brother(&tree_node, BACKUP_SIZE_STR, 
			(void*)backup->size, CONTACT_INT);
	}
    
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_upload_request_xml(void *pvalue, char *buffer, size_t size)
{
    int ret;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	prx_backup *backup = (prx_backup*)pvalue;

	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, UPLOAD_DATA_REQUEST_CMD);
	tree_node = create_child(tree_root, BACKUP_SIZE_STR, 
					(void*)backup->size, CONTACT_INT);
    
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_upload_response_xml(void *pvalue, char *buffer, size_t size)
{
    int ret;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	prx_backup *backup = (prx_backup*)pvalue;

	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);

	tree_root = create_xml_head(xml_tree, UPLOAD_DATA_RESPONSE_CMD);
	tree_node = create_child(tree_root, RESULT_CODE_STR, 
					(void*)backup->result, CONTACT_INT);
	if (tree_node && !backup->result)
	{
		create_brother(&tree_node, BACKUP_MAGIC_STR, 
			(void*)backup->magic, CONTACT_INT);
		create_brother(&tree_node, BACKUP_PORT_STR, 
			(void*)backup->port, CONTACT_INT);
	}

	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

# endif 

//##########################################################################
static int merge_channel_info_xml(ChannelInfoPacket *ch_info, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	if (NULL == ch_info)
	{
		printf("ch_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)ch_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)ch_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)ch_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
						(void*)ch_info->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, CHANNEL_NO_STR, 
						(void*)ch_info->ch_no, CONTACT_INT);
		create_brother(&tree_node, CHANNEL_TYPE_STR, 
						(void*)ch_info->ch_type, CONTACT_INT);
		create_brother(&tree_node, CHANNEL_STATUS_STR, 
						(void*)ch_info->ch_status, CONTACT_INT);
		create_brother(&tree_node, CHANNEL_NAME_STR, 
						(void*)ch_info->ch_name, CONTACT_STRING);
		
		create_brother(&tree_node, REMOTE_CHANNEL_INFO_STR, NULL, CONTACT_NULL);
		tree_child = create_child(tree_node, CHANNEL_NO_STR, 
						(void*)ch_info->rmt_ch_info.ch_no, CONTACT_INT);
		if (tree_child)
		{
			create_brother(&tree_child, PROTOCOL_STR, 
				(void*)ch_info->rmt_ch_info.protocol, CONTACT_INT);
			create_brother(&tree_child, AUDIO_ENABLE_STR, 
				(void*)ch_info->rmt_ch_info.audio_enable, CONTACT_INT);
			create_brother(&tree_child, USERNAME_STR, 
				(void*)ch_info->rmt_ch_info.user_name, CONTACT_STRING);
			create_brother(&tree_child, PASSWORD_STR, 
				(void*)ch_info->rmt_ch_info.user_pwd, CONTACT_STRING);
			create_brother(&tree_child, WINDOW_MODE_STR, 
				(void*)ch_info->rmt_ch_info.win_mode, CONTACT_INT);
			create_brother(&tree_child, WIN_MAX_STREAM_STR, 
				(void*)ch_info->rmt_ch_info.win_max_strm, CONTACT_INT);
			create_brother(&tree_child, WIN_MIN_STREAM_STR, 
				(void*)ch_info->rmt_ch_info.win_min_strm, CONTACT_INT);
		}
		
		create_brother(&tree_node, REMOTE_DEVICE_INFO_STR, NULL, CONTACT_NULL);
		tree_child = create_child(tree_node, IP_STR, 
						(void*)ch_info->rmt_dev_info.ip, CONTACT_STRING);
		if (tree_child)
		{
			create_brother(&tree_child, PORT_STR, 
				(void*)ch_info->rmt_dev_info.port, CONTACT_INT);
			create_brother(&tree_child, DEV_TYPE_STR, 
				(void*)ch_info->rmt_dev_info.dev_type, CONTACT_INT);
			create_brother(&tree_child, CHANNEL_SUM_STR, 
				(void*)ch_info->rmt_dev_info.ch_sum, CONTACT_INT);
			create_brother(&tree_child, DNS_ENABLE_STR, 
				(void*)ch_info->rmt_dev_info.dns_enable, CONTACT_INT);
			create_brother(&tree_child, DNS_STR, 
				(void*)ch_info->rmt_dev_info.dns, CONTACT_STRING);
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_channel_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_CHANNEL_INFO_REQUEST_CMD);
}
int merge_get_channel_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	ChannelInfoPacket *ch_info = (ChannelInfoPacket*)pvalue;
	
	return merge_channel_info_xml(ch_info, buffer, size, GET_CHANNEL_INFO_RESPONSE_CMD);
}
int merge_set_channel_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	ChannelInfoPacket *ch_info = (ChannelInfoPacket*)pvalue;
	
	return merge_channel_info_xml(ch_info, buffer, size, SET_CHANNEL_INFO_REQUEST_CMD);
}
int merge_set_channel_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_CHANNEL_INFO_RESULT_CMD);
}

//##########################################################################
static int merge_picture_info_xml(PictureInfoPacket *pic_info, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pic_info)
	{
		printf("pic_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
			(void*)pic_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)pic_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)pic_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)pic_info->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, MIRROR_STR, 
			(void*)pic_info->mirror, CONTACT_INT);
		create_brother(&tree_node, FLIP_STR, 
			(void*)pic_info->flip, CONTACT_INT);
		create_brother(&tree_node, HZ_STR, 
			(void*)pic_info->hz, CONTACT_INT);
		create_brother(&tree_node, AWB_MODE_STR, 
			(void*)pic_info->awb_mode, CONTACT_INT);
		create_brother(&tree_node, AWB_RED_STR, 
			(void*)pic_info->awb_red, CONTACT_INT);
		create_brother(&tree_node, AWB_BULE_STR, 
			(void*)pic_info->awb_blue, CONTACT_INT);
		create_brother(&tree_node, WDR_STR, 
			(void*)pic_info->wdr, CONTACT_INT);
		create_brother(&tree_node, IRIS_TYPE_STR, 
			(void*)pic_info->iris_type, CONTACT_INT);
		create_brother(&tree_node, EXP_COMENSATION_STR, 
			(void*)pic_info->exp_compensation, CONTACT_INT);
		create_brother(&tree_node, AE_MODE_STR, 
			(void*)pic_info->ae_mode, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_picture_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_PICTURE_INFO_REQUEST_CMD);
}
int merge_get_picture_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	PictureInfoPacket *pic_info = (PictureInfoPacket*)pvalue;
	
	return merge_picture_info_xml(pic_info, buffer, size, GET_PICTURE_INFO_RESPONSE_CMD);
}
int merge_set_picture_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	PictureInfoPacket *pic_info = (PictureInfoPacket*)pvalue;
	
	return merge_picture_info_xml(pic_info, buffer, size, SET_PICTURE_INFO_REQUEST_CMD);
}
int merge_set_picture_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_PICTURE_INFO_RESULT_CMD);
}

//##########################################################################
/*static int merge_hl_picture_info_xml(HLPictureInfoPacket *hl_pic_info, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	if (NULL == hl_pic_info)
	{
		printf("hl_pic_info NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)hl_pic_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)hl_pic_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)hl_pic_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)hl_pic_info->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, DEFECT_PIX_STR, 
			(void*)hl_pic_info->defect_pix, CONTACT_INT);
		create_brother(&tree_node, WDR_STR, 
			(void*)hl_pic_info->wdr, CONTACT_INT);
		create_brother(&tree_node, _2d_DENOISE_STR, 
			(void*)hl_pic_info->_2d_denoise, CONTACT_INT);
		create_brother(&tree_node, _3d_DENOISE_STR, 
			(void*)hl_pic_info->_3d_denoise, CONTACT_INT);
		create_brother(&tree_node, _3d_SF_CONS_STH1_STR, 
			(void*)hl_pic_info->_3d_fcos_sth1, CONTACT_INT);
		create_brother(&tree_node, _3d_SF_CONS_STH2_STR, 
			(void*)hl_pic_info->_3d_fcos_sth2, CONTACT_INT);
		create_brother(&tree_node, _3d_TF_STH_STR, 
			(void*)hl_pic_info->_3d_tf_sth, CONTACT_INT);
		create_brother(&tree_node, AWB_MODE_STR, 
			(void*)hl_pic_info->awb_mode, CONTACT_INT);
		create_brother(&tree_node, AWB_RED_STR, 
			(void*)hl_pic_info->awb_red, CONTACT_INT);
		create_brother(&tree_node, AWB_BULE_STR, 
			(void*)hl_pic_info->awb_blue, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_hl_picture_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_HL_PICTURE_INFO_REQUEST_CMD);
}
int merge_get_hl_picture_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	HLPictureInfoPacket *hl_pic_info = (HLPictureInfoPacket*)pvalue;
	
	return merge_hl_picture_info_xml(hl_pic_info, buffer, size, GET_HL_PICTURE_INFO_RESPONSE_CMD);
}
int merge_set_hl_picture_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	HLPictureInfoPacket *hl_pic_info = (HLPictureInfoPacket*)pvalue;
	
	return merge_hl_picture_info_xml(hl_pic_info, buffer, size, SET_HL_PICTURE_INFO_REQUEST_CMD);
}
int merge_set_hl_picture_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_HL_PICTURE_INFO_RESULT_CMD);
}*/

//##########################################################################
static int merge_wifi_config_xml(WifiConfigPacket *wifi_cfg, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == wifi_cfg)
	{
		printf("wifi_cfg NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)wifi_cfg->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)wifi_cfg->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)wifi_cfg->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)wifi_cfg->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, WIFI_ENABLE_STR, 
			(void*)wifi_cfg->wifi_enable, CONTACT_INT);
		create_brother(&tree_node, ESSID_STR, 
			(void*)wifi_cfg->essid, CONTACT_STRING);
		create_brother(&tree_node, PASSWORD_STR, 
			(void*)wifi_cfg->pwd, CONTACT_STRING);
		create_brother(&tree_node, ENCRYPT_TYPE_STR, 
			(void*)wifi_cfg->encrypt_type, CONTACT_INT);
		create_brother(&tree_node, AUTH_MODE_STR, 
			(void*)wifi_cfg->auth_mode, CONTACT_INT);
		create_brother(&tree_node, SECRET_KEY_TYPE_STR, 
			(void*)wifi_cfg->secret_key_type, CONTACT_INT);
		create_brother(&tree_node, WIFI_STATUS_STR, 
			(void*)wifi_cfg->wifi_st, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_wifi_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_WIFI_CONFIG_REQUEST_CMD);
}
int merge_get_wifi_config_response_xml(void *pvalue, char *buffer, size_t size)
{
	WifiConfigPacket *wifi_cfg = (WifiConfigPacket*)pvalue;
	
	return merge_wifi_config_xml(wifi_cfg, buffer, size, GET_WIFI_CONFIG_RESPONSE_CMD);
}
int merge_set_wifi_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	WifiConfigPacket *wifi_cfg = (WifiConfigPacket*)pvalue;
	
	return merge_wifi_config_xml(wifi_cfg, buffer, size, SET_WIFI_CONFIG_REQUEST_CMD);
}
int merge_set_wifi_config_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_WIFI_CONFIG_RESULT_CMD);
}

//##########################################################################
static int merge_wifi_search_xml(WifiSearchResPacket *wifi_cfg, 
				char *buffer, size_t size, const char *command)
{
	int i, ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	if (NULL == wifi_cfg)
	{
		printf("wifi_cfg NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)wifi_cfg->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)wifi_cfg->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)wifi_cfg->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)wifi_cfg->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, WIFI_AP_INFO_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)wifi_cfg->count, CONTACT_INT);

		for (i=0; i<wifi_cfg->count; i++)
		{
			if (!i)
				tree_child = create_child(tree_node, ACCESS_POINT_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, ACCESS_POINT_STR, NULL, CONTACT_NULL);

			if (tree_child)
			{
				tree_son = create_child(tree_child, ESSID_STR, 
							(void*)wifi_cfg->wifi_ap[i].essid, CONTACT_STRING);
				
				create_brother(&tree_son, ENCRYPT_TYPE_STR, 
					(void*)wifi_cfg->wifi_ap[i].encrypt_type, CONTACT_INT);
				create_brother(&tree_son, AUTH_MODE_STR, 
					(void*)wifi_cfg->wifi_ap[i].auth_mode, CONTACT_INT);
				create_brother(&tree_son, SECRET_KEY_TYPE_STR, 
					(void*)wifi_cfg->wifi_ap[i].secret_key_type, CONTACT_INT);
				create_brother(&tree_son, SIGNAL_QUALITY_STR, 
					(void*)wifi_cfg->wifi_ap[i].quality, CONTACT_INT);
				create_brother(&tree_son, BIT_RATE_STR, 
					(void*)wifi_cfg->wifi_ap[i].bit_rate, CONTACT_INT);
			}
		}		
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_wifi_search_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, WIFI_SEARCH_REQUEST_CMD);
}
int merge_wifi_search_response_xml(void *pvalue, char *buffer, size_t size)
{
	WifiSearchResPacket *wifi_search = (WifiSearchResPacket*)pvalue;
	
	return merge_wifi_search_xml(wifi_search, buffer, size, WIFI_SEARCH_RESPONSE_CMD);
}

//##########################################################################
static int merge_network_status_xml(NetworkStatusPacket *net_status, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == net_status)
	{
		printf("net_status NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)net_status->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)net_status->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)net_status->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)net_status->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, ETH_STATUS_STR, 
			(void*)net_status->eth_st, CONTACT_INT);
		create_brother(&tree_node, WIFI_STATUS_STR, 
			(void*)net_status->wifi_st, CONTACT_INT);
		create_brother(&tree_node, PPPOE_STATUS_STR, 
			(void*)net_status->pppoe_st, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_network_status_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_NETWORK_STATUS_REQUEST_CMD);
}
int merge_get_network_status_response_xml(void *pvalue, char *buffer, size_t size)
{
	NetworkStatusPacket *net_status = (NetworkStatusPacket*)pvalue;
	
	return merge_network_status_xml(net_status, buffer, size, GET_NETWORK_STATUS_RESPONSE_CMD);
}
//##########################################################################
int merge_control_device_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	ControlDevicePacket *cntrl_dev;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	cntrl_dev = (ControlDevicePacket*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, CONTROL_DEVICE_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
								(void*)cntrl_dev->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)cntrl_dev->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)cntrl_dev->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, COMMAND_STR, 
			(void*)cntrl_dev->command, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_control_device_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, CONTROL_DEVICE_RESULT_CMD);
}

//##########################################################################
static int merge_ddns_config_xml(DdnsConfigPacket *ddns_cfg, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == ddns_cfg)
	{
		printf("ddns_cfg NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)ddns_cfg->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)ddns_cfg->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)ddns_cfg->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)ddns_cfg->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, DDNS_ACCOUNT_STR, 
			(void*)ddns_cfg->ddns_account, CONTACT_STRING);
		create_brother(&tree_node, USERNAME_STR, 
			(void*)ddns_cfg->ddns_usr, CONTACT_STRING);
		create_brother(&tree_node, PASSWORD_STR, 
			(void*)ddns_cfg->ddns_pwd, CONTACT_STRING);
		create_brother(&tree_node, DDNS_OPNT_STR, 
			(void*)ddns_cfg->ddns_open, CONTACT_INT);
		create_brother(&tree_node, DDNS_TYPE_STR, 
			(void*)ddns_cfg->ddns_type, CONTACT_INT);
		create_brother(&tree_node, DDNS_PORT_STR, 
			(void*)ddns_cfg->ddns_port, CONTACT_INT);
		create_brother(&tree_node, DDNS_TIMES_STR, 
			(void*)ddns_cfg->ddns_times, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_ddns_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_DDNS_CONFIG_REQUEST_CMD);
}
int merge_get_ddns_config_response_xml(void *pvalue, char *buffer, size_t size)
{
	DdnsConfigPacket *ddns_cfg = (DdnsConfigPacket*)pvalue;
	
	return merge_ddns_config_xml(ddns_cfg, buffer, size, GET_DDNS_CONFIG_RESPONSE_CMD);
}
int merge_set_ddns_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	DdnsConfigPacket *ddns_cfg = (DdnsConfigPacket*)pvalue;
	
	return merge_ddns_config_xml(ddns_cfg, buffer, size, SET_DDNS_CONFIG_REQUEST_CMD);
}
int merge_set_ddns_config_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_DDNS_CONFIG_RESULT_CMD);
}

int merge_get_def_display_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_DEF_DISPLAY_INFO_REQUEST_CMD);
}

int merge_get_def_display_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	DisplayParameterPacket *display_para = (DisplayParameterPacket*)pvalue;
	
	return merge_display_parameter_xml(display_para, buffer, size, GET_DEF_DISPLAY_INFO_RESPONSE_CMD);
}
int merge_get_def_picture_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_DEF_PICTURE_INFO_REQUEST_CMD);
}
int merge_get_def_picture_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	PictureInfoPacket *pic_info = (PictureInfoPacket*)pvalue;
	
	return merge_picture_info_xml(pic_info, buffer, size, GET_DEF_PICTURE_INFO_RESPONSE_CMD);
}

//##########################################################################
static int merge_avd_config_xml(AvdConfigPacket *avd_cfg, 
				char *buffer, size_t size, const char *command)
{
	int i, ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	if (NULL == avd_cfg)
	{
		printf("avd_cfg NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)avd_cfg->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)avd_cfg->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)avd_cfg->domain_id, CONTACT_STRING);
		create_brother(&tree_node, PU_ID_STR, 
			(void*)avd_cfg->pu_id, CONTACT_STRING);
		
		create_brother(&tree_node, AVD_ENABLE_STR, 
			(void*)(int)avd_cfg->enable, CONTACT_INT);
		create_brother(&tree_node, AVD_SEGMENT_STR, NULL, CONTACT_NULL);
		for (i=0; i<J_SDK_MAX_SEG_SZIE; i++)
		{
			if (0 ==i)
				tree_child = create_child(tree_node, SEGMENT_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, SEGMENT_STR, NULL, CONTACT_NULL);

			create_attr(&tree_child, SEG_INDEX_STR, (void*)i, CONTACT_INT);
			tree_son = create_child(tree_child, SEG_OPNT_STR, 
							(void*)(int)avd_cfg->sched_time[i].open, CONTACT_INT);
			create_brother(&tree_son, BEGIN_TIMES_STR, 
				(void*)(int)avd_cfg->sched_time[i].begin_sec, CONTACT_INT);
			create_brother(&tree_son, END_TIMES_STR, 
				(void*)(int)avd_cfg->sched_time[i].end_sec, CONTACT_INT);
		}
		
		create_brother(&tree_node, AVD_RULE_STR, NULL, CONTACT_NULL);
		for (i=0; i<MAX_IVS_AVD_RULES; i++)
		{
			if (0 ==i)
				tree_child = create_child(tree_node, RULE_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, RULE_STR, NULL, CONTACT_NULL);

			create_attr(&tree_child, RULE_TYPE_STR, (void*)i, CONTACT_INT);
			tree_son = create_child(tree_child, RULE_ENABLE_STR, 
							(void*)(int)avd_cfg->avd_rule[i].enable, CONTACT_INT);
			create_brother(&tree_son, LEVEL_STR, 
				(void*)(int)avd_cfg->avd_rule[i].level, CONTACT_INT);
			create_brother(&tree_son, ALARM_TIMES_STR, 
				(void*)(int)avd_cfg->avd_rule[i].alarm_times, CONTACT_INT);
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_avd_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_AVD_CONFIG_REQUEST_CMD);
}
int merge_get_avd_config_response_xml(void *pvalue, char *buffer, size_t size)
{
	AvdConfigPacket *avd_cfg = (AvdConfigPacket*)pvalue;
	
	return merge_avd_config_xml(avd_cfg, buffer, size, GET_AVD_CONFIG_RESPONSE_CMD);
}
int merge_set_avd_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	AvdConfigPacket *avd_cfg = (AvdConfigPacket*)pvalue;
	
	return merge_avd_config_xml(avd_cfg, buffer, size, SET_AVD_CONFIG_REQUEST_CMD);
}
int merge_set_avd_config_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_AVD_CONFIG_RESULT_CMD);
}


























static __inline__ int 
merge_transparent_param_xml(TransparentPacket *trans, 
	char *buffer, size_t size, const char *cmd)
{
	int ret;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, cmd);

	if (-1 == trans->type)
		ret = -1;
	else
		ret = 0;
	
	tree_node = create_child(tree_root, RESULT_CODE_STR, 
					(void*)ret, CONTACT_INT);
	
	if (tree_node)
	{
		if (!ret)
		{
			create_brother(&tree_node, SESSION_ID_STR, 
				(void*)trans->session_id, CONTACT_STRING);
			create_brother(&tree_node, DOMAIN_ID_STR, 
				(void*)trans->domain_id, CONTACT_STRING);
			create_brother(&tree_node, PU_ID_STR, 
				(void*)trans->pu_id, CONTACT_STRING);
		
			create_brother(&tree_node, TRANSPARENT_TYPE_STR, 
				(void*)trans->type, CONTACT_INT);
			create_brother(&tree_node, TRANSPARENT_CHANNEL_STR, 
				(void*)trans->channel, CONTACT_INT);		
			create_brother(&tree_node, TRANSPARENT_LENGTH_STR, 
				(void*)trans->length, CONTACT_INT);
			create_brother(&tree_node, TRANSPARENT_DATA_STR, 
				(void*)trans->data, CONTACT_STRING);
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_transparent_param_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	TransparentPacket *trans;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	trans = (TransparentPacket*)pvalue;
	
	return merge_transparent_param_xml(trans, buffer, size, 
			GET_TRANSPARENTPARAM_REQUEST_CMD);
}
int merge_get_transparent_param_response_xml(void *pvalue, 
		char *buffer, size_t size)
{
	TransparentPacket *trans;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	trans = (TransparentPacket*)pvalue;
	
	return merge_transparent_param_xml(trans, buffer, size, 
			GET_TRANSPARENTPARAM_RESPONSE_CMD);
}
int merge_set_transparent_param_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	TransparentPacket *trans;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	trans = (TransparentPacket*)pvalue;
	
	return merge_transparent_param_xml(trans, buffer, size, 
			SET_TRANSPARENTPARAM_REQUEST_CMD);
}
int merge_set_transparent_param_response_xml(void *pvalue, 
		char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_result_info_xml((int*)pvalue, buffer, size, 
			SET_TRANSPARENTPARAM_RESPONSE_CMD);
}
int merge_transparent_notify_enevt_xml(void *pvalue, 
		char *buffer, size_t size)
{
	TransparentPacket *trans;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	trans = (TransparentPacket*)pvalue;
	
	return merge_transparent_param_xml(trans, buffer, size, 
			TRANSPARENTPARAM_NOTIFYEVENT_CMD);
}
int merge_transparent_control_device_xml(void *pvalue, 
		char *buffer, size_t size)
{
	TransparentPacket *trans;
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	trans = (TransparentPacket*)pvalue;
	
	return merge_transparent_param_xml(trans, buffer, size, 
			TRANSPARENTPARAM_CONTROLDEVICE_CMD);
}

//##########################################################################
# ifdef _USE_DECODER_PROTO_
static int merge_division_mode(DivisionModePacket *div_mode, 
				char *buffer, size_t size, const char *command)
{
	int count;
	int i, ret = -1;
	JDivMode *mode;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	if (NULL == div_mode)
	{
		printf("div_mode NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)div_mode->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, TOTAL_COUNT_STR, 
			(void*)div_mode->div_mode.total, CONTACT_INT);
		
		if (J_SDK_MAX_MODE_SIZE > div_mode->div_mode.count)
			count = div_mode->div_mode.count;
		else
			count = J_SDK_MAX_MODE_SIZE;
		
		create_brother(&tree_node, DIVISION_INFO_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)count, CONTACT_INT);
		
		for (i=0; i<count; i++)
		{
			mode = &div_mode->div_mode.mode[i];
			if (0 == i)
				tree_child = create_child(tree_node, DIVISION_MODE_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, DIVISION_MODE_STR, NULL, CONTACT_NULL);
			
			tree_son = create_child(tree_child, DIVISION_ID_STR, 
							(void*)mode->div_id, CONTACT_INT);
			
			create_brother(&tree_son, DIVISION_NAME_STR, 
				(void*)mode->mode_name, CONTACT_STRING);
			create_brother(&tree_son, DESCRIPTION_STR, 
				(void*)mode->description, CONTACT_STRING);
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_query_division_mode_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	DivisionModePacket *div_mode = (DivisionModePacket*)pvalue;

	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, QUERY_DIVISION_MODE_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, PU_ID_STR, 
								(void*)div_mode->pu_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, PAGE_SIZE_STR, 
			(void*)div_mode->div_mode.page_size, CONTACT_INT);
		create_brother(&tree_node, START_ROW_STR, 
			(void*)div_mode->div_mode.start_row, CONTACT_INT);
	}
	else
	{
		printf("create_child(PU_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_query_division_mode_response_xml(void *pvalue, char *buffer, size_t size)
{
	DivisionModePacket *div_mode = (DivisionModePacket*)pvalue;
	
	return merge_division_mode(div_mode, buffer, size, QUERY_DIVISION_MODE_RESPONSE_CMD);
}

static int merge_get_screen_state(ScreenStatePacket *scr_state, 
				char *buffer, size_t size, const char *command)
{
	int count;
	int i, ret = -1;
	JDivisionInfo *division;
	JFullScreenMode *full_screen;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	if (NULL == scr_state)
	{
		printf("scr_state NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)scr_state->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)scr_state->session_id, CONTACT_STRING);
		create_brother(&tree_node, DIVISION_ID_STR, 
			(void*)scr_state->scr_state.div_id, CONTACT_INT);
		
		if (J_SDK_MAX_DIVISION_SIZE > scr_state->scr_state.count)
			count = scr_state->scr_state.count;
		else
			count = J_SDK_MAX_DIVISION_SIZE;
		
		create_brother(&tree_node, SCREEN_IFNO_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)count, CONTACT_INT);
		
		for (i=0; i<count; i++)
		{
			division = &scr_state->scr_state.division[i];
			if (0 == i)
				tree_child = create_child(tree_node, DIVISION_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, DIVISION_STR, NULL, CONTACT_NULL);
			
			tree_son = create_child(tree_child, DIVISION_NO_STR, 
							(void*)division->div_no, CONTACT_INT);
			
			create_brother(&tree_son, ENCODER_NAME_STR, 
				(void*)division->encoder, CONTACT_STRING);
			create_brother(&tree_son, ENCODER_CHANNEL_STR, 
				(void*)(int)division->enc_chn, CONTACT_INT);
			create_brother(&tree_son, LEVEL_STR, 
				(void*)(int)division->level, CONTACT_INT);
			create_brother(&tree_son, ACTION_TYPE_STR, 
				(void*)division->action, CONTACT_INT);
			create_brother(&tree_son, ACTION_RESULT_STR, 
				(void*)division->result, CONTACT_INT);
		}
        
		create_brother(&tree_node, FULL_SCREEN_STATE_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, FULL_SCREEN_MODE_STR, 
            (void*)scr_state->scr_state.fs_mode, CONTACT_INT);
        if (scr_state->scr_state.fs_mode)
        {
            full_screen = &scr_state->scr_state.full_screen;
            tree_child = create_child(tree_node, ENCODER_NAME_STR, 
                            full_screen->encoder, CONTACT_STRING);
            
			create_brother(&tree_child, ENCODER_CHANNEL_STR, 
				(void*)(int)full_screen->enc_chn, CONTACT_INT);
			create_brother(&tree_child, LEVEL_STR, 
				(void*)(int)full_screen->level, CONTACT_INT);
			create_brother(&tree_child, ACTION_TYPE_STR, 
				(void*)full_screen->action, CONTACT_INT);
			create_brother(&tree_child, ACTION_RESULT_STR, 
				(void*)full_screen->result, CONTACT_INT);
        }
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_screen_state_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	ScreenStatePacket *scr_state = (ScreenStatePacket*)pvalue;

	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, GET_SCREEN_STATE_REQUEST_CMD);
	tree_node = create_child(tree_root, SESSION_ID_STR, 
					(void *)scr_state->session_id, CONTACT_STRING);
	if (NULL != tree_node)
	{
		create_brother(&tree_node, DIS_CHANNEL_STR, 
			(void*)scr_state->dis_channel, CONTACT_INT);
	}
	else
	{
		printf("create_child(DIS_CHANNEL_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_screen_state_response_xml(void *pvalue, char *buffer, size_t size)
{
	ScreenStatePacket *scr_state = (ScreenStatePacket*)pvalue;
	
	return merge_get_screen_state(scr_state, buffer, size, GET_SCREEN_STATE_RESPONSE_CMD);
}

/*static int merge_result_code_xml(int result, 
			char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == command)
	{
		printf("command NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
							 (void*)result, CONTACT_INT)))
	{
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
		
	xml_tree_delete(xml_tree);
	return ret;
}*/

int merge_set_division_mode_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	JChangeDMode *cd_mode;
	ChangeDModePacket *packet;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	packet = (ChangeDModePacket*)pvalue;
	cd_mode = &packet->cd_mode;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, SET_DIVISION_MODE_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, DIS_CHANNEL_STR, 
								(void*)cd_mode->dis_chn, CONTACT_INT)))
	{
		create_brother(&tree_node, DIVISION_ID_STR, 
			(void*)cd_mode->div_id, CONTACT_INT);
	}
	else
	{
		printf("create_child(DIS_CHANNEL_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_set_division_mode_response_xml(void *pvalue, char *buffer, size_t size)
{
	return merge_result_info_xml((int*)pvalue, buffer, size, SET_DIVISION_MODE_RESULT_CMD);
}

int merge_set_full_screen_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	JScreen *screen;
	FullScreenPacket *packet;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	packet = (FullScreenPacket*)pvalue;
	screen = &packet->screen;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, SET_FULL_SCREEN_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, DIS_CHANNEL_STR, 
								(void*)screen->dis_chn, CONTACT_INT)))
	{
		create_brother(&tree_node, DIVISION_NO_STR, 
			(void*)screen->div_no, CONTACT_INT);
	}
	else
	{
		printf("create_child(DIS_CHANNEL_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_set_full_screen_response_xml(void *pvalue, char *buffer, size_t size)
{
	return merge_result_info_xml((int*)pvalue, buffer, size, SET_FULL_SCREEN_RESULT_CMD);
}

int merge_exit_full_screen_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, EXIT_FULL_SCREEN_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, DIS_CHANNEL_STR, 
								(void*)*(int*)pvalue, CONTACT_INT)))
	{
	}
	else
	{
		printf("create_child(DIS_CHANNEL_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_exit_full_screen_response_xml(void *pvalue, char *buffer, size_t size)
{
	return merge_result_info_xml((int*)pvalue, buffer, size, EXIT_FULL_SCREEN_RESULT_CMD);
}

static int merge_tv_wall_play(TVWallPlayPacket *tv_wall, 
				char *buffer, size_t size, const char *command)
{
	int count;
	int i, ret = -1;
	JDivisionInfo *division;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	if (NULL == tv_wall)
	{
		printf("tv_wall NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)tv_wall->result.code, CONTACT_INT)))
	{
		if (!strcmp(TV_WALL_PLAY_REQUEST_CMD, command))
		{
			create_brother(&tree_node, ACTION_TYPE_STR, 
				(void*)tv_wall->tv_play.action, CONTACT_INT);
			create_brother(&tree_node, NAME_STR, 
				(void*)tv_wall->tv_play.name, CONTACT_STRING);
			create_brother(&tree_node, STEP_NO_STR, 
				(void*)tv_wall->tv_play.step, CONTACT_INT);
			create_brother(&tree_node, DIS_CHANNEL_STR, 
				(void*)tv_wall->tv_play.dis_chn, CONTACT_INT);
			create_brother(&tree_node, KEEP_OTHER_STR, 
				(void*)tv_wall->tv_play.k_other, CONTACT_INT);
		}
		create_brother(&tree_node, DIVISION_ID_STR, 
			(void*)tv_wall->tv_play.div_id, CONTACT_INT);
		
		if (J_SDK_MAX_DIVISION_SIZE > tv_wall->tv_play.count)
			count = tv_wall->tv_play.count;
		else
			count = J_SDK_MAX_DIVISION_SIZE;
		
		create_brother(&tree_node, DIVISIONS_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)count, CONTACT_INT);
		
		for (i=0; i<count; i++)
		{
			division = &tv_wall->tv_play.division[i];
			if (0 == i)
				tree_child = create_child(tree_node, DIVISION_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, DIVISION_STR, NULL, CONTACT_NULL);
			
			tree_son = create_child(tree_child, DIVISION_NO_STR, 
							(void*)division->div_no, CONTACT_INT);
			
			create_brother(&tree_son, ENCODER_NAME_STR, 
				(void*)division->encoder, CONTACT_STRING);
			if (!strcmp(TV_WALL_PLAY_REQUEST_CMD, command))
			{
				create_brother(&tree_son, ENCODER_URL_STR, 
					(void*)division->url, CONTACT_STRING);
				create_brother(&tree_son, CLEAR_FLAG_STR, 
					(void*)division->flag, CONTACT_INT);
			}
			else
			{
				create_brother(&tree_son, ENCODER_CHANNEL_STR, 
					(void*)(int)division->enc_chn, CONTACT_INT);
				create_brother(&tree_son, LEVEL_STR, 
					(void*)(int)division->level, CONTACT_INT);
				create_brother(&tree_son, ACTION_RESULT_STR, 
					(void*)division->result, CONTACT_INT);
			}
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_tv_wall_play_request_xml(void *pvalue, char *buffer, size_t size)
{
	TVWallPlayPacket *tv_wall = (TVWallPlayPacket*)pvalue;
	
	return merge_tv_wall_play(tv_wall, buffer, size, TV_WALL_PLAY_REQUEST_CMD);
}
int merge_tv_wall_play_response_xml(void *pvalue, char *buffer, size_t size)
{
	TVWallPlayPacket *tv_wall = (TVWallPlayPacket*)pvalue;
	
	return merge_tv_wall_play(tv_wall, buffer, size, TV_WALL_PLAY_RESULT_CMD);
}

int merge_clear_division_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	JScreen *screen;
	ClearDivisionPacket *packet;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	packet = (ClearDivisionPacket*)pvalue;
	screen = &packet->screen;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, CLEAR_DIVISION_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, DIS_CHANNEL_STR, 
								(void*)screen->dis_chn, CONTACT_INT)))
	{
		create_brother(&tree_node, DIVISION_NO_STR, 
			(void*)screen->div_no, CONTACT_INT);
	}
	else
	{
		printf("create_child(DIS_CHANNEL_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_clear_division_response_xml(void *pvalue, char *buffer, size_t size)
{
	return merge_result_info_xml((int*)pvalue, buffer, size, CLEAR_DIVISION_RESULT_CMD);
}

# endif //_USE_DECODER_PROTO_


static __inline__ int 
merge_log_info(OperationLogPacket *log, char *buffer, 
	size_t size, const char *cmd)
{
	int i, ret = -1;
	JOperationLogItem *item;
    
  	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);

	tree_root = create_xml_head(xml_tree, cmd);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)log->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, TOTAL_COUNT_STR, 
			(void *)log->opt_log.total_count, CONTACT_INT);
		create_brother(&tree_node, LOG_INFO_STR, (void*)NULL, CONTACT_NULL);
		create_attr(&tree_node, NODE_COUNT_STR, 
			(void*)log->opt_log.node_count, CONTACT_INT);

		for (i=0; i<log->opt_log.node_count; i++)
		{
			item = &log->opt_log.item[i];
			if (!i)
			{
				tree_child = create_child(tree_node, LOG_ITEM_STR, 
								(void*)NULL, CONTACT_NULL);
			}
			else
			{
				create_brother(&tree_child, LOG_ITEM_STR, 
					(void*)NULL, CONTACT_NULL);
			}
			
			tree_son = create_child(tree_child, LOG_TIMES_STR, 
						(void*)item->times, CONTACT_INT);
			create_brother(&tree_son, MAJOR_STR, 
				(void*)(int)item->major_type, CONTACT_INT);
			create_brother(&tree_son, MINOR_STR, 
				(void*)(int)item->minor_type, CONTACT_INT);
			create_brother(&tree_son, ARGS_STR, 
				(void*)(int)item->args, CONTACT_INT);
			create_brother(&tree_son, USERNAME_STR, 
				(void*)item->user, CONTACT_STRING);
			create_brother(&tree_son, IP_ADDR_STR, 
				(void*)item->ip, CONTACT_INT);
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}

	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
    
	xml_tree_delete(xml_tree);
	return ret;
}


static __inline__ int 
merge_cond_info(JOperationLogCond *cond, char *buffer, 
	size_t size, const char *cmd)
{
	int ret = -1;
    
  	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);

	tree_root = create_xml_head(xml_tree, cmd);
	tree_node = create_child(tree_root, COND_INFO_STR, NULL, CONTACT_NULL);
	if (NULL != tree_node)
	{
		create_brother(&tree_node, BEG_TIME_STR, 
			(void*)cond->beg_time, CONTACT_INT);
		create_brother(&tree_node, END_TIME_STR, 
			(void*)cond->end_time, CONTACT_INT);
		create_brother(&tree_node, LOG_TYPE_STR, 
			(void*)cond->type, CONTACT_INT);
		create_brother(&tree_node, USERNAME_STR, 
			(void*)cond->user, CONTACT_STRING);
		create_brother(&tree_node, IP_ADDR_STR, 
			(void*)cond->ip, CONTACT_INT);
		create_brother(&tree_node, BEG_NODE_STR, 
			(void*)cond->beg_node, CONTACT_INT);
		create_brother(&tree_node, END_NODE_STR, 
			(void*)cond->end_node, CONTACT_INT);
		create_brother(&tree_node, SESS_ID_STR, 
			(void*)cond->sess_id, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
 
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
	    //printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
    
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_operation_log_request_xml(void *pvalue, 
		char *buffer, size_t size)
{
	OperationLogPacket *log;

	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	log = (OperationLogPacket*)pvalue;	
	return merge_cond_info(&log->opt_log.cond, buffer, size, GET_OPERATION_LOG_REQUEST_CMD);
}
int merge_get_operation_log_response_xml(void *pvalue, 
		char *buffer, size_t size)
{
	OperationLogPacket *log;

	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	log = (OperationLogPacket*)pvalue;
	
	return merge_log_info(log, buffer, size, GET_OPERATION_LOG_RESPONSE_CMD);
}

int merge_set_alarm_upload_config_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	JAlarmUploadCfg *au_cfg;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	au_cfg = (JAlarmUploadCfg*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, SET_ALARM_UPLOAD_CONFIG_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, ALARM_UPLOAD_ENABLE_STR, 
								(void*)au_cfg->enable, CONTACT_INT)))
	{
		create_brother(&tree_node, ALARM_UPLOAD_HOST_STR, 
			(void*)au_cfg->host, CONTACT_STRING);
		create_brother(&tree_node, ALARM_UPLOAD_PROT_STR, 
			(void*)(int)au_cfg->port, CONTACT_INT);
		create_brother(&tree_node, ALARM_UPLOAD_TYPE_STR, 
			(void*)(int)au_cfg->type, CONTACT_INT);
	}
	else
	{
		printf("create_child(DIS_CHANNEL_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_set_alarm_upload_config_response_xml(void *pvalue, char *buffer, size_t size)
{
	return merge_result_info_xml((int*)pvalue, buffer, size, SET_ALARM_UPLOAD_CONFIG_RESULT_CMD);
}

//##########################################################################
static int merge_preset_point_set_xml(PPSetPacket *pp_set, 
				char *buffer, size_t size, const char *command)
{
	int i, count, ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	JPresetPoint *pp;
	
	if (NULL == pp_set)
	{
		printf("pp_set NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)pp_set->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)pp_set->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)pp_set->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)pp_set->gu_id, CONTACT_STRING);
		
		count = pp_set->pp_set.pp_count;
		if (J_SDK_MAX_PRESET_PORT_SIZE > count)
			;
		else
			count = J_SDK_MAX_PRESET_PORT_SIZE;
		
		create_brother(&tree_node, PRESET_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)count, CONTACT_INT);
		
		for (i=0; i<count; i++)
		{
			pp = &pp_set->pp_set.pp[i];
			if (!i)
				tree_child = create_child(tree_node, PRESET_POINT_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, PRESET_POINT_STR, NULL, CONTACT_NULL);
			
			tree_son = create_child(tree_child, PRESET_NAME_STR, 
							(void*)pp->name, CONTACT_STRING);
			create_brother(&tree_son, PRESET_NO_STR, 
				(void*)pp->preset, CONTACT_INT);
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_preset_point_set_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, GET_PRESET_POINT_SET_REQUEST_CMD);
}
int merge_get_preset_point_set_response_xml(void *pvalue, char *buffer, size_t size)
{
	PPSetPacket *pp_set = (PPSetPacket*)pvalue;
	
	return merge_preset_point_set_xml(pp_set, buffer, size, GET_PRESET_POINT_SET_RESPONSE_CMD);
}

static int merge_preset_point_xml(PPConfigPacket *pp_cfg, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == pp_cfg)
	{
		printf("pp_cfg NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
								(void*)pp_cfg->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)pp_cfg->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)pp_cfg->gu_id, CONTACT_STRING);
		
		create_brother(&tree_node, PRESET_ACTION_STR, 
			(void*)pp_cfg->pp_cfg.action, CONTACT_INT);
		create_brother(&tree_node, PRESET_NAME_STR, 
			(void*)pp_cfg->pp_cfg.pp.name, CONTACT_STRING);
		create_brother(&tree_node, PRESET_NO_STR, 
			(void*)pp_cfg->pp_cfg.pp.preset, CONTACT_INT);
		
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_set_preset_point_request_xml(void *pvalue, char *buffer, size_t size)
{
	PPConfigPacket *pp_cfg = (PPConfigPacket*)pvalue;
	
	return merge_preset_point_xml(pp_cfg, buffer, size, SET_PRESET_POINT_REQUEST_CMD);
}
int merge_set_preset_point_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_PRESET_POINT_RESULT_CMD);
}

static int merge_cruise_way_set_xml(CruiseWaySetPacket *crz_set, 
				char *buffer, size_t size, const char *command)
{
	int i, /*j, */ret = -1;
	int crz_count;//, preset_count;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
//	XmlTreeNode *tree_son_child = NULL;
//	XmlTreeNode *tree_son_son = NULL;

	//JCruisePoint *crz_pp;
	//JCruiseWay  *crz_way;
	JCruiseInfo *crz_info;
	
	if (NULL == crz_set)
	{
		printf("crz_way NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)crz_set->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)crz_set->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)crz_set->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
			(void*)crz_set->gu_id, CONTACT_STRING);
		
		crz_count = crz_set->crz_set.crz_count;
		if (J_SDK_MAX_CRUISE_WAY_SIZE > crz_count)
			;
		else
			crz_count = J_SDK_MAX_CRUISE_WAY_SIZE;
		
		create_brother(&tree_node, CRUISE_SET_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)crz_count, CONTACT_INT);

		for (i=0; i<crz_count; i++)
		{
			crz_info = &crz_set->crz_set.crz_info[i];
			if (0 == i)
				tree_child = create_child(tree_node, CRUISE_WAY_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, CRUISE_WAY_STR, NULL, CONTACT_NULL);

			tree_son = create_child(tree_child, CRUISE_NAME_STR, 
				(void*)crz_info->crz_name, CONTACT_STRING);
			create_brother(&tree_son, CRUISE_NO_STR, 
				(void*)crz_info->crz_no, CONTACT_INT);

			/*preset_count = crz_way->count;
			if (J_SDK_MAX_PRESET_PORT_SIZE > preset_count)
				;
			else
				preset_count = J_SDK_MAX_PRESET_PORT_SIZE;
			
			tree_son = create_child(tree_child, CRUISE_STR, NULL, CONTACT_NULL);
			create_attr(&tree_son, COUNT_STR, (void*)preset_count, CONTACT_INT);
			
			for (j=0; j<preset_count; j++)
			{
				crz_pp = &crz_way->crz_pp[j];
				if (0 == j)
					tree_son_child = create_child(tree_son, PRESET_POINT_STR, NULL, CONTACT_NULL);
				else
					create_brother(&tree_son_child, PRESET_POINT_STR, NULL, CONTACT_NULL);

				tree_son_son = create_child(tree_son_child, PRESET_NO_STR, NULL, CONTACT_NULL);
				create_brother(&tree_son_son, CRUISE_SPEED_STR, 
					(void*)(int)crz_pp->speed, CONTACT_INT);
				create_brother(&tree_son_son, DWELL_TIME_STR, 
					(void*)(int)crz_pp->dwell, CONTACT_INT);
			}*/
		}
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

int merge_get_cruise_way_set_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}	
	
	return merge_request_action_xml((Request*)pvalue, buffer, size, GET_CRUISE_WAY_SET_REQUEST_CMD);
}
int merge_get_cruise_way_set_response_xml(void *pvalue, char *buffer, size_t size)
{
	CruiseWaySetPacket *crz_set;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	crz_set = (CruiseWaySetPacket*)pvalue;

	return merge_cruise_way_set_xml(crz_set, buffer, size, GET_CRUISE_WAY_SET_RESPONSE_CMD);
}

static int merge_cruise_way_xml(CruiseWayPacket *crz_way, 
				char *buffer, size_t size, const char *command)
{
	int i, count, ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	JCruisePoint *crz_pp;
	JCruiseInfo *crz_info;
	
	if (NULL == crz_way)
	{
		printf("crz_way NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (!strcmp(GET_CRUISE_WAY_RESPONSE_CMD, command))
	{
		tree_node = create_child(tree_root, RESULT_CODE_STR, 
			(void*)crz_way->result.code, CONTACT_INT);
		if (tree_node)
			create_brother(&tree_node, SESSION_ID_STR, 
				(void*)crz_way->session_id, CONTACT_STRING);
	}
	else
	{
		tree_node = create_child(tree_root, SESSION_ID_STR, 
			(void*)crz_way->session_id, CONTACT_STRING);
	}
	
	if (tree_node)
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)crz_way->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
			(void*)crz_way->gu_id, CONTACT_STRING);

		crz_info = &crz_way->crz_way.crz_info;
		create_brother(&tree_node, CRUISE_NAME_STR, 
			(void*)crz_info->crz_name, CONTACT_STRING);
		create_brother(&tree_node, CRUISE_NO_STR, 
			(void*)crz_info->crz_no, CONTACT_INT);

		count = crz_way->crz_way.pp_count;
		if (J_SDK_MAX_PRESET_PORT_SIZE > count)
			;
		else
			count = J_SDK_MAX_PRESET_PORT_SIZE;
		
		create_brother(&tree_node, CRUISE_WAY_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)count, CONTACT_INT);

		for (i=0; i<count; i++)
		{
			crz_pp = &crz_way->crz_way.crz_pp[i];
			if (0 == i)
				tree_child = create_child(tree_node, PRESET_POINT_STR, NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, PRESET_POINT_STR, NULL, CONTACT_NULL);
			
			tree_son = create_child(tree_child, PRESET_NO_STR, 
						(void*)(int)crz_pp->preset, CONTACT_INT);
			create_brother(&tree_son, CRUISE_SPEED_STR, 
				(void*)(int)crz_pp->speed, CONTACT_INT);
			create_brother(&tree_son, DWELL_TIME_STR, 
				(void*)(int)crz_pp->dwell, CONTACT_INT);
		}
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_cruise_way_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}	
	
	return merge_request_action_xml((Request*)pvalue, buffer, size, GET_CRUISE_WAY_REQUEST_CMD);
}
int merge_get_cruise_way_response_xml(void *pvalue, char *buffer, size_t size)
{
	CruiseWayPacket *crz_info;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	crz_info = (CruiseWayPacket*)pvalue;

	return merge_cruise_way_xml(crz_info, buffer, size, GET_CRUISE_WAY_RESPONSE_CMD);
}
int merge_add_cruise_way_request_xml(void *pvalue, char *buffer, size_t size)
{
	CruiseWayPacket *crz_way = (CruiseWayPacket*)pvalue;
	
	return merge_cruise_way_xml(crz_way, buffer, size, ADD_CRUISE_WAY_REQUEST_CMD);
}
int merge_add_cruise_way_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, ADD_CRUISE_WAY_RESULT_CMD);
}
int merge_modify_cruise_way_request_xml(void *pvalue, char *buffer, size_t size)
{
	CruiseWayPacket *crz_way = (CruiseWayPacket*)pvalue;
	
	return merge_cruise_way_xml(crz_way, buffer, size, MODIFY_CRUISE_WAY_REQUEST_CMD);
}
int merge_modify_cruise_way_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, MODIFY_CRUISE_WAY_RESULT_CMD);
}

static int merge_cruise_config_xml(CruiseConfigPacket *crz_cfg, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == crz_cfg)
	{
		printf("crz_cfg NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
								(void*)crz_cfg->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)crz_cfg->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)crz_cfg->gu_id, CONTACT_STRING);
		
		create_brother(&tree_node, CRUISE_ACTION_STR, 
			(void*)crz_cfg->crz_cfg.action, CONTACT_INT);
		create_brother(&tree_node, CRUISE_NO_STR, 
			(void*)crz_cfg->crz_cfg.crz_no, CONTACT_INT);
	}
	else
	{
		printf("create_child(SESSION_ID_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_set_cruise_way_request_xml(void *pvalue, char *buffer, size_t size)
{
	CruiseConfigPacket *crz_cfg = (CruiseConfigPacket*)pvalue;
	
	return merge_cruise_config_xml(crz_cfg, buffer, size, SET_CRUISE_WAY_REQUEST_CMD);
}
int merge_set_cruise_way_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_CRUISE_WAY_RESULT_CMD);
}

static int merge_3d_control_xml(_3DControlPacket *_3d_ctr, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == _3d_ctr)
	{
		printf("_3d_ctr NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
								(void*)_3d_ctr->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)_3d_ctr->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)_3d_ctr->gu_id, CONTACT_STRING);
		
		create_brother(&tree_node, X_OFFSET_STR, 
			(void*)_3d_ctr->_3d_ctr.x_offset, CONTACT_INT);
		create_brother(&tree_node, Y_OFFSET_STR, 
			(void*)_3d_ctr->_3d_ctr.y_offset, CONTACT_INT);
		create_brother(&tree_node, AMPLIFY_STR, 
			(void*)_3d_ctr->_3d_ctr.amplify, CONTACT_INT);
		
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_3d_control_request_xml(void *pvalue, char *buffer, size_t size)
{
	_3DControlPacket *_3d_ctr = (_3DControlPacket*)pvalue;
	
	return merge_3d_control_xml(_3d_ctr, buffer, size, _3D_CONTROL_REQUEST_CMD);
}
int merge_3d_control_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, _3D_CONTROL_RESULT_CMD);
}

int merge_3d_goback_request_xml(void *pvalue, char *buffer, size_t size)
{
	Request *request = (Request*)pvalue;
	
	return merge_request_action_xml(request, buffer, size, _3D_GOBACK_REQUEST_CMD);
}
int merge_3d_goback_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, _3D_GOBACK_RESULT_CMD);
}

int merge_alarm_link_io_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
    
	LinkIOPacket *link_io = (LinkIOPacket*)pvalue;  
	
	if (NULL == link_io)
	{
		printf("link_io NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, ALARM_LINK_IO_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
								(void*)link_io->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)link_io->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)link_io->gu_id, CONTACT_STRING);
		
		/*create_brother(&tree_node, ALARM_TYPE_STR, 
			(void*)link_io->link_io.alarm_type, CONTACT_INT);*/
		create_brother(&tree_node, TIME_LEN_STR, 
			(void*)link_io->link_io.time_len, CONTACT_INT);
		create_brother(&tree_node, DATA_STR, 
			(void*)link_io->link_io.data, CONTACT_STRING);
		
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
	
}

int merge_alarm_link_io_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, ALARM_LINK_IO_RESULT_CMD);
}

int merge_alarm_link_preset_request_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
    
	LinkPresetPacket *link_preset = (LinkPresetPacket*)pvalue;  
	
	if (NULL == link_preset)
	{
		printf("link_preset NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, ALARM_LINK_PRESET_REQUEST_CMD);
	if (NULL != (tree_node = create_child(tree_root, SESSION_ID_STR, 
								(void*)link_preset->session_id, CONTACT_STRING)))
	{
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)link_preset->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)link_preset->gu_id, CONTACT_STRING);
		create_brother(&tree_node, PRESET_NO_STR, 
			(void*)link_preset->link_preset.preset, CONTACT_INT);
		
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
	
}

int merge_alarm_link_preset_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, ALARM_LINK_PRESET_RESULT_CMD);
}

static int merge_resolution_info_xml(ResolutionInfoPacket *rsl_info, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)rsl_info->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)rsl_info->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)rsl_info->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)rsl_info->gu_id, CONTACT_STRING);
		
		create_brother(&tree_node, RESOLUTION_STR, 
			(void*)rsl_info->rsl_info.resolution, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_resolution_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}	
	
	return merge_request_action_xml((Request*)pvalue, buffer, size, GET_RESOLUTION_INFO_REQUEST_CMD);
}
int merge_get_resolution_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	ResolutionInfoPacket *rsl_info;
	
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	rsl_info = (ResolutionInfoPacket*)pvalue;

	return merge_resolution_info_xml(rsl_info, buffer, size, GET_RESOLUTION_INFO_RESPONSE_CMD);
}
int merge_set_resolution_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	ResolutionInfoPacket *rsl_info = (ResolutionInfoPacket*)pvalue;
	
	return merge_resolution_info_xml(rsl_info, buffer, size, SET_RESOLUTION_INFO_REQUEST_CMD);
}
int merge_set_resolution_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_RESOLUTION_INFO_RESULT_CMD);
}

static int merge_ircut_control_info_xml(IrcutControlPacket *ircut_ctrl, 
				char *buffer, size_t size, const char *command)
{
    JIrcut *ircut;
	int i, j, k, count, value, ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	XmlTreeNode *tree_grandgrandson = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)ircut_ctrl->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)ircut_ctrl->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)ircut_ctrl->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)ircut_ctrl->gu_id, CONTACT_STRING);

        count = ircut_ctrl->ircut_ctrl.count;
		create_brother(&tree_node, IRCUT_CONTROL_STR, NULL, CONTACT_NULL);
		create_attr(&tree_node, COUNT_STR, (void*)count, CONTACT_INT);

        for (i=0; i<count && i<J_SDK_MAX_CHN_SIZE; i++)
        {
			if (!i)
				tree_child = create_child(tree_node, IRCUT_STR, (void*)NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, IRCUT_STR, (void*)NULL, CONTACT_NULL);

            ircut = &ircut_ctrl->ircut_ctrl.ircut[i];
    		create_attr(&tree_child, CHANNEL_STR, (void*)i, CONTACT_INT);

            tree_son = create_child(tree_child, SWITCH_MODE_STR, (void*)ircut->ircut_mode, CONTACT_INT);
            create_brother(&tree_son, AUTO_C2B_STR, (void*)ircut->auto_c2b, CONTACT_INT);

            switch (ircut->ircut_mode)
            {
            case IRCUT_AUTO:
                create_brother(&tree_son, AUTO_SWITCH_STR, (void*)NULL, CONTACT_NULL);
                create_child(tree_son, SENSITIVE_STR, (void*)ircut->autos.sensitive, CONTACT_INT);
                break;
            case IRCUT_RTC:
                create_brother(&tree_son, RTC_SWITCH_STR, (void*)NULL, CONTACT_NULL);
                create_child(tree_son, RTC_STR, (void*)ircut->rtcs.rtc, CONTACT_INT);
                break;
            case IRCUT_TIMER:
                create_brother(&tree_son, TIMER_SWITCH_STR, (void*)NULL, CONTACT_NULL);
                for (j=0; j<J_SDK_MAX_DAY_SIZE; j++)
                {
        			if (!j)
        				tree_grandson = create_child(tree_son, DAY_STR, (void*)NULL, CONTACT_NULL);
        			else
        				create_brother(&tree_grandson, DAY_STR, (void*)NULL, CONTACT_NULL);

                    create_attr(&tree_grandson, ID_STR, (void*)j, CONTACT_INT);
                    for (k=0; k<4; k++)
                    {
            			if (!k)
            				tree_grandgrandson = create_child(tree_grandson, SEGMENT_STR, (void*)NULL, CONTACT_NULL);
            			else
            				create_brother(&tree_grandgrandson, SEGMENT_STR, (void*)NULL, CONTACT_NULL);
                        
                        XmlTreeNode *node;
                        value = ircut->timers[j].seg_time[k].open;
                        node = create_child(tree_grandgrandson, SEG_OPNT_STR, (void*)value, CONTACT_INT);
                        value = ircut->timers[j].seg_time[k].begin_sec;
                        create_brother(&node, BEGIN_TIMES_STR, (void*)value, CONTACT_INT);
                        value = ircut->timers[j].seg_time[k].end_sec;
                        create_brother(&node, END_TIMES_STR, (void*)value, CONTACT_INT);
                    }
                }
                break;
            }
        }
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_ircut_control_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}	
	
	return merge_request_action_xml((Request*)pvalue, buffer, size, GET_IRCUT_CONTROL_REQUEST_CMD);
}
int merge_get_ircut_control_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	return merge_ircut_control_info_xml((IrcutControlPacket*)pvalue, buffer, size, GET_IRCUT_CONTROL_RESPONSE_CMD);
}
int merge_set_ircut_control_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_ircut_control_info_xml((IrcutControlPacket*)pvalue, buffer, size, SET_IRCUT_CONTROL_REQUEST_CMD);
}
int merge_set_ircut_control_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_IRCUT_CONTROL_RESULT_CMD);
}

int merge_get_extranet_port_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}	
	
	return merge_request_action_xml((Request*)pvalue, buffer, size, GET_EXTRANET_PORT_REQUEST_CMD);
}
int merge_get_extranet_port_response_xml(void *pvalue, char *buffer, size_t size)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
    
	ExtranetPortPacket *extranet = (ExtranetPortPacket*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, GET_EXTRANET_PORT_RESPONSE_CMD);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
								(void*)extranet->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
			(void*)extranet->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
			(void*)extranet->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
				(void*)extranet->gu_id, CONTACT_STRING);
		
		create_brother(&tree_node, DATA_PORT_STR, 
			(void*)(int)extranet->extranet.data_port, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
	
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}

static int merge_herd_analyse_info(HerdAnalysePacket *herd_analyse, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	int idx, count;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	XmlTreeNode *tree_grandgrandson = NULL;
	
	char time_buffer[J_SDK_MAX_TIME_LEN];
	memset(time_buffer, 0, sizeof(time_buffer));
	
	if (NULL == herd_analyse)
	{
		printf("herd_analyse NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)herd_analyse->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)herd_analyse->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)herd_analyse->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)herd_analyse->gu_id, CONTACT_STRING);
		create_brother(&tree_node, FODDER_EANBLE_STR, 
						(void*)(int)herd_analyse->herd_analyse.fodder_eable, CONTACT_INT);
		create_brother(&tree_node, REPORT_INTV_STR, 
						(void*)(int)herd_analyse->herd_analyse.report_intv, CONTACT_INT);
		create_brother(&tree_node, MAX_WIDTH_STR, 
						(void*)herd_analyse->herd_analyse.max_width, CONTACT_INT);
		create_brother(&tree_node, MAX_HEIGHT_STR, 
						(void*)herd_analyse->herd_analyse.max_height, CONTACT_INT);

        JWeek *week = &herd_analyse->herd_analyse.week;
		create_brother(&tree_node, WEEK_DAY_S_STR, NULL, CONTACT_NULL);
		for (idx=0; idx<week->count; idx++)
		{
			if (!idx)
				tree_child = create_child(tree_node, WEEK_DAY_STR, 
											NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, WEEK_DAY_STR, 
									NULL, CONTACT_NULL);
			
			create_attr(&tree_child, WEEK_DAY_ID_STR, 
				(void*)week->days[idx].day_id, CONTACT_INT);
			if (tree_child)
			{
				tree_son = create_child(tree_child, TIME_SEG_S_STR, 
										NULL, CONTACT_NULL);
				
				for (count=0; count<week->days[idx].count; count++)
				{
					if (!merge_time_part_string(time_buffer, 
								sizeof(time_buffer), 
								&week->days[idx].seg[count].time_start, 
								&week->days[idx].seg[count].time_end))
					{
						if (!count)
							tree_grandson = create_child(tree_son, TIME_SEG_STR, 
											(void*)time_buffer, CONTACT_STRING);
						else
							create_brother(&tree_grandson, TIME_SEG_STR, 
									(void*)time_buffer, CONTACT_STRING);
						
						create_attr(&tree_grandson, TIME_SEG_ENABLE_STR, 
							(void*)week->days[idx].seg[count].enable, CONTACT_INT);
					}
				}
			}
		}

        
        JField *field = &herd_analyse->herd_analyse.field;
		create_brother(&tree_node, TROUGH_PARAMS_STR, NULL, CONTACT_NULL);
		for (idx=0; idx<field->trough_count; idx++)
		{
			if (!idx)
				tree_child = create_child(tree_node, TROUGH_PARAM_STR, 
											NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, TROUGH_PARAM_STR, 
									NULL, CONTACT_NULL);
			
			create_attr(&tree_child, TROUGH_ID_STR, 
				(void*)field->trough[idx].id, CONTACT_INT);
            
			tree_son = create_child(tree_child, NAME_STR, 
                            (void*)field->trough[idx].name, CONTACT_STRING);
            if (tree_son)
            {
                create_brother(&tree_son, QUADR_ANGLE_STR, NULL, CONTACT_NULL);
                for (count=0; count<4; count++)
                {
                    if (!count)
                        tree_grandson = create_child(tree_son, POINT_STR, 
                                            NULL, CONTACT_NULL);
                    else
                        create_brother(&tree_grandson, POINT_STR, NULL, CONTACT_NULL);
                    
                    JPoint *point = &field->trough[idx].quadr.angle[count];
                    tree_grandgrandson = create_child(tree_grandson, 
                        POINT_X_STR, (void*)(int)point->x, CONTACT_INT);
                    create_brother(&tree_grandgrandson, POINT_Y_STR, 
									(void*)(int)point->y, CONTACT_INT);
                }
            }
        }
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_herd_analyse_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}	
	
	return merge_request_action_xml((Request*)pvalue, buffer, size, GET_HERD_ANALYSE_REQUEST_CMD);
}
int merge_get_herd_analyse_info_response_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	return merge_herd_analyse_info((HerdAnalysePacket*)pvalue, buffer, size, GET_HERD_ANALYSE_RESPONSE_CMD);
}
int merge_set_herd_analyse_info_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return merge_herd_analyse_info((HerdAnalysePacket*)pvalue, buffer, size, SET_HERD_ANALYSE_REQUEST_CMD);
}
int merge_set_herd_analyse_info_result_xml(void *pvalue, char *buffer, size_t size)
{
	Result *result = (Result*)pvalue;
	
	return merge_result_action_xml(result, buffer, size, SET_HERD_ANALYSE_RESULT_CMD);
}

static int merge_grass_percent(GrassPercentPacket *grass_percent, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	int idx;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	char time_buffer[J_SDK_MAX_TIME_LEN];
	memset(time_buffer, 0, sizeof(time_buffer));
	
	if (NULL == grass_percent)
	{
		printf("grass_percent NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)grass_percent->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, SESSION_ID_STR, 
						(void*)grass_percent->session_id, CONTACT_STRING);
		create_brother(&tree_node, DOMAIN_ID_STR, 
						(void*)grass_percent->domain_id, CONTACT_STRING);
		create_brother(&tree_node, GU_ID_STR, 
						(void*)grass_percent->gu_id, CONTACT_STRING);
        
        JField *field = &grass_percent->grass.field;
		create_brother(&tree_node, TROUGH_PARAMS_STR, NULL, CONTACT_NULL);
		for (idx=0; idx<field->trough_count; idx++)
		{
			if (!idx)
				tree_child = create_child(tree_node, TROUGH_PARAM_STR, 
											NULL, CONTACT_NULL);
			else
				create_brother(&tree_child, TROUGH_PARAM_STR, 
									NULL, CONTACT_NULL);
			
			create_attr(&tree_child, TROUGH_ID_STR, 
				(void*)field->trough[idx].id, CONTACT_INT);
            
			tree_son = create_child(tree_child, NAME_STR, 
                            (void*)field->trough[idx].name, CONTACT_STRING);
            create_brother(&tree_son, GRASS_PERCENT_STR, 
                            (void*)field->trough[idx].percent, CONTACT_INT);
        }
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_grass_percent_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}	
	
	return merge_request_action_xml((Request*)pvalue, buffer, size, GET_GRASS_PERCENT_REQUEST_CMD);
}
int merge_get_grass_percent_response_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	return merge_grass_percent((GrassPercentPacket*)pvalue, buffer, size, GET_GRASS_PERCENT_RESPONSE_CMD);
}

static int merge_p2p_id(P2PIdPacketReq *p2p_id, 
				char *buffer, size_t size, const char *command)
{
	int ret = -1;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	tree_root = create_xml_head(xml_tree, command);
	if (NULL != (tree_node = create_child(tree_root, RESULT_CODE_STR, 
										 (void*)p2p_id->result.code, CONTACT_INT)))
	{
		create_brother(&tree_node, PU_ID_STR, 
						(void*)p2p_id->p2p.pu_id, CONTACT_STRING);
		create_brother(&tree_node, P2P_ID_STR, 
						(void*)p2p_id->p2p.p2p_id, CONTACT_STRING);
		create_brother(&tree_node, CHANNELCOUNT,
						(void *)p2p_id->channel_count, CONTACT_INT);
	}
	else
	{
		printf("create_child(RESULT_CODE_STR) failed.\n");
		xml_tree_delete(xml_tree);
		return -1;
	}
		
	if (0 < (ret = xml_tree_merge_by_mxml(xml_tree, buffer, size)))
	{
		//printf("xml_tree_merge_by_mxml succ... [len: %d]\n", ret);
	}
	
	xml_tree_delete(xml_tree);
	return ret;
}
int merge_get_p2p_id_request_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}	
	//P2PIdPacketReq
	//return merge_p2p_id((P2PIdPacket*)pvalue, buffer, size, GET_P2P_ID_REQUEST_CMD);
	return merge_p2p_id((P2PIdPacketReq *)pvalue, buffer, size, GET_P2P_ID_REQUEST_CMD);
}
int merge_get_p2p_id_response_xml(void *pvalue, char *buffer, size_t size)
{
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	return merge_p2p_id((P2PIdPacket*)pvalue, buffer, size, GET_P2P_ID_RESPONSE_CMD);
}


//end
