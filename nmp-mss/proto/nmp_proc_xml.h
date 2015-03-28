#ifndef __NMP_PROCXML_H__
#define __NMP_PROCXML_H__

#include "nmp_sysmsg.h"

NmpSysMsg * 
nmp_get_sysmsg_from_xml(char *xml_buff, gint xml_len, guint seq);

gint nmp_proto_create_xml_str(char *xml_buff,
          gint *buff_size, NmpSysMsg *sys_msg);
#endif
