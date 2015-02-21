
#include <string.h>

#include "mxml.h"
#include "config.h"
#include "xml-tree.h"
#include "nmp_xmlmem.h"
#include "nmp_xmlinfo.h"
#include "nmp_packet.h"


#ifndef DEBUG_PARSE
#define DEBUG_PARSE					0
#endif

#ifndef DEBUG_MERGE
#define DEBUG_MERGE					0
#endif

#ifndef DEBUG_DELETE
#define DEBUG_DELETE				0
#endif

#define MAX_WHITE_SPACE_SIZE		62
#define DEFUALT_START_YEAR			1900

#define DEFAULT_MAX_CONTACT_LEN     1024*64

void show_time_struct(JTime *time)
{
	printf("year  : %d\n", time->year);
	printf("month : %02d\n", time->month);
	printf("date  : %02d\n", time->date);
	printf("hour  : %02d\n", time->hour);
	printf("minute: %02d\n", time->minute);
	printf("second: %02d\n", time->second);
}

int parse_time_string(const char *time_string, JTime *time)
{
	char *p_first, *p_second;
	char time_buf[J_SDK_MAX_TIME_LEN];
	int year, month, date, hour, minute, second;

	if (!strchr(time_string, ' '))
		return -1;
	else
	{
		strncpy(time_buf, time_string, sizeof(time_buf)-1);
		time_buf[sizeof(time_buf)-1] = '\0';
	}
	
	//先将时间分割成两段,例如:2010-8-11 14:48:33   --> [2010-8-11]和[14:48:33]
	p_first = strtok(time_buf, " ");	//以空格位置取得第一段
	p_second = strtok(NULL, "");    	//取得第二段
	
	//printf("first : %s\n", p_first);
	//printf("second: %s\n", p_second);

	if(NULL != p_first)
	{
		if (NULL != (p_first = strtok(p_first, "-")))
		{
			sscanf(p_first, "%d", &year);
			time->year = year - DEFUALT_START_YEAR;
		}
		else
		{
			printf("year NULL.\n");
			return -1;
		}
		
		if (NULL != (p_first = strtok(NULL, "-")))
		{
			sscanf(p_first, "%d", &month);
			time->month = month;
		}
		else
		{
			printf("month NULL.\n");
			return -1;
		}
		
		if (NULL != (p_first = strtok(NULL, "-")))
		{
			sscanf(p_first, "%d", &date);
			time->date = date;
		}
		else
		{
			printf("date NULL.\n");
			return -1;
		}
	}	
	else
	{	
		printf("p_first NULL.\n");
		return -1;
	}
	
	if(NULL != p_second) 
	{		
		if (NULL != (p_second = strtok(p_second, ":")))
		{
			sscanf(p_second, "%d", &hour);
			time->hour = hour;
		}
		else
		{
			printf("hour NULL.\n");
			return -1;
		}
		
		if (NULL != (p_second = strtok(NULL, ":")))
		{
			sscanf(p_second, "%d", &minute);
			time->minute = minute;
		}
		else
		{
			printf("minute NULL.\n");
			return -1;
		}
		
		if (NULL != (p_second = strtok(NULL, ":")))
		{
			sscanf(p_second, "%d", &second);
			time->second = second;
		}
		else
		{
			printf("second NULL.\n");
			return -1;
		}

	}
	else
	{
		printf("p_second NULL.\n");
		return -1;
	}

	return 0;
}

int merge_time_string(char *buffer, size_t size, JTime *time)
{
	snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d", 
			 time->year + DEFUALT_START_YEAR, 
			 time->month, 
			 time->date, 
			 time->hour, 
			 time->minute, 
			 time->second);
	
	return 0;
}

void show_time_part(JTime *time_start, JTime *time_end)
{
	printf("start->hour  : %02d\n", time_start->hour);
	printf("       minute: %02d\n", time_start->minute);
	printf("       second: %02d\n", time_start->second);
	printf("end  ->hour  : %02d\n", time_end->hour);
	printf("       minute: %02d\n", time_end->minute);
	printf("       second: %02d\n", time_end->second);
}

