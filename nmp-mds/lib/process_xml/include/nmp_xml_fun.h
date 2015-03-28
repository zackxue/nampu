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
 * 1. nmp_get_message_type
 *    Parse an XML in-memory document and build a tree.
 *    
 * 2. nmp_get_node
 *    Parse an XML in-memory document and build a tree.
 *
 * 3. nmp_get_node_content: 
 *    Parse an XML in-memory document and get a node content.
 *
 * 4. nmp_get_node_attr_value
 *    Get value of an attribute
 *
 * 5. nmp_add_new_node
 *    Add a new node 
 *
 * 6. nmp_del_node
 *    Delete a node from XML text
 * 
 * 7. nmp_modify_node_cont
 *    Modify node content
 *
 * 8. nmp_modify_attr_value
 *    Modify attribute's value in XML document
 *
 * 9. nmp_parse_xml
 *    Process XML document tree
 *
 * 10. nmp_parse_xml_str
 *     Process XML text existing in memory buffer
 *
 * 11. nmp_parse_xml_file
 *     Open XML file and process it
 *
 * 12. nmp_create_xml
 *     Generate XML document tree according to command type and its
 *     parameter
 *
 * 13. nmp_create_xml_str
 *     Generate XML string according to command type and its parameter
 *
 * 14. nmp_create_xml_file
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
#include "nmp_xml.h"

