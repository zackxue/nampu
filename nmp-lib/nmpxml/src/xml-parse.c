
#include "config.h"
#include "xml-tree.h"
#include "nmp_xmlmem.h"
#include "nmp_xmlinfo.h"
#include "xml-api.h"

extern int parse_time_string(const char *time_string, JTime *time);
extern int parse_time_part_string(const char *time_part, 
				JTime *time_start, JTime *time_end);
extern void init_xml_tree_attr(XmlTreeAttr **new_jxj_attr);
extern void init_xml_tree_node(XmlTreeNode **new_jxj_node);
extern int xml_tree_delete(XmlTreeNode *xml_tree);
extern int xml_tree_parse_by_mxml(XmlTreeNode *xml_tree, 
				const char *buffer);


static __inline__ void 
get_weekday_info(XmlTreeNode *tree_node, JWeek *week)
{
	int ret, day_id;
	int day_count = 0;
	int seg_count = 0;
	
	XmlTreeNode *xml_node, *xml_child, *xml_son;
	XmlTreeAttr *node_attr = NULL;

	J_ASSERT(tree_node);
	
	xml_node = tree_node->child;
	while (xml_node)
	{
		if (!strcmp(WEEK_DAY_STR, xml_node->element.name) &&
			J_SDK_MAX_DAY_SIZE > day_count)
		{
			node_attr = xml_node->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(WEEK_DAY_ID_STR, node_attr->name)))
			{
				sscanf(node_attr->value, "%d", &day_id);
				week->days[day_count].day_id = day_id;
			}
			else
				continue;
			
			xml_child = xml_node->child;
			while (xml_child)
			{
				if (!strcmp(ALL_DAY_ENABLE_STR, xml_child->element.name))
				{
					if (NULL != xml_child->element.contact)
					{
						sscanf(xml_child->element.contact, "%d", 
								&(week->days[day_id].all_day_enable));
					}
				}
				else if (!strcmp(TIME_SEG_S_STR, xml_child->element.name))
				{
					seg_count = 0;
					xml_son = xml_child->child;
					while (xml_son)
					{
						if (!strcmp(TIME_SEG_STR, xml_son->element.name) &&
							seg_count < J_SDK_MAX_SEG_SZIE)
						{
							node_attr = xml_son->element.attr;
							if ((NULL != node_attr) && 
								(0 == strcmp(TIME_SEG_ENABLE_STR, node_attr->name)))
							{
								sscanf(node_attr->value, "%d", 
										&(week->days[day_id].seg[seg_count].enable));
							}
							
							if (xml_son->element.contact)
							{
								ret = parse_time_part_string(xml_son->element.contact, 
												&(week->days[day_id].seg[seg_count].time_start), 
												&(week->days[day_id].seg[seg_count].time_end));
								if (0 != ret)
								{
									//xml_tree_delete(xml_tree);
									//return -1;
								}
								seg_count++;
							}
						}
						xml_son = xml_son->next;
					}
					week->days[day_id].count = seg_count;
				}
				xml_child = xml_child->next;
			}
			day_count++;
		}
		xml_node = xml_node->next;
	}
	week->count = day_count;
}

static __inline__ void 
get_dection_area_info(XmlTreeNode *tree_node, JArea *area)
{
	int value, rect_count = 0;
	XmlTreeNode *child, *son;
	XmlTreeAttr *node_attr;
	
	J_ASSERT(tree_node);
	
	node_attr = tree_node->element.attr;
	if (NULL != node_attr) 
	{
		if (0 == strcmp(HIDE_NUM_STR, node_attr->name))
		{
			if (NULL != node_attr->value)
			{
				sscanf(node_attr->value, "%d", &(area->count));
			}
		}
	}

	child = tree_node->child;
	while (child)
	{
		if (0 == strcmp(RECTANGLE_STR, child->element.name) &&
			J_SDK_MAX_DECTION_AREA_SIEZ > rect_count)
		{
			son = child->child;
			while (son)
			{
				if (0 == strcmp(RECT_LEFT_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", &value);
						area->rect[rect_count].left = value;
					}
				}
				else if (0 == strcmp(RECT_TOP_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", &value);
						area->rect[rect_count].top = value;
					}
				}
				else if (0 == strcmp(RECT_RIGHT_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", &value);
						area->rect[rect_count].right = value;
					}
				}
				else if (0 == strcmp(RECT_BOTTOM_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", &value);
						area->rect[rect_count].bottom = value;
					}
				}
				son = son->next;
			}
			rect_count++;
		}
		child = child->next;
	}
	area->count = rect_count;
}

static __inline__ int get_disk_info(XmlTreeNode *tree_node, JDiskInfo *disk)
{
	int index = 0;
	XmlTreeNode *child, *son;
	XmlTreeAttr *node_attr;
	
	J_ASSERT(tree_node);

	child = tree_node->child;
	while (child)
	{
		if (0 == strcmp(DISK_STR, child->element.name) &&
			J_SDK_MAX_DISK_NUMBER > index)
		{
			node_attr = child->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(DISK_NO_STR, node_attr->name)))
			{
				sscanf(node_attr->value, "%d", 
						&(disk[index].disk_no));
			}
			
			son = child->child;
			while (son)
			{
				if (0 == strcmp(TOTAL_SIZE_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", 
								&(disk[index].total_size));
					}
				}
				else if (0 == strcmp(FREE_SIZE_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", 
								&(disk[index].free_size));
					}
				}
				else if (0 == strcmp(IS_BACKUP_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", 
								&(disk[index].is_backup));
					}
				}
				else if (0 == strcmp(DISK_TYPE_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", 
								&(disk[index].disk_type));
					}
				}
				else if (0 == strcmp(DISK_STATUS_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", 
								&(disk[index].status));
					}
				}
				else if (0 == strcmp(SYS_FILE_TYPE_STR, son->element.name))
				{
					if (NULL != son->element.contact)
					{
						sscanf(son->element.contact, "%d", 
								&(disk[index].sys_file_type));
					}
				}
				son = son->next;
			}
			index++;
		}
		child = child->next;
	}
	return index;
}
static __inline__ void get_joint_info(XmlTreeNode *tree_node, JJoint *joint)
{
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	if (NULL == tree_node)
	{
		printf("tree_node NULL.\n");
		return ;
	}
 	else if (NULL == joint)
	{
		printf("joint NULL.\n");
		return ;
	}

	tree_child = tree_node->child;
	while (tree_child)
	{
		if (0 == strcmp(JOINT_RECORD_STR, tree_child->element.name))
		{
			tree_son = tree_child->child;
			while (tree_son)
			{
				if (0 == strcmp(JOINT_RECORD_ENABLE_CHANNEL_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_record_enable_channel));
					}
				}
				else if (0 == strcmp(JOINT_RECORD_SECOND_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_record_second));
					}
				}
				tree_son = tree_son->next;
			}
		}
		else if (0 == strcmp(JOINT_IO_STR, tree_child->element.name))
		{
			tree_son = tree_child->child;
			while (tree_son)
			{
				if (0 == strcmp(JOINT_BEEP_ENABLE_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_beep_enable));
					}
				}
				else if (0 == strcmp(JOINT_BEEP_SECOND_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_beep_second));
					}
				}
				else if (0 == strcmp(JOINT_OUTPUT_TIMES_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_output_times));
					}
				}
				else if (0 == strcmp(JOINT_OUTPUT_ENABLE_CHANNEL_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_output_enable_channel));
					}
				}
				tree_son = tree_son->next;
			}
		}
		else if (0 == strcmp(JOINT_SNAP_STR, tree_child->element.name))
		{
			tree_son = tree_child->child;
			while (tree_son)
			{
				if (0 == strcmp(JOINT_SNAP_ENABLE_CHANNEL_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_snap_enable_channel));
					}
				}
				else if (0 == strcmp(JOINT_SNAP_INTERVAL_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_snap_interval));
					}
				}
				else if (0 == strcmp(JOINT_SNAP_TIMES_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_snap_times));
					}
				}
				tree_son = tree_son->next;
			}
		}
		else if (0 == strcmp(JOINT_EMAIL_STR, tree_child->element.name))
		{
			tree_son = tree_child->child;
			while (tree_son)
			{
				if (0 == strcmp(JOINT_EMAIL_ENABLE_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&(joint->joint_email_enable));
					}
				}
				tree_son = tree_son->next;
			}
		}
		tree_child = tree_child->next;
	}
}

//##########################################################################
static __inline__ int parse_request_action_xml(Request *request, 
											const char *buffer, 
											const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)request->session_id, 
									sizeof(request->session_id), 
									"%s", tree_node->element.contact);
						}
						goto NEXT;
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)request->domain_id, 
									sizeof(request->domain_id), 
									"%s", tree_node->element.contact);
						}
						goto NEXT;
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)request->pu_or_gu_id, 
									sizeof(request->pu_or_gu_id), 
									"%s", tree_node->element.contact);
						}
						goto NEXT;
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)request->pu_or_gu_id, 
									sizeof(request->pu_or_gu_id), 
									"%s", tree_node->element.contact);
						}
						goto NEXT;
					}

					if (0 == strcmp(ALARM_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&request->reserve);
						}
					}
					else if (0 == strcmp(SERIAL_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&request->reserve);
						}
					}
					else if (0 == strcmp(CRUISE_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&request->reserve);
						}
					}

					NEXT:
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}

static __inline__ int parse_result_action_xml(Result *result, 
										 const char *buffer, 
										 const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
										(0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)result->session_id, 
									sizeof(result->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)result->domain_id, 
									sizeof(result->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)result->pu_or_gu_id, 
									sizeof(result->pu_or_gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)result->pu_or_gu_id, 
									sizeof(result->pu_or_gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&result->result.code);
						}
					}
					else if (0 == strcmp(CRUISE_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&result->reserve);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}


//##########################################################################
int parse_get_css_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	PuGetCssPacket *get_css = (PuGetCssPacket*)pvalue;
		
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
										(0 == strcmp(PU_GET_CSS_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DEVICE_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)get_css->dev_code, 
									sizeof(get_css->dev_code), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_VERSION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)get_css->software_ver, 
									sizeof(get_css->software_ver), 
									"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_css_response_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	PuGetCssPacket *get_css = (PuGetCssPacket*)pvalue;
		
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
										(0 == strcmp(PU_GET_CSS_RESPONSE_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									(int*)&get_css->result.code);
						}
					}
					else if (0 == strcmp(CSS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)get_css->css_ip, 
									sizeof(get_css->css_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CSS_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									(int*)&get_css->css_port);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_pu_register_css_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	PuRegisterCssPacket *reg_css = (PuRegisterCssPacket*)pvalue;
		
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
										(0 == strcmp(REGISTER_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DEVICE_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_css->dev_code, 
									sizeof(reg_css->dev_code), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DEVICE_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_css->dev_ip, 
									sizeof(reg_css->dev_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CSS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_css->css_ip, 
									sizeof(reg_css->css_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_VERSION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_css->software_ver, 
									sizeof(reg_css->software_ver), 
									"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_register_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	RegisterRequestPacket *reg_request = (RegisterRequestPacket*)pvalue;
		
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
										(0 == strcmp(REGISTER_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_request->pu_id, 
									sizeof(reg_request->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									(int*)&(reg_request->pu_type));
						}
					}
					else if (0 == strcmp(DEVICE_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_request->dev_ip, 
									sizeof(reg_request->dev_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CMS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_request->cms_ip, 
									sizeof(reg_request->cms_ip), 
									"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}


int parse_register_response_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	RegisterResponsePacket *reg_response = (RegisterResponsePacket*)pvalue;
		
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(REGISTER_RESPONSE_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_response->pu_id, 
									sizeof(reg_response->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(reg_response->result.code));
						}
					}
					else if (0 == strcmp(KEEP_ALIVE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(reg_response->keep_alive));
							//printf("reg_response->keep_alive: %d\n", reg_response->keep_alive);
						}
					}
					else if (0 == strcmp(MDS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)reg_response->mds_ip, 
									sizeof(reg_response->mds_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MDS_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(reg_response->mds_port));
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_heart_beat_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	HeartBeatRequestPacket *heart_beat = (HeartBeatRequestPacket*)pvalue;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(HEART_BEAT_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)heart_beat->pu_id, 
									sizeof(heart_beat->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DEVICE_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)heart_beat->dev_ip, 
									sizeof(heart_beat->dev_ip), 
									"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}

int parse_heart_beat_response_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	HeartBeatResponsePacket *heart_beat_resp = (HeartBeatResponsePacket*)pvalue;
		
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(HEART_BEAT_RESPONSE_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(heart_beat_resp->result.code));
						}
					}
					else if (0 == strcmp(SERVER_TIME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							if (0 == parse_time_string(tree_node->element.contact, 
														&(heart_beat_resp->server_time)))
							{
							}
							else
							{
								xml_tree_delete(xml_tree);
								return -1;
							}
						}
					}
					else if (0 == strcmp(PLT_TYPE_STR, tree_node->element.name))
					{
						heart_beat_resp->plt_type = (int)atoi((const char *)tree_node->element.contact);
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}	

//##########################################################################
int parse_get_mds_info_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	MdsInfoPacket *mds_info = (MdsInfoPacket*)pvalue;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(GET_MDS_INFO_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)mds_info->pu_id, sizeof(mds_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CMS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)mds_info->cms_ip, sizeof(mds_info->cms_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)mds_info->mds_ip, sizeof(mds_info->mds_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(mds_info->mds_port));
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_mds_info_response_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	MdsInfoPacket *mds_info = (MdsInfoPacket*)pvalue;

	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(GET_MDS_INFO_RESPONSE_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&mds_info->result.code);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)mds_info->pu_id, sizeof(mds_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)mds_info->mds_ip, sizeof(mds_info->mds_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&mds_info->mds_port);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}	
int parse_change_dispatch_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	ChangeDispatchPacket *change_disp = (ChangeDispatchPacket*)pvalue;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(CHANGE_DISPATCH_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)change_disp->session_id, 
									sizeof(change_disp->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)change_disp->domain_id, 
									sizeof(change_disp->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)change_disp->pu_id, sizeof(change_disp->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MDS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)change_disp->mds_ip, sizeof(change_disp->mds_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MDS_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(change_disp->mds_port));
						}
					}
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}

int parse_change_dispatch_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, CHANGE_DISPATCH_RESULT_CMD);
}

//##########################################################################
int parse_get_device_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_DEVICE_INFO_CMD);
}

int parse_device_info_response_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	DeviceInfoPacket *dev_info = (DeviceInfoPacket*)pvalue;
		
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(DEVICE_INFO_RESPONSE_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_info->session_id, 
									sizeof(dev_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_info->domain_id, 
									sizeof(dev_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_info->pu_id, sizeof(dev_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_info->result.code));
						}
					}
					else if (0 == strcmp(PU_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_info->pu_type));
						}
					}
					else if (0 == strcmp(PU_SUB_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_info->sub_type));
						}
					}
					else if (0 == strcmp(MANU_INFO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_info->manu_info, 
									sizeof(dev_info->manu_info), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(RELEASE_DATE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_info->release_date, 
									sizeof(dev_info->release_date), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DEV_VERSION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_info->dev_version, 
									sizeof(dev_info->dev_version), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(HW_VERSION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_info->hw_version, 
									sizeof(dev_info->hw_version), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DI_NUM_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_info->di_num));
						}
					}
					else if (0 == strcmp(DO_NUM_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_info->do_num));
						}
					}
					else if (0 == strcmp(CHANNEL_NUM_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_info->channel_num));
						}
					}
					else if (0 == strcmp(RS485_NUM_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_info->rs485_num));
						}
					}
					else if (0 == strcmp(RS232_NUM_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_info->rs232_num));
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}


//##########################################################################
static int parse_device_ntp_info_xml(DeviceNTPInfoPacket *dev_ntp_info, 
															const char *buffer, 
															const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_ntp_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_ntp_info->session_id, 
									sizeof(dev_ntp_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_ntp_info->domain_id, 
									sizeof(dev_ntp_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_ntp_info->pu_id, 
									sizeof(dev_ntp_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(NTP_SERVER_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_ntp_info->ntp_server_ip, 
									sizeof(dev_ntp_info->ntp_server_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(TIME_ZONE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_ntp_info->time_zone));
						}
					}
					else if (0 == strcmp(TIME_INTERVAL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_ntp_info->time_interval));
						}
					}
					else if (0 == strcmp(NTP_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_ntp_info->ntp_enable));
						}
					}
					else if (0 == strcmp(DST_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_ntp_info->dst_enable));
						}
					}
					else if (0 == strcmp(RESERVE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_ntp_info->reserve));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_device_ntp_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_DEVICE_NTP_INFO_CMD);
}

int parse_device_ntp_info_response_xml(void *pvalue, const char *buffer)
{
	DeviceNTPInfoPacket *dev_ntp_info = (DeviceNTPInfoPacket*)pvalue;
	
	return parse_device_ntp_info_xml(dev_ntp_info, buffer, DEVICE_NTP_INFO_RESPONSE_CMD);
}

int parse_set_device_ntp_info_xml(void *pvalue, const char *buffer)
{
	DeviceNTPInfoPacket *dev_ntp_info = (DeviceNTPInfoPacket*)pvalue;
	
	return parse_device_ntp_info_xml(dev_ntp_info, buffer, SET_DEVICE_NTP_INFO_CMD);
}

int parse_device_ntp_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, DEVICE_NTP_INFO_RESULT_CMD);
}

//##########################################################################
int parse_device_time_xml(DeviceTimePacket *dev_time, 
		const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_time->session_id, 
									sizeof(dev_time->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_time->domain_id, 
									sizeof(dev_time->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)dev_time->pu_id, 
									sizeof(dev_time->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(TIME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							if (0 == parse_time_string(tree_node->element.contact, 
														&(dev_time->time)))
							{
							}
							else
							{
								xml_tree_delete(xml_tree);
								return -1;
							}
						}
					}
					else if (0 == strcmp(TIME_ZONE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_time->zone));
						}
					}
					else if (0 == strcmp(SYNC_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_time->sync_enable));
						}
					}
					else if (0 == strcmp(SET_FLAG_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(dev_time->set_flag));
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_device_time_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_DEVICE_TIME_CMD);
}
int parse_device_time_response_xml(void *pvalue, const char *buffer)
{
	DeviceTimePacket *dev_time;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	dev_time = (DeviceTimePacket*)pvalue;
	
	return parse_device_time_xml(dev_time, buffer, DEVICE_TIME_RESPONSE_CMD);
}
int parse_set_device_time_xml(void *pvalue, const char *buffer)
{
	DeviceTimePacket *dev_time;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	dev_time = (DeviceTimePacket*)pvalue;
	
	return parse_device_time_xml(dev_time, buffer, SET_DEVICE_TIME_CMD);
}

int parse_device_time_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, DEVICE_TIME_RESULT_CMD);
}

//##########################################################################
static int parse_platform_info_xml(PlatformInfoPacket *pltf_info, 
				const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pltf_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pltf_info->session_id, 
									sizeof(pltf_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pltf_info->domain_id, 
									sizeof(pltf_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pltf_info->pu_id, 
									sizeof(pltf_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CMS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pltf_info->cms_ip, 
									sizeof(pltf_info->cms_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CMS_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pltf_info->cms_port));
						}
					}
					else if (0 == strcmp(MDS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pltf_info->mds_ip, sizeof(pltf_info->mds_ip), 
								"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MDS_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pltf_info->mds_port));
						}
					}
					else if (0 == strcmp(PROTOCOL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pltf_info->protocol));
						}
					}
					else if (0 == strcmp(IS_CON_CMS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pltf_info->is_con_cms));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_platform_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_PLATFORM_INFO_CMD);
}

int parse_platform_info_response_xml(void *pvalue, const char *buffer)
{
	PlatformInfoPacket *pltf_info = (PlatformInfoPacket*)pvalue;
	
	return parse_platform_info_xml(pltf_info, buffer, PLATFORM_INFO_RESPONSE_CMD);
}