int parse_time_part_string(const char *time_part, JTime *time_start, JTime *time_end)
{
	char *p_start, *p_end;
	char time_buf[J_SDK_MAX_TIME_LEN];
	int hour, minute, second;
	
	if (!strchr(time_part, '-'))
		return -1;
	else
	{
		strncpy(time_buf, time_part, sizeof(time_buf)-1);
		time_buf[sizeof(time_buf)-1] = '\0';
	}
	
	p_start = strtok(time_buf, "-");
	p_end = strtok(NULL, "");

	if(NULL != p_start)
	{
		if (NULL != (p_start = strtok(p_start, ":")))
		{
			sscanf(p_start, "%d", &hour);
			time_start->hour = hour;
		}
		else
		{
			printf("start:hour NULL.\n");
			return -1;
		}
		
		if (NULL != (p_start = strtok(NULL, ":")))
		{
			sscanf(p_start, "%d", &minute);
			time_start->minute = minute;
		}
		else
		{
			printf("start:minute NULL.\n");
			return -1;
		}
		
		if (NULL != (p_start = strtok(NULL, ":")))
		{
			sscanf(p_start, "%d", &second);
			time_start->second = second;
		}
		else
		{
			printf("start:second NULL.\n");
			return -1;
		}
	}
	else
	{
		printf("p_start NULL.\n");
		return -1;
	}
	
	if(NULL != p_end)
	{
		if (NULL != (p_end = strtok(p_end, ":")))
		{
			sscanf(p_end, "%d", &hour);
			time_end->hour = hour;
		}
		else
		{
			printf("end:hour NULL.\n");
			return -1;
		}
		
		if (NULL != (p_end = strtok(NULL, ":")))
		{
			sscanf(p_end, "%d", &minute);
			time_end->minute = minute;
		}
		else
		{
			printf("end:minute NULL.\n");
			return -1;
		}
		
		if (NULL != (p_end = strtok(NULL, ":")))
		{
			sscanf(p_end, "%d", &second);
			time_end->second = second;
		}
		else
		{
			printf("end:second NULL.\n");
			return -1;
		}
	}
	else
	{
		printf("p_end NULL.\n");
		return -1;
	}

	return 0;
}

int merge_time_part_string(char *buffer, size_t size, 
										JTime *time_start, JTime *time_end)
{
	snprintf(buffer, size, "%02d:%02d:%02d-%02d:%02d:%02d", 
			 time_start->hour, 
			 time_start->minute, 
			 time_start->second, 
			 time_end->hour, 
			 time_end->minute, 
			 time_end->second);
	
	return 0;
}

__inline__ void init_xml_tree_node(XmlTreeNode **new_tree_node)
{
	(*new_tree_node)->prev = NULL;
	(*new_tree_node)->next = NULL;
	(*new_tree_node)->parent = NULL;
	(*new_tree_node)->child = NULL;
	(*new_tree_node)->element.attr = NULL;
	(*new_tree_node)->element.extend_contact = NULL;
	memset((*new_tree_node)->element.name, 0, sizeof((*new_tree_node)->element.name));
	memset((*new_tree_node)->element.contact, 0, sizeof((*new_tree_node)->element.contact));
}

__inline__ void init_xml_tree_attr(XmlTreeAttr **new_tree_attr)
{
	(*new_tree_attr)->prev = NULL;
	(*new_tree_attr)->next = NULL;
	memset((*new_tree_attr)->name, 0, sizeof((*new_tree_attr)->name));
	memset((*new_tree_attr)->value, 0, sizeof((*new_tree_attr)->value));
}

