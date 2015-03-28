
/**
 * @file     nmp_deal_xml.c
 * @author   Yang Ying
 * @section  LICENSE
 *
 * Copyright by Shenzhen JXJ Electronic Co.Ltd, 2011.
 * Website: www.szjxj.com
 *
 * @section  DESCRIPTION
 *
 * 1. nmp_cmd_type_register
 *    Every new message must be registered before using
 *    it. this function can finish it.
 *    
 * 2. nmp_init_cmd_id
 *    When adding a new message id, user only need to expand
 *    this function look piece "to do ...." in it.
 *
 * 3. nmp_deal_cmd: 
 *    Used to find a specific command in XML document, if 
 *    existed in XML, then call its processing function.
 *
 * history 
 * 2011.06.03 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 * 2011.06.09 - Zhang Shiyong, add file description and code comments;
 */
#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>

#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_cms_xml.h"

#define CMD_ID_LEN              32
#define NODE_VALUE_LEN          64

NmpCmdType nmp_cmd_sets;

/**
 * nmp_cmd_type_register: Register a new command type in platform
 *
 * @self:   current command set of platform
 * @id:     new command id
 * @p_xml:  callback function to process XML
 * @c_xml:  callback function to generate XML for the new command type
 * @return: succeed 0, else -1
 */
int
nmp_cmd_type_register(
	NmpCmdType      *self,
    NmpCmdID        id, 
	NmpParseXml     p_xml, 
    NmpCreateXml    c_xml
)
{
	if (self->counts >= MAX_CMD_ENTRIES)
	{
		xml_error("command number over max number\n");
		return -1;  //return ENOMEM ;
	}

	CMD_SET(self->cmd_enties[self->counts].cmd_id, id);
	GET_PARSE_XML_BYINDEX(self,self->counts)    = p_xml;
	GET_CREATE_XML_BYINDEX(self,self->counts)   = c_xml;
	
	++self->counts;

	return 0;
}

/**
 * nmp_init_cmd_id: initiate command set of platform
 * 
 * When platform startup, firstly, invoke this function to initiate
 * all commands in command set
 *
 * @self: platform's command set
 * @return none
 */
void
nmp_init_cmd_id(NmpCmdType *self)
{
	//memset(self, 0 ,sizeof(NmpCmdType));

	 nmp_cmd_type_register(self,
        MDS_REGISTER_CMS, 
        NULL, 
        nmp_create_mds_register);
			
    nmp_cmd_type_register(self,
        MDS_REGISTER_CMS_RESP, 
        nmp_parse_mds_register_resp, 
        NULL);

    nmp_cmd_type_register(self,
        MDS_HEART, 
        NULL, 
        nmp_create_mds_heart);	
		
    nmp_cmd_type_register(self,
        MDS_HEART_RESP, 
        nmp_parse_mds_heart_resp, 
        NULL);		
  	printf("init enter nmp_create_xml cmd num=%d\n",CMD_TYPE_COUNTS(self));
    // to do ..., add new command id here
}


void
nmp_init_xml_cmd()
{
	nmp_init_cmd_id(&nmp_cmd_sets);
	
}