int parse_set_platform_info_xml(void *pvalue, const char *buffer)
{
	PlatformInfoPacket *pltf_info = (PlatformInfoPacket*)pvalue;
	
	return parse_platform_info_xml(pltf_info, buffer, SET_PLATFORM_INFO_CMD);
}

int parse_platform_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, PLATFORM_INFO_RESULT_CMD);
}

//##########################################################################
static __inline__ int get_network_child_info(XmlTreeNode *tree_node, JNetwork *network)
{
	XmlTreeNode *child = NULL;

	if (NULL == tree_node)
	{
		printf("tree node NULL.\n");
		return -1;
	}
		
	child = tree_node->child;
	while (NULL != child)
	{
		if (0 == strcmp(DHCP_ENABLE_STR, child->element.name))
		{
			if (NULL != child->element.contact)
			{
				sscanf(child->element.contact, "%d", 
						&(network->dhcp_enable));
			}
		}
		else if (0 == strcmp(MAC_STR, child->element.name))
		{
			if (NULL != child->element.contact)
			{
				snprintf((char*)network->mac, 
						sizeof(network->mac), 
						"%s", child->element.contact);
			}
		}
		else if (0 == strcmp(IP_STR, child->element.name))
		{
			if (NULL != child->element.contact)
			{
				snprintf((char*)network->ip, 
						sizeof(network->ip), 
						"%s", child->element.contact);
			}
		}
		else if (0 == strcmp(NETMASK_STR, child->element.name))
		{
			if (NULL != child->element.contact)
			{
				snprintf((char*)network->netmask, 
						sizeof(network->netmask), 
						"%s", child->element.contact);
			}
		}
		else if (0 == strcmp(GATEWAY_STR, child->element.name))
		{
			if (NULL != child->element.contact)
			{
				snprintf((char*)network->gateway, 
						sizeof(network->gateway), 
						"%s", child->element.contact);
			}
		}
		child = child->next;
	}
	
	return 0;
}

static int parse_network_info_xml(NetworkInfoPacket *net_info, 
													const char *buffer, 
													const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)net_info->session_id, 
									sizeof(net_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)net_info->domain_id, 
									sizeof(net_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)net_info->pu_id, 
									sizeof(net_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(NETWORK_STR, tree_node->element.name))
					{
						node_attr = tree_node->element.attr;
						if (0 == strcmp(NETWORK_TYPE_STR, node_attr->name))
						{
							int type;
							sscanf(node_attr->value, "%d", &type);
							switch (type)
							{
								case J_SDK_ETH0:
									net_info->network[type].type = type;
									get_network_child_info(tree_node, &(net_info->network[type]));
									break;
									
								case J_SDK_WIFI:
									net_info->network[type].type = type;
									get_network_child_info(tree_node, &(net_info->network[type]));
									break;
									
								case J_SDK_3G:
									net_info->network[type].type = type;
									get_network_child_info(tree_node, &(net_info->network[type]));
									break;
									
								default:
									break;
							}
						}
						
					}
					else if (0 == strcmp(MAIN_DNS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)net_info->main_dns, 
									sizeof(net_info->main_dns), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(BACKUP_DNS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)net_info->backup_dns, 
									sizeof(net_info->backup_dns), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(AUTO_DNS_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_info->web_port));
						}
					}
					else if (0 == strcmp(CMD_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_info->cmd_port));
						}
					}
					else if (0 == strcmp(DATA_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_info->data_port));
						}
					}
					else if (0 == strcmp(WEB_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_info->web_port));
						}
					}
					else if (0 == strcmp(TALK_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_info->talk_port));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_network_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_NETWORK_INFO_CMD);
}

int parse_network_info_response_xml(void *pvalue, const char *buffer)
{
	NetworkInfoPacket *net_info = (NetworkInfoPacket*)pvalue;
	
	return parse_network_info_xml(net_info, buffer, NETWORK_INFO_RESPONSE_CMD);
}

int parse_set_network_info_xml(void *pvalue, const char *buffer)
{
	NetworkInfoPacket *net_info = (NetworkInfoPacket*)pvalue;
	
	return parse_network_info_xml(net_info, buffer, SET_NETWORK_INFO_CMD);
}

int parse_network_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, NETWORK_INFO_RESULT_CMD);
}

//##########################################################################
static int parse_pppoe_info_xml(PPPOEInfoPacket *pppoe_info, 
				const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pppoe_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pppoe_info->session_id, 
									sizeof(pppoe_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pppoe_info->domain_id, 
									sizeof(pppoe_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pppoe_info->pu_id, 
									sizeof(pppoe_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PPPOE_INTERFACE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pppoe_info->type));
						}
					}
					else if (0 == strcmp(PPPOE_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pppoe_info->enable));
						}
					}
					else if (0 == strcmp(PPPOE_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pppoe_info->ip, 
									sizeof(pppoe_info->ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PPPOE_ACCOUNT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pppoe_info->account, 
									sizeof(pppoe_info->account), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PPPOE_PASSED_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pppoe_info->passwd, 
									sizeof(pppoe_info->passwd), 
									"%s", tree_node->element.contact);
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_pppoe_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_PPPOE_INFO_CMD);
}

int parse_pppoe_info_response_xml(void *pvalue, const char *buffer)
{
	PPPOEInfoPacket *pppoe_info = (PPPOEInfoPacket*)pvalue;
	
	return parse_pppoe_info_xml(pppoe_info, buffer, PPPOE_INFO_RESPONSE_CMD);
}

int parse_set_pppoe_info_xml(void *pvalue, const char *buffer)
{
	PPPOEInfoPacket *pppoe_info = (PPPOEInfoPacket*)pvalue;
	
	return parse_pppoe_info_xml(pppoe_info, buffer, SET_PPPOE_INFO_CMD);
}

int parse_pppoe_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, PPPOE_INFO_RESULT_CMD);
}


//##########################################################################
static int parse_encode_parameter_xml(EncodeParameterPacket *encode_para, 
																const char *buffer, 
																const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)encode_para->session_id, 
									sizeof(encode_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)encode_para->domain_id, 
									sizeof(encode_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)encode_para->gu_id, 
									sizeof(encode_para->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->result.code));
						}
					}
					else if (0 == strcmp(FRAME_RATE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->frame_rate));
						}
					}
					else if (0 == strcmp(I_FRAME_INTERVAL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->i_frame_interval));
						}
					}
					else if (0 == strcmp(VIDEO_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->video_type));
						}
					}
					else if (0 == strcmp(AUDIO_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->audio_type));
						}
					}
					else if (0 == strcmp(AUDIO_INPUT_MODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->au_in_mode));
						}
					}
					else if (0 == strcmp(AUDIO_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->audio_enble));
						}
					}
					else if (0 == strcmp(RESOLUTION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->resolution));
						}
					}
					else if (0 == strcmp(QPVALUE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->qp_value));
						}
					}
					else if (0 == strcmp(CODE_RATE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->code_rate));
						}
					}
					else if (0 == strcmp(FRAME_PRIORITY_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->frame_priority));
						}
					}
					else if (0 == strcmp(VIDEO_FORMAT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->format));
						}
					}
					else if (0 == strcmp(BIT_RATE_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->bit_rate));
						}
					}
					else if (0 == strcmp(ENCODE_LEVEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(encode_para->encode_level));
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}			
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_encode_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_ENCODE_PARAMETER_CMD);
}

int parse_encode_parameter_response_xml(void *pvalue, const char *buffer)
{
	EncodeParameterPacket *encode_para = (EncodeParameterPacket*)pvalue;
	
	return parse_encode_parameter_xml(encode_para, buffer, ENCODE_PARAMETER_RESPONSE_CMD);
}

int parse_set_encode_parameter_xml(void *pvalue, const char *buffer)
{
	EncodeParameterPacket *encode_para = (EncodeParameterPacket*)pvalue;
	
	return parse_encode_parameter_xml(encode_para, buffer, SET_ENCODE_PARAMETER_CMD);
}

int parse_encode_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, ENCODE_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_display_parameter_xml(DisplayParameterPacket *display_para, 
																const char *buffer, 
																const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(display_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)display_para->session_id, 
									sizeof(display_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)display_para->domain_id, 
									sizeof(display_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)display_para->gu_id, 
									sizeof(display_para->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CONTRAST_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(display_para->contrast));
						}
					}
					else if (0 == strcmp(BRIGHT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d",
									&(display_para->bright));
						}
					}
					else if (0 == strcmp(HUE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(display_para->hue));
						}
					}
					else if (0 == strcmp(SATURATION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(display_para->saturation));
						}
					}
					else if (0 == strcmp(SHARPNESS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(display_para->sharpness));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_display_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_DISPLAY_PARAMETER_CMD);
}

int parse_display_parameter_response_xml(void *pvalue, const char *buffer)
{
	DisplayParameterPacket *display_para = (DisplayParameterPacket*)pvalue;
	
	return parse_display_parameter_xml(display_para, buffer, DISPLAY_PARAMETER_RESPONSE_CMD);
}

int parse_set_display_parameter_xml(void *pvalue, const char *buffer)
{
	DisplayParameterPacket *display_para = (DisplayParameterPacket*)pvalue;
	
	return parse_display_parameter_xml(display_para, buffer, SET_DISPLAY_PARAMETER_CMD);
}

int parse_display_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, DISPLAY_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_record_parameter_xml(RecordParameterPacket *record_para, 
																const char *buffer, 
																const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeAttr *node_attr = NULL;
	
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(record_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)record_para->session_id, 
									sizeof(record_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)record_para->domain_id, 
									sizeof(record_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)record_para->gu_id, 
									sizeof(record_para->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PRE_RECORD_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(record_para->pre_record));
						}
					}
					else if (0 == strcmp(AUTO_COVER_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(record_para->auto_cover));
						}
					}
					else if (0 == strcmp(WEEK_DAY_S_STR, tree_node->element.name))
					{
						get_weekday_info(tree_node, &(record_para->week));
					}
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_record_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_RECORD_PARAMETER_CMD);
}

int parse_record_parameter_response_xml(void *pvalue, const char *buffer)
{
	RecordParameterPacket *record_para = (RecordParameterPacket*)pvalue;
	
	return parse_record_parameter_xml(record_para, buffer, RECORD_PARAMETER_RESPONSE_CMD);
}

int parse_set_record_parameter_xml(void *pvalue, const char *buffer)
{
	RecordParameterPacket *record_para = (RecordParameterPacket*)pvalue;
	
	return parse_record_parameter_xml(record_para, buffer, SET_RECORD_PARAMETER_CMD);
}

int parse_record_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, RECORD_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_hide_parameter_xml(HideParameterPacket *hide_para, 
													const char *buffer, 
													const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hide_para->session_id, 
									sizeof(hide_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hide_para->domain_id, 
									sizeof(hide_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hide_para->gu_id, 
									sizeof(hide_para->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(HIDE_AREA_STR, tree_node->element.name))
					{
						get_dection_area_info(tree_node, &(hide_para->hide_area));
					}
					else if (0 == strcmp(HIDE_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_para->hide_enable));
						}
					}
					else if (0 == strcmp(HIDE_COLOR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_para->hide_color));
						}
					}
					else if (0 == strcmp(MAX_WIDTH_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_para->max_width));
						}
					}
					else if (0 == strcmp(MAX_HEIGHT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_para->max_height));
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_hide_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_HIDE_PARAMETER_CMD);
}

int parse_hide_parameter_response_xml(void *pvalue, const char *buffer)
{
	HideParameterPacket *hide_para = (HideParameterPacket*)pvalue;
	
	return parse_hide_parameter_xml(hide_para, buffer, HIDE_PARAMETER_RESPONSE_CMD);
}

int parse_set_hide_parameter_xml(void *pvalue, const char *buffer)
{
	HideParameterPacket *hide_para = (HideParameterPacket*)pvalue;
	
	return parse_hide_parameter_xml(hide_para, buffer, SET_HIDE_PARAMETER_CMD);
}

int parse_hide_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, HIDE_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_serial_parameter_xml(SerialParameterPacket *serial_para, 
															const char *buffer, 
															const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(serial_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)serial_para->session_id, 
									sizeof(serial_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)serial_para->domain_id, 
									sizeof(serial_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)serial_para->pu_id, 
									sizeof(serial_para->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(SERIAL_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(serial_para->serial_no));
						}
					}
					else if (0 == strcmp(BAUD_RATE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(serial_para->baud_rate));
						}
					}
					else if (0 == strcmp(DATA_BIT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(serial_para->data_bit));
						}
					}
					else if (0 == strcmp(STOP_BIT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(serial_para->stop_bit));
						}
					}
					else if (0 == strcmp(VERIFY_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(serial_para->verify));
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_serial_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_SERIAL_PARAMETER_CMD);
}

int parse_serial_parameter_response_xml(void *pvalue, const char *buffer)
{
	SerialParameterPacket *serial_para = (SerialParameterPacket*)pvalue;
	
	return parse_serial_parameter_xml(serial_para, buffer, SERIAL_PARAMETER_RESPONSE_CMD);
}

int parse_set_serial_parameter_xml(void *pvalue, const char *buffer)
{
	SerialParameterPacket *serial_para = (SerialParameterPacket*)pvalue;
	
	return parse_serial_parameter_xml(serial_para, buffer, SET_SERIAL_PARAMETER_CMD);
}

int parse_serial_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, SERIAL_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_osd_parameter_xml(OSDParameterPacket *osd_para, 
				const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)osd_para->session_id, 
									sizeof(osd_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)osd_para->domain_id, 
									sizeof(osd_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)osd_para->gu_id, 
									sizeof(osd_para->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DISPLAY_TIME_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->time_enable));
						}
					}
					else if (0 == strcmp(TIME_DISPLAY_X_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->time_display_x));
						}
					}
					else if (0 == strcmp(TIME_DISPLAY_Y_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->time_display_y));
						}
					}
					else if (0 == strcmp(TIME_DISPLAY_COLOR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->time_display_color));
						}
					}
					else if (0 == strcmp(DISPLAY_TEXT_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->text_enable));
						}
					}
					else if (0 == strcmp(TEXT_DISPLAY_X_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->text_display_x));
						}
					}
					else if (0 == strcmp(TEXT_DISPLAY_Y_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->text_display_y));
						}
					}
					else if (0 == strcmp(TEXT_DISPLAY_COLOR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->text_display_color));
						}
					}
					else if (0 == strcmp(TEXT_DISPLAY_DATA_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)osd_para->text_data, 
									sizeof(osd_para->text_data), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MAX_WIDTH_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->max_width));
						}
					}
					else if (0 == strcmp(MAX_HEIGHT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->max_height));
						}
					}
					else if (0 == strcmp(DISPLAY_STREAM_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->stream_enable));
						}
					}
					else if (0 == strcmp(TIME_DISPLAY_W_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->time_display_w));
						}
					}
					else if (0 == strcmp(TIME_DISPLAY_H_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->time_display_h));
						}
					}
					else if (0 == strcmp(TEXT_DISPLAY_W_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->text_display_w));
						}
					}
					else if (0 == strcmp(TEXT_DISPLAY_H_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->text_display_h));
						}
					}
					else if (0 == strcmp(OSD_INVERT_COLOR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->osd_invert_color));
						}
					}
					
					else if (0 == strcmp(TEXT1_DISPLAY_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text1_enable));
						}
					}
                    else if (0 == strcmp(TEXT1_DISPLAY_DATA_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)osd_para->ext_osd.ext_text1_data, 
									sizeof(osd_para->ext_osd.ext_text1_data), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(TEXT1_DISPLAY_X_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text1_display_x));
						}
					}
					else if (0 == strcmp(TEXT1_DISPLAY_Y_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text1_display_y));
						}
					}
					else if (0 == strcmp(TEXT1_DISPLAY_W_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text1_display_w));
						}
					}
					else if (0 == strcmp(TEXT1_DISPLAY_H_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text1_display_h));
						}
					}
                    else if (0 == strcmp(TEXT2_DISPLAY_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text2_enable));
						}
					}
                    else if (0 == strcmp(TEXT2_DISPLAY_DATA_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)osd_para->ext_osd.ext_text2_data, 
									sizeof(osd_para->ext_osd.ext_text2_data), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(TEXT2_DISPLAY_X_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text2_display_x));
						}
					}
					else if (0 == strcmp(TEXT2_DISPLAY_Y_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text2_display_y));
						}
					}
					else if (0 == strcmp(TEXT2_DISPLAY_W_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text2_display_w));
						}
					}
					else if (0 == strcmp(TEXT2_DISPLAY_H_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(osd_para->ext_osd.ext_text2_display_h));
						}
					}
                    
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_osd_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_OSD_PARAMETER_CMD);
}

int parse_osd_parameter_response_xml(void *pvalue, const char *buffer)
{
	OSDParameterPacket *osd_para = (OSDParameterPacket*)pvalue;
	
	return parse_osd_parameter_xml(osd_para, buffer, OSD_PARAMETER_RESPONSE_CMD);
}

int parse_set_osd_parameter_xml(void *pvalue, const char *buffer)
{
	OSDParameterPacket *osd_para = (OSDParameterPacket*)pvalue;
	
	return parse_osd_parameter_xml(osd_para, buffer, SET_OSD_PARAMETER_CMD);
}

int parse_osd_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, OSD_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_ptz_parameter_xml(PTZParameterPacket *ptz_para, 
													const char *buffer, 
													const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ptz_para->session_id, 
									sizeof(ptz_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ptz_para->domain_id, 
									sizeof(ptz_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ptz_para->gu_id, 
									sizeof(ptz_para->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PTZ_SERIAL_NO, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_para->serial_no));
						}
					}
					else if (0 == strcmp(PTZ_ADDR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_para->ptz_addr));
						}
					}
					else if (0 == strcmp(PTZ_PROTOCOL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_para->protocol));
						}
					}
					else if (0 == strcmp(BAUD_RATE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_para->baud_rate));
						}
					}
					else if (0 == strcmp(DATA_BIT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_para->data_bit));
						}
					}
					else if (0 == strcmp(STOP_BIT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_para->stop_bit));
						}
					}
					else if (0 == strcmp(VERIFY_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_para->verify));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_ptz_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_PTZ_PARAMETER_CMD);
}

int parse_ptz_parameter_response_xml(void *pvalue, const char *buffer)
{
	PTZParameterPacket *ptz_para = (PTZParameterPacket*)pvalue;
	
	return parse_ptz_parameter_xml(ptz_para, buffer, PTZ_PARAMETER_RESPONSE_CMD);
}

int parse_set_ptz_parameter_xml(void *pvalue, const char *buffer)
{
	PTZParameterPacket *ptz_para = (PTZParameterPacket*)pvalue;
	
	return parse_ptz_parameter_xml(ptz_para, buffer, SET_PTZ_PARAMETER_CMD);
}