static int show_tree_node_info(XmlTreeNode *tree_node, int flags)
{
	int i = flags;
	char *tmp = "    "; 
	char whitespace[MAX_WHITE_SPACE_SIZE];
	
	XmlTreeNode *p_tree_node = NULL;
	XmlTreeAttr *tree_attr = NULL;
	
	if (NULL == tree_node)
	{
		printf("child NULL.\n");
		return -1;
	}
	
	memset(whitespace, 0, sizeof(whitespace));

	while (0 != i--)
	{
		strcat(whitespace, tmp);
	}
	
	p_tree_node = tree_node;
	while (NULL != p_tree_node)
	{
		if (strlen(p_tree_node->element.name))
		{
			printf("%selment name : %s\n", whitespace, p_tree_node->element.name);
		}
		
		tree_attr = p_tree_node->element.attr;
		while (NULL != tree_attr)
		{
			printf("%s%sattr name : %s\n", tmp, whitespace, tree_attr->name);
			printf("%s%sattr value: %s\n", tmp, whitespace, tree_attr->value);

			tree_attr = tree_attr->next;
		}
		
		if (strlen(p_tree_node->element.contact))// && !p_tree_node->child)
		{
			printf("%s%selment value: %s\n", tmp, whitespace, p_tree_node->element.contact);
		}
		
		if (NULL != p_tree_node->child)
		{
			show_tree_node_info(p_tree_node->child, flags+1);
		}
		
		p_tree_node = p_tree_node->next;
	}

	return 0;
}

int show_xml_tree_info(XmlTreeNode *xml_tree)
{
	XmlTreeNode *p_node = NULL;
	
	if (NULL == xml_tree)
	{
		printf("xml_tree NULL.\n");
		return -1;
	}
	
	p_node = xml_tree;
	show_tree_node_info(p_node, 0);

	return 0;
}

static int xml_tree_node_free(XmlTreeNode *xml_node, int flags)
{
    int size;
	XmlTreeNode *p_xml_node = NULL;
	XmlTreeAttr *tree_attr = NULL;
	XmlTreeAttr *p_attr = NULL;
	
	if (NULL == xml_node)
	{
		printf("xml_node NULL.\n");
		return -1;
	}

#if DEBUG_DELETE
	int i = flags;
	char *tmp = "    "; 
	char whitespace[MAX_WHITE_SPACE_SIZE];
	memset(whitespace, 0, sizeof(whitespace));

	while (0 != i--)
	{
		strcat(whitespace, tmp);
	}
#endif
	
	while (NULL != xml_node)
	{
		p_xml_node = xml_node->next;
		
		#if DEBUG_DELETE
		if (strlen(xml_node->element.name))
		{
			printf("%selment name : %s\n", whitespace, xml_node->element.name);
		}
		if (strlen(xml_node->element.contact))// && !xml_node->child)
		{
			printf("%s%selment value: %s\n", tmp, whitespace, xml_node->element.contact);
		}
		#endif

		tree_attr = xml_node->element.attr;
		while (NULL != tree_attr)
		{
			#if DEBUG_DELETE
			printf("%s%sattr name : %s\n", tmp, whitespace, tree_attr->name);
			printf("%s%sattr value: %s\n", tmp, whitespace, tree_attr->value);
			#endif
			
			p_attr = tree_attr->next;
			j_xml_dealloc(tree_attr, sizeof(XmlTreeAttr));
			tree_attr = p_attr;
		}
		
		if (NULL != xml_node->child)
		{
			xml_tree_node_free(xml_node->child, flags+1);
		}

        if (xml_node->element.extend_contact)
        {
            xml_node->element.extend_contact -= sizeof(int);
            size = atoi(xml_node->element.extend_contact) + sizeof(int) + 1;
            j_xml_dealloc(xml_node->element.extend_contact, size);
        }
        
		j_xml_dealloc(xml_node, sizeof(XmlTreeNode));
		xml_node = p_xml_node;
	}

	return 0;
}

int xml_tree_delete(XmlTreeNode *xml_tree)
{
	if (NULL == xml_tree)
	{
		printf("xml_tree NULL.\n");
		return -1;
	}
	else
	{
		if (0 == xml_tree_node_free(xml_tree, 0))
			xml_tree = NULL;
	}
	
	return 0;
}

