/**
 * @file     nmp_xml_fun.h
 * @author   Yang Ying
 * @section  LICENSE
 *
 * Copyright by Shenzhen JXJ Electronic Co.Ltd, 2011.
 * Website: www.szjxj.com
 *
 * @section  DESCRIPTION
 *
 * 1. jpf_get_message_type
 *    Parse an XML in-memory document and build a tree.
 *
 * 2. jpf_get_node
 *    Parse an XML in-memory document and build a tree.
 *
 * 3. jpf_get_node_content:
 *    Parse an XML in-memory document and get a node content.
 *
 * 4. jpf_get_node_attr_value
 *    Get value of an attribute
 *
 * 5. jpf_add_new_node
 *    Add a new node
 *
 * 6. jpf_del_node
 *    Delete a node from XML text
 *
 * 7. jpf_modify_node_cont
 *    Modify node content
 *
 * 8. jpf_modify_attr_value
 *    Modify attribute's value in XML document
 *
 * 9. jpf_parse_xml
 *    Process XML document tree
 *
 * 10. jpf_parse_xml_str
 *     Process XML text existing in memory buffer
 *
 * 11. jpf_parse_xml_file
 *     Open XML file and process it
 *
 * 12. jpf_create_xml
 *     Generate XML document tree according to command type and its
 *     parameter
 *
 * 13. jpf_create_xml_str
 *     Generate XML string according to command type and its parameter
 *
 * 14. jpf_create_xml_file
 *     Generate XML string and save to a file
 *
 * history
 * 2011.06.03 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang Ying modify
 * 2011.06.09 - Zhang Shiyong, add file description and code comments;
 */

#ifndef __XML_FUN_H__
#define __XML_FUN_H__

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <string.h>
#include "nmp_msg_share.h"
#include "nmp_xml.h"