int parse_ptz_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, PTZ_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_ftp_parameter_xml(FTPParameterPacket *ftp_para, 
														const char *buffer, 
														const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ftp_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ftp_para->session_id, 
									sizeof(ftp_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ftp_para->domain_id, 
									sizeof(ftp_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ftp_para->pu_id, 
									sizeof(ftp_para->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(FTP_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ftp_para->ftp_ip, 
									sizeof(ftp_para->ftp_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(FTP_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ftp_para->ftp_port));
						}
					}
					else if (0 == strcmp(FTP_USR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ftp_para->ftp_usr, 
									sizeof(ftp_para->ftp_usr), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(FTP_PWD_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ftp_para->ftp_pwd, 
									sizeof(ftp_para->ftp_pwd), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(FTP_PATH_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ftp_para->ftp_path, 
									sizeof(ftp_para->ftp_path), 
									"%s", tree_node->element.contact);
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_ftp_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_FTP_PARAMETER_CMD);
}

int parse_ftp_parameter_response_xml(void *pvalue, const char *buffer)
{
	FTPParameterPacket *ftp_para = (FTPParameterPacket*)pvalue;
	
	return parse_ftp_parameter_xml(ftp_para, buffer, FTP_PARAMETER_RESPONSE_CMD);
}

int parse_set_ftp_parameter_xml(void *pvalue, const char *buffer)
{
	FTPParameterPacket *ftp_para = (FTPParameterPacket*)pvalue;
	
	return parse_ftp_parameter_xml(ftp_para, buffer, SET_FTP_PARAMETER_CMD);
}

int parse_ftp_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, FTP_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_smtp_parameter_xml(SMTPParameterPacket *smtp_para, 
														const char *buffer, 
														const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(smtp_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->session_id, 
									sizeof(smtp_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->domain_id, 
									sizeof(smtp_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->pu_id, 
									sizeof(smtp_para->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MAIL_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->mail_ip, 
									sizeof(smtp_para->mail_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MAIL_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(smtp_para->mail_port));
						}
					}
					else if (0 == strcmp(MAIL_ADDR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->mail_addr, 
									sizeof(smtp_para->mail_addr), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MAIL_USR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->mail_usr, 
									sizeof(smtp_para->mail_usr), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MAIL_PWD_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->mail_pwd, 
									sizeof(smtp_para->mail_pwd), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MAIL_RCTP1_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->mail_rctp1, 
									sizeof(smtp_para->mail_rctp1), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MAIL_RCTP2_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->mail_rctp2, 
									sizeof(smtp_para->mail_rctp2), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MAIL_RCTP3_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)smtp_para->mail_rctp3, 
									sizeof(smtp_para->mail_rctp3), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(SSL_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(smtp_para->ssl_enable));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_smtp_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_SMTP_PARAMETER_CMD);
}

int parse_smtp_parameter_response_xml(void *pvalue, const char *buffer)
{
	SMTPParameterPacket *smtp_para = (SMTPParameterPacket*)pvalue;
	
	return parse_smtp_parameter_xml(smtp_para, buffer, SMTP_PARAMETER_RESPONSE_CMD);
}

int parse_set_smtp_parameter_xml(void *pvalue, const char *buffer)
{
	SMTPParameterPacket *smtp_para = (SMTPParameterPacket*)pvalue;
	
	return parse_smtp_parameter_xml(smtp_para, buffer, SET_SMTP_PARAMETER_CMD);
}

int parse_smtp_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, SMTP_PARAMETER_RESULT_CMD);
}

//##########################################################################
static int parse_upnp_parameter_xml(UPNPParameterPacket *upnp_para, 
														const char *buffer, 
														const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upnp_para->session_id, 
									sizeof(upnp_para->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upnp_para->domain_id, 
									sizeof(upnp_para->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upnp_para->pu_id, 
									sizeof(upnp_para->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(UPNP_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upnp_para->upnp_ip, 
									sizeof(upnp_para->upnp_ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(UPNP_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_enable));
						}
					}
					else if (0 == strcmp(UPNP_ETH_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_eth_no));
						}
					}
					else if (0 == strcmp(UPNP_MODEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_model));
						}
					}
					else if (0 == strcmp(UPNP_REFRESH_TIME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_refresh_time));
						}
					}
					else if (0 == strcmp(UPNP_DATA_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_data_port));
						}
					}
					else if (0 == strcmp(UPNP_WEB_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_web_port));
						}
					}
					else if (0 == strcmp(UPNP_DATA_PORT_RESULT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_data_port_result));
						}
					}
					else if (0 == strcmp(UPNP_WEB_PORT_RESULT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_web_port_result));
						}
					}
					else if (0 == strcmp(UPNP_CMD_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_cmd_port));
						}
					}
                    else if (0 == strcmp(UPNP_TALK_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_talk_port));
						}
					}
                    else if (0 == strcmp(UPNP_CMD_PORT_RESULT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_cmd_port_result));
						}
					}
                    else if (0 == strcmp(UPNP_TALK_PORT_RESULT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upnp_para->upnp_talk_port_result));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_upnp_parameter_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_UPNP_PARAMETER_CMD);
}

int parse_upnp_parameter_response_xml(void *pvalue, const char *buffer)
{
	UPNPParameterPacket *upnp_para = (UPNPParameterPacket*)pvalue;
	
	return parse_upnp_parameter_xml(upnp_para, buffer, UPNP_PARAMETER_RESPONSE_CMD);
}

int parse_set_upnp_parameter_xml(void *pvalue, const char *buffer)
{
	UPNPParameterPacket *upnp_para = (UPNPParameterPacket*)pvalue;
	
	return parse_upnp_parameter_xml(upnp_para, buffer, SET_UPNP_PARAMETER_CMD);
}

int parse_upnp_parameter_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, UPNP_PARAMETER_RESULT_CMD);
}

//##########################################################################
int parse_get_device_disk_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_DEVICE_DISK_INFO_CMD);
}

int parse_device_disk_info_response_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	DeviceDiskInfoPacket *disk_info = (DeviceDiskInfoPacket*)pvalue;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(DEVICE_DISK_INFO_RESPONSE_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(disk_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)disk_info->session_id, 
									sizeof(disk_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)disk_info->domain_id, 
									sizeof(disk_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)disk_info->pu_id, 
									sizeof(disk_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DISK_INFO_STR, tree_node->element.name))
					{
						disk_info->disk_num = get_disk_info(tree_node, 
										(JDiskInfo*)&(disk_info->disk));
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

//##########################################################################
static int parse_format_disk_info_xml(FormatDiskPacket *format_disk, 
				const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(format_disk->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)format_disk->session_id, 
									sizeof(format_disk->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)format_disk->domain_id, 
									sizeof(format_disk->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)format_disk->pu_id, 
									sizeof(format_disk->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DISK_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(format_disk->disk_no));
						}
					}
					else if (0 == strcmp(FORMAT_PROGRESS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(format_disk->progress));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_format_disk_request_xml(void *pvalue, const char *buffer)
{
	FormatDiskPacket *format_disk = (FormatDiskPacket*)pvalue;
	
	return parse_format_disk_info_xml(format_disk, buffer, FORMAT_DISK_REQUEST_CMD);
}

int parse_format_disk_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, FORMAT_DISK_RESULT_CMD);
}

int parse_submit_format_progress_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	FormatProgressPacket *format;

	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (NULL == pvalue)
	{
		printf("pvalue NULL.\n");
		return -1;
	}
	format = (FormatProgressPacket*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(SUBMIT_FORMAT_PROGRESS_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)format->pu_id, 
									sizeof(format->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DISK_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(format->disk_no));
						}
					}
					else if (0 == strcmp(FORMAT_PROGRESS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(format->progress));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

//##########################################################################
/*static __inline__ void get_weekday_info(XmlTreeNode *tree_node, JWeekDay *weekday)
{
	int ret;
	int flag0 = 0, flag1 = 0;
	XmlTreeNode *xml_node, *xml_child, *xml_son;
	
	Day *new_day, *cur_day;
	JTimeSeg *new_seg, *cur_seg;
	
	J_ASSERT(tree_node);
		
	xml_node = tree_node->child;
	while (xml_node)
	{
		if (!strcmp(WEEK_DAY_STR, xml_node->element.name))
		{
			xml_child = xml_node->child;
			if (xml_child)
			{
				new_day = (Day*)j_xml_alloc(sizeof(Day));
				if (!flag0)
				{//printf("0.++++++++++++++\n");
					new_day->prev = NULL;
					new_day->next = NULL;
					
					weekday->day = new_day;					
					cur_day = new_day;
					flag0++;
				}
				else
				{//printf("1.++++++++++++++\n");
					new_day->prev = cur_day;
					cur_day->next = new_day;
					new_day->next = NULL;
					
					cur_day = new_day;
				}
			}
			
			if (xml_child && !strcmp(TIME_SEG_S_STR, xml_child->element.name))
			{
				flag1 = 0;
				xml_son = xml_child->child;
				while (xml_son)
				{
					if (!strcmp(TIME_SEG_STR, xml_son->element.name))
					{
						if (xml_son->element.contact)
						{
							new_seg = (JTimeSeg*)j_xml_alloc(sizeof(JTimeSeg));
							if (!flag1)
							{//printf("    0.----------------\n");
								new_seg->prev = NULL;
								new_seg->next = NULL;
								
								cur_day->seg = new_seg;
								cur_seg = new_seg;
								flag1++;
							}
							else
							{//printf("    1.----------------\n");
								new_seg->prev = cur_seg;
								cur_seg->next = new_seg;
								new_seg->next = NULL;
								
								cur_seg = new_seg;
							}
							
							ret = parse_time_part_string(xml_son->element.contact, 
											&(new_seg->time_start), 
											&(new_seg->time_end));
							if (0 != ret)
							{
								//xml_tree_delete(xml_tree);
								//return -1;
							}
						}
					}
					xml_son = xml_son->next;
				}
			}
		}
		xml_node = xml_node->next;
	}
}*/
static int parse_move_alarm_info_xml(MoveAlarmPacket *move_alarm, 
														const char *buffer, 
														const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(move_alarm->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)move_alarm->session_id, 
									sizeof(move_alarm->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)move_alarm->domain_id, 
									sizeof(move_alarm->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)move_alarm->gu_id, 
									sizeof(move_alarm->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MOVE_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(move_alarm->move_enable));
						}
					}
					else if (0 == strcmp(SENSITIVE_LEVEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(move_alarm->sensitive_level));
						}
					}
					else if (0 == strcmp(DETECT_INTERVAL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(move_alarm->detect_interval));
						}
					}
					else if (0 == strcmp(MAX_WIDTH_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(move_alarm->max_width));
						}
					}
					else if (0 == strcmp(MAX_HEIGHT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(move_alarm->max_height));
						}
					}
					else if (0 == strcmp(DETECT_AREA_STR, tree_node->element.name))
					{
						get_dection_area_info(tree_node, &(move_alarm->detect_area));
					}
					else if (0 == strcmp(WEEK_DAY_S_STR, tree_node->element.name))
					{
						get_weekday_info(tree_node, &(move_alarm->week));
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_move_alarm_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_MOVE_ALARM_INFO_CMD);
}

int parse_move_alarm_info_response_xml(void *pvalue, const char *buffer)
{
	MoveAlarmPacket *move_alarm = (MoveAlarmPacket*)pvalue;
	
	return parse_move_alarm_info_xml(move_alarm, buffer, MOVE_ALARM_INFO_RESPONSE_CMD);
}

int parse_set_move_alarm_info_xml(void *pvalue, const char *buffer)
{
	MoveAlarmPacket *move_alarm = (MoveAlarmPacket*)pvalue;
	
	return parse_move_alarm_info_xml(move_alarm, buffer, SET_MOVE_ALARM_INFO_CMD);
}

int parse_move_alarm_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, MOVE_ALARM_INFO_RESULT_CMD);
}

//##########################################################################
static int parse_lost_alarm_info_xml(LostAlarmPacket *lost_alarm, 
														const char *buffer, 
														const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(lost_alarm->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)lost_alarm->session_id, 
									sizeof(lost_alarm->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)lost_alarm->domain_id, 
									sizeof(lost_alarm->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)lost_alarm->gu_id, 
									sizeof(lost_alarm->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					/*else if (0 == strcmp(CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(lost_alarm->channel));
						}
					}
					else if (0 == strcmp(GU_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(lost_alarm->gu_type));
						}
					}*/
					else if (0 == strcmp(LOST_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(lost_alarm->lost_enable));
						}
					}
					else if (0 == strcmp(DETECT_INTERVAL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(lost_alarm->detect_interval));
						}
					}
					else if (0 == strcmp(WEEK_DAY_S_STR, tree_node->element.name))
					{
						get_weekday_info(tree_node, &(lost_alarm->week));
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_lost_alarm_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_LOST_ALARM_INFO_CMD);
}

int parse_lost_alarm_info_response_xml(void *pvalue, const char *buffer)
{
	LostAlarmPacket *lost_alarm = (LostAlarmPacket*)pvalue;
	
	return parse_lost_alarm_info_xml(lost_alarm, buffer, LOST_ALARM_INFO_RESPONSE_CMD);
}

int parse_set_lost_alarm_info_xml(void *pvalue, const char *buffer)
{
	LostAlarmPacket *lost_alarm = (LostAlarmPacket*)pvalue;
	
	return parse_lost_alarm_info_xml(lost_alarm, buffer, SET_LOST_ALARM_INFO_CMD);
}

int parse_lost_alarm_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, LOST_ALARM_INFO_RESULT_CMD);
}

//##########################################################################
static int parse_hide_alarm_info_xml(HideAlarmPacket *hide_alarm, 
														const char *buffer, 
														const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_alarm->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hide_alarm->session_id, 
									sizeof(hide_alarm->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hide_alarm->domain_id, 
									sizeof(hide_alarm->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hide_alarm->gu_id, 
									sizeof(hide_alarm->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(HIDE_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_alarm->hide_enable));
						}
					}
					else if (0 == strcmp(DETECT_INTERVAL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_alarm->detect_interval));
						}
					}
					else if (0 == strcmp(SENSITIVE_LEVEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_alarm->sensitive_level));
						}
					}
					else if (0 == strcmp(MAX_WIDTH_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_alarm->max_width));
						}
					}
					else if (0 == strcmp(MAX_HEIGHT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hide_alarm->max_height));
						}
					}
					else if (0 == strcmp(DETECT_AREA_STR, tree_node->element.name))
					{
						get_dection_area_info(tree_node, &(hide_alarm->detect_area));
					}
					else if (0 == strcmp(WEEK_DAY_S_STR, tree_node->element.name))
					{
						get_weekday_info(tree_node, &(hide_alarm->week));
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_hide_alarm_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_HIDE_ALARM_INFO_CMD);
}

int parse_hide_alarm_info_response_xml(void *pvalue, const char *buffer)
{
	HideAlarmPacket *hide_alarm = (HideAlarmPacket*)pvalue;
	
	return parse_hide_alarm_info_xml(hide_alarm, buffer, HIDE_ALARM_INFO_RESPONSE_CMD);
}

int parse_set_hide_alarm_info_xml(void *pvalue, const char *buffer)
{
	HideAlarmPacket *hide_alarm = (HideAlarmPacket*)pvalue;
	
	return parse_hide_alarm_info_xml(hide_alarm, buffer, SET_HIDE_ALARM_INFO_CMD);
}

int parse_hide_alarm_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, HIDE_ALARM_INFO_RESULT_CMD);
}

//##########################################################################
static int parse_io_alarm_info_xml(IoAlarmPacket *io_alarm, 
												const char *buffer, 
												const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(io_alarm->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)io_alarm->session_id, 
									sizeof(io_alarm->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)io_alarm->domain_id, 
									sizeof(io_alarm->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)io_alarm->gu_id, 
									sizeof(io_alarm->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(IO_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(io_alarm->io_type));
						}
					}
					else if (0 == strcmp(IO_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(io_alarm->alarm_enable));
						}
					}
					else if (0 == strcmp(DETECT_INTERVAL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(io_alarm->detect_interval));
						}
					}
					else if (0 == strcmp(WEEK_DAY_S_STR, tree_node->element.name))
					{
						get_weekday_info(tree_node, &(io_alarm->week));
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_io_alarm_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_IO_ALARM_INFO_CMD);
}

int parse_io_alarm_info_response_xml(void *pvalue, const char *buffer)
{
	IoAlarmPacket *io_alarm = (IoAlarmPacket*)pvalue;
	
	return parse_io_alarm_info_xml(io_alarm, buffer, IO_ALARM_INFO_RESPONSE_CMD);
}

int parse_set_io_alarm_info_xml(void *pvalue, const char *buffer)
{
	IoAlarmPacket *io_alarm = (IoAlarmPacket*)pvalue;
	
	return parse_io_alarm_info_xml(io_alarm, buffer, SET_IO_ALARM_INFO_CMD);
}

int parse_io_alarm_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, IO_ALARM_INFO_RESULT_CMD);
}

//##########################################################################
static int parse_joint_info_xml(JointActionPacket *joint_action, 
									const char *buffer, 
									const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(joint_action->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)joint_action->session_id, 
									sizeof(joint_action->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)joint_action->domain_id, 
									sizeof(joint_action->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)joint_action->gu_id, 
									sizeof(joint_action->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(ALARM_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(joint_action->alarm_type));
						}
					}
					else if (0 == strcmp(JOINT_ACTION_STR, tree_node->element.name))
					{
						get_joint_info(tree_node, &(joint_action->joint));
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_joint_action_info_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_JOINT_ACTION_INFO_CMD);
}

int parse_joint_action_info_response_xml(void *pvalue, const char *buffer)
{
	JointActionPacket *joint_action = (JointActionPacket*)pvalue;
	
	return parse_joint_info_xml(joint_action, buffer, JOINT_ACTION_INFO_RESPONSE_CMD);
}

int parse_set_joint_action_info_xml(void *pvalue, const char *buffer)
{
	JointActionPacket *joint_action = (JointActionPacket*)pvalue;
	
	return parse_joint_info_xml(joint_action, buffer, SET_JOINT_ACTION_INFO_CMD);
}

int parse_joint_action_info_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, JOINT_ACTION_INFO_RESULT_CMD);
}

//##########################################################################
static int parse_control_ptz_xml(PTZControlPacket *ptz_ctrl, 
										const char *buffer, 
										const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_ctrl->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ptz_ctrl->session_id, 
									sizeof(ptz_ctrl->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ptz_ctrl->domain_id, 
									sizeof(ptz_ctrl->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ptz_ctrl->gu_id, 
									sizeof(ptz_ctrl->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PTZ_DIRECTION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_ctrl->action));
						}
					}
					else if (0 == strcmp(PTZ_PARAM_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ptz_ctrl->param));
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_control_ptz_cmd_xml(void *pvalue, const char *buffer)
{
	PTZControlPacket *ptz_ctrl = (PTZControlPacket*)pvalue;
	
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
	
	return parse_control_ptz_xml(ptz_ctrl, buffer, CONTROL_PTZ_COMMAND_CMD);
}

int parse_ptz_cmd_response_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
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
	
	return parse_result_action_xml(result, buffer, PTZ_COMMAND_RESULT_CMD);
}

//##########################################################################
int parse_submit_alarm_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(SUBMIT_ALARM_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)sub_alarm->pu_id, 
									sizeof(sub_alarm->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(sub_alarm->channel));
						}
					}
					else if (0 == strcmp(ALARM_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(sub_alarm->alarm_type));
						}
					}
					else if (0 == strcmp(ACTION_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(sub_alarm->action_type));
						}
					}
					else if (0 == strcmp(ALARM_TIME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							if (0 == parse_time_string(tree_node->element.contact, 
														&(sub_alarm->alarm_time)))
							{
							}
							else
							{
								xml_tree_delete(xml_tree);
								return -1;
							}
						}
					}
					else if (0 == strcmp(DATA_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)sub_alarm->data, 
									sizeof(sub_alarm->data), 
									"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

//##########################################################################
static __inline__ int 
parse_media_url_xml(MediaUrlPacket *media, 
		const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(media->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)media->session_id, 
									sizeof(media->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)media->domain_id, 
									sizeof(media->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)media->gu_id, 
									sizeof(media->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MEDIA_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(media->media_type));
						}
					}
					else if (0 == strcmp(CMS_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)media->ip, sizeof(media->ip), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(URL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)media->url, sizeof(media->url), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CU_IP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)media->cu_ip, sizeof(media->cu_ip), 
									"%s", tree_node->element.contact);
						}
					}
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_media_url_request_xml(void *pvalue, const char *buffer)
{
	MediaUrlPacket *media = (MediaUrlPacket*)pvalue;
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
	
	return parse_media_url_xml(media, buffer, GET_MEDIA_URL_REQUEST_CMD);
}
int parse_get_media_url_response_xml(void *pvalue, const char *buffer)
{	
	MediaUrlPacket *media = (MediaUrlPacket*)pvalue;
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
	
	return parse_media_url_xml(media, buffer, GET_MEDIA_URL_RESPONSE_CMD);
}