char *parse_xml_cmd_by_mxml(const char *xml_buf, char cmd_buf[], size_t size)
{
	mxml_node_t *mxml_node_tree = NULL;
	mxml_node_t *mxml_node_node = NULL;
	mxml_attr_t *mxml_attr = NULL;
	
	if (NULL == xml_buf)
	{
		printf("xml_buf NULL.\n");
		return NULL;
	}
	
	if (NULL == (mxml_node_tree = mxmlLoadString(NULL, xml_buf, MXML_TEXT_CALLBACK)))
	{
		printf("mxmlLoadString error.\n");
		return NULL;
	}
	
	mxml_node_node = mxmlFindElement(mxml_node_tree, mxml_node_tree, 
						NULL, NULL, NULL, MXML_DESCEND_FIRST);
	if (NULL != mxml_node_node)
	{
		if (0 == strcmp(mxml_node_node->value.element.name, MESSAGE_STR))
		{
			if (0 < mxml_node_node->value.element.num_attrs)
			{
				mxml_attr = mxml_node_node->value.element.attrs;
				if (0 == strcmp(mxml_attr->name, MESSAGE_TYPE_STR))
				{
					snprintf(cmd_buf, size, "%s", mxml_attr->value);
					
					mxmlDelete(mxml_node_tree);
					return cmd_buf;
				}
			}
		}
	}
	
	mxmlDelete(mxml_node_tree);
	return NULL;	
}

static __inline__ int 
get_mxml_node_info(XmlTreeNode *tree_node, 
	mxml_node_t *mxml_node, int flags)
{
	int n, len, size, mxml_attr_num = 0;
	mxml_attr_t *mxml_attr = NULL;
	mxml_node_t *mxml_node_value = NULL;
	XmlTreeAttr *tree_attr = NULL;
	XmlTreeAttr *p_tree_attr = NULL;
	
#if DEBUG_PARSE
	int i = flags;
	char *tmp = "    ";
	char whitespace[MAX_WHITE_SPACE_SIZE];
	memset(whitespace, 0, sizeof(whitespace));
	
	while (0 != i--)
	{
		strcat(whitespace, tmp);
	}
#endif
	
	if (NULL == mxml_node)
	{
		printf("mxml_node NULL.\n");
		return -1;
	}
	
#if DEBUG_PARSE
	printf("%snode->name: %s\n", whitespace, mxml_node->value.element.name);
#endif
	
	snprintf(tree_node->element.name, sizeof(tree_node->element.name), 
										 "%s", 
										 mxml_node->value.element.name);

	mxml_attr     = mxml_node->value.element.attrs;
	mxml_attr_num = mxml_node->value.element.num_attrs;
	for (n=0; n<mxml_attr_num; n++)
	{
		tree_attr = (XmlTreeAttr*)j_xml_alloc(sizeof(XmlTreeAttr));
		init_xml_tree_attr(&tree_attr);

		#if DEBUG_PARSE
		printf("%s%sattr name : %s\n", tmp, whitespace, mxml_attr[n].name);
		printf("%s%sattr value: %s\n", tmp, whitespace, mxml_attr[n].value);
		#endif
		
		snprintf(tree_attr->name, sizeof(tree_attr->name), "%s", mxml_attr[n].name);
		snprintf(tree_attr->value, sizeof(tree_attr->value), "%s", mxml_attr[n].value);

		if (NULL == tree_node->element.attr)
		{
			tree_node->element.attr = tree_attr;
			p_tree_attr = tree_node->element.attr;
		}
		else
		{
			p_tree_attr->next = tree_attr;
			p_tree_attr = p_tree_attr->next;
		}
	}
	
	mxml_node_value = mxml_node->child;
	if ((NULL != mxml_node_value) && (MXML_TEXT == mxml_node_value->type))
	{
		if (0 == mxml_node_value->value.text.whitespace)	//忽略空白
		{
			char tmp_buf[DEFAULT_MAX_CONTACT_LEN];
			memset(tmp_buf, 0, sizeof(tmp_buf));
			while ((NULL != mxml_node_value) && 
					(MXML_TEXT == mxml_node_value->type) && 
					(NULL != mxml_node_value->value.text.string))
			{
				if (1 == mxml_node_value->value.text.whitespace)
					strcat(tmp_buf, " ");
				
				strcat(tmp_buf, mxml_node_value->value.text.string);
				mxml_node_value = mxml_node_value->next;
			}
			
			#if DEBUG_PARSE
			printf("%s%stext.string: %s\n", tmp, whitespace, tmp_buf);
			#endif

            len = strlen(tmp_buf);
			if (MAX_MXML_CONTACT_LEN > len)
			{
				snprintf(tree_node->element.contact, 
						 sizeof(tree_node->element.contact), 
						 "%s", tmp_buf);
			}
            else
			{
			    size = sizeof(int)+len+1;/*head+body+end*/
			    tree_node->element.extend_contact = j_xml_alloc(size);
                tree_node->element.extend_contact[size-1] = '\0';
                memcpy(tree_node->element.extend_contact, &len, sizeof(int));
                tree_node->element.extend_contact += sizeof(int);
				memcpy(tree_node->element.extend_contact, tmp_buf, len);
			}
		}
	}
	else
	{
		#if DEBUG_PARSE
		printf("%s%stext.string: \n", tmp, whitespace);
		#endif
	}

	return 0;
}

