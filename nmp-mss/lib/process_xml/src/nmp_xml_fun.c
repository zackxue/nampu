/**
 * @file     nmp_xml_fun.c
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
 * 2012.04.14 - Yang Ying, initiate to create;
 */

#include <stdio.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>

#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_errno.h"
#include "nmp_xml.h"

#define NODE_VALUE_LEN  64
#define ECODE_UFT8      "UTF-8"
#define XML_VER         "1.0"

extern NmpCmdType nmp_cmd_sets;

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
    const char  *xml_buff,
    int         xml_len,
    char     *message_type
)
{
    xmlDocPtr doc; 
    xmlNodePtr cur;
    char *cmd;
    ASSERT(message_type != NULL);   
    doc = xmlParseMemory(xml_buff, xml_len);    //doc = xmlParseDoc(&xml_buff); 
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return -1;
    }
    
    cur = xmlDocGetRootElement(doc); //确定文档根元素   
    if (cur == NULL)
    {   
        xml_error("empty document\n");    
        xmlFreeDoc(doc); 
        return -1;   
    }
    
    while (cur != NULL)
    {
        if (!xmlStrcmp(cur->name, (const xmlChar *) "message")) 
        {
            cmd = (char *)xmlGetProp(cur, (const xmlChar *)"type");
            if ((message_type != NULL)&&(cmd != NULL))
                memcpy(message_type,cmd,strlen(cmd));
            xmlFree(cmd);
            xmlFreeDoc(doc);
            return 0;
        }
        cur = cur->next;
    }

    xmlFreeDoc(doc);

    return -1;
}


/**
 * nmp_get_node: parse an XML in-memory document and build a tree.
 *
 * @doc:    input, a pointer to document tree
 * @xpath:  input, addressing parts of an XML document
 * @return: the resulting document tree, else NULL
 */
xmlXPathObjectPtr 
nmp_get_node(xmlDocPtr doc, const xmlChar *xpath)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    
    context = xmlXPathNewContext(doc);
    if (context == NULL) 
    {
        xml_error("context is NULL\n");
        return NULL; 
    }
    
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    if (result == NULL)
    {
        xml_error("xmlXPathEvalExpression return NULL\n"); 
        return NULL; 
    }
    
    if (xmlXPathNodeSetIsEmpty(result->nodesetval)) 
    {
        xmlXPathFreeObject(result);
        xml_error("nodeset is empty\n");
        return NULL;
    }
    
    return result;
}


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
)
{
    ASSERT(value_num != NULL);
    xmlDocPtr doc; 
    xmlChar *value;
    xmlXPathObjectPtr app_result;
    int i;
    
    doc = xmlParseMemory(xml_buff, xml_len);  //doc = xmlParseDoc(&xml_buff);   
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return -1;
    }
    
    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
        return 0;
    
    if (app_result)
    {
        xmlNodeSetPtr nodeset = app_result->nodesetval;
        *value_num = nodeset->nodeNr;

        for (i=0; i < nodeset->nodeNr; i++)
        {
            value = xmlNodeGetContent(nodeset->nodeTab[i]);
            if (value == NULL)
            {
              xml_warning("value is NULL\n");
              continue;
            }   
            memcpy(node_value[i],value,NODE_VALUE_LEN);         
            xmlFree(value);
        }
        xmlXPathFreeObject(app_result);
    }
    
    xmlFreeDoc(doc);
    
    return 0;
}


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
)
{
    ASSERT((xml_buff != NULL) && (xpath!=NULL) && 
            (attribute != NULL) && (attr_value != NULL));
    xmlDocPtr doc; 
    char *cmd;
    xmlXPathObjectPtr app_result;
    int i;
    
    doc = xmlParseMemory(xml_buff, xml_len);    //doc = xmlParseDoc(&xml_buff); 
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return -1;
    }
    
    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
      goto end;
    
    if (app_result)
    {
        xmlNodeSetPtr nodeset = app_result->nodesetval;
        //*value_num = nodeset->nodeNr;
        xml_debug(" nodeset->nodeNr=%d\n", nodeset->nodeNr);
        for (i=0; i < nodeset->nodeNr; i++)
        {
            cmd = (char *)xmlGetProp(nodeset->nodeTab[i], (const xmlChar *)attribute);
            xml_debug("cmd type =%s\n",cmd);
            memcpy(attr_value,cmd,strlen(cmd));
            xml_debug("attr_value type =%s\n",attr_value);
            xmlFree(cmd);
        }
        xmlXPathFreeObject(app_result);
    }  
    
    end: 
    xmlFreeDoc(doc);
    
    return 0;
}


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
)
{
    ASSERT(xml_in != NULL && xml_out!= NULL && xpath != NULL);
    xmlDocPtr doc; 
    xmlNodePtr node;
    xmlXPathObjectPtr app_result;
    int node_num;;
    xmlChar *xml_buffer;
    
    doc = xmlParseMemory(xml_in, len_in);   //doc = xmlParseDoc(&xml_buff); 
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return -1;
    }
    
    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
      goto end;
    
    if (app_result)
    {
        xmlNodeSetPtr nodeset = app_result->nodesetval;
        node_num = nodeset->nodeNr - 1;
        node = xmlNewNode(NULL, BAD_CAST node_name);    
        xmlNodeSetContent(node, (const xmlChar *)node_cont);
        xmlAddNextSibling ( nodeset->nodeTab[node_num],node);
        xmlSaveFormatFileEnc("-", doc, ECODE_UFT8, 1);
        xmlDocDumpMemoryEnc(doc, &xml_buffer, len_out,ECODE_UFT8); 
        xml_debug("xml_buffer=%s\n", xml_buffer);        
        memcpy(xml_out,xml_buffer,*len_out);       
        xmlFree(xml_buffer);
        xmlXPathFreeObject(app_result);
    }
    
    end:
    xmlFreeDoc(doc);
    
    
    return 0;
}