static __inline__ void  
parse_recode_info(Store *store, XmlTreeNode *tree_node)
{
	int count = 0;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	tree_child = tree_node->child;
	
	while (NULL != tree_child)
	{
		if (!strcmp(REC_NODE_STR, tree_child->element.name))
		{
			tree_son = tree_child->child;
			while (tree_son)
			{
				if (!strcmp(REC_TYPE_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&store[count].rec_type);
					}                
				}
				else if (!strcmp(FILE_SIZE_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&store[count].file_size);
					}
				}
				else if (!strcmp(BEG_TIME_STR, tree_son->element.name))
				{
					if (tree_son->element.contact)
					{
						parse_time_string(tree_son->element.contact, 
							&(store[count].beg_time));
					}
				}
				else if (!strcmp(END_TIME_STR, tree_son->element.name))
				{
					if (tree_son->element.contact)
					{
						parse_time_string(tree_son->element.contact, 
							&(store[count].end_time));
					}
				}
				else if (!strcmp(PROPERTY_STR, tree_son->element.name))
				{
					if (tree_son->element.contact)
					{
						snprintf((char*)store[count].property, sizeof(store[count].property), 
								"%s", tree_son->element.contact);
					}
				}

				tree_son = tree_son->next;
			}
			count++;
		}
		
		tree_child = tree_child->next;
	}
}
static __inline__ int 
parse_store_log_info(StoreLogPacket *store_log, 
	const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)store_log->session_id, sizeof(store_log->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)store_log->domain_id, sizeof(store_log->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)store_log->gu_id, sizeof(store_log->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(REC_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&store_log->rec_type);
						}
					}
					else if (0 == strcmp(BEG_NODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&store_log->beg_node);
						}
					}
					else if (0 == strcmp(END_NODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&store_log->end_node);
						}
					}
					else if (0 == strcmp(BEG_TIME_STR, tree_node->element.name))
					{
						if (tree_node->element.contact)
						{
							parse_time_string(tree_node->element.contact, 
								&(store_log->beg_time));
						}
					}
					else if (0 == strcmp(END_TIME_STR, tree_node->element.name))
					{
						if (tree_node->element.contact)
						{
							parse_time_string(tree_node->element.contact, 
								&(store_log->end_time));
						}
					}
					else if (0 == strcmp(TOTAL_LOG_COUNT_STR, tree_node->element.name))
					{
						if (tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&store_log->total_count);
						}
					}
					else if (0 == strcmp(SESS_ID_STR, tree_node->element.name))
					{
						if (tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&store_log->sess_id);
						}
					}
					else if (0 == strcmp(REC_INFO_STR, tree_node->element.name))
					{
						node_attr = tree_node->element.attr;
						if ((NULL != node_attr) && 
							(!strcmp(NODE_COUNT_STR, node_attr->name)))
						{
							sscanf(node_attr->value, "%d", 
								&store_log->node_count);
						}
						parse_recode_info(store_log->store, tree_node);
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_store_log_request_xml(void *pvalue, 
		const char *buffer)
{
	StoreLogPacket *store_log = (StoreLogPacket*)pvalue;
	
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
	
	return parse_store_log_info(store_log, buffer, GET_STORE_LOG_REQUEST_CMD);
}
int parse_get_store_log_response_xml(void *pvalue, 
          const char *buffer)
{
   StoreLogPacket *store_log = (StoreLogPacket*)pvalue;
	
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
	
	return parse_store_log_info(store_log, buffer, GET_STORE_LOG_RESPONSE_CMD);

  /*int ret;
  int index = 0;
  
  XmlTreeNode *xml_tree = NULL;
  XmlTreeNode *tree_root = NULL;
  XmlTreeAttr *node_attr = NULL;
  XmlTreeNode *tree_node = NULL;
  XmlTreeNode *tree_child = NULL;

  char time_buffer[J_SDK_MAX_TIME_LEN];
  struct _prx_store_log_list *store_log_list = (struct _prx_store_log_list *)pvalue;
  
  xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
  init_xml_tree_node(&xml_tree);
  
  if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
  {
    tree_root = xml_tree->child;

    if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
    {
      node_attr = tree_root->element.attr;
      if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
                   (0 == strcmp(GET_STORE_LOG_ID_RESULT_CMD, node_attr->value)))
      {
        tree_node = tree_root->child;

        if (NULL != tree_node)
        {
          if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
          {
            if (NULL != tree_node->element.contact)
            {
              sscanf(tree_node->element.contact, "%d", 
                &store_log_list->result);
            }
          }
         
          tree_node = tree_node->next;              //recInfo
          while (NULL != tree_node)
          {
            tree_child= tree_node->child;
            while (NULL != tree_child)
            {
              if (0 == strcmp(REC_TYPE_STR, tree_child->element.name))
              {

                if (NULL != tree_child->element.contact)
                {
                  sscanf(tree_child->element.contact, "%d", 
                    &store_log_list->store_log[index].record_type);
                }                
              }              
              else if (0 == strcmp(BEG_TIME_STR, tree_child->element.name))
              {
              	if (tree_child->element.contact)
      					{
      						ret = parse_time_string(tree_child->element.contact, 
      										&(store_log_list->store_log[index].start_time));
      						if (0 != ret)
      						{
      							xml_tree_delete(xml_tree);
      							return -1;
      						}
      					}
              }
              else if (0 == strcmp(END_TIME_STR, tree_child->element.name))
              {
              	if (tree_child->element.contact)
      					{
      						ret = parse_time_string(tree_child->element.contact, 
      										&(store_log_list->store_log[index].stop_time));
      						if (0 != ret)
      						{
      							xml_tree_delete(xml_tree);
      							return -1;
      						}
      					}
              }

              else if (0 == strcmp(FILE_SIZE_STR, tree_child->element.name))
              {
                if (NULL != tree_child->element.contact)
                {
                  sscanf(tree_child->element.contact, "%d", 
                    &store_log_list->store_log[index].file_size);
                } 
              }

              tree_child = tree_child->next;
            }
            if (0 == strcmp(TOTAL_LOG_COUNT_STR, tree_node->element.name))
            {
              if (NULL != tree_node->element.contact)
              {
                sscanf(tree_node->element.contact, "%d", 
                  &store_log_list->log_count);
              } 
            }
            tree_node = tree_node->next;
          }  
          index++;
        }   
        xml_tree_delete(xml_tree);
	      return 0;
     }
   }
  }

  xml_tree_delete(xml_tree);
  return 0;*/
}



//##########################################################################
static __inline__ int 
parse_user_info_xml(struct __UserInfo *user_info, 
		const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(USERNAME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)user_info->username, 
									sizeof(user_info->username), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PASSWORD_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)user_info->password, 
									sizeof(user_info->password), 
									"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
static __inline__ int 
parse_result_info_xml(int *result, 
		const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", result);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_user_login_request_xml(void *pvalue, const char *buffer)
{
	struct __UserInfo *user_info = (struct __UserInfo*)pvalue;
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
	
	return parse_user_info_xml(user_info, buffer, USER_LONGIN_REQUEST_CMD);
}

int parse_user_login_result_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_result_info_xml((int*)pvalue, buffer, USER_LONGIN_RESULT_CMD);
}

int parse_user_heart_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
//	struct __UserHeart *heart = (struct __UserHeart*)pvalue;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(USER_HEART_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(USERNAME_STR, tree_node->element.name))
					{
						/*if (NULL != tree_node->element.contact)
						{
							snprintf((char*)heart->username, 
									sizeof(heart->username), 
									"%s", tree_node->element.contact);
						}*/
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}

int parse_user_heart_response_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	struct __UserHeart *heart = (struct __UserHeart*)pvalue;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(USER_HEART_RESPONSE_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SERVER_TIME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							if (0 == parse_time_string(tree_node->element.contact, 
														&(heart->server_time)))
							{
							}
							else
							{
								xml_tree_delete(xml_tree);
								return -1;
							}
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}



//##########################################################################
static __inline__ int 
parse_firmware_upgrade_info_xml(FirmwareUpgradePacket *upgrade_info, 
		const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upgrade_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upgrade_info->session_id, 
									sizeof(upgrade_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upgrade_info->domain_id, 
									sizeof(upgrade_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upgrade_info->pu_id, 
									sizeof(upgrade_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(UPGREADE_ADDR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upgrade_info->addr, 
									sizeof(upgrade_info->addr), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(UPGREADE_DATA_LEN_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upgrade_info->data_len));
						}
					}
					else if (0 == strcmp(UPGREADE_DATA_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upgrade_info->data, 
									sizeof(upgrade_info->data), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(FILE_LENGTH_LEN_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upgrade_info->file_len));
						}
					}
					else if (0 == strcmp(SESS_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upgrade_info->sess_id));
						}
					}
					
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_firmware_upgrade_request_xml(void *pvalue, const char *buffer)
{
	FirmwareUpgradePacket *upgrade_info = (FirmwareUpgradePacket*)pvalue;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_firmware_upgrade_info_xml(upgrade_info, buffer, FIRMWARE_UPGRADE_REQUEST_CMD);
}
int parse_firmware_upgrade_response_xml(void *pvalue, const char *buffer)
{
	FirmwareUpgradePacket *upgrade_info = (FirmwareUpgradePacket*)pvalue;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_firmware_upgrade_info_xml(upgrade_info, buffer, FIRMWARE_UPGRADE_RESPONSE_CMD);
}
int parse_submit_upgrade_progress_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	UpgradeProgressPacket *upgrade = (UpgradeProgressPacket*)pvalue;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(SUBMIT_UPGRADE_PROGRESS_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)upgrade->pu_id, 
									sizeof(upgrade->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(UPGREADE_PROGRESS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(upgrade->percent));
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

#ifdef HAVE_PROXY_INFO

//##########################################################################
int parse_add_user_request_xml(void *pvalue, const char *buffer)
{
	struct __UserInfo *user_info = (struct __UserInfo*)pvalue;
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
	
	return parse_user_info_xml(user_info, buffer, ADD_USER_REQUEST_CMD);
}

int parse_add_user_result_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_result_info_xml((int*)pvalue, buffer, ADD_USER_RESULT_CMD);
}

int parse_del_user_request_xml(void *pvalue, const char *buffer)
{
	struct __UserInfo *user_info = (struct __UserInfo*)pvalue;
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
	
	return parse_user_info_xml(user_info, buffer, DEL_USER_REQUEST_CMD);
}

int parse_del_user_result_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_result_info_xml((int*)pvalue, buffer, DEL_USER_RESULT_CMD);
}

//##########################################################################
int parse_proxy_user_list_xml(void *pvalue, const char *buffer)
{
	int entries = 0;
	struct _prx_user_st *user_list;
	struct __UserInfo *user_info;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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

	user_list = (struct _prx_user_st*)pvalue;
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(USER_LIST_INFO_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					entries++;
					tree_node = tree_node->next;
				}
				if (!entries)
				{
					xml_tree_delete(xml_tree);
					return 0;
				}
				
				user_list->user_info = j_xml_alloc(entries * sizeof(struct __UserInfo));
				
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					user_info = &user_list->user_info[user_list->count++];
					
					snprintf((char*)user_info->username, sizeof(user_info->username)-1, 
						"%s", tree_node->element.name);
					snprintf((char*)user_info->password, sizeof(user_info->password)-1, 
						"%s", tree_node->element.contact);
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

//##########################################################################
int parse_fuzzy_find_user_request_xml(void *pvalue, const char *buffer)
{
	struct __UserInfo *user_info = (struct __UserInfo*)pvalue;
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
	
	return parse_user_info_xml(user_info, buffer, FUZZY_FIND_USER_REQUEST_CMD);
}

int parse_fuzzy_find_user_result_xml(void *pvalue, const char *buffer)
{
	int entries = 0;
	struct _prx_user_st *user_list;
	struct __UserInfo *user_info;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);

	user_list = (struct _prx_user_st*)pvalue;
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(FUZZY_FIND_USER_RESULT_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &user_list->result);
						}
					}
					else if (0 == strcmp(USER_LIST_STR, tree_node->element.name))
					{					
						tree_child = tree_node->child;
						while (NULL != tree_child)
						{
							entries++;
							tree_child = tree_child->next;					
						}
						if (!entries)
							return 0;
						
						user_list->user_info = j_xml_alloc(entries * sizeof(struct __UserInfo));
						
						tree_child = tree_node->child;
						while (NULL != tree_child)
						{
							user_info = &user_list->user_info[user_list->count++];
							
							if (0 == strcmp(USERNAME_STR, tree_child->element.name))
							{
								if (NULL != tree_child->element.contact)
								{
									snprintf((char*)user_info->username, 
											sizeof(user_info->username), 
											"%s", tree_child->element.contact);
								}
							}
							
							tree_child = tree_child->next;
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

//##########################################################################
int parse_modify_password_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	prx_modify_pwd *modify_pwd = (prx_modify_pwd*)pvalue;
	
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(MODIFY_PASSWORD_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(USERNAME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)modify_pwd->username, 
									sizeof(modify_pwd->username), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(OLD_PASSWORD_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)modify_pwd->old_pwd, 
									sizeof(modify_pwd->old_pwd), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(NEW_PASSWORD_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)modify_pwd->new_pwd, 
									sizeof(modify_pwd->new_pwd), 
									"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_modify_password_result_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_result_info_xml((int*)pvalue, 
			buffer, MODIFY_PASSWORD_RESULT_CMD);
}


//##########################################################################
static __inline__ void 
get_proxy_device_info(prx_device_info *dev_info, XmlTreeNode *node)
{
	if (0 == strcmp(RESULT_CODE_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			sscanf(node->element.contact, "%d", 
					&(dev_info->result));
		}
	}
	else if (0 == strcmp(PU_ID_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			snprintf((char*)dev_info->pu_id, 
					sizeof(dev_info->pu_id), 
					"%s", node->element.contact);
		}
	}
	else if (0 == strcmp(DEVICE_ID_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			sscanf(node->element.contact, "%d", 
					&(dev_info->device_id));
		}
	}
	else if (0 == strcmp(DEVICE_TYPE_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			sscanf(node->element.contact, "%d", 
					&(dev_info->pu_type));
		}
	}
	else if (0 == strcmp(PROTOCOL_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			sscanf(node->element.contact, "%d", 
					&(dev_info->protocol));
		}
	}
	else if (0 == strcmp(FACTORY_NAME_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			snprintf((char*)dev_info->factory, 
					sizeof(dev_info->factory), 
					"%s", node->element.contact);
		}
	}
	else if (0 == strcmp(SDK_VERSION_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			snprintf((char*)dev_info->sdk_version, 
					sizeof(dev_info->sdk_version), 
					"%s", node->element.contact);
		}
	}
	else if (0 == strcmp(USERNAME_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			snprintf((char*)dev_info->username, 
					sizeof(dev_info->username), 
					"%s", node->element.contact);
		}
	}
	else if (0 == strcmp(PASSWORD_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			snprintf((char*)dev_info->password, 
					sizeof(dev_info->password), 
					"%s", node->element.contact);
		}
	}
	else if (0 == strcmp(DEVICE_IP_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			snprintf((char*)dev_info->device_ip, 
					sizeof(dev_info->device_ip), 
					"%s", node->element.contact);
		}
	}
	else if (0 == strcmp(DEVICE_PORT_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			sscanf(node->element.contact, "%d", 
					&(dev_info->device_port));
		}
	}
	else if (0 == strcmp(PLATFORM_IP_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			snprintf((char*)dev_info->platform_ip, 
					sizeof(dev_info->platform_ip), 
					"%s", node->element.contact);
		}
	}
	else if (0 == strcmp(PLATFORM_PORT_STR, node->element.name))
	{
		if (NULL != node->element.contact)
		{
			sscanf(node->element.contact, "%d", 
					&(dev_info->platform_port));
		}
	}
}

static __inline__ int 
parse_proxy_device_info_xml(prx_device_info *dev_info, 
									const char *buffer, 
									const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					get_proxy_device_info(dev_info, tree_node);
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_proxy_device_list_xml(void *pvalue, const char *buffer)
{
	int entries = 0;
	struct _prx_device_st *dev_list;
	struct _prx_device_info *dev_info;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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

	dev_list = (struct _prx_device_st*)pvalue;
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(DEVICE_LIST_INFO_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					entries++;
					tree_node = tree_node->next;
				}
				if (!entries)
				{
					xml_tree_delete(xml_tree);
					return 0;
				}
					
				dev_list->device_info = j_xml_alloc(entries * sizeof(struct _prx_device_info));
				
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DEVICE_STR, tree_node->element.name))
					{
						dev_info = &dev_list->device_info[dev_list->count++];
						
						node_attr = tree_node->element.attr;
						if (0 == strcmp(ID_STR, node_attr->name))
						{
							sscanf(node_attr->value, "%d", 
									&(dev_info->device_id));
						}

						tree_child = tree_node->child;
						while (NULL != tree_child)
						{
							get_proxy_device_info(dev_info, tree_child);
							tree_child = tree_child->next;
						}
						
						tree_node = tree_node->next;
					}
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_add_device_request_xml(void *pvalue, const char *buffer)
{
	prx_device_info *dev_info = (prx_device_info*)pvalue;
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
	
	return parse_proxy_device_info_xml(dev_info, buffer, ADD_DEVICE_REQUEST_CMD);
}

int parse_add_device_result_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_result_info_xml((int*)pvalue, buffer, ADD_DEVICE_RESULT_CMD);
}
static __inline__ int 
parse_device_id_xml(int *device_id, 
									const char *buffer, 
									const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DEVICE_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", device_id);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_del_device_request_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_device_id_xml((int*)pvalue, buffer, DEL_DEVICE_REQUEST_CMD);
}
int parse_del_device_result_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_result_info_xml((int*)pvalue, buffer, DEL_DEVICE_RESULT_CMD);
}
static __inline__ int 
parse_page_device_info_xml(struct _prx_page_device *page_dev, 
	const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_chile = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(CONDITION_STR, tree_node->element.name))
					{
						tree_chile = tree_node->child;
						while (tree_chile)
						{
							if (0 == strcmp(FACTORY_NAME_STR, tree_chile->element.name))
							{
								if (NULL != tree_chile->element.contact)
								{
									snprintf((char*)page_dev->factory_name, 
											sizeof(page_dev->factory_name), 
											"%s", tree_chile->element.contact);
								}
							}
							else if (0 == strcmp(DEVICE_TYPE_STR, tree_chile->element.name))
							{
								if (NULL != tree_chile->element.contact)
								{
									sscanf(tree_chile->element.contact, "%d", 
										&page_dev->machine_type);
								}
							}
							else if (0 == strcmp(SDK_VERSION_STR, tree_chile->element.name))
							{
								if (NULL != tree_chile->element.contact)
								{
									snprintf((char*)page_dev->sdk_version, 
											sizeof(page_dev->sdk_version), 
											"%s", tree_chile->element.contact);
								}
							}
							else if (0 == strcmp(DEVICE_ID_STR, tree_chile->element.name))
							{
								if (NULL != tree_chile->element.contact)
								{
									sscanf(tree_chile->element.contact, "%d", 
										&page_dev->dev_id);
								}
							}
							
							tree_chile = tree_chile->next;
						}
					}
					else if (0 == strcmp(CONDITION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_chile->element.contact, "%d", 
								&page_dev->machine_type);
						}
					}
					else if (0 == strcmp(OFFSET_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&page_dev->offset);
						}
					}
					else if (0 == strcmp(COUNT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&page_dev->count);
						}
					}	
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_device_info_request_xml(void *pvalue, const char *buffer)
{	
	struct _prx_page_device *page_dev = (struct _prx_page_device*)pvalue;
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
	
	return parse_page_device_info_xml(page_dev, buffer, GET_DEVICE_INFO_REQUEST_CMD);
}
int parse_get_device_info_result_xml(void *pvalue, const char *buffer)
{
	prx_device_info *dev_info = (prx_device_info*)pvalue;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_proxy_device_info_xml(dev_info, buffer, GET_DEVICE_INFO_RESULT_CMD);
}
int parse_set_device_info_request_xml(void *pvalue, const char *buffer)
{
	prx_device_info *dev_info = (prx_device_info*)pvalue;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_proxy_device_info_xml(dev_info, buffer, SET_DEVICE_INFO_REQUEST_CMD);
}
int parse_set_device_info_result_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_DEVICE_INFO_RESULT_CMD);
}
int parse_get_all_device_id_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(GET_ALL_DEVICE_ID_REQUEST_CMD, node_attr->value)))
			{
				/*tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DEVICE_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", device_id);
						}
					}
					
					tree_node = tree_node->next;
				}*/
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_all_device_id_result_xml(void *pvalue, const char *buffer)
{
	int index = 0;
	struct _prx_device_id_st *id_list;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(GET_ALL_DEVICE_ID_RESULT_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				if (0 == strcmp(DEVICE_ID_LIST_STR, tree_node->element.name))
				{
					tree_child = tree_node->next;
					while (NULL != tree_child)
					{
						index++;
						tree_child = tree_child->next;
					}
					if (!index)
						return 0;

					id_list->device_id = (int*)j_xml_alloc(index * sizeof(int));

					tree_child = tree_node->next;
					while (NULL != tree_child)
					{
						if (0 == strcmp(DEVICE_ID_STR, tree_child->element.name))
						{
							if (NULL != tree_child->element.contact)
							{
								sscanf(tree_child->element.contact, "%d", 
									&id_list->device_id[id_list->count++]);
							}
						}
						
						tree_child = tree_child->next;
					}
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

static __inline__ int 
parse_proxy_factory_info_xml(prx_factory_list *fct_list, 
	const char *buffer, const char *command)
{
	int entries = 0;
	struct _prx_factory_info *fct_info;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(command, node_attr->value)))
			{//FACTORY_STR
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					entries++;
					tree_node = tree_node->next;
				}
				if (!entries)
				{
					xml_tree_delete(xml_tree);
					return 0;
				}
					
				fct_list->factory = j_xml_alloc(entries * sizeof(struct _prx_factory_info));
				
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					fct_info = &fct_list->factory[fct_list->count++];					

					tree_child = tree_node->child;
					while (NULL != tree_child)
					{
						if (0 == strcmp(FACTORY_NAME_STR, tree_child->element.name))
						{
							if (NULL != tree_child->element.contact)
							{
								snprintf((char*)fct_info->factory_name, 
										sizeof(fct_info->factory_name), 
										"%s", tree_child->element.contact);
							}
						}
						else if (0 == strcmp(MACHINE_LIST_STR, tree_child->element.name))
						{
							tree_son = tree_child->child;
							while (tree_son)
							{
								if (0 == strcmp(DEVICE_TYPE_STR, tree_son->element.name))
								{
									if (NULL != tree_son->element.contact)
									{
										sscanf(tree_son->element.contact, "%d", 
											&fct_info->type[fct_info->type_count]);
									}
								}
								else if (0 == strcmp(VERSION_LIST_STR, tree_son->element.name))
								{
									tree_grandson = tree_son->child;
									while (tree_grandson)
									{
										if (0 == strcmp(SDK_VERSION_STR, tree_grandson->element.name))
										{
											if (NULL != tree_grandson->element.contact)
											{
												snprintf((char*)fct_info->sdk_version[fct_info->type_count][fct_info->ver_count[fct_info->type_count]++], 
													sizeof(fct_info->sdk_version[fct_info->type_count][fct_info->ver_count[fct_info->type_count]]), 
													"%s", tree_grandson->element.contact);
											}
										}
										tree_grandson = tree_grandson->next;
									}
								}
								tree_son = tree_son->next;
							}
							fct_info->type_count++;
						}
						
						tree_child = tree_child->next;
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_proxy_factory_list_xml(void *pvalue, 
          const char *buffer)
{
	prx_factory_list *factory_list = (prx_factory_list*)pvalue;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_proxy_factory_info_xml(factory_list, buffer, FACTORY_LIST_INFO_CMD);
}

int parse_get_factory_info_request_xml(void *pvalue, 
          const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;

	//prx_factory_list *factory_list = (prx_factory_list*)pvalue;
		
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));	
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(GET_FACTORY_REQUEST_CMD, node_attr->value)))
			{
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_set_factory_info_request_xml(void *pvalue, 
          const char *buffer)
{
	return 0;
}
int parse_get_factory_info_response_xml(void *pvalue, 
          const char *buffer)
{
	prx_factory_list *factory_list;

	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	factory_list = (prx_factory_list*)pvalue;
	
	return parse_proxy_factory_info_xml(factory_list, buffer, GET_FACTORY_RESPONSE_CMD);
}
int parse_set_factory_info_response_xml(void *pvalue, 
          const char *buffer)
{
	return 0;
}

//##########################################################################
int parse_proxy_page_user_request_xml(void *pvalue, const char *buffer)
{
	struct _prx_page_user *page_user;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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

	page_user = (struct _prx_page_user*)pvalue;
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(GET_USER_INFO_REQUEST_CMD, node_attr->value)))
			{				
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(USERNAME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)page_user->username, 
								sizeof(page_user->username), 
								"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(OFFSET_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&page_user->offset);
						}
					}
					else if (0 == strcmp(COUNT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&page_user->count);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}