static int parse_xml_tree_node(XmlTreeNode *tree_node, mxml_node_t *mxml_node_child, 
										mxml_node_t *mxml_node_parent, int flags)
{
	mxml_node_t *mxml_node_son = NULL;
	XmlTreeNode *p_tree_node = NULL;
	XmlTreeNode *tree_node_child = NULL;
		
	if (NULL == tree_node)
	{
		printf("p_node NULL.\n");
		return -1;
	}	
	if (NULL == mxml_node_child)
	{
		printf("mxml_node_child NULL.\n");
		return -1;
	}
	if (NULL == mxml_node_parent)
	{
		printf("mxml_node_parent NULL.\n");
		return -1;
	}
	
	while (NULL != mxml_node_child)
	{
		if (MXML_ELEMENT == mxml_node_child->type)
		{
			p_tree_node = (XmlTreeNode *)j_xml_alloc(sizeof(XmlTreeNode));
			init_xml_tree_node(&p_tree_node);

			if (0 == get_mxml_node_info(p_tree_node, mxml_node_child, flags))
			{
				if (NULL == tree_node->child)
				{
					tree_node->child = p_tree_node;
					p_tree_node->parent = tree_node;
					
					tree_node_child = p_tree_node;
				}
				else
				{
					tree_node_child->next= p_tree_node;
					p_tree_node->prev = tree_node_child;
					
					tree_node_child = p_tree_node;
				}
			}
	
			//MXML_NO_DESCEND		不查看任何的子节点在XML元素层次中，仅查看同级的伙伴节点或者父节点
			//MXML_DESCEND_FIRST	向下搜索到一个节点的第一个匹配子节点，但不再继续向下搜索，一般使用于遍历一个父节点的直接的子节点
			//MXML_DESCEND			一直向下直到树的根部
			
			if (mxml_node_child->value.element.name == 
				strstr(mxml_node_child->value.element.name, DEF_MXML_IGNORE_STR))
			{
				#if DEBUG_PARSE
				printf("DEF_MXML_IGNORE_STR**********************************\n");
				#endif
			}
			else if (mxml_node_child->value.element.name == 
				strstr(mxml_node_child->value.element.name, DEF_MXML_CDATA_STR))
			{
				#if DEBUG_PARSE
				printf("DEF_MXML_CDATA_STR**********************************\n");
				#endif
			}
			else
			{
				if (NULL != mxml_node_child->child)
				{
					mxml_node_son = mxmlFindElement(mxml_node_child, mxml_node_child, 
													NULL, NULL, NULL, MXML_DESCEND_FIRST);
					if (NULL != mxml_node_son)
					{
						if (0 == strcmp(mxml_node_child->value.element.name, 
										mxml_node_son->parent->value.element.name))
						{
							parse_xml_tree_node(tree_node_child, mxml_node_son, 
												mxml_node_child, flags+1);
						}
					}
				}
			}
		}
		
		if (0 == strcmp(mxml_node_parent->value.element.name, 
						mxml_node_child->parent->value.element.name))
		{
			mxml_node_child = mxmlFindElement(mxml_node_child, mxml_node_parent, 
											  NULL, NULL, NULL, MXML_NO_DESCEND);
		}
	}

	return 0;
}