int
nmp_add_xml_node(NmpXmlNode *xml_node, NmpXmlNodeInfo *node_info)
{
    if (node_info->attr_flag == 0)
        xml_node->add_node = xmlNewChild(
                                 xml_node->parent_node, 
                                 NULL, 
                                 BAD_CAST node_info->node_name,
                                 BAD_CAST node_info->node_value
                                 );
    else if(node_info->attr_flag == 1)
        xmlNewProp(
            xml_node->parent_node, 
            BAD_CAST node_info->node_name, 
            BAD_CAST node_info->node_value
            );
    else
        return -1;
        
    return 0;
}


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
)
{
    ASSERT(xml_out != NULL);
    xmlDocPtr doc; 
    xmlXPathObjectPtr app_result;
    int node_num;;
    xmlChar *xml_buffer;
    
    doc = xmlParseMemory(xml_in, len_in);   //doc = xmlParseDoc(&xml_buff); 
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return -1;
    }
    
    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
      goto end;
    
    if (app_result)
    {
        xmlNodeSetPtr nodeset = app_result->nodesetval;
        node_num = nodeset->nodeNr - 1;
        xmlUnlinkNode(nodeset->nodeTab[node_num]);
        xmlFreeNode(nodeset->nodeTab[node_num]);
        xmlSaveFormatFileEnc("-", doc, ECODE_UFT8, 1);
        xmlDocDumpMemoryEnc(doc, &xml_buffer, len_out,ECODE_UFT8); 
        memcpy(xml_out, xml_buffer, *len_out);
        xmlFree(xml_buffer);
        xmlXPathFreeObject(app_result);
    }
    
    end:
    xmlFreeDoc(doc);

    return 0;
}


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
)
{
    ASSERT(xml_out != NULL);
    xmlDocPtr doc; 
    xmlXPathObjectPtr app_result;
    int node_num;;
    xmlChar *xml_buffer;
    
    doc = xmlParseMemory(xml_in, len_in);   //doc = xmlParseDoc(&xml_buff); 
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return -1;
    }
    
    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
      goto end;
    
    if (app_result)
    {
        xmlNodeSetPtr nodeset = app_result->nodesetval;
        node_num = nodeset->nodeNr - 1;
        xmlNodeSetContent(nodeset->nodeTab[0], BAD_CAST node_cont);
        xmlSaveFormatFileEnc("-", doc, ECODE_UFT8, 1);
        xmlDocDumpMemoryEnc(doc, &xml_buffer, len_out,ECODE_UFT8); 
        memcpy(xml_out, xml_buffer, *len_out);
        xmlFree(xml_buffer);
        xmlXPathFreeObject(app_result);
    }
    
    end:
        xmlFreeDoc(doc);
   
    return 0;
}


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
int
nmp_modify_attr_value(
    const char  *xml_in,
    int         len_in,
    xmlChar     *xml_out,
    int         *len_out,
    xmlChar     *xpath,
    xmlChar     *attribute,
    xmlChar     *attr_value
)
{
    ASSERT(xml_out != NULL && attribute!=NULL && attr_value!= NULL);
    xmlDocPtr doc; 
    xmlXPathObjectPtr app_result;
    int node_num;;
    xmlChar *xml_buffer;
    
    doc = xmlParseMemory(xml_in, len_in);   //doc = xmlParseDoc(&xml_buff); 
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return -1;
    }
    
    app_result = nmp_get_node(doc, (const xmlChar *)xpath);
    if (app_result == NULL)
    {
        goto end;
    }
    
    if (app_result)
    {
        xmlNodeSetPtr nodeset = app_result->nodesetval;
        //*value_num = nodeset->nodeNr;
        xml_debug(" nodeset->nodeNr=%d\n", nodeset->nodeNr);
        node_num = nodeset->nodeNr - 1;
        xmlSetProp(nodeset->nodeTab[0], BAD_CAST attribute, BAD_CAST attr_value);
        xmlSaveFormatFileEnc("-", doc, ECODE_UFT8, 1);
        xmlDocDumpMemoryEnc(doc, &xml_buffer, len_out,ECODE_UFT8); 
        memcpy(xml_out, xml_buffer, *len_out);
        xmlFree(xml_buffer);
        xmlXPathFreeObject(app_result);
    }
   end: 
    xmlFreeDoc(doc);

    return 0;
}


