
#ifndef __NMP_XML__
#define __NMP_XML__

/*
#include "nmp_pu_xml.h"
#include "nmp_cu_xml.h"
#include "nmp_bss_xml.h"
#include "nmp_xml_fun.h"
*/
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
//#include "nmp_xml_fun.h"
#define MAX_CMD_ID_LEN                  64
//typedef char NmpCmdID[MAX_CMD_ID_LEN];
//typedef char NmpMsgID[MAX_CMD_ID_LEN];

/**/
typedef void (*NmpMsgInfoPrivDes)(void* priv, int size);

typedef struct _NmpMsgInfo NmpMsgInfo;
struct _NmpMsgInfo
{
   	char       		msg_id[MAX_CMD_ID_LEN];
    int			     private_size;
    void 			 *private_data;
    NmpMsgInfoPrivDes       priv_destroy;
};

NmpMsgInfo *
nmp_parse_str_xml(const char *xml_buff, int xml_len, unsigned int seq);

int
nmp_create_str_xml(char *xml_buff, int *buff_len, NmpMsgInfo *sys_msg);

void
nmp_init_xml_cmd();
void
nmp_free_msginfo(NmpMsgInfo *sys_msg);

void
nmp_free_msginfo_head(NmpMsgInfo *sys_msg);

void
nmp_xml_parse_time_policy(char *time_policy, void *res_info);
#endif