//##########################################################################
int parse_broadcast_add_user_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(BROADCAST_ADD_USER_CMD, node_attr->value)))
			{				
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(USERNAME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pvalue, sizeof(*pvalue), 
								"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_server_config_xml(prx_server_config *srv_cfg, 
	const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
								   (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SERVER_CONFIG_STR, tree_node->element.name))
					{
						tree_child = tree_node->child;
						while (NULL != tree_child)
						{
							if (0 == strcmp(SERVER_IP_STR, tree_child->element.name))
							{
								if (NULL != tree_child->element.contact)
								{
									snprintf((char*)srv_cfg->server_ip, sizeof(srv_cfg->server_ip), 
										"%s", tree_child->element.contact);
								}
							}
							else if (0 == strcmp(LISTEN_PORT_STR, tree_child->element.name))
							{
								if (NULL != tree_child->element.contact)
								{
									sscanf(tree_child->element.contact, "%d", 
											&(srv_cfg->listen_port));
								}
							}
							else if (0 == strcmp(RTSP_PORT_STR, tree_child->element.name))
							{
								if (NULL != tree_child->element.contact)
								{
									sscanf(tree_child->element.contact, "%d", 
											&(srv_cfg->rtsp_port));
								}
							}
							tree_child = tree_child->next;
						}
					}
					tree_node = tree_node->next;
				}
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_server_config_request_xml(void *pvalue, 
		const char *buffer)
{	
	prx_server_config *srv_cfg = (prx_server_config*)pvalue;
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
	
	return parse_server_config_xml(srv_cfg, buffer, GET_SERVER_CONFIG_REQUEST_CMD);
}
int parse_get_server_config_response_xml(void *pvalue, 
          const char *buffer)
{
	prx_server_config *srv_cfg = (prx_server_config*)pvalue;
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
	
	return parse_server_config_xml(srv_cfg, buffer, GET_SERVER_CONFIG_RESPONSE_CMD);
}
int parse_set_server_config_request_xml(void *pvalue, 
		const char *buffer)
{	
	prx_server_config *srv_cfg = (prx_server_config*)pvalue;
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
	
	return parse_server_config_xml(srv_cfg, buffer, SET_SERVER_CONFIG_REQUEST_CMD);
}
int parse_set_server_config_result_xml(void *pvalue, const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_SERVER_CONFIG_RESULT_CMD);
}

static __inline__ int 
parse_backup_xml(prx_backup *backup, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
                (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
                (0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&backup->result);
						}
					}
					else if (0 == strcmp(BACKUP_MAGIC_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&backup->magic);
						}
					}
					else if (0 == strcmp(BACKUP_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&backup->port);
						}
					}
					else if (0 == strcmp(BACKUP_SIZE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&backup->size);
						}
					}
					tree_node = tree_node->next;
				}
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}
	
	xml_tree_delete(xml_tree);
	return -1;
}
int parse_download_request_xml(void *pvalue, 
		const char *buffer)
{
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
	
	return parse_backup_xml(backup, buffer, DOWNLOAD_DATA_REQUEST_CMD);
}
int parse_download_response_xml(void *pvalue, 
          const char *buffer)
{
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
	
	return parse_backup_xml(backup, buffer, DOWNLOAD_DATA_RESPONSE_CMD);
}
int parse_upload_request_xml(void *pvalue, 
		const char *buffer)
{
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

	return parse_backup_xml(backup, buffer, UPLOAD_DATA_REQUEST_CMD);
}
int parse_upload_response_xml(void *pvalue, const char *buffer)
{
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
	
	return parse_backup_xml(backup, buffer, UPLOAD_DATA_RESPONSE_CMD);
}

int parse_limit_broadcast_status_xml(void *pvalue, const char *buffer)
{
	prx_limit *limit = pvalue;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && 
            (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
                (0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
                (0 == strcmp(LIMIT_BROADCASE_STATUE_CMD, node_attr->value)))
			{				
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(START_DEVICE_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&limit->start);
						}
					}
                    else if (0 == strcmp(END_DEVICE_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&limit->end);
						}
					}

					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}


#endif

//##########################################################################
static __inline__ int 
get_remote_channel_info(JRemoteChannelInfo *rmt_ch_info, XmlTreeNode *node)
{
	while (node)
	{
		if (0 == strcmp(CHANNEL_NO_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_ch_info->ch_no));
			}
		}
		else if (0 == strcmp(PROTOCOL_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_ch_info->protocol));
			}
		}
		else if (0 == strcmp(AUDIO_ENABLE_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_ch_info->audio_enable));
			}
		}
		else if (0 == strcmp(USERNAME_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				snprintf((char*)rmt_ch_info->user_name, 
						sizeof(rmt_ch_info->user_name), 
						"%s", node->element.contact);
			}
		}
		else if (0 == strcmp(PASSWORD_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				snprintf((char*)rmt_ch_info->user_pwd, 
						sizeof(rmt_ch_info->user_pwd), 
						"%s", node->element.contact);
			}
		}
		else if (0 == strcmp(WINDOW_MODE_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_ch_info->win_mode));
			}
		}
		else if (0 == strcmp(WIN_MAX_STREAM_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_ch_info->win_max_strm));
			}
		}
		else if (0 == strcmp(WIN_MIN_STREAM_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_ch_info->win_min_strm));
			}
		}

		node = node->next;
	}

	return 0;
}

static __inline__ void  
get_remote_device_info(JRemoteDeviceInfo *rmt_dev_info, XmlTreeNode *node)
{
	while (node)
	{
		if (0 == strcmp(PORT_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_dev_info->port));
			}
		}
		else if (0 == strcmp(DEV_TYPE_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_dev_info->dev_type));
			}
		}
		else if (0 == strcmp(CHANNEL_SUM_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_dev_info->ch_sum));
			}
		}
		else if (0 == strcmp(DNS_ENABLE_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				sscanf(node->element.contact, "%d", 
						&(rmt_dev_info->dns_enable));
			}
		}
		else if (0 == strcmp(IP_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				snprintf((char*)rmt_dev_info->ip, 
						sizeof(rmt_dev_info->ip), 
						"%s", node->element.contact);
			}
		}
		else if (0 == strcmp(DNS_STR, node->element.name))
		{
			if (NULL != node->element.contact)
			{
				snprintf((char*)rmt_dev_info->dns, 
						sizeof(rmt_dev_info->dns), 
						"%s", node->element.contact);
			}
		}

		node = node->next;
	}
}

static __inline__ int 
parse_channel_info(ChannelInfoPacket *ch_info, 
	const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ch_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ch_info->session_id, sizeof(ch_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ch_info->domain_id, sizeof(ch_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ch_info->pu_id, sizeof(ch_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CHANNEL_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&ch_info->ch_no);
						}
					}
					else if (0 == strcmp(CHANNEL_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&ch_info->ch_type);
						}
					}
					else if (0 == strcmp(CHANNEL_STATUS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&ch_info->ch_status);
						}
					}
					else if (0 == strcmp(CHANNEL_NAME_STR, tree_node->element.name))
					{
						if (tree_node->element.contact)
						{
							snprintf((char*)ch_info->ch_name, sizeof(ch_info->ch_name), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(REMOTE_CHANNEL_INFO_STR, tree_node->element.name))
					{
						get_remote_channel_info(&ch_info->rmt_ch_info, tree_node->child);
					}
					else if (0 == strcmp(REMOTE_DEVICE_INFO_STR, tree_node->element.name))
					{
						get_remote_device_info(&ch_info->rmt_dev_info, tree_node->child);
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_channel_info_request_xml(void *pvalue, 
		const char *buffer)
{	
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_CHANNEL_INFO_REQUEST_CMD);
}
int parse_get_channel_info_response_xml(void *pvalue, 
          const char *buffer)
{
	ChannelInfoPacket *ch_info = (ChannelInfoPacket*)pvalue;
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
	
	return parse_channel_info(ch_info, buffer, GET_CHANNEL_INFO_RESPONSE_CMD);
}
int parse_set_channel_info_request_xml(void *pvalue, 
		const char *buffer)
{
	ChannelInfoPacket *ch_info = (ChannelInfoPacket*)pvalue;
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
	
	return parse_channel_info(ch_info, buffer, SET_CHANNEL_INFO_REQUEST_CMD);
}
int parse_set_channel_info_result_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_CHANNEL_INFO_RESULT_CMD);
}

//##########################################################################
static __inline__ int 
parse_picture_info(PictureInfoPacket *pic_info, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pic_info->session_id, sizeof(pic_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pic_info->domain_id, sizeof(pic_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pic_info->pu_id, sizeof(pic_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(MIRROR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->mirror));
						}
					}
					else if (0 == strcmp(FLIP_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->flip));
						}
					}
					else if (0 == strcmp(HZ_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->hz));
						}
					}
					else if (0 == strcmp(AWB_MODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->awb_mode));
						}
					}
					else if (0 == strcmp(AWB_RED_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->awb_red));
						}
					}
					else if (0 == strcmp(AWB_BULE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->awb_blue));
						}
					}
					else if (0 == strcmp(WDR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->wdr));
						}
					}
					else if (0 == strcmp(IRIS_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->iris_type));
						}
					}
					else if (0 == strcmp(EXP_COMENSATION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->exp_compensation));
						}
					}
					else if (0 == strcmp(AE_MODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pic_info->ae_mode));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_picture_info_request_xml(void *pvalue, 
		const char *buffer)
{	
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_PICTURE_INFO_REQUEST_CMD);
}
int parse_get_picture_info_response_xml(void *pvalue, 
          const char *buffer)
{
	PictureInfoPacket *pic_info = (PictureInfoPacket*)pvalue;
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
	
	return parse_picture_info(pic_info, buffer, GET_PICTURE_INFO_RESPONSE_CMD);
}
int parse_set_picture_info_request_xml(void *pvalue, 
		const char *buffer)
{	
	PictureInfoPacket *pic_info = (PictureInfoPacket*)pvalue;
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
	
	return parse_picture_info(pic_info, buffer, SET_PICTURE_INFO_REQUEST_CMD);
}
int parse_set_picture_info_result_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_PICTURE_INFO_RESULT_CMD);
}

//##########################################################################
/*static __inline__ int 
parse_hl_picture_info(HLPictureInfoPacket *hl_pic_info, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hl_pic_info->session_id, sizeof(hl_pic_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hl_pic_info->domain_id, sizeof(hl_pic_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)hl_pic_info->pu_id, sizeof(hl_pic_info->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DEFECT_PIX_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->defect_pix));
						}
					}
					else if (0 == strcmp(WDR_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->wdr));
						}
					}
					else if (0 == strcmp(_2d_DENOISE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->_2d_denoise));
						}
					}
					else if (0 == strcmp(_3d_DENOISE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->_3d_denoise));
						}
					}
					else if (0 == strcmp(_3d_SF_CONS_STH1_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->_3d_fcos_sth1));
						}
					}
					else if (0 == strcmp(_3d_SF_CONS_STH2_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->_3d_fcos_sth2));
						}
					}
					else if (0 == strcmp(_3d_TF_STH_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->_3d_tf_sth));
						}
					}
					else if (0 == strcmp(AWB_MODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->awb_mode));
						}
					}
					else if (0 == strcmp(AWB_RED_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->awb_red));
						}
					}
					else if (0 == strcmp(AWB_BULE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(hl_pic_info->awb_blue));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_hl_picture_info_request_xml(void *pvalue, 
		const char *buffer)
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
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_HL_PICTURE_INFO_REQUEST_CMD);
}
int parse_get_hl_picture_info_response_xml(void *pvalue, 
          const char *buffer)
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
	HLPictureInfoPacket *hl_pic_info = (HLPictureInfoPacket*)pvalue;
	
	return parse_hl_picture_info(hl_pic_info, buffer, GET_HL_PICTURE_INFO_RESPONSE_CMD);
}
int parse_set_hl_picture_info_request_xml(void *pvalue, 
		const char *buffer)
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
	HLPictureInfoPacket *hl_pic_info = (HLPictureInfoPacket*)pvalue;
	
	return parse_hl_picture_info(hl_pic_info, buffer, SET_HL_PICTURE_INFO_REQUEST_CMD);
}
int parse_set_hl_picture_info_result_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_HL_PICTURE_INFO_RESULT_CMD);
}*/

//##########################################################################
static __inline__ int 
parse_wifi_config(WifiConfigPacket *wifi_cfg, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(wifi_cfg->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)wifi_cfg->session_id, sizeof(wifi_cfg->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)wifi_cfg->domain_id, sizeof(wifi_cfg->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)wifi_cfg->pu_id, sizeof(wifi_cfg->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(WIFI_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(wifi_cfg->wifi_enable));
						}
					}
					else if (0 == strcmp(ESSID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)wifi_cfg->essid, sizeof(wifi_cfg->essid), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PASSWORD_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)wifi_cfg->pwd, sizeof(wifi_cfg->pwd), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(ENCRYPT_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(wifi_cfg->encrypt_type));
						}
					}
					else if (0 == strcmp(AUTH_MODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(wifi_cfg->auth_mode));
						}
					}
					else if (0 == strcmp(SECRET_KEY_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(wifi_cfg->secret_key_type));
						}
					}
					else if (0 == strcmp(WIFI_STATUS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(wifi_cfg->wifi_st));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_wifi_config_request_xml(void *pvalue, 
		const char *buffer)
{	
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_WIFI_CONFIG_REQUEST_CMD);
}
int parse_get_wifi_config_response_xml(void *pvalue, 
          const char *buffer)
{
	WifiConfigPacket *wifi_cfg = (WifiConfigPacket*)pvalue;
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
	
	return parse_wifi_config(wifi_cfg, buffer, GET_WIFI_CONFIG_RESPONSE_CMD);
}
int parse_set_wifi_config_request_xml(void *pvalue, 
		const char *buffer)
{	
	WifiConfigPacket *wifi_cfg = (WifiConfigPacket*)pvalue;
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
	
	return parse_wifi_config(wifi_cfg, buffer, SET_WIFI_CONFIG_REQUEST_CMD);
}
int parse_set_wifi_config_result_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_WIFI_CONFIG_RESULT_CMD);
}

