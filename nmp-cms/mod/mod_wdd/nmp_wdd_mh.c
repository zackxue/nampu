#include "nmp_mod_wdd.h"
#include "nmp_message.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_internal_msg.h"



JpfMsgFunRet
jpf_mod_wdd_get_license_info_b(JpfAppObj *app_obj, JpfSysMsg *msg)
{
	JpfGetLicenseInfo *req_info;
	JpfGetLicenseInfoRes res_info;
	gint ret;
	memset(&res_info, 0, sizeof(res_info));

	req_info = (JpfGetLicenseInfo *)MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	ret = jpf_wdd_get_expired_time(&res_info.expired_time);
	res_info.version = jpf_wdd_get_version();

	SET_CODE(&res_info, ret);
	strncpy(res_info.session, req_info->session, SESSION_ID_LEN - 1);
	jpf_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CU);

	return MFR_DELIVER_BACK;
}


void
jpf_mod_wdd_register_msg_handler(JpfModWdd *self)
{
	JpfAppMod *super_self = (JpfAppMod*)self;

	jpf_app_mod_register_msg(
		super_self,
		MESSAGE_GET_LICENSE_INFO,
		NULL,
		jpf_mod_wdd_get_license_info_b,
		0
	);
}