/**
 * nmp_deal_cmd: process all command in platform's command set
 *
 * @self:   platform's command set, which include all command
 * @doc:    a pointer to document tree
 * @cur:    current node position in document tree
 * @cmd:    string, indicate command id
 * @param:  parameter used during command parsing
 * @return: succeed 0, else -1
 */
NmpMsgInfo *  
nmp_deal_cmd(NmpCmdType *self, 
             xmlDocPtr doc, 
             xmlNodePtr cur, 
             char *cmd, 
             uint seq
)
{
    NmpParseXml parse_xml;
    int i;
    
    for (i = 0; i < CMD_TYPE_COUNTS(self); i++)
    {
        // search in command entry array
        if (!CMD_CMP(cmd,GET_CMD_BYINDEX(self,i)))
            continue;
            
        parse_xml = GET_PARSE_XML_BYINDEX(self,i);
        return (*parse_xml)(doc, cur, cmd); // invoke function to parse

    }

    return NULL;    
}


/**
 * nmp_parse_xml: process XML document tree
 *
 * @doc:            iutput, XML document tree
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo *  
nmp_parse_xml(NmpCmdType *self, xmlDocPtr doc, unsigned int seq)
{
    xmlNodePtr    cur;
    char       *cmd;
    char  msg_id[MAX_CMD_ID_LEN]={0};
    NmpParseXml   parse_xml;
    NmpMsgInfo     *sys_msg;
    int i;
    
    cur = xmlDocGetRootElement(doc); //确定文档根元素   
    if (cur == NULL)
    {   
        xml_error("empty document\n");       
        return NULL;   
    }
    
    while ( cur && xmlIsBlankNode ( cur ) ) 
    {
        cur = cur->next;
    }
    
    while (cur != NULL)
    {
        if (!xmlStrcmp(cur->name, (const xmlChar *) "message")) 
        {
            cmd = (char *)xmlGetProp(cur, (const xmlChar *)"type");
            xml_error("1---parse cmd =%s\n",cmd);
            if (!cmd)
            {
                return NULL;    
            }
            msg_id[MAX_CMD_ID_LEN - 1] = 0;
            strncpy(msg_id, cmd, MAX_CMD_ID_LEN - 1);
			  xmlFree(cmd);		
            for (i = 0; i < CMD_TYPE_COUNTS(self); i++)
            {     
                if (!CMD_CMP(msg_id,GET_CMD_BYINDEX(self,i)))
                    continue;
                    
                parse_xml = GET_PARSE_XML_BYINDEX(self,i);
                sys_msg = (*parse_xml)(doc, cur, msg_id); // invoke function to parse            
                return sys_msg;
            }       
        }
        cur = cur->next;
        while ( cur && xmlIsBlankNode ( cur ) ) 
        {
            cur = cur->next;
        }
    }
    
    xml_error(" message not exists  \n");   
    return NULL;
}


/**
 * nmp_parse_xml_str: process XML text existing in memory buffer
 *
 * @xml_buf:        iutput, memory address used to contain xml text
 * @xml_len:        iutput, indicate length of xmf_buf, unit bytes 
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo * 
nmp_parse_xml_str(NmpCmdType *self, const char *xml_buff, int xml_len,
    unsigned int seq)
{
    xmlDocPtr doc; 
    NmpMsgInfo *sys_msg; 
   // xml_error("--------------=======xml_buff=%s\n",xml_buff);   
    doc = xmlParseMemory(xml_buff, xml_len);
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return NULL;
    }
    
    sys_msg = nmp_parse_xml(self, doc, seq);
    if(!sys_msg)
    {
       xml_error("parse xml fail!\n");  
    }
  
    xmlFreeDoc(doc);

    return sys_msg;
}


NmpMsgInfo * 
nmp_parse_str_xml(const char *xml_buff, int xml_len, unsigned int seq)
{
    ASSERT(xml_buff != NULL);
    
    return nmp_parse_xml_str(&nmp_cmd_sets, xml_buff, xml_len, seq);    
}


/**
 * nmp_parse_xml_file: open XML file and process it
 *
 * @filename:       iutput, XML filename
 * @seq:            input, sequence of message
 * @return:         succeed NmpMsgInfo, else NULL
 */