//##########################################################################
static __inline__ void 
get_access_point_info(JWifiApInfo *wifi_ap, XmlTreeNode *node)
{
//	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *child = NULL;
	
	while (node)
	{
		if (0 == strcmp(ACCESS_POINT_STR, node->element.name))
		{
			child = node->child;
			while (child)
			{
				if (0 == strcmp(ESSID_STR, child->element.name))
				{
					if (NULL != child->element.contact)
					{
						snprintf((char*)wifi_ap->essid, sizeof(wifi_ap->essid), 
								"%s", child->element.contact);
					}
				}
				else if (0 == strcmp(ENCRYPT_TYPE_STR, child->element.name))
				{
					if (NULL != child->element.contact)
					{
						sscanf(child->element.contact, "%d", 
								&(wifi_ap->encrypt_type));
					}
				}
				else if (0 == strcmp(AUTH_MODE_STR, child->element.name))
				{
					if (NULL != child->element.contact)
					{
						sscanf(child->element.contact, "%d", 
								&(wifi_ap->auth_mode));
					}
				}
				else if (0 == strcmp(SECRET_KEY_TYPE_STR, child->element.name))
				{
					if (NULL != child->element.contact)
					{
						sscanf(child->element.contact, "%d", 
								&(wifi_ap->secret_key_type));
					}
				}
				else if (0 == strcmp(SIGNAL_QUALITY_STR, child->element.name))
				{
					if (NULL != child->element.contact)
					{
						sscanf(child->element.contact, "%d", 
								&(wifi_ap->quality));
					}
				}
				else if (0 == strcmp(BIT_RATE_STR, child->element.name))
				{
					if (NULL != child->element.contact)
					{
						sscanf(child->element.contact, "%d", 
								&(wifi_ap->bit_rate));
					}
				}
				
				child = child->next;
			}
		}
		
		node = node->next;
	}
}
static __inline__ int 
parse_wifi_search(WifiSearchResPacket *wifi_ap, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(wifi_ap->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)wifi_ap->session_id, sizeof(wifi_ap->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)wifi_ap->domain_id, sizeof(wifi_ap->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)wifi_ap->pu_id, sizeof(wifi_ap->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(WIFI_AP_INFO_STR, tree_node->element.name))
					{
						get_access_point_info((JWifiApInfo*)&wifi_ap->wifi_ap, tree_node);
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_wifi_search_request_xml(void *pvalue, 
		const char *buffer)
{
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, WIFI_SEARCH_REQUEST_CMD);
}
int parse_wifi_search_response_xml(void *pvalue, 
          const char *buffer)
{
	WifiSearchResPacket *wifi_ap = (WifiSearchResPacket*)pvalue;
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
	
	return parse_wifi_search(wifi_ap, buffer, WIFI_SEARCH_RESPONSE_CMD);
}

//##########################################################################
static __inline__ int 
parse_network_status(NetworkStatusPacket *net_status, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_status->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)net_status->session_id, sizeof(net_status->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)net_status->domain_id, sizeof(net_status->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)net_status->pu_id, sizeof(net_status->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(ETH_STATUS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_status->eth_st));
						}
					}
					else if (0 == strcmp(WIFI_STATUS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_status->wifi_st));
						}
					}
					else if (0 == strcmp(PPPOE_STATUS_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(net_status->pppoe_st));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_network_status_request_xml(void *pvalue, 
		const char *buffer)
{	
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_NETWORK_STATUS_REQUEST_CMD);
}
int parse_get_network_status_response_xml(void *pvalue, 
          const char *buffer)
{
	NetworkStatusPacket *net_status = (NetworkStatusPacket*)pvalue;
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
	
	return parse_network_status(net_status, buffer, GET_NETWORK_STATUS_RESPONSE_CMD);
}

//##########################################################################
static __inline__ int 
parse_control_device(ControlDevicePacket *cntrl_dev, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(cntrl_dev->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)cntrl_dev->session_id, sizeof(cntrl_dev->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)cntrl_dev->domain_id, sizeof(cntrl_dev->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)cntrl_dev->pu_id, sizeof(cntrl_dev->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(COMMAND_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(cntrl_dev->command));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_control_device_request_xml(void *pvalue, 
		const char *buffer)
{	
	ControlDevicePacket *cntrl_dev = (ControlDevicePacket*)pvalue;
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
	
	return parse_control_device(cntrl_dev, buffer, CONTROL_DEVICE_REQUEST_CMD);
}
int parse_control_device_result_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, CONTROL_DEVICE_RESULT_CMD);
}

//##########################################################################
static __inline__ int 
parse_ddns_config(DdnsConfigPacket *ddns_cfg, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ddns_cfg->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ddns_cfg->session_id, sizeof(ddns_cfg->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ddns_cfg->domain_id, sizeof(ddns_cfg->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ddns_cfg->pu_id, sizeof(ddns_cfg->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DDNS_ACCOUNT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ddns_cfg->ddns_account, sizeof(ddns_cfg->ddns_account), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(USERNAME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ddns_cfg->ddns_usr, sizeof(ddns_cfg->ddns_usr), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PASSWORD_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ddns_cfg->ddns_pwd, sizeof(ddns_cfg->ddns_pwd), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DDNS_OPNT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ddns_cfg->ddns_open));
						}
					}
					else if (0 == strcmp(DDNS_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ddns_cfg->ddns_type));
						}
					}
					else if (0 == strcmp(DDNS_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ddns_cfg->ddns_port));
						}
					}
					else if (0 == strcmp(DDNS_TIMES_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ddns_cfg->ddns_times));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_ddns_config_request_xml(void *pvalue, 
		const char *buffer)
{
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_DDNS_CONFIG_REQUEST_CMD);
}
int parse_get_ddns_config_response_xml(void *pvalue, 
          const char *buffer)
{
	DdnsConfigPacket *ddns_cfg = (DdnsConfigPacket*)pvalue;
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
	
	
	return parse_ddns_config(ddns_cfg, buffer, GET_DDNS_CONFIG_RESPONSE_CMD);
}
int parse_set_ddns_config_request_xml(void *pvalue, 
		const char *buffer)
{
	DdnsConfigPacket *ddns_cfg = (DdnsConfigPacket*)pvalue;
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
	
	return parse_ddns_config(ddns_cfg, buffer, SET_DDNS_CONFIG_REQUEST_CMD);
}
int parse_set_ddns_config_result_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_DDNS_CONFIG_RESULT_CMD);
}

int parse_get_def_display_info_request_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_DEF_DISPLAY_INFO_REQUEST_CMD);
}
int parse_get_def_display_info_response_xml(void *pvalue, const char *buffer)
{
	DisplayParameterPacket *display_para = (DisplayParameterPacket*)pvalue;
	
	return parse_display_parameter_xml(display_para, buffer, GET_DEF_DISPLAY_INFO_RESPONSE_CMD);
}
int parse_get_def_picture_info_request_xml(void *pvalue, const char *buffer)
{	
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_DEF_PICTURE_INFO_REQUEST_CMD);
}
int parse_get_def_picture_info_response_xml(void *pvalue, const char *buffer)
{
	PictureInfoPacket *pic_info = (PictureInfoPacket*)pvalue;
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
	
	return parse_picture_info(pic_info, buffer, GET_DEF_PICTURE_INFO_RESPONSE_CMD);
}

//##########################################################################
static __inline__ void parse_avd_segment(XmlTreeNode *tree_node, JSegment *avd_seg)
{
//	int index = 0;
//	int tmp_value;
	XmlTreeNode *tree_child = NULL;
//	XmlTreeNode *tree_son = NULL;
//	XmlTreeAttr *tree_attr = NULL;
	
	tree_child = tree_node->child;
	/*while (tree_child)
	{
		if (!strcmp(SEGMENT_STR, tree_child->element.name))
		{
			tree_attr = tree_child->element.attr;
			if (!strcmp(SEG_INDEX_STR, tree_attr->name))
				sscanf(tree_attr->value, "%d", &index);
			
			tree_son = tree_child->child;
			while (tree_son)
			{
				if (!strcmp(SEG_OPNT_STR, tree_son->element.name))
				{
					sscanf(tree_son->element.contact, 
						"%d", &tmp_value);
					avd_seg[index].open = tmp_value;
				}
				else if (!strcmp(BEGIN_TIMES_STR, tree_son->element.name))
				{
					sscanf(tree_son->element.contact, 
						"%d", &tmp_value);
					avd_seg[index].begin_sec = tmp_value;
				}
				else if (!strcmp(END_TIMES_STR, tree_son->element.name))
				{
					sscanf(tree_son->element.contact, 
						"%d", &tmp_value);
					avd_seg[index].end_sec = tmp_value;
				}

				tree_son = tree_son->next;
			}
		}
		tree_child = tree_child->next;
	}*/
}
static __inline__ void parse_avd_rule(XmlTreeNode *tree_node, JAvdRule *avd_rule)
{
	int type = 0;
	int tmp_value;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeAttr *tree_attr = NULL;
	
	tree_child = tree_node->child;
	while (tree_child)
	{
		if (!strcmp(RULE_STR, tree_child->element.name))
		{
			tree_attr = tree_child->element.attr;
			if (!strcmp(RULE_TYPE_STR, tree_attr->name))
				sscanf(tree_attr->value, "%d", &type);
			
			tree_son = tree_child->child;
			while (tree_son)
			{
				if (!strcmp(RULE_ENABLE_STR, tree_son->element.name))
				{
					sscanf(tree_son->element.contact, 
						"%d", &tmp_value);
					avd_rule[type].enable = tmp_value;
				}
				else if (!strcmp(LEVEL_STR, tree_son->element.name))
				{
					sscanf(tree_son->element.contact, 
						"%d", &tmp_value);
					avd_rule[type].level = tmp_value;
				}
				else if (!strcmp(ALARM_TIMES_STR, tree_son->element.name))
				{
					sscanf(tree_son->element.contact, 
						"%d", &tmp_value);
					avd_rule[type].alarm_times = tmp_value;
				}

				tree_son = tree_son->next;
			}
		}

		tree_child = tree_child->next;
	}
}

static __inline__ int 
parse_avd_config(AvdConfigPacket *avd_cfg, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(avd_cfg->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)avd_cfg->session_id, sizeof(avd_cfg->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)avd_cfg->domain_id, sizeof(avd_cfg->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)avd_cfg->pu_id, sizeof(avd_cfg->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(AVD_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&avd_cfg->enable);
						}
					}
					else if (0 == strcmp(AVD_SEGMENT_STR, tree_node->element.name))
					{
						parse_avd_segment(tree_node, avd_cfg->sched_time);
					}
					else if (0 == strcmp(AVD_RULE_STR, tree_node->element.name))
					{
						parse_avd_rule(tree_node, avd_cfg->avd_rule);
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_avd_config_request_xml(void *pvalue, 
		const char *buffer)
{
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_AVD_CONFIG_REQUEST_CMD);
}
int parse_get_avd_config_response_xml(void *pvalue, 
          const char *buffer)
{
	AvdConfigPacket *avd_cfg = (AvdConfigPacket*)pvalue;
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
	
	return parse_avd_config(avd_cfg, buffer, GET_AVD_CONFIG_RESPONSE_CMD);
}
int parse_set_avd_config_request_xml(void *pvalue, 
		const char *buffer)
{	
	AvdConfigPacket *avd_cfg = (AvdConfigPacket*)pvalue;
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
	
	return parse_avd_config(avd_cfg, buffer, SET_AVD_CONFIG_REQUEST_CMD);
}
int parse_set_avd_config_result_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_AVD_CONFIG_RESULT_CMD);
}






















//##########################################################################
static __inline__ int 
parse_transparent_param_xml(TransparentPacket *trans, 
		const char *buffer, const char *command)
{
	//int data_len = 0;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)trans->session_id, sizeof(trans->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)trans->domain_id, sizeof(trans->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)trans->pu_id, sizeof(trans->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(TRANSPARENT_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&trans->type);
						}
					}
					else if (0 == strcmp(TRANSPARENT_CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&trans->channel);
						}
					}
					else if (0 == strcmp(TRANSPARENT_LENGTH_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &trans->length);
						}
					}
					else if (0 == strcmp(TRANSPARENT_DATA_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							if (trans->length > 0)
							{
								trans->data = j_xml_alloc(trans->length);
                                if (MAX_MXML_CONTACT_LEN > trans->length)
                                {
    								memcpy((void*)trans->data, tree_node->element.contact, 
    									trans->length);
                                }
                                else
                                {
    								memcpy((void*)trans->data, tree_node->element.extend_contact, 
    									trans->length);
                                }
							}
						}
					}
					
					tree_node = tree_node->next;
				}
			
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_transparent_param_request_xml(void *pvalue, const char *buffer)
{
	TransparentPacket *trans = (TransparentPacket*)pvalue;
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
	
	return parse_transparent_param_xml(trans, buffer, 
			GET_TRANSPARENTPARAM_REQUEST_CMD);
}
int parse_get_transparent_param_response_xml(void *pvalue, const char *buffer)
{
	TransparentPacket *trans = (TransparentPacket*)pvalue;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_transparent_param_xml(trans, buffer, 
			GET_TRANSPARENTPARAM_RESPONSE_CMD);
}
int parse_set_transparent_param_request_xml(void *pvalue, const char *buffer)
{
	TransparentPacket *trans = (TransparentPacket*)pvalue;
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
	
	return parse_transparent_param_xml(trans, buffer, 
			SET_TRANSPARENTPARAM_REQUEST_CMD);
}
int parse_set_transparent_param_response_xml(void *pvalue, const char *buffer)
{
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_result_info_xml((int*)pvalue, buffer, 
			SET_TRANSPARENTPARAM_RESPONSE_CMD);
}
int parse_transparent_notify_enevt_xml(void * pvalue,const char * buffer)
{
	TransparentPacket *trans = (TransparentPacket*)pvalue;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_transparent_param_xml(trans, buffer, 
			TRANSPARENTPARAM_NOTIFYEVENT_CMD);
}
int parse_transparent_control_device_xml(void * pvalue,const char * buffer)
{
	TransparentPacket *trans = (TransparentPacket*)pvalue;
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	
	return parse_transparent_param_xml(trans, buffer, 
			TRANSPARENTPARAM_CONTROLDEVICE_CMD);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

# ifdef _USE_DECODER_PROTO_

static __inline__ int 
parse_query_division_mode(DivisionModePacket *div_mode, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (!xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (!strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (!strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)div_mode->pu_id, sizeof(div_mode->pu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (!strcmp(PAGE_SIZE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&div_mode->div_mode.page_size);
						}
					}
					else if (!strcmp(START_ROW_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&div_mode->div_mode.start_row);
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

static __inline__ int 
parse_return_division_mode(DivisionModePacket *div_mode, const char *buffer, const char *command)
{
	JDivMode *mode;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (!xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (!strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (!strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(div_mode->result.code));
						}
					}
					else if (!strcmp(TOTAL_COUNT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&div_mode->div_mode.total);
						}
					}
					else if (!strcmp(DIVISION_INFO_STR, tree_node->element.name))
					{
						int i;
						if (!strcmp(COUNT_STR, tree_node->element.attr->name))
							sscanf(tree_node->element.attr->value, "%d", &div_mode->div_mode.count);
						
						
						tree_child = tree_node->child;
						for (i=1; i<div_mode->div_mode.count && tree_child; i++)
						{
							if (!strcmp(DIVISION_MODE_STR, tree_child->element.name))
							{
								mode = &div_mode->div_mode.mode[i];
								tree_son = tree_child->child;
								while (tree_son)
								{
									if (!strcmp(DIVISION_ID_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													&mode->div_id);
										}
									}
									else if (!strcmp(DIVISION_NAME_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											snprintf((char*)mode->mode_name, sizeof(mode->mode_name), 
													"%s", tree_son->element.contact);
										}
									}
									else if (!strcmp(DESCRIPTION_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											snprintf((char*)mode->description, sizeof(mode->description), 
													"%s", tree_son->element.contact);
										}
									}

									tree_son = tree_son->next;
								}
							}
                            tree_child = tree_child->next;
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}


int parse_query_division_mode_request_xml(void *pvalue, 
		const char *buffer)
{
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
	
	return parse_query_division_mode(div_mode, buffer, QUERY_DIVISION_MODE_REQUEST_CMD);
}
int parse_query_division_mode_response_xml(void *pvalue, 
          const char *buffer)
{
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
	
	return parse_return_division_mode(div_mode, buffer, QUERY_DIVISION_MODE_RESPONSE_CMD);
}

static __inline__ int 
parse_get_screen_state(ScreenStatePacket *scr_state, const char *buffer, const char *command)
{
	JDivisionInfo *division;
	JFullScreenMode *full_screen;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (!xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (!strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (!strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(scr_state->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)scr_state->session_id, sizeof(scr_state->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (!strcmp(DIVISION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&scr_state->scr_state.div_id);
						}
					}
					else if (!strcmp(SCREEN_IFNO_STR, tree_node->element.name))
					{
						int i;

						if (!strcmp(COUNT_STR, tree_node->element.attr->name))
						{
							sscanf(tree_node->element.attr->value, "%d", 
								&scr_state->scr_state.count);
						}
						
						tree_child = tree_node->child;
						for (i=1; i<scr_state->scr_state.count && tree_child; i++)
						{
							if (!strcmp(DIVISION_STR, tree_child->element.name))
							{
								division = &scr_state->scr_state.division[i];
								tree_son = tree_child->child;
								while (tree_son)
								{
									if (!strcmp(DIVISION_NO_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													(int*)&division->div_no);
										}
									}
									else if (!strcmp(ENCODER_NAME_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											snprintf((char*)division->encoder, sizeof(division->encoder), 
													"%s", tree_son->element.contact);
										}
									}
									else if (!strcmp(ENCODER_CHANNEL_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													(int*)&division->enc_chn);
										}
									}
									else if (!strcmp(LEVEL_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													(int*)&division->level);
										}
									}
									else if (!strcmp(ACTION_TYPE_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													(int*)&division->action);
										}
									}
									else if (!strcmp(ACTION_RESULT_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													(int*)&division->result);
										}
									}

									tree_son = tree_son->next;
								}
							}
                            tree_child = tree_child->next;
						}
					}
					else if (!strcmp(FULL_SCREEN_STATE_STR, tree_node->element.name))
					{
            			node_attr = tree_node->element.attr;
            			if ((NULL != node_attr) && (!strcmp(FULL_SCREEN_MODE_STR, node_attr->name)))
                        {
							sscanf(node_attr->value, "%d", 
									&scr_state->scr_state.fs_mode);
                        }

                        if (scr_state->scr_state.fs_mode)
                        {
							full_screen = &scr_state->scr_state.full_screen;
                            tree_child = tree_node->child;
							while (tree_child)
							{
								if (!strcmp(ENCODER_NAME_STR, tree_child->element.name))
								{
									if (NULL != tree_child->element.contact)
									{
										snprintf((char*)full_screen->encoder, sizeof(full_screen->encoder), 
												"%s", tree_child->element.contact);
									}
								}
								else if (!strcmp(ENCODER_CHANNEL_STR, tree_child->element.name))
								{
									if (NULL != tree_child->element.contact)
									{
										sscanf(tree_child->element.contact, "%d", 
												(int*)&full_screen->enc_chn);
									}
								}
								else if (!strcmp(LEVEL_STR, tree_child->element.name))
								{
									if (NULL != tree_child->element.contact)
									{
										sscanf(tree_child->element.contact, "%d", 
												(int*)&full_screen->level);
									}
								}
								else if (!strcmp(ACTION_TYPE_STR, tree_child->element.name))
								{
									if (NULL != tree_child->element.contact)
									{
										sscanf(tree_child->element.contact, "%d", 
												(int*)&full_screen->action);
									}
								}
								else if (!strcmp(ACTION_RESULT_STR, tree_child->element.name))
								{
									if (NULL != tree_child->element.contact)
									{
										sscanf(tree_child->element.contact, "%d", 
												(int*)&full_screen->result);
									}
								}

								tree_child = tree_child->next;
							}
                        }
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}


int parse_get_screen_state_request_xml(void *pvalue, 
		const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(GET_SCREEN_STATE_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)scr_state->session_id, sizeof(scr_state->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DIS_CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &scr_state->dis_channel);
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_screen_state_response_xml(void *pvalue, 
          const char *buffer)
{
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
	
	return parse_get_screen_state(scr_state, buffer, GET_SCREEN_STATE_RESPONSE_CMD);
}

int parse_set_division_mode_request_xml(void *pvalue, 
		const char *buffer)
{
	JChangeDMode *cd_mode;
	ChangeDModePacket *packet;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(SET_DIVISION_MODE_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DIS_CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &cd_mode->dis_chn);
						}
					}
					else if (0 == strcmp(DIVISION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &cd_mode->div_id);
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_set_division_mode_response_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_DIVISION_MODE_RESULT_CMD);
}

int parse_set_full_screen_request_xml(void *pvalue, 
		const char *buffer)
{
	JScreen *screen;
	FullScreenPacket *packet;
	
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(SET_FULL_SCREEN_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DIS_CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &screen->dis_chn);
						}
					}
					else if (0 == strcmp(DIVISION_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &screen->div_no);
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_set_full_screen_response_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_FULL_SCREEN_RESULT_CMD);
}

int parse_exit_full_screen_request_xml(void *pvalue, 
		const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(EXIT_FULL_SCREEN_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DIS_CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", (int*)pvalue);
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_exit_full_screen_response_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, EXIT_FULL_SCREEN_RESULT_CMD);
}

static __inline__ int 
parse_tv_wall_play(TVWallPlayPacket *tv_wall, const char *buffer, const char *command)
{
	int i;
	JDivisionInfo *division;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (!xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (!strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (!strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(tv_wall->result.code));
						}
					}
					else if (!strcmp(ACTION_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&tv_wall->tv_play.action);
						}
					}
					else if (!strcmp(NAME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)tv_wall->tv_play.name, sizeof(tv_wall->tv_play.name), 
									"%s", tree_node->element.contact);
						}
					}
					else if (!strcmp(STEP_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&tv_wall->tv_play.step);
						}
					}
					else if (!strcmp(DIS_CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&tv_wall->tv_play.dis_chn);
						}
					}
					else if (!strcmp(DIVISION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&tv_wall->tv_play.div_id);
						}
					}
					else if (!strcmp(KEEP_OTHER_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&tv_wall->tv_play.k_other);
						}
					}
					else if (!strcmp(DIVISIONS_STR, tree_node->element.name))
					{
						if (!strcmp(COUNT_STR, tree_node->element.attr->name))
						{
							sscanf(tree_node->element.attr->value, "%d", 
								&tv_wall->tv_play.count);
						}

						tree_child = tree_node->child;
						for (i=0; i<tv_wall->tv_play.count && tree_child; i++)
						{
							if (!strcmp(DIVISION_STR, tree_child->element.name))
							{
								division = &tv_wall->tv_play.division[i];
								tree_son = tree_child->child;
								while (tree_son)
								{
									if (!strcmp(DIVISION_NO_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													&division->div_no);
										}
									}
									else if (!strcmp(ACTION_RESULT_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													&division->result);
										}
									}
									else if (!strcmp(ENCODER_NAME_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											snprintf((char*)division->encoder, sizeof(division->encoder), 
													"%s", tree_son->element.contact);
										}
									}
									else if (!strcmp(ENCODER_CHANNEL_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													(int*)&division->enc_chn);
										}
									}
									else if (!strcmp(LEVEL_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													(int*)&division->level);
										}
									}
									else if (!strcmp(ENCODER_URL_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											snprintf((char*)division->url, sizeof(division->url), 
													"%s", tree_son->element.contact);
										}
									}
									else if (!strcmp(CLEAR_FLAG_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													(int*)&division->flag);
										}
									}

									tree_son = tree_son->next;
								}
							}
                            tree_child = tree_child->next;
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_tv_wall_play_request_xml(void *pvalue, 
		const char *buffer)
{
	TVWallPlayPacket *tv_wall = (TVWallPlayPacket*)pvalue;
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
	
	return parse_tv_wall_play(tv_wall, buffer, TV_WALL_PLAY_REQUEST_CMD);
}
int parse_tv_wall_play_response_xml(void *pvalue, 
          const char *buffer)
{
	TVWallPlayPacket *tv_wall = (TVWallPlayPacket*)pvalue;

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
	
	return parse_tv_wall_play(tv_wall, buffer, TV_WALL_PLAY_RESULT_CMD);
}

