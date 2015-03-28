/*
 *	@author:	zyt
 *	@time:	2013/3/16
 */
#ifndef __NMP_IPSAN_H__
#define __NMP_IPSAN_H__

#include "nmp_share_struct.h"

gint nmp_get_ipsan_info(NmpIpsanInfo *info);
gint nmp_set_ipsan_info(NmpIpsanInfo *info);
gint nmp_add_one_ipsan(NmpIpsanIp *info);
void nmp_del_one_ipsan(NmpIpsanIp *info);

#endif	/* __NMP_IPSAN_H__ */
