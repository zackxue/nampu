/********************************************************************
 * nmp_pu_xml.h  - deal xml of pu, parse and create xml
 * Function£ºparse or create xml relate to pu. 
 * Author:yangy
 * Description:users can add parse or create message of pu,define 
 *             struct about pu information
 * History: 
 * 2011.06.01 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 * 2011.06.11 - Zhang Shiyong, optimize code
 ********************************************************************/
 
#ifndef __JPF_CMS_XML__
#define __JPF_CMS_XML__
	
#include "nmp_xml_fun.h"					

#define MAX_GU_ENTRIES             16

#define MDS_REGISTER_CMS                 "MdsRegister"
#define MDS_REGISTER_CMS_RESP       "MdsRegisterResponse"
#define MDS_HEART                               "MdsHeart"
#define MDS_HEART_RESP                     "MdsHeartResponse"
    
JpfMsgInfo *  
nmp_parse_mds_register_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int 
nmp_create_mds_register(xmlDocPtr doc, JpfMsgInfo *sys_msg);

JpfMsgInfo * 
nmp_parse_mds_heart_resp(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int 
nmp_create_mds_heart(xmlDocPtr doc, JpfMsgInfo *sys_msg);

#endif	/* __JPF_CMS_XML__ */