int parse_clear_division_request_xml(void *pvalue, 
		const char *buffer)
{
	JScreen *screen;
	FullScreenPacket *packet;

	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(CLEAR_DIVISION_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(DIS_CHANNEL_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &screen->dis_chn);
						}
					}
					else if (0 == strcmp(DIVISION_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &screen->div_no);
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_clear_division_response_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, CLEAR_DIVISION_RESULT_CMD);
}

# endif //_USE_DECODER_PROTO_

static __inline__ void  
parse_log_info(JOperationLogItem *item, XmlTreeNode *tree_node)
{
	int i;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	tree_child = tree_node->child;
	
	for (i=0; tree_child; i++)
	{
		if (!strcmp(LOG_ITEM_STR, tree_child->element.name))
		{
			tree_son = tree_child->child;
			while (tree_son)
			{
				if (!strcmp(LOG_TIMES_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&item[i].times);
					}                
				}
				else if (!strcmp(MAJOR_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							(int*)&item[i].major_type);
					}
				}
				else if (!strcmp(MINOR_STR, tree_son->element.name))
				{
					if (tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							(int*)&item[i].minor_type);
					}
				}
				else if (!strcmp(ARGS_STR, tree_son->element.name))
				{
					if (tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							(int*)&item[i].args);
					}
				}
				else if (!strcmp(USERNAME_STR, tree_son->element.name))
				{
					if (tree_son->element.contact)
					{
						snprintf((char*)item[i].user, sizeof(item[i].user), 
								"%s", tree_son->element.contact);
					}
				}
				else if (!strcmp(IP_ADDR_STR, tree_son->element.name))
				{
					if (NULL != tree_son->element.contact)
					{
						sscanf(tree_son->element.contact, "%d", 
							&item[i].ip);
					}
				}

				tree_son = tree_son->next;
			}
		}
		
		tree_child = tree_child->next;
	}
}

static __inline__ void  
parse_cond_info(JOperationLogCond *cond, XmlTreeNode *tree_node)
{
	while (tree_node)
	{
		if (!strcmp(BEG_TIME_STR, tree_node->element.name))
		{
			if (tree_node->element.contact)
			{
				sscanf(tree_node->element.contact, "%d", 
					&cond->beg_time);
			}
		}
		else if (!strcmp(END_TIME_STR, tree_node->element.name))
		{
			if (tree_node->element.contact)
			{
				sscanf(tree_node->element.contact, "%d", 
					&cond->end_time);
			}
		}
		else if (!strcmp(LOG_TYPE_STR, tree_node->element.name))
		{
			if (NULL != tree_node->element.contact)
			{
				sscanf(tree_node->element.contact, "%d", 
					&cond->type);
			}                
		}
		else if (!strcmp(USERNAME_STR, tree_node->element.name))
		{
			if (tree_node->element.contact)
			{
				snprintf((char*)cond->user, sizeof(cond->user), 
						"%s", tree_node->element.contact);
			}
		}
		else if (!strcmp(IP_ADDR_STR, tree_node->element.name))
		{
			if (NULL != tree_node->element.contact)
			{
				sscanf(tree_node->element.contact, "%d", 
					&cond->ip);
			}
		}
		else if (0 == strcmp(BEG_NODE_STR, tree_node->element.name))
		{
			if (NULL != tree_node->element.contact)
			{
				sscanf(tree_node->element.contact, "%d", 
					&cond->beg_node);
			}
		}
		else if (0 == strcmp(END_NODE_STR, tree_node->element.name))
		{
			if (NULL != tree_node->element.contact)
			{
				sscanf(tree_node->element.contact, "%d", 
					&cond->end_node);
			}
		}
		else if (0 == strcmp(SESS_ID_STR, tree_node->element.name))
		{
			if (tree_node->element.contact)
			{
				sscanf(tree_node->element.contact, "%d", 
					&cond->sess_id);
			}
		}

		tree_node = tree_node->next;
	}
}
static __inline__ int 
parse_operation_log_info(OperationLogPacket *log, 
	const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (!strcmp(COND_INFO_STR, tree_node->element.name))
					{
						parse_cond_info(&log->opt_log.cond, tree_node->child);
					}
					else if (!strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&log->result.code);
						}
					}
					else if (0 == strcmp(TOTAL_COUNT_STR, tree_node->element.name))
					{
						if (tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
								&log->opt_log.total_count);
						}
					}
					else if (0 == strcmp(LOG_INFO_STR, tree_node->element.name))
					{
						node_attr = tree_node->element.attr;
						if ((NULL != node_attr) && 
							(!strcmp(NODE_COUNT_STR, node_attr->name)))
						{
							sscanf(node_attr->value, "%d", &log->opt_log.node_count);
						}
						
						parse_log_info(log->opt_log.item, tree_node);
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_operation_log_request_xml(void *pvalue, 
		const char *buffer)
{
	OperationLogPacket *log = (OperationLogPacket*)pvalue;
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
	
	return parse_operation_log_info(log, buffer, GET_OPERATION_LOG_REQUEST_CMD);
}
int parse_get_operation_log_response_xml(void *pvalue, 
          const char *buffer)
{
	OperationLogPacket *log = (OperationLogPacket*)pvalue;

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
	
	return parse_operation_log_info(log, buffer, GET_OPERATION_LOG_RESPONSE_CMD);
}

int parse_set_alarm_upload_config_request_xml(void *pvalue, 
		const char *buffer)
{
	JAlarmUploadCfg *au_cfg;

	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
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
	
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(0 == strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(0 == strcmp(SET_ALARM_UPLOAD_CONFIG_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(ALARM_UPLOAD_ENABLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", &au_cfg->enable);
						}
					}
					else if (0 == strcmp(ALARM_UPLOAD_HOST_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							strncpy((char*)au_cfg->host, tree_node->element.contact, sizeof(au_cfg->host)-1);
						}
					}
					else if (0 == strcmp(ALARM_UPLOAD_PROT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", (int*)&au_cfg->port);
						}
					}
					else if (0 == strcmp(ALARM_UPLOAD_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", (int*)&au_cfg->type);
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			}
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_set_alarm_upload_config_response_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_result_info_xml((int*)pvalue, buffer, SET_ALARM_UPLOAD_CONFIG_RESULT_CMD);
}

static __inline__ int 
parse_preset_point_set(PPSetPacket *pp_set, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	int i;
	JPresetPoint *pp;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pp_set->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pp_set->session_id, sizeof(pp_set->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pp_set->domain_id, sizeof(pp_set->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pp_set->gu_id, sizeof(pp_set->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PRESET_STR, tree_node->element.name))
					{
						if (!strcmp(COUNT_STR, tree_node->element.attr->name))
						{
							sscanf(tree_node->element.attr->value, "%d", 
								&pp_set->pp_set.pp_count);
						}

						tree_child = tree_node->child;
						for (i=0; i<pp_set->pp_set.pp_count && tree_child; i++)
						{
							if (!strcmp(PRESET_POINT_STR, tree_child->element.name))
							{
								pp = &pp_set->pp_set.pp[i];
								tree_son = tree_child->child;
								while (tree_son)
								{
									if (!strcmp(PRESET_NAME_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											snprintf((char*)pp->name, sizeof(pp->name), 
													"%s", tree_son->element.contact);
										}
									}
									else if (!strcmp(PRESET_NO_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													&pp->preset);
										}
									}
									tree_son = tree_son->next;
								}
							}
							tree_child = tree_child->next;
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_preset_point_set_request_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_PRESET_POINT_SET_REQUEST_CMD);
}

int parse_get_preset_point_set_response_xml(void *pvalue, const char *buffer)
{
	PPSetPacket *pp_set = (PPSetPacket*)pvalue;
	
	return parse_preset_point_set(pp_set, buffer, GET_PRESET_POINT_SET_RESPONSE_CMD);
}

static __inline__ int 
parse_preset_point_info(PPConfigPacket *pp_cfg, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pp_cfg->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pp_cfg->session_id, sizeof(pp_cfg->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pp_cfg->domain_id, sizeof(pp_cfg->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pp_cfg->gu_id, sizeof(pp_cfg->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PRESET_ACTION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pp_cfg->pp_cfg.action));
						}
					}
					else if (0 == strcmp(PRESET_NAME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)pp_cfg->pp_cfg.pp.name, 
								sizeof(pp_cfg->pp_cfg.pp.name), 
								"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PRESET_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(pp_cfg->pp_cfg.pp.preset));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_set_preset_point_request_xml(void *pvalue, const char *buffer)
{
	PPConfigPacket *pp_cfg = (PPConfigPacket*)pvalue;
	
	return parse_preset_point_info(pp_cfg, buffer, SET_PRESET_POINT_REQUEST_CMD);
}

int parse_set_preset_point_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, SET_PRESET_POINT_RESULT_CMD);
}

static __inline__ int 
parse_cruise_point_info(JCruisePoint *crz_point, XmlTreeNode *tree_node)
{
	int i = 0, tmp;
	XmlTreeNode *tree_child = NULL;

	while (tree_node)
	{
		if (0 == strcmp(PRESET_POINT_STR, tree_node->element.name))
		{
			tree_child = tree_node->child;
			while (tree_child)
			{
				if (0 == strcmp(PRESET_NO_STR, tree_child->element.name))
				{
					if (NULL != tree_child->element.contact)
					{
						sscanf(tree_child->element.contact, "%d", &tmp);
						crz_point[i].preset = tmp;
					}
				}
				else if (0 == strcmp(CRUISE_SPEED_STR, tree_child->element.name))
				{
					if (NULL != tree_child->element.contact)
					{
						sscanf(tree_child->element.contact, "%d", &tmp);
						crz_point[i].speed = tmp;
					}
				}
				else if (0 == strcmp(DWELL_TIME_STR, tree_child->element.name))
				{
					if (NULL != tree_child->element.contact)
					{
						sscanf(tree_child->element.contact, "%d", &tmp);
						crz_point[i].dwell = tmp;
					}
				}
				tree_child = tree_child->next;
			}
		}

		i++;
		tree_node = tree_node->next;
	}

	return 0;
}
static __inline__ int 
parse_cruise_way_set(CruiseWaySetPacket *crz_set, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	
	int i;
	JCruiseInfo *crz_info;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(crz_set->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_set->session_id, sizeof(crz_set->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_set->domain_id, sizeof(crz_set->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_set->gu_id, sizeof(crz_set->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CRUISE_SET_STR, tree_node->element.name))
					{
						if (!strcmp(COUNT_STR, tree_node->element.attr->name))
						{
							sscanf(tree_node->element.attr->value, "%d", 
								&crz_set->crz_set.crz_count);
						}
						
						tree_child = tree_node->child;
						for (i=0; i<crz_set->crz_set.crz_count && tree_child; i++)
						{
							if (!strcmp(CRUISE_WAY_STR, tree_child->element.name))
							{
								crz_info = &crz_set->crz_set.crz_info[i];
								tree_son = tree_child->child;
								while (tree_son)
								{
									if (!strcmp(CRUISE_NAME_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											snprintf((char*)crz_info->crz_name, sizeof(crz_info->crz_name), 
													"%s", tree_son->element.contact);
										}
									}
									else if (!strcmp(CRUISE_NO_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													&crz_info->crz_no);
										}
									}
									
									tree_son = tree_son->next;
								}
							}
							
							tree_child = tree_child->next;
						}
						/*for (i=0; i<crz_set->crz_set.count && tree_child; i++)
						{
							if (!strcmp(CRUISE_WAY_STR, tree_child->element.name))
							{
								crz_way = &crz_set->crz_set.way[i];
								tree_son = tree_child->child;
								while (tree_son)
								{
									if (!strcmp(CRUISE_NAME_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											snprintf((char*)crz_way->crz_name, sizeof(crz_way->crz_name), 
													"%s", tree_son->element.contact);
										}
									}
									else if (!strcmp(CRUISE_NO_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", 
													&crz_way->crz_no);
										}
									}
									else if (!strcmp(CRUISE_STR, tree_son->element.name))
									{
										if (!strcmp(COUNT_STR, tree_son->element.attr->name))
										{
											sscanf(tree_son->element.contact, "%d", 
												&crz_way->count);
										}
										parse_cruise_point_info(crz_way->crz_pp, tree_son->child);
									}
									
									tree_son = tree_son->next;
								}
							}
							
							tree_child = tree_child->next;
						}*/
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_get_cruise_way_set_request_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_CRUISE_WAY_SET_REQUEST_CMD);
}

int parse_get_cruise_way_set_response_xml(void *pvalue, const char *buffer)
{
	CruiseWaySetPacket *crz_set = (CruiseWaySetPacket*)pvalue;
	
	return parse_cruise_way_set(crz_set, buffer, GET_CRUISE_WAY_SET_RESPONSE_CMD);
}

static __inline__ int 
parse_cruise_config(CruiseConfigPacket *crz_cfg, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(crz_cfg->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_cfg->session_id, sizeof(crz_cfg->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_cfg->domain_id, sizeof(crz_cfg->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_cfg->gu_id, sizeof(crz_cfg->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CRUISE_ACTION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(crz_cfg->crz_cfg.action));
						}
					}
					else if (0 == strcmp(CRUISE_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(crz_cfg->crz_cfg.crz_no));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_set_cruise_way_request_xml(void *pvalue, const char *buffer)
{
	CruiseConfigPacket *crz_cfg = (CruiseConfigPacket*)pvalue;
	
	return parse_cruise_config(crz_cfg, buffer, SET_CRUISE_WAY_REQUEST_CMD);
}

int parse_set_cruise_way_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, SET_CRUISE_WAY_RESULT_CMD);
}

static __inline__ int 
parse_config_cruise_way(CruiseWayPacket *crz_way, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;

	int i, tmp;
	JCruiseInfo *crz_info;
	JCruisePoint *crz_pp;
	
	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && 
			(0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				crz_info = &crz_way->crz_way.crz_info;
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(crz_way->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_way->session_id, sizeof(crz_way->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_way->domain_id, sizeof(crz_way->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_way->gu_id, sizeof(crz_way->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CRUISE_NAME_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)crz_info->crz_name, 
									sizeof(crz_info->crz_name), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(CRUISE_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&crz_info->crz_no);
						}
					}
					else if (0 == strcmp(CRUISE_WAY_STR, tree_node->element.name))
					{
						if (!strcmp(COUNT_STR, tree_node->element.attr->name))
						{
							sscanf(tree_node->element.attr->value, "%d", 
								&crz_way->crz_way.pp_count);
						}
						
						tree_child = tree_node->child;
						for (i=0; i<crz_way->crz_way.pp_count && tree_child; i++)
						{
							if (!strcmp(PRESET_POINT_STR, tree_child->element.name))
							{
								crz_pp = &crz_way->crz_way.crz_pp[i];
								tree_son = tree_child->child;
								while (tree_son)
								{
									if (!strcmp(PRESET_NO_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", &tmp);
											crz_pp->preset = tmp;
										}
									}
									else if (!strcmp(CRUISE_SPEED_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", &tmp);
											crz_pp->speed = tmp;
										}
									}
									else if (!strcmp(DWELL_TIME_STR, tree_son->element.name))
									{
										if (NULL != tree_son->element.contact)
										{
											sscanf(tree_son->element.contact, "%d", &tmp);
											crz_pp->dwell = tmp;
										}
									}
									
									tree_son = tree_son->next;
								}
							}
							
							tree_child = tree_child->next;
						}
					}
					
					tree_node = tree_node->next;
				}
				
				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_cruise_way_request_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, GET_CRUISE_WAY_REQUEST_CMD);
}

int parse_get_cruise_way_response_xml(void *pvalue, const char *buffer)
{
	CruiseWayPacket *crz_way = (CruiseWayPacket*)pvalue;
	
	return parse_config_cruise_way(crz_way, buffer, GET_CRUISE_WAY_RESPONSE_CMD);
}
int parse_add_cruise_way_request_xml(void *pvalue, const char *buffer)
{
	CruiseWayPacket *crz_way = (CruiseWayPacket*)pvalue;
	
	return parse_config_cruise_way(crz_way, buffer, ADD_CRUISE_WAY_REQUEST_CMD);
}

int parse_add_cruise_way_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, ADD_CRUISE_WAY_RESULT_CMD);
}
int parse_modify_cruise_way_request_xml(void *pvalue, const char *buffer)
{
	CruiseWayPacket *crz_way = (CruiseWayPacket*)pvalue;
	
	return parse_config_cruise_way(crz_way, buffer, MODIFY_CRUISE_WAY_REQUEST_CMD);
}
int parse_modify_cruise_way_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, MODIFY_CRUISE_WAY_RESULT_CMD);
}

static __inline__ int 
parse_3d_control(_3DControlPacket *_3d_ctr, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					/*if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(_3d_ctr->result.code));
						}
					}
					else */if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)_3d_ctr->session_id, sizeof(_3d_ctr->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)_3d_ctr->domain_id, sizeof(_3d_ctr->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)_3d_ctr->gu_id, sizeof(_3d_ctr->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(X_OFFSET_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(_3d_ctr->_3d_ctr.x_offset));
						}
					}
					else if (0 == strcmp(Y_OFFSET_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(_3d_ctr->_3d_ctr.y_offset));
						}
					}
					else if (0 == strcmp(AMPLIFY_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(_3d_ctr->_3d_ctr.amplify));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_3d_control_request_xml(void *pvalue, const char *buffer)
{
	_3DControlPacket *_3d_ctr = (_3DControlPacket*)pvalue;
	
	return parse_3d_control(_3d_ctr, buffer, _3D_CONTROL_REQUEST_CMD);
}

int parse_3d_control_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, _3D_CONTROL_RESULT_CMD);
}

