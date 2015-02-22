#ifndef __NMP_CU_MSG_H__
#define __NMP_CU_MSG_H__
#include "nmp_internal_msg.h"

gint jpf_mod_cu_get_user_info(JpfModCu *self, JpfUsr *user);
gint jpf_mod_cu_get_group_info(JpfModCu *self, JpfUsrGroup *grp);

gint
jpf_mod_cu_get_user_login_info(JpfModCu *self, JpfMsgUserLoginInfoRes *res);

#endif	//__NMP_CU_MSG_H__