#define  db_printf(fmt, arg...) \
                        do{\
							printf("FILE %s FUNC %s (%d)=>|"fmt,\
							__FILE__, __FUNCTION__, __LINE__ , ##arg);\
                        }while(0)

#define ASSERT                          assert
#define MAX_CMD_ENTRIES                 512

typedef struct _NmpXmlNodeInfo NmpXmlNodeInfo;
typedef struct _NmpXmlNode     NmpXmlNode;

struct _NmpXmlNodeInfo
{
	xmlChar     *node_name;
    xmlChar     *node_value;
    int          attr_flag;	
};

struct _NmpXmlNode
{
	xmlNodePtr  parent_node;
	xmlNodePtr  add_node;
};

/**
 * NmpParseXml: callback function, used to parse xml docuemnt
 *
 * @doc:            input, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 */
typedef NmpMsgInfo* (*NmpParseXml)(
	xmlDocPtr doc,
	xmlNodePtr cur, 
	char *cmd
);
                               
/**
 * NmpCreateXml: callback function, used to generate xml docuemnt
 *
 * @doc:            output, pointer, containing xml document
 * @sys_msg:        input, struct of the command information
 */
typedef int (*NmpCreateXml)(xmlDocPtr doc, NmpMsgInfo *sys_msg);
/**
 * NmpCmdType: define command set
 *
 * @cmd_enties:    array, contain command id and its processing-function
 * @counts:        indicate the number of command id containing in set
 */
 typedef char NmpCmdID[MAX_CMD_ID_LEN];
typedef struct _NmpCmdType NmpCmdType;
struct _NmpCmdType
{
	struct _NmpCmdTypeEntry {
		NmpCmdID         cmd_id;
		NmpParseXml      parse_xml;
		NmpCreateXml     create_xml;
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
 * nmp_get_message_type: parse an XML in-memory document and build a tree.
 *
 * @xml_buff:       input, pointer of xml text buffer
 * @xml_len:        input, xml text buffer length
 * @message_type:   output, contain message type ( command id )
 * @return:         succeed 0, else -1 
 */
int
nmp_get_message_type(
    const char *xml_buff,
    int        xml_len, 
    char    *message_type
);
                            
/**
 * nmp_get_node: parse an XML in-memory document and build a tree.
 *
 * @doc:    input, a pointer to document tree
 * @xpath:  input, addressing parts of an XML document
 * @return: the resulting document tree, else NULL
 */
xmlXPathObjectPtr
nmp_get_node(xmlDocPtr doc, const xmlChar *xpath);

/**
 * nmp_get_node_content: parse an XML in-memory document and get a
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
nmp_get_node_content(
    const char  *xml_buff,
    int         xml_len,
    xmlChar     *xpath, 
    int         *value_num,
    xmlChar     node_value[][MAX_CMD_ID_LEN]
);  

/**
 * nmp_get_node_attr_value: get value of an attribute
 *
 * @xml_buf:    input, pointer of xml text buffer
 * @xml_len:    input, xml text buffer length
 * @xpath:      input, addressing parts of an XML document
 * @attribute:  input, attribute
 * @attr_value: output, attribute value
 * @return:     succeed 0, else -1
 */
int
nmp_get_node_attr_value(
    const char  *xml_buff,
    int         xml_len,
    xmlChar     *xpath, 
    xmlChar     *attribute,
    xmlChar     *attr_value
);    

/**
 * nmp_add_new_node: add a new node 
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
nmp_add_new_node(
    const char  *xml_in,
    int         len_in,
    xmlChar     *xml_out, 
    int         *len_out,
    xmlChar     *xpath,
    xmlChar     *node_name,
    xmlChar     *node_cont
);  

/**
 * nmp_del_node: delete a node from XML text
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
nmp_del_node(
    const char  *xml_in,
    int         len_in,
    xmlChar     *xml_out,
    int         *len_out,
    xmlChar     *xpath
);

/**
 * nmp_modify_node_cont: modify node content
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
nmp_modify_node_cont(
    const char  *xml_in,
    int         len_in,
    xmlChar     *xml_out, 
    int         *len_out,
    xmlChar     *xpath,
    xmlChar     *node_cont
);

/**
 * nmp_modify_attr_value: modify attribute's value in XML document
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
int nmp_modify_attr_value(
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
nmp_create_xml_type(xmlDocPtr doc,
					xmlNodePtr root_node,
					char  *attribute_type, 
					char *message_type);

/**
 * nmp_deal_text: get a node's content
 *
 * @doc:            input, a pointer to document tree
 * @cur:            input, a pointer to the tree's node
 * @des:            output, a pointer to the node's content
 * @len:            input, len of des
 */
void nmp_deal_text(xmlDocPtr doc, xmlNodePtr cur, char *des, int len);

/**
 * nmp_deal_value: get a node's content
 *
 * @doc:            input, a pointer to document tree
 * @cur:            input, a pointer to the tree's current node
 * @des:            output, value of the node
 */
void nmp_deal_value(xmlDocPtr doc, xmlNodePtr cur, int *des);            


/**
 * nmp_parse_xml_str: process XML text existing in memory buffer
 *
 * @xml_buf:        iutput, memory address used to contain xml text
 * @xml_len:        iutput, indicate length of xmf_buf, unit bytes 
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo * 
nmp_parse_xml_str(NmpCmdType *self, const char *xml_buff, int xml_len, unsigned int seq);

/**
 * nmp_parse_xml_file: open XML file and process it
 *
 * @filename:       iutput, XML filename
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo * 
nmp_parse_xml_file(NmpCmdType *self, char *filename, unsigned int seq);

/**
 * nmp_create_xml_str: generate XML string according to sys_msg
 *
 * @xml_buf:        output, memory address used to contain xml text
 * @sys_msg:        input, struct, system message information
 * @return:         succeed:lens of xml, else -1
 */
int 
nmp_create_xml_str(NmpCmdType *self, 
                  char *xml_buff, 
                  int *buff_size, 
                  NmpMsgInfo *sys_msg);

/**
 * nmp_create_xml_file: generate XML string and save to a file
 *
 * @self:           input, platform's command set, which include 
 *                  all command
 * @filename:       input, filename which will be created
 * @sys_msg:        input, struct, system message information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_xml_file(NmpCmdType *self, const char *filename, NmpMsgInfo *sys_msg);

void nmp_init_cmd_id(NmpCmdType *self);
//for test,cms will offter
void *nmp_get_msginfo_data(NmpMsgInfo *sys_msg);


//for test,cms will offter
NmpMsgInfo *nmp_msginfo_new(const char *msg_id, void *data, int size);    
                   
#endif