int parse_3d_goback_request_xml(void *pvalue, const char *buffer)
{
	Request *request = (Request*)pvalue;
	
	return parse_request_action_xml(request, buffer, _3D_GOBACK_REQUEST_CMD);
}

int parse_3d_goback_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, _3D_GOBACK_RESULT_CMD);
}

int parse_alarm_link_io_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	LinkIOPacket *link_io = (LinkIOPacket*)pvalue;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(ALARM_LINK_IO_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)link_io->session_id, sizeof(link_io->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)link_io->domain_id, sizeof(link_io->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)link_io->gu_id, sizeof(link_io->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					/*else if (0 == strcmp(ALARM_TYPE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(link_io->link_io.alarm_type));
						}
					}*/
					else if (0 == strcmp(TIME_LEN_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(link_io->link_io.time_len));
						}
					}
					else if (0 == strcmp(DATA_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)link_io->link_io.data, 
                                    sizeof(link_io->link_io.data), 
									"%s", tree_node->element.contact);
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_alarm_link_io_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, ALARM_LINK_IO_RESULT_CMD);
}

int parse_alarm_link_preset_request_xml(void *pvalue, const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	LinkPresetPacket *link_preset = (LinkPresetPacket*)pvalue;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(ALARM_LINK_PRESET_REQUEST_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)link_preset->session_id, sizeof(link_preset->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)link_preset->domain_id, sizeof(link_preset->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)link_preset->gu_id, sizeof(link_preset->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(PRESET_NO_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(link_preset->link_preset.preset));
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

int parse_alarm_link_preset_result_xml(void *pvalue, const char *buffer)
{
	Result *result = (Result*)pvalue;
	
	return parse_result_action_xml(result, buffer, ALARM_LINK_PRESET_RESULT_CMD);
}

static __inline__ int 
parse_resolution_info(ResolutionInfoPacket *rsl_info, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{ 

		tree_root = xml_tree->child;

		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(rsl_info->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)rsl_info->session_id, sizeof(rsl_info->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)rsl_info->domain_id, sizeof(rsl_info->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)rsl_info->gu_id, sizeof(rsl_info->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(RESOLUTION_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&rsl_info->rsl_info.resolution);
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_resolution_info_request_xml(void *pvalue, 
		const char *buffer)
{
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_RESOLUTION_INFO_REQUEST_CMD);
}
int parse_get_resolution_info_response_xml(void *pvalue, 
          const char *buffer)
{
	ResolutionInfoPacket *rsl_info = (ResolutionInfoPacket*)pvalue;
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
	
	return parse_resolution_info(rsl_info, buffer, GET_RESOLUTION_INFO_RESPONSE_CMD);
}
int parse_set_resolution_info_request_xml(void *pvalue, 
		const char *buffer)
{	
	ResolutionInfoPacket *rsl_info = (ResolutionInfoPacket*)pvalue;
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
	
	return parse_resolution_info(rsl_info, buffer, SET_RESOLUTION_INFO_REQUEST_CMD);
}
int parse_set_resolution_info_result_xml(void *pvalue, 
          const char *buffer)
{
	Result *result = (Result*)pvalue;
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
	
	return parse_result_action_xml(result, buffer, SET_RESOLUTION_INFO_RESULT_CMD);
}

static __inline__ void 
parse_each_ircut_info(JIrcut *ircut, XmlTreeNode *tree_node)
{
	int id = 0;
	XmlTreeNode *tree_child = NULL;
	XmlTreeNode *tree_son = NULL;
	XmlTreeNode *tree_grandson = NULL;
	XmlTreeAttr *node_attr = NULL;

    while (tree_node)
    {
        if (!strcmp(SWITCH_MODE_STR, tree_node->element.name))
        {
            if (NULL != tree_node->element.contact)
            {
                sscanf(tree_node->element.contact, "%d", 
                    &ircut->ircut_mode);
            }
        }
        else if (!strcmp(AUTO_C2B_STR, tree_node->element.name))
        {
            if (NULL != tree_node->element.contact)
            {
                sscanf(tree_node->element.contact, "%d", 
                    &ircut->auto_c2b);
            }
        }
        else if (!strcmp(AUTO_SWITCH_STR, tree_node->element.name))
        {
            tree_child = tree_node->child;
            if (!strcmp(SENSITIVE_STR, tree_child->element.name))
            {
                if (NULL != tree_child->element.contact)
                {
                    sscanf(tree_child->element.contact, "%d", 
                        &ircut->autos.sensitive);
                }
            }
        }
        else if (!strcmp(RTC_SWITCH_STR, tree_node->element.name))
        {
            tree_child = tree_node->child;
            if (!strcmp(RTC_STR, tree_child->element.name))
            {
                if (NULL != tree_child->element.contact)
                {
                    sscanf(tree_child->element.contact, "%d", 
                        &ircut->rtcs.rtc);
                }
            }
        }
        else if (!strcmp(TIMER_SWITCH_STR, tree_node->element.name))
        {
            tree_child = tree_node->child;
            while (tree_child)
            {
                if (!strcmp(DAY_STR, tree_child->element.name))
                {
    				node_attr = tree_child->element.attr;
    				if ((NULL != node_attr) && 
    					(!strcmp(ID_STR, node_attr->name)))
    				{
    					sscanf(node_attr->value, "%d", &id);
    				}
                    
					if (J_SDK_SUNDAY<=id && J_SDK_MAX_DAY_SIZE>id)
                    {
                        int value, seg = 0;
                        tree_son = tree_child->child;
                        while (tree_son)
                        {
                            if (!strcmp(SEGMENT_STR, tree_son->element.name))
                            {
                                tree_grandson = tree_son->child;
                                while (tree_grandson)
                                {
                                    if (!strcmp(SEG_OPNT_STR, tree_grandson->element.name))
                                    {
                                        if (NULL != tree_grandson->element.contact)
                                        {
                                            sscanf(tree_grandson->element.contact, "%d", &value);
                                            ircut->timers[id].seg_time[seg].open = value;
                                        }
                                    }
                                    else if (!strcmp(BEGIN_TIMES_STR, tree_grandson->element.name))
                                    {
                                        if (NULL != tree_grandson->element.contact)
                                        {
                                            sscanf(tree_grandson->element.contact, "%d", &value);
                                            ircut->timers[id].seg_time[seg].begin_sec = value;
                                        }
                                    }
                                    else if (!strcmp(END_TIMES_STR, tree_grandson->element.name))
                                    {
                                        if (NULL != tree_grandson->element.contact)
                                        {
                                            sscanf(tree_grandson->element.contact, "%d", &value);
                                            ircut->timers[id].seg_time[seg].end_sec = value;
                                        }
                                    }
                                    tree_grandson = tree_grandson->next;
                                }
                            }
                            tree_son = tree_son->next;
                            if (++seg>4)
                                break;
                        }
                    }
                }
                tree_child = tree_child->next;
            }
        }
        tree_node = tree_node->next;
    }
}

static __inline__ int 
parse_ircut_control_info(IrcutControlPacket *ircut_ctrl, const char *buffer, const char *command)
{
    int channel, count = 0;
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;
	XmlTreeNode *tree_child = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(ircut_ctrl->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ircut_ctrl->session_id, sizeof(ircut_ctrl->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ircut_ctrl->domain_id, sizeof(ircut_ctrl->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)ircut_ctrl->gu_id, sizeof(ircut_ctrl->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(IRCUT_CONTROL_STR, tree_node->element.name))
					{
						node_attr = tree_node->element.attr;
						if ((NULL != node_attr) && 
							(!strcmp(COUNT_STR, node_attr->name)))
						{
							sscanf(node_attr->value, "%d", &count);
                            ircut_ctrl->ircut_ctrl.count = count;
						}
                        
                        tree_child = tree_node->child;
                        for (; count>0; count--)
                        {
                            if (!strcmp(IRCUT_STR, tree_child->element.name))
                            {
        						node_attr = tree_child->element.attr;
        						if ((NULL != node_attr) && 
        							(!strcmp(CHANNEL_STR, node_attr->name)))
        						{
        							sscanf(node_attr->value, "%d", &channel);
        						}
                                
                                if (0<=channel && J_SDK_MAX_CHN_SIZE>channel)
                                    parse_each_ircut_info(&ircut_ctrl->ircut_ctrl.ircut[channel], 
                                        tree_child->child);
                            }
                            tree_child = tree_child->next;
                        }
					}
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_ircut_control_info_request_xml(void *pvalue, 
		const char *buffer)
{
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_IRCUT_CONTROL_REQUEST_CMD);
}
int parse_get_ircut_control_info_response_xml(void *pvalue, 
          const char *buffer)
{
	IrcutControlPacket *ircut_ctrl = (IrcutControlPacket*)pvalue;
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
	
	return parse_ircut_control_info(ircut_ctrl, buffer, GET_IRCUT_CONTROL_RESPONSE_CMD);
}
int parse_set_ircut_control_info_request_xml(void *pvalue, 
		const char *buffer)
{
	IrcutControlPacket *ircut_ctrl = (IrcutControlPacket*)pvalue;
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
	
	return parse_ircut_control_info(ircut_ctrl, buffer, SET_IRCUT_CONTROL_REQUEST_CMD);
}
int parse_set_ircut_control_info_result_xml(void *pvalue, 
          const char *buffer)
{
	Result *result = (Result*)pvalue;
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
	
	return parse_result_action_xml(result, buffer, SET_IRCUT_CONTROL_RESULT_CMD);
}

int parse_get_extranet_port_request_xml(void *pvalue, 
		const char *buffer)
{
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_EXTRANET_PORT_REQUEST_CMD);
}
int parse_get_extranet_port_response_xml(void *pvalue, 
          const char *buffer)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	ExtranetPortPacket *extranet = (ExtranetPortPacket*)pvalue;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(GET_EXTRANET_PORT_RESPONSE_CMD, node_attr->value)))
			{
				tree_node = tree_root->child;

				while (NULL != tree_node)
				{
					if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)extranet->session_id, sizeof(extranet->session_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)extranet->domain_id, sizeof(extranet->domain_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)extranet->gu_id, sizeof(extranet->gu_id), 
									"%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DATA_PORT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
						    int port;
							sscanf(tree_node->element.contact, "%d", 
									&port);
                            extranet->extranet.data_port = port;
						}
					}
					
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}

static __inline__ int 
parse_trough_params(XmlTreeNode *tree_node, JField *field)
{
	int count = 0;
    JTrough *trough;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *xml_root, *xml_father, *xml_child, *xml_son;

    xml_root = tree_node->child;
    while (xml_root)
    {
		if (!strcmp(TROUGH_PARAM_STR, xml_root->element.name) &&
			MAX_TROUGH_SIZE > count)
		{
		    trough = &field->trough[count];

			node_attr = xml_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(TROUGH_ID_STR, node_attr->name)))
			{
				sscanf(node_attr->value, "%d", &trough->id);
			}
			else
    		{
    			xml_root = xml_root->next;
				continue;
    		}

            xml_father = xml_root->child;
            while (xml_father)
            {
                if (!strcmp(NAME_STR, xml_father->element.name))
				{
					if (NULL != xml_father->element.contact)
					{
						snprintf((char*)trough->name, sizeof(trough->name), 
								"%s", xml_father->element.contact);
					}
				}
				else if (!strcmp(GRASS_PERCENT_STR, xml_father->element.name))
				{
					if (NULL != xml_father->element.contact)
					{
            			sscanf(xml_father->element.contact, "%d", &trough->percent);
					}
				}
				else if (!strcmp(QUADR_ANGLE_STR, xml_father->element.name))
				{
        			int idx = 0;
				    xml_child = xml_father->child;
                    while (xml_child)
                    {
                        if (!strcmp(POINT_STR, xml_child->element.name) &&
                            4 > idx)
        				{
        				    xml_son = xml_child->child;
                            while (xml_son)
                            {
                                if (!strcmp(POINT_X_STR, xml_son->element.name))
            					{
            						if (NULL != xml_son->element.contact)
            						{
            						    int x;
            							sscanf(xml_son->element.contact, "%d", &x);
                                        trough->quadr.angle[idx].x = x;
                                        
            						}
            					}
            					else if (!strcmp(POINT_Y_STR, xml_son->element.name))
            					{
            						if (NULL != xml_son->element.contact)
            						{
            						    int y;
            							sscanf(xml_son->element.contact, "%d", &y);
                                        trough->quadr.angle[idx].y = y;
            						}
            					}
                                xml_son = xml_son->next;
                            }
        				}
                        idx++;
                        xml_child = xml_child->next;
                    }
				}
                xml_father = xml_father->next;
            }
        };

        count++;
        xml_root = xml_root->next;
    }

    return count;
}

static __inline__ int 
parse_herd_analyse_info(HerdAnalysePacket *herd_analyse, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(herd_analyse->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)herd_analyse->session_id, 
                                sizeof(herd_analyse->session_id), 
                                "%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)herd_analyse->domain_id, 
                                sizeof(herd_analyse->domain_id), 
                                "%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)herd_analyse->gu_id, 
                                sizeof(herd_analyse->gu_id), 
                                "%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(FODDER_EANBLE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
						    int enable;
							sscanf(tree_node->element.contact, "%d", &enable);
                            herd_analyse->herd_analyse.fodder_eable = enable;
						}
					}
					else if (0 == strcmp(REPORT_INTV_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
						    int report;
							sscanf(tree_node->element.contact, "%d", &report);
                            herd_analyse->herd_analyse.report_intv = report;
						}
					}
					else if (0 == strcmp(MAX_WIDTH_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
                                &herd_analyse->herd_analyse.max_width);
						}
					}
					else if (0 == strcmp(MAX_HEIGHT_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
                                &herd_analyse->herd_analyse.max_height);
						}
					}
					else if (0 == strcmp(WEEK_DAY_S_STR, tree_node->element.name))
					{
						get_weekday_info(tree_node, &herd_analyse->herd_analyse.week);
					}
					else if (0 == strcmp(TROUGH_PARAMS_STR, tree_node->element.name))
					{
					    int count = parse_trough_params(tree_node, 
                                        &herd_analyse->herd_analyse.field);
                        if (count>0)
                            herd_analyse->herd_analyse.field.trough_count = count;
					}
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_herd_analyse_info_request_xml(void *pvalue, 
		const char *buffer)
{
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_HERD_ANALYSE_REQUEST_CMD);
}
int parse_get_herd_analyse_info_response_xml(void *pvalue, 
          const char *buffer)
{
	HerdAnalysePacket *herd_analyse = (HerdAnalysePacket*)pvalue;
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
	
	return parse_herd_analyse_info(herd_analyse, buffer, GET_HERD_ANALYSE_RESPONSE_CMD);
}
int parse_set_herd_analyse_info_request_xml(void *pvalue, 
		const char *buffer)
{
	HerdAnalysePacket *herd_analyse = (HerdAnalysePacket*)pvalue;
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
	
	return parse_herd_analyse_info(herd_analyse, buffer, SET_HERD_ANALYSE_REQUEST_CMD);
}
int parse_set_herd_analyse_info_result_xml(void *pvalue, 
          const char *buffer)
{
	Result *result = (Result*)pvalue;
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
	
	return parse_result_action_xml(result, buffer, SET_HERD_ANALYSE_RESULT_CMD);
}

static __inline__ int 
parse_grass_percent(GrassPercentPacket *grass_percent, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(grass_percent->result.code));
						}
					}
					else if (0 == strcmp(SESSION_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)grass_percent->session_id, 
                                sizeof(grass_percent->session_id), 
                                "%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(DOMAIN_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)grass_percent->domain_id, 
                                sizeof(grass_percent->domain_id), 
                                "%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(GU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)grass_percent->gu_id, 
                                sizeof(grass_percent->gu_id), 
                                "%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(TROUGH_PARAMS_STR, tree_node->element.name))
					{
					    int count = parse_trough_params(tree_node, 
                                        &grass_percent->grass.field);
                        if (count>0)
                            grass_percent->grass.field.trough_count = count;
					}
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_grass_percent_request_xml(void *pvalue, 
		const char *buffer)
{
	Request *request = (Request*)pvalue;
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
	
	return parse_request_action_xml(request, buffer, GET_GRASS_PERCENT_REQUEST_CMD);
}
int parse_get_grass_percent_response_xml(void *pvalue, 
          const char *buffer)
{
	GrassPercentPacket *grass_percent = (GrassPercentPacket*)pvalue;
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
	
	return parse_grass_percent(grass_percent, buffer, GET_GRASS_PERCENT_RESPONSE_CMD);
}

static __inline__ int 
parse_p2p_id(P2PIdPacket *p2p_id, const char *buffer, const char *command)
{
	XmlTreeNode *xml_tree = NULL;
	XmlTreeNode *tree_root = NULL;
	XmlTreeAttr *node_attr = NULL;
	XmlTreeNode *tree_node = NULL;

	xml_tree = (XmlTreeNode*)j_xml_alloc(sizeof(XmlTreeNode));
	init_xml_tree_node(&xml_tree);
    
	if (0 == xml_tree_parse_by_mxml(xml_tree, buffer))
	{
		tree_root = xml_tree->child;
		if ((NULL != tree_root) && (0 == strcmp(MESSAGE_STR, tree_root->element.name)))
		{
			node_attr = tree_root->element.attr;
			if ((NULL != node_attr) && 
				(!strcmp(MESSAGE_TYPE_STR, node_attr->name)) && 
				(!strcmp(command, node_attr->value)))
			{
				tree_node = tree_root->child;
				while (NULL != tree_node)
				{
					if (0 == strcmp(RESULT_CODE_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							sscanf(tree_node->element.contact, "%d", 
									&(p2p_id->result.code));
						}
					}
					else if (0 == strcmp(PU_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)p2p_id->p2p.pu_id, 
                                sizeof(p2p_id->p2p.pu_id), 
                                "%s", tree_node->element.contact);
						}
					}
					else if (0 == strcmp(P2P_ID_STR, tree_node->element.name))
					{
						if (NULL != tree_node->element.contact)
						{
							snprintf((char*)p2p_id->p2p.p2p_id, 
                                sizeof(p2p_id->p2p.p2p_id), 
                                "%s", tree_node->element.contact);
						}
					}
					tree_node = tree_node->next;
				}

				xml_tree_delete(xml_tree);
				return 0;
			} 
		}
	}

	xml_tree_delete(xml_tree);
	return -1;
}
int parse_get_p2p_id_request_xml(void *pvalue, 
		const char *buffer)
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
	
	return parse_p2p_id((P2PIdPacket*)pvalue, buffer, GET_P2P_ID_REQUEST_CMD);
}
int parse_get_p2p_id_response_xml(void *pvalue, 
          const char *buffer)
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
	
	return parse_p2p_id((P2PIdPacket*)pvalue, buffer, GET_P2P_ID_RESPONSE_CMD);
}