int xml_tree_parse_by_mxml(XmlTreeNode *xml_tree, const char *buffer)
{
	mxml_node_t *mxml_node_tree = NULL;
	mxml_node_t *mxml_node_root = NULL;
	mxml_node_t *mxml_node_child = NULL;
	
	XmlTreeNode *p_tree_node = NULL;
	XmlTreeNode *tree_node_child = NULL;
	
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}

	if (NULL == (mxml_node_tree = mxmlLoadString(NULL, buffer, MXML_TEXT_CALLBACK)))
	{
		printf("mxmlLoadString error.\n");
		return -1;
	}
	
	if (MXML_ELEMENT == mxml_node_tree->type)
	{
		#if DEBUG_PARSE
		printf("tree->value: %s\n", mxml_node_tree->value.element.name);
		#endif
		
		snprintf(xml_tree->element.name, sizeof(xml_tree->element.name), "%s", 
												mxml_node_tree->value.element.name);
	}
	
	mxml_node_root = mxmlFindElement(mxml_node_tree, mxml_node_tree, 
									 NULL, NULL, NULL, MXML_DESCEND_FIRST);	
	if (NULL != mxml_node_root)
	{
		if (MXML_ELEMENT == mxml_node_root->type)
		{
			p_tree_node = (XmlTreeNode *)j_xml_alloc(sizeof(XmlTreeNode));
			init_xml_tree_node(&p_tree_node);
			
			if (0 == get_mxml_node_info(p_tree_node, mxml_node_root, 1))
			{
				xml_tree->child = p_tree_node;
				p_tree_node->parent = xml_tree;
				
				tree_node_child = p_tree_node;
				
				mxml_node_child = mxmlFindElement(mxml_node_root, mxml_node_root, 
												  NULL, NULL, NULL, MXML_DESCEND_FIRST);
				if (NULL != mxml_node_child)
				{
					parse_xml_tree_node(tree_node_child, mxml_node_child, mxml_node_root, 2);
				}
			}
		}
	}
	else
	{
		mxmlDelete(mxml_node_tree);
		return -1;
	}

	mxmlDelete(mxml_node_tree);
	return 0;
}