NmpMsgInfo * 
nmp_parse_xml_file(NmpCmdType *self, char *filename, unsigned int seq)
{
    xmlDocPtr doc; 
    NmpMsgInfo *sys_msg; 
    
    doc = xmlReadFile(filename,"UTF-8",XML_PARSE_NOBLANKS);
    //doc = xmlParseFile(filename);
    if (doc == NULL ) 
    {    
        xml_error("Document not parsed successfully. \n");  
        return NULL;
    }
    
    sys_msg = nmp_parse_xml(self, doc, seq);
    if(!sys_msg)
    {
        xml_error("parse xml fail!\n"); 
    }
    
    xmlFreeDoc(doc);

    return sys_msg;
}


/**
 * nmp_create_xml: generate XML docment tree according to command type and
 *                 its parameter
 *
 * @doc:            output, a pointer to document tree
 * @sys_msg:        input, struct, system message information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_xml(NmpCmdType *self, xmlDocPtr doc, NmpMsgInfo *sys_msg)
{
    NmpCreateXml create_xml;
    int i,ret;

    for (i = 0; i < CMD_TYPE_COUNTS(self); i++)
    {
        if (!CMD_CMP(sys_msg->msg_id,GET_CMD_BYINDEX(self,i)))
            continue;
        create_xml = GET_CREATE_XML_BYINDEX(self,i);
        if(create_xml)
        {
            ret = (*create_xml)(doc, sys_msg);
            if(ret < 0)
            {
                xml_error("create xml error, no:%d \n", ret);
                return ret;     
            }
        }
        else
            return -E_NOXMLFUN;
        return 0;
    }
    return -E_NOXMLFUN;
}


/**
 * nmp_create_xml_str: generate XML string according to sys_msg
 *
 * @xml_buf:        output, memory address used to contain xml text
 * @sys_msg:        input, struct, system message information
 * @return:         succeed:lens of xml, else -1
 */
int 
nmp_create_xml_str(NmpCmdType *self, char *xml_buff,
        int *buff_size, NmpMsgInfo *sys_msg)
{
    ASSERT(self != NULL && xml_buff != NULL && buff_size != NULL &&
		sys_msg != NULL);
    xmlDocPtr doc = NULL; 
    xmlChar *xml_buffer = NULL;
    int size;

    doc = xmlNewDoc(BAD_CAST XML_VER);
    if(!doc)
        return -E_CREATEDOC;
        
    if ((size = nmp_create_xml(self, doc, sys_msg)))
    {
       goto end_create_xml;
    }

    xmlSaveFormatFileEnc( "-", doc, "UTF-8", 1);
    xmlDocDumpMemoryEnc(doc, &xml_buffer, &size, ECODE_UFT8);   

    if(size > *buff_size)
    {
	    *buff_size = size;
        size = -E_XML2LONG;

        if(xml_buffer)
            xmlFree(xml_buffer);
        goto end_create_xml;
    }       
    memcpy(xml_buff, (char*)xml_buffer, size);
    if(xml_buffer)
        xmlFree(xml_buffer);  
        
    end_create_xml:      
        xmlFreeDoc(doc);    

    return size;
}


int 
nmp_create_str_xml(char *xml_buff, int *buff_size, NmpMsgInfo *sys_msg)
{
    ASSERT(xml_buff != NULL &&  buff_size != NULL && sys_msg != NULL);
    return nmp_create_xml_str(&nmp_cmd_sets, xml_buff, buff_size, sys_msg);
}

/**
 * nmp_create_xml_file: generate XML string and save to a file
 *
 * @filename:       input, filename which will be created
 * @sys_msg:        input, struct, system message information
 * @return:         succeed 0, else -1
 */
int 
nmp_create_xml_file(NmpCmdType *self, const char *filename, NmpMsgInfo *sys_msg)
{
    ASSERT(filename != NULL && sys_msg != NULL);
    xmlDocPtr doc = NULL; 
    int ret;
    
    doc = xmlNewDoc(BAD_CAST XML_VER);
    if (!doc)
        return -E_CREATEDOC;
        
    if ((ret = nmp_create_xml(self, doc, sys_msg))) 
    {
        xmlFreeDoc(doc);    
        return ret;
    }
    
    xmlSaveFormatFileEnc(filename, doc, ECODE_UFT8, 1);
    xmlFreeDoc(doc);    
    
    return 0;
}


