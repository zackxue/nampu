#include "nmp_message.h"
#include "nmp_internal_msg.h"
#include "nmp_xml.h"
#include "nmp_sysmsg.h"
#include "nmp_mysql_fun.h"
#include "nmp_mod_dbs.h"
#include "nmp_dbs_fun.h"
#include "nmp_memory.h"
#include "nmp_debug.h"
#include "db_connect_pool.h"
#include "nmp_errno.h"
#include "message/nmp_msg_cu.h"
#include "nmp_shared.h"
#include "nmp_list_head.h"
#include "nmp_dbs_struct.h"
#include "nmp_res_ctl.h"
#include "nmp_dbs_fun.h"
#include "nmp_wdd.h"


static __inline__ void
jpf_dbs_covert_cms_modules_bits(guint *new_bits, guint *old_bits)
{
	*new_bits = 0;

	if (*old_bits & (1<<CMS_HIK))
	{
		*new_bits |= MF_HIK_BIT;
	}
	if (*old_bits & (1<<CMS_DAH))
	{
		*new_bits |= MF_DAH_BIT;
	}
	if (*old_bits & (1<<CMS_HBGK))
	{
		*new_bits |= MF_HBGK_BIT;
	}
	if (*old_bits & (1<<CMS_HNW))
	{
		*new_bits |= MF_HNW_BIT;
	}
	if (*old_bits & (1<<CMS_XMT))
	{
		*new_bits |= MF_XMT_BIT;
	}
	if (*old_bits & (1<<CMS_TPS))
	{
		*new_bits |= MF_TPS_BIT;
	}
}


static __inline__ void
jpf_dbs_covert_tw_modules_bits(guint *new_bits, guint *old_bits)
{
	*new_bits = 0;

	if (*old_bits & (1<<TW_TOUR))
	{
		*new_bits |= TW_TOUR_BIT;
	}
	if (*old_bits & (1<<TW_GROUP))
	{
		*new_bits |= TW_GROUP_BIT;
	}
	if (*old_bits & (1<<TW_KEYBOARD))
	{
		*new_bits |= TW_KEYBOARD_BIT;
	}
}


static __inline__ void
jpf_dbs_covert_ams_modules_bits(guint *new_bits, guint *old_bits)
{
	*new_bits = 0;

	if (*old_bits & (1<<LINKED_REC))
	{
		*new_bits |= ACTION_RECORD_BIT;
	}
	if (*old_bits & (1<<LINKED_MSG))
	{
		*new_bits |= ACTION_SMS_BIT;
	}
	if (*old_bits & (1<<LINKED_MMS))
	{
		*new_bits |= ACTION_MMS_BIT;
	}
	if (*old_bits & (1<<LINKED_CAP))
	{
		*new_bits |= ACTION_CAP_BIT;
	}
	if (*old_bits & (1<<LINKED_PRESET))
	{
		*new_bits |= ACTION_PRESET_BIT;
	}
	if (*old_bits & (1<<LINKED_TW))
	{
		*new_bits |= ACTION_TW_BIT;
	}
	if (*old_bits & (1<<LINKED_IO))
	{
		*new_bits |= ACTION_IO_BIT;
	}
	if (*old_bits & (1<<LINKED_EMAIL))
	{
		*new_bits |= ACTION_EMAIL_BIT;
	}
	if (*old_bits & (1<<LINKED_EMAP))
	{
		*new_bits |= ACTION_EMAP_BIT;
	}
}


static __inline__ void
jpf_dbs_covert_modules_bits(JpfResourcesCap *new_res, JpfMsgWddDevCapInfo *old_res)
{
	guint new_bits = 0, old_bits;

	old_bits = old_res->module_bits;
	if (old_bits & (1<<MODULES_CMS))
	{
		new_bits |= MODULE_CMS_BIT;
		jpf_dbs_covert_cms_modules_bits(&new_res->modules_data[SYS_MODULE_CMS],
			&old_res->modules_data[MODULES_CMS]);
	}
	if (old_bits & (1<<MODULES_MDS))
	{
		new_bits |= MODULE_MDS_BIT;
		new_res->modules_data[SYS_MODULE_MDS] = old_res->modules_data[MODULES_MDS];
	}
	if (old_bits & (1<<MODULES_MSS))
	{
		new_bits |= MODULE_MSS_BIT;
		new_res->modules_data[SYS_MODULE_MSS] = old_res->modules_data[MODULES_MSS];
	}
	if (old_bits & (1<<MODULES_ALM))
	{
		new_bits |= MODULE_ALM_BIT;
		jpf_dbs_covert_ams_modules_bits(&new_res->modules_data[SYS_MODULE_ALM],
			&old_res->modules_data[MODULES_ALM]);
	}
	if (old_bits & (1<<MODULES_EM))
	{
		new_bits |= MODULE_EM_BIT;
		new_res->modules_data[SYS_MODULE_EM] = old_res->modules_data[MODULES_EM];
	}
	if (old_bits & (1<<MODULES_TW))
	{
		new_bits |= MODULE_TW_BIT;
		jpf_dbs_covert_tw_modules_bits(&new_res->modules_data[SYS_MODULE_TW],
			&old_res->modules_data[MODULES_TW]);
	}

	new_res->module_bits = new_bits;
}

