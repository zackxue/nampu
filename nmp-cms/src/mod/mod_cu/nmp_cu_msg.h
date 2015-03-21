#ifndef __NMP_CU_MSG_H__
#define __NMP_CU_MSG_H__
#include "nmp_internal_msg.h"

gint nmp_mod_cu_get_user_info(NmpModCu *self, NmpUsr *user);
gint nmp_mod_cu_get_group_info(NmpModCu *self, NmpUsrGroup *grp);

gint
nmp_mod_cu_get_user_login_info(NmpModCu *self, NmpMsgUserLoginInfoRes *res);

#endif	//__NMP_CU_MSG_H__
