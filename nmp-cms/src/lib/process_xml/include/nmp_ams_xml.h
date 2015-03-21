/********************************************************************
 * nmp_ams_xml.h  - deal xml of ams, parse and create xml
 * Function£ºparse or create xml relate to pu.
 * Author:yangy
 * Description:users can add parse or create message of pu,define
 *             struct about pu information
 * History:
 * 2012.04.16 - Yang Ying, initiate to create;
 ********************************************************************/

#ifndef __NMP_AMS_XML__
#define __NMP_AMS_XML__

#include "nmp_xml_fun.h"

#define AMS_REGISTER_CMS            "AmsRegister"
#define AMS_REGISTER_CMS_RESP       "AmsRegisterResponse"
#define AMS_HEART                   "AmsHeart"
#define AMS_HEART_RESP              "AmsHeartResponse"
#define AMS_DEVICE_INFO_CHANGE		"AmsDeviceInfoChange"
#define AMS_GET_DEVICE_INFO			"AmsGetDeviceInfo"
#define AMS_GET_DEVICE_INFO_RESP	"AmsGetDeviceInfoResponse"


NmpMsgInfo *
nmp_parse_ams_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
nmp_create_ams_register_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *
nmp_parse_ams_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
nmp_create_ams_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

int
nmp_create_ams_device_info_change(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *
nmp_parse_ams_get_device_info(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
nmp_create_ams_get_device_info_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);


#endif  //__NMP_AMS_XML__