void
jpf_dbs_wdd_set_resources_cap(JpfMsgWddDevCapInfo *wdd_cap)
{
	JpfResourcesCap resources_cap;

	memset(&resources_cap, 0, sizeof(JpfResourcesCap));
	jpf_dbs_covert_modules_bits(&resources_cap, wdd_cap);
	resources_cap.expired_time = wdd_cap->expired_time;
	resources_cap.dev_count = wdd_cap->max_dev;
	resources_cap.av_count = wdd_cap->max_av;
	resources_cap.ds_count = wdd_cap->max_ds;
	resources_cap.ai_count = wdd_cap->max_ai;
	resources_cap.ao_count = wdd_cap->max_ao;
	resources_cap.system_version = wdd_cap->version;
	nmp_mod_set_resource_cap(&resources_cap);
}

NmpMsgFunRet
jpf_dbs_wdd_check_resourse_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	JpfModDbs * self = (JpfModDbs *)app_obj;
	JpfMsgWddDevCapInfo *req_info;
	JpfNotifyMessage notify_info;
	gint ret;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	jpf_dbs_wdd_set_resources_cap(req_info);
	ret = jpf_dbs_check_gu_type_count(app_obj, 0, req_info->max_av, AV_TYPE);
	if (ret == -1)
		self->res_over_flag |= AV_OVER_FLAG;
	if (ret == 0)
		self->res_over_flag &= ~AV_OVER_FLAG;

	ret = jpf_dbs_check_gu_type_count(app_obj, 0, req_info->max_ds, DS_TYPE);
	if (ret == -1)
		self->res_over_flag |= DS_OVER_FLAG;
	if (ret == 0)
		self->res_over_flag &= ~DS_OVER_FLAG;

	ret = jpf_dbs_check_gu_type_count(app_obj, 0, req_info->max_ai, AI_TYPE);
	if (ret == -1)
		self->res_over_flag |= AI_OVER_FLAG;
	if (ret == 0)
		self->res_over_flag &= ~AI_OVER_FLAG;

	ret = jpf_dbs_check_gu_type_count(app_obj, 0, req_info->max_ao, AO_TYPE);
	if (ret == -1)
		self->res_over_flag |= AO_OVER_FLAG;
	if (ret == 0)
		self->res_over_flag &= ~AO_OVER_FLAG;

	if (self->res_over_flag)
	{
		memset(&notify_info, 0, sizeof(notify_info));
		notify_info.msg_id = MSG_DEV_GU_OVER;
		sprintf(notify_info.param1, "%d", self->res_over_flag);
	      nmp_cms_mod_deliver_msg_2((NmpAppObj *)self, BUSSLOT_POS_CU,
	        	MESSAGE_BROADCAST_GENERAL_MSG, &notify_info, sizeof(notify_info));

		  if (req_info->version == VER_TEST)
		  {
		  	memset(&notify_info, 0, sizeof(notify_info));
			notify_info.msg_id = MSG_WDD_ABNORMAL;
			nmp_cms_mod_deliver_msg_2((NmpAppObj *)self, BUSSLOT_POS_CU,
				MESSAGE_BROADCAST_GENERAL_MSG, &notify_info, sizeof(notify_info));
		  }
      }

	self->wdd_status = 0;
	self->authorization_expired = 0;
	self->time_status = 0;
      jpf_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


NmpMsgFunRet
jpf_dbs_wdd_set_auth_expired_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	G_ASSERT(app_obj != NULL && msg != NULL);

	JpfModDbs * self = (JpfModDbs *)app_obj;
	JpfMsgWddAuthErrorInfo *notify_info;
	JpfResourcesCap resources_cap;

	memset(&resources_cap, 0, sizeof(JpfResourcesCap));
	notify_info = MSG_GET_DATA(msg);
	BUG_ON(!notify_info);

	if (notify_info->type == WDD_AUTH_EXPIRED)
	{
		self->authorization_expired = 1;
		resources_cap.expired_time.type = WDD_AUTH_OVERDUE;
	}
	if (notify_info->type == WDD_AUTH_INEXISTENT)
	{
		self->wdd_status = 1;
		resources_cap.expired_time.type = WDD_AUTH_NO_SOFTDOG;
	}
	if (notify_info->type == WDD_SYS_TIME_ERROR)
		self->time_status = 1;

	nmp_mod_set_resource_cap(&resources_cap);
	jpf_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


void
nmp_mod_dbs_register_wdd_msg_handler(JpfModDbs *self)
{
	NmpAppMod *super_self = (NmpAppMod*)self;

	nmp_app_mod_register_msg(
		super_self,
		MSG_WDD_DEV_CAP_INFO,
		NULL,
		jpf_dbs_wdd_check_resourse_b,
		0
	);

	nmp_app_mod_register_msg(
		super_self,
		MSG_WDD_AUTH_ERROR,
		NULL,
		jpf_dbs_wdd_set_auth_expired_b,
		0
	);
}