#define  db_printf(fmt, arg...) \
                        do{\
							printf("FILE %s FUNC %s (%d)=>|"fmt,\
							__FILE__, __FUNCTION__, __LINE__ , ##arg);\
                        }while(0)

#define ASSERT                          assert
//#define MAX_CMD_ID_LEN                  64
#define MAX_CMD_ENTRIES                 1024
/*
typedef void (*JpfMsgInfoPrivDes)(void* priv, int size);
typedef struct _JpfMsgInfo JpfMsgInfo;
struct _JpfMsgInfo
{
	char       		msg_id[MAX_CMD_ID_LEN];
	int			    private_size;
	void 			*private_data;
	    JpfMsgInfoPrivDes       priv_destroy;
};

*/


typedef struct _JpfXmlNodeInfo JpfXmlNodeInfo;
typedef struct _JpfXmlNode     JpfXmlNode;

struct _JpfXmlNodeInfo
{
	xmlChar     *node_name;
    xmlChar     *node_value;
    int          attr_flag;
};

struct _JpfXmlNode
{
	xmlNodePtr  parent_node;
	xmlNodePtr  add_node;
};

/**
 * JpfParseXml: callback function, used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 */
typedef JpfMsgInfo* (*JpfParseXml)(
	xmlDocPtr doc,
	xmlNodePtr cur,
	char *cmd
);

/**
 * JpfCreateXml: callback function, used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 */
typedef int (*JpfCreateXml)(xmlDocPtr doc, JpfMsgInfo *sys_msg);
/**
 * JpfCmdType: define command set
 *
 * @cmd_enties:    array, contain command id and its processing-function
 * @counts:        indicate the number of command id containing in set
 */
 typedef char JpfCmdID[MAX_CMD_ID_LEN];
typedef struct _JpfCmdType JpfCmdType;
struct _JpfCmdType
{
	struct _JpfCmdTypeEntry {
		JpfCmdID         cmd_id;
		JpfParseXml      parse_xml;
		JpfCreateXml     create_xml;
	}cmd_enties[MAX_CMD_ENTRIES];
	int counts;
};


#define CMD_TYPE_COUNTS(t)              ((t)->counts)
#define GET_CMD_BYINDEX(t, i)           ((t)->cmd_enties[i].cmd_id)
#define GET_PARSE_XML_BYINDEX(t, i)     ((t)->cmd_enties[i].parse_xml)
#define GET_CREATE_XML_BYINDEX(t, i)    ((t)->cmd_enties[i].create_xml)

#define CMD_CMP(cmdtype, cmdid)         (!strcmp(cmdtype, cmdid))
#define CMD_SET(cmdtype, cmdid) \
do { \
	strncpy(cmdtype, cmdid, MAX_CMD_ID_LEN - 1); \
	cmdtype[MAX_CMD_ID_LEN - 1] = 0; \
}while (0)

/**
 * jpf_get_message_type: parse an XML in-memory document and build a tree.
 *
 * @xml_buff:       input, pointer of xml text buffer
 * @xml_len:        input, xml text buffer length
 * @message_type:   output, contain message type ( command id )
 * @return:         succeed 0, else -1
 */
int
jpf_get_message_type(
    const char *xml_buff,
    int        xml_len,
    char    *message_type
);

/**
 * jpf_get_node: parse an XML in-memory document and build a tree.
 *
 * @doc:    input, a pointer to document tree
 * @xpath:  input, addressing parts of an XML document
 * @return: the resulting document tree, else NULL
 */
xmlXPathObjectPtr
jpf_get_node(xmlDocPtr doc, const xmlChar *xpath);

/**
 * jpf_get_node_content: parse an XML in-memory document and get a
 *                       node content.
 *
 * @xml_buf:    input, pointer of xml text buffer
 * @xml_len:    input, xml text buffer length
 * @xpath:      input, addressing parts of an XML document
 * @value_num   output, indicate node number in array node_value
 * @node_value  output,
 * @return:     succeed 0, else -1
 */
int
jpf_get_node_content(
    const char  *xml_buff,
    int         xml_len,
    xmlChar     *xpath,
    int         *value_num,
    xmlChar     node_value[][MAX_CMD_ID_LEN]
);

/**
 * jpf_get_node_attr_value: get value of an attribute
 *
 * @xml_buf:    input, pointer of xml text buffer
 * @xml_len:    input, xml text buffer length
 * @xpath:      input, addressing parts of an XML document
 * @attribute:  input, attribute
 * @attr_value: output, attribute value
 * @return:     succeed 0, else -1
 */
int
jpf_get_node_attr_value(
    const char  *xml_buff,
    int         xml_len,
    xmlChar     *xpath,
    xmlChar     *attribute,
    xmlChar     *attr_value
);

/**
 * jpf_add_new_node: add a new node
 *
 * @xml_in:     input, pointer of xml text buffer
 * @len_in:     iutput, xml text buffer length
 * @xml_out:    output, pointer of buffer block, containing xml text
 *              after deleting a node
 * @len_out:    output, indicate buffer length of xml_out
 * @xpath:      addressing parts of an XML document
 * @node_name:  node name
 * @node_cont:  node content
 * @return:     succeed 0, else -1
 */
int
jpf_add_new_node(
    const char  *xml_in,
    int         len_in,
    xmlChar     *xml_out,
    int         *len_out,
    xmlChar     *xpath,
    xmlChar     *node_name,
    xmlChar     *node_cont
);

/**
 * jpf_del_node: delete a node from XML text
 *
 * @xml_in:     input, pointer of xml text buffer
 * @len_in:     iutput, xml text buffer length
 * @xml_out:    output, pointer of buffer block, containing xml text
 *              after deleting a node
 * @len_out:    output, indicate buffer length of xml_out
 * @xpath:      addressing parts of an XML document
 * @return:     succeed 0, else -1
 */
int
jpf_del_node(
    const char  *xml_in,
    int         len_in,
    xmlChar     *xml_out,
    int         *len_out,
    xmlChar     *xpath
);

/**
 * jpf_modify_node_cont: modify node content
 *
 * @xml_in:     input, pointer of xml text buffer
 * @len_in:     iutput, xml text buffer length
 * @xml_out:    output, pointer of buffer block, containing xml text
 *              after modifying attribute value
 * @len_out:    output, indicate buffer length of xml_out
 * @xpath:      addressing parts of an XML document
 * @node_cont   input, node content that will be updated
 * @return:     succeed 0, else -1
 */
int
jpf_modify_node_cont(
    const char  *xml_in,
    int         len_in,
    xmlChar     *xml_out,
    int         *len_out,
    xmlChar     *xpath,
    xmlChar     *node_cont
);

/**
 * jpf_modify_attr_value: modify attribute's value in XML document
 *
 * @xml_in:     input, pointer of xml text buffer
 * @len_in:     iutput, xml text buffer length
 * @xml_out:    output, pointer of buffer block, containing xml text
 *              after modifying attribute value
 * @len_out:    output, indicate buffer length of xml_out
 * @xpath:      addressing parts of an XML document
 * @attribute:  XML element attribute
 * @attr_value: value of a specific attribute in XML document
 * @return:     succeed 0, else -1
 */
int jpf_modify_attr_value(
    const char  *xml_in,
    int         len_in,
    xmlChar     *xml_out,
    int         *len_out,
    xmlChar     *xpath,
    xmlChar     *attribute,
    xmlChar     *attr_value
);



/**
 * create_xml_type: generate XML heard
 *
 * @doc:            output, a pointer to document tree
 * @message_type:   input, string, indicate command id
 * @root_node:      input, a pointer to the tree's root
 * @return:         succeed xmlNodePtr, else NULL
 */
xmlNodePtr
jpf_create_xml_type(xmlDocPtr doc,
					xmlNodePtr root_node,
					char  *attribute_type,
					char *message_type);

/**
 * jpf_deal_text: get a node's content
 *
 * @doc:            input, a pointer to document tree
 * @cur:            input, a pointer to the tree's node
 * @des:            output, a pointer to the node's content
 * @len:            input, len of des
 */
void jpf_deal_text(xmlDocPtr doc, xmlNodePtr cur, char *des, int len);

/**
 * jpf_deal_value: get a node's content
 *
 * @doc:            input, a pointer to document tree
 * @cur:            input, a pointer to the tree's current node
 * @des:            output, value of the node
 */
void jpf_deal_value(xmlDocPtr doc, xmlNodePtr cur, gint *des);

void jpf_deal_float_value(xmlDocPtr doc, xmlNodePtr cur, double *des);
/**
 * jpf_parse_xml_str: process XML text existing in memory buffer
 *
 * @xml_buf:        iutput, memory address used to contain xml text
 * @xml_len:        iutput, indicate length of xmf_buf, unit bytes
 * @seq:            input, sequence of message
 * @return:         succeed JpfMsgInfo, else NULL
 */
JpfMsgInfo *
jpf_parse_xml_str(JpfCmdType *self, const char *xml_buff, int xml_len, guint seq);

/**
 * jpf_parse_xml_file: open XML file and process it
 *
 * @filename:       iutput, XML filename
 * @seq:            input, sequence of message
 * @return:         succeed JpfMsgInfo, else NULL
 */
JpfMsgInfo *
jpf_parse_xml_file(JpfCmdType *self, char *filename, guint seq);

/**
 * jpf_create_xml_str: generate XML string according to sys_msg
 *
 * @xml_buf:        output, memory address used to contain xml text
 * @sys_msg:        input, struct, system message information
 * @return:         succeed:lens of xml, else -1
 */
int
jpf_create_xml_str(JpfCmdType *self,
                  char *xml_buff,
                  int *buff_size,
                  JpfMsgInfo *sys_msg);

/**
 * jpf_create_xml_file: generate XML string and save to a file
 *
 * @self:           input, platform's command set, which include
 *                  all command
 * @filename:       input, filename which will be created
 * @sys_msg:        input, struct, system message information
 * @return:         succeed 0, else -1
 */
int
jpf_create_xml_file(JpfCmdType *self, const char *filename, JpfMsgInfo *sys_msg);

void jpf_init_cmd_id(JpfCmdType *self);
//for test,cms will offter
void *jpf_get_msginfo_data(JpfMsgInfo *sys_msg);


//for test,cms will offter
JpfMsgInfo *jpf_msginfo_new(const char *msg_id, void *data, int size);
JpfMsgInfo* jpf_msginfo_new_2(const char *msg_id, void *data, int size,
		JpfMsgInfoPrivDes des);
void jpf_free_msginfo_head(JpfMsgInfo *sys_msg);

#endif
