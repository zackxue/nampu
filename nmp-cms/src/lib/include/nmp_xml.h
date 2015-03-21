
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
//typedef char JpfCmdID[MAX_CMD_ID_LEN];
//typedef char NmpMsgID[MAX_CMD_ID_LEN];

/**/
typedef void (*JpfMsgInfoPrivDes)(void* priv, int size);

typedef struct _JpfMsgInfo JpfMsgInfo;
struct _JpfMsgInfo
{
   	char       		msg_id[MAX_CMD_ID_LEN];
    int			     private_size;
    void 			 *private_data;
    JpfMsgInfoPrivDes       priv_destroy;
};

JpfMsgInfo *
jpf_parse_str_xml(const char *xml_buff, int xml_len, unsigned int seq);

int
jpf_create_str_xml(char *xml_buff, int *buff_len, JpfMsgInfo *sys_msg);

void
nmp_init_xml_cmd();
void
jpf_free_msginfo(JpfMsgInfo *sys_msg);

void
jpf_free_msginfo_head(JpfMsgInfo *sys_msg);

void
jpf_xml_parse_time_policy(char *time_policy, void *res_info);
#endif