static int merge_xml_tree_node(XmlTreeNode *tree_node, 
			mxml_node_t *mxml_node_tree, int flags)
{
	mxml_node_t *mxml_node = NULL;
	XmlTreeAttr *tree_attr = NULL;
	
#if DEBUG_MERGE
	int i = flags;
	char *tmp = "    "; 
	char whitespace[MAX_WHITE_SPACE_SIZE];
	memset(whitespace, 0, sizeof(whitespace));
	
	while (0 != i--)
	{
		strcat(whitespace, tmp);
	}
#endif
	
	if (NULL == tree_node)
	{
		printf("tree_node NULL.\n");
		return -1;
	}
	if (NULL == mxml_node_tree)
	{
		printf("mxml_node_tree NULL.\n");
		return -1;
	}
	
	while (NULL != tree_node)
	{
		if (strlen(tree_node->element.name))
		{
			#if DEBUG_MERGE
			printf("%selment name : %s\n", whitespace, tree_node->element.name);
			#endif
			
			if (NULL == (mxml_node = mxmlNewElement(mxml_node_tree, tree_node->element.name)))
			{
				printf("mxmlNewElement error.\n");
				return -1;
			}
			else
			{
				tree_attr = tree_node->element.attr;
				while (NULL != tree_attr)
				{
					#if DEBUG_MERGE
					printf("%s%sattr name : %s\n", tmp, whitespace, tree_attr->name);
					printf("%s%sattr value: %s\n", tmp, whitespace, tree_attr->value);
					#endif
					
					mxmlElementSetAttr(mxml_node, tree_attr->name, tree_attr->value);
					tree_attr = tree_attr->next;
				}
				
				if (NULL != tree_node->child)
				{
					merge_xml_tree_node(tree_node->child, mxml_node, flags+1);
				}
				else if (strlen(tree_node->element.contact) || tree_node->element.extend_contact)
				{
					#if DEBUG_MERGE
					printf("%s%selment value: %s\n", tmp, whitespace, tree_node->element.contact);
					#endif

                    if (!tree_node->element.extend_contact)
    					mxmlNewText(mxml_node, 0, tree_node->element.contact);
                    else
                        mxmlNewText(mxml_node, 0, tree_node->element.extend_contact);
				}
				else
				{
					if (tree_node->element.name == strstr(tree_node->element.name, DEF_MXML_IGNORE_STR))
					{
						#if DEBUG_MERGE
						printf("%s%sDEF_MXML_IGNORE_STR**********************\n", tmp, whitespace);
						#endif
					}
					else if (tree_node->element.name == strstr(tree_node->element.name, DEF_MXML_CDATA_STR))
					{
						#if DEBUG_MERGE
						printf("%s%sDEF_MXML_CDATA_STR**********************\n", tmp, whitespace);
						#endif
					}
					else
					{
						#if DEBUG_MERGE
						printf("%s%selment value: \n", tmp, whitespace);
						//printf("%s%selment value: [add whitespace here]\n", tmp, whitespace);
						//mxmlNewText(node, 0, " ");
						#endif
					}
				}
			}
		}
		else
		{
			printf("tree_node element name NULL.\n");
			return -1;
		}
		
		tree_node = tree_node->next;
	}
	
	return 0;
}

int xml_tree_merge_by_mxml(XmlTreeNode *xml_tree, char *buffer, size_t size)
{
	int xml_len;
	mxml_node_t *mxml_node_tree = NULL;
	XmlTreeNode *tree_node = NULL;
	
	if (NULL == xml_tree)
	{
		printf("xml_tree NULL.\n");
		return -1;
	}
	if (NULL == buffer)
	{
		printf("buffer NULL.\n");
		return -1;
	}
	if (0 >= size)
	{
		printf("size invalid.\n");
		return -1;
	}

	if (strlen(xml_tree->element.name))
	{
		#if DEBUG_MERGE
		printf("element name: %s\n", xml_tree->element.name);
		#endif
		if (NULL == (mxml_node_tree = mxmlNewElement(MXML_NO_PARENT, xml_tree->element.name)))
		{
			printf("mxmlNewElement error.\n");
			return -1;
		}
		else
		{
			tree_node = xml_tree->child;
			
			if (NULL != tree_node)
			{
				merge_xml_tree_node(tree_node, mxml_node_tree, 1);
				
				xml_len = mxmlSaveString(mxml_node_tree, buffer, size, MXML_TEXT_CALLBACK);
				if (size <= xml_len)
				{
					printf("mxmlSaveString error. size[%d] <= xml_len[%d]\n", size, xml_len);
					mxmlDelete(mxml_node_tree);
					return -1;
				}
	
				mxmlDelete(mxml_node_tree);
				//printf("xml_len: %d********************************\n", xml_len);
				return xml_len;
			}
			else
			{
				printf("tree_node NULL.\n");
				mxmlDelete(mxml_node_tree);
				return -1;
			}
		}
	}
	else
	{
		printf("xml_tree element name NULL.\n");
		return -1;
	}
}