/**
 * create_xml_type: generate XML heard
 *
 * @doc:            output, a pointer to document tree
 * @message_type:   input, string, indicate command id
 * @root_node:      input, a pointer to the tree's root
 * @return:         succeed root node, else NULL
 */
xmlNodePtr
nmp_create_xml_type(xmlDocPtr doc,
                    xmlNodePtr root_node,
                    char  *attribute_type, 
                    char *message_type)
{
    xmlNodePtr node;
    xmlAttrPtr newattr;
    ASSERT(doc != NULL && root_node != NULL\
        && attribute_type!=NULL && message_type!=NULL);

    newattr = xmlNewProp(root_node, BAD_CAST attribute_type, BAD_CAST message_type);
    node = xmlDocSetRootElement(doc, root_node);
    
    return node;
}


/**
 * nmp_deal_text: get a node's content
 *
 * @doc:            input, a pointer to document tree
 * @cur:            input, a pointer to the tree's node
 * @des:            output, a pointer to the node's content
 * @len:            input, len of des
 */
void nmp_deal_text(xmlDocPtr doc, xmlNodePtr cur, char *des, int len)
{
    xmlChar *value = NULL;

    value = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);   
    if (NULL != value)  
    {         
        des[len - 1] = 0;
        strncpy(des, (char*)value, len - 1);
        xmlFree(value);
        value = NULL;
    }
    else
    {
        strcpy(des, "");
        xml_warning("the node %s value is NULL", cur->name);        
    }
}


/**
 * nmp_deal_value: get a node's content
 *
 * @doc:            input, a pointer to document tree
 * @cur:            input, a pointer to the tree's current node
 * @des:            output, value of the node
 */
void nmp_deal_value(xmlDocPtr doc, xmlNodePtr cur, int *des)
{
    xmlChar *value = NULL;

    value = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);   
    if (NULL != value)  
    {
        *des = atoi((char *)value);     
        xmlFree(value);
    }
    else
    {
        *des = -1;
        xml_warning("the node %s value is NULL", cur->name);        
    }
}


void *
nmp_get_msginfo_data(NmpMsgInfo *sys_msg)
{
    return sys_msg->private_data;
}


static void nmp_private_free(void* priv, int size)
{
	free(priv);
}


NmpMsgInfo*
nmp_msginfo_new(const char *msg_id, void *data, int size)
{
    NmpMsgInfo *sys_msg = NULL;
    ASSERT(msg_id != NULL && size >= 0);
       
    sys_msg = malloc(sizeof(NmpMsgInfo));
    if (!sys_msg)
        return NULL;

    if(data)
    {
        sys_msg->private_data = malloc(size);
        if (!sys_msg->private_data)
            return NULL;
    }
	
    sys_msg->priv_destroy= nmp_private_free;
    sys_msg->private_size = size;
    sys_msg->msg_id[MAX_CMD_ID_LEN - 1] = 0;
    strncpy(sys_msg->msg_id, msg_id, MAX_CMD_ID_LEN - 1);
    
    if (data)
        memcpy(sys_msg->private_data, data, size);
    else
        sys_msg->private_data = NULL;
    
    return sys_msg;
}

NmpMsgInfo*
nmp_msginfo_new_2(const char *msg_id, void *data, int size, NmpMsgInfoPrivDes des)
{
    NmpMsgInfo *sys_msg = NULL;
    ASSERT(msg_id != NULL);
       
    sys_msg = malloc(sizeof(NmpMsgInfo));
    if (!sys_msg)
        return NULL;
		
    sys_msg->priv_destroy = des;
    sys_msg->private_data = data;
    sys_msg->private_size = size;
    sys_msg->msg_id[MAX_CMD_ID_LEN - 1] = 0;
    strncpy(sys_msg->msg_id, msg_id, MAX_CMD_ID_LEN - 1);
        
    return sys_msg;
}

void
nmp_free_msginfo(NmpMsgInfo *sys_msg)
{
    ASSERT(sys_msg != NULL);

    if(sys_msg->private_data)
    {
       (* sys_msg->priv_destroy)(sys_msg->private_data, sys_msg->private_size);
    }

    free(sys_msg);
}

void
nmp_free_msginfo_head(NmpMsgInfo *sys_msg)
{
	ASSERT(sys_msg != NULL);
	free(sys_msg);
}