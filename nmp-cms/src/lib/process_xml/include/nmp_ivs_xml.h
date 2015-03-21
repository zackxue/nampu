/********************************************************************
 * nmp_pu_xml.h  - deal xml of pu, parse and create xml
 * Function£ºparse or create xml relate to pu.
 * Author:yangy
 * Description:users can add parse or create message of pu,define
 *             struct about pu information
 * History:
 * 2013.08.15 - Yang Ying, initiate to create;
 ********************************************************************/

#ifndef __NMP_IVS_XML__
#define __NMP_IVS_XML__

#include "nmp_xml_fun.h"

#define IVS_REGISTER_CMS				"IvsRegister"
#define IVS_REGISTER_CMS_RESP			"IvsRegisterResponse"
#define IVS_HEART						"IvsHeart"
#define IVS_HEART_RESP					"IvsHeartResponse"
#define SUBMIT_HERD_ALARM               "SubmitHerdAlarm"

NmpMsgInfo *
nmp_parse_ivs_register(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
nmp_create_ivs_register_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *
nmp_parse_ivs_heart(xmlDocPtr doc ,xmlNodePtr cur, char *cmd);

int
nmp_create_ivs_heart_resp(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *
nmp_parse_ivs_get_herd_plan_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
nmp_create_ivs_get_herd_plan(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *
nmp_parse_ivs_set_herd_plan_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
nmp_create_ivs_set_herd_plan(xmlDocPtr doc, NmpMsgInfo *sys_msg);

NmpMsgInfo *
nmp_parse_ivs_get_trough_history_state_resp(xmlDocPtr doc,xmlNodePtr cur, char *cmd);

int
nmp_create_ivs_get_trough_history_state(xmlDocPtr doc, NmpMsgInfo *sys_msg);

int
nmp_create_ivs_submit_alarm(xmlDocPtr doc, NmpMsgInfo *sys_msg);

#endif  //__NMP_IVS_XML__

