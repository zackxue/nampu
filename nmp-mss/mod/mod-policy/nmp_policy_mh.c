#include "nmp_mod_policy.h"
#include "nmp_mods.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_tq.h"
#include "nmp_guid.h"
#include "nmp_gu.h"
#include "nmp_message.h"
#include "nmp_utility.h"


gpointer
nmp_mod_policy_sync_req(NmpModPolicy *self, gint dst, NmpMsgID msg_id,			
       gpointer req, gint req_size, gint *res_size)
{
    gint err = 0;
    NmpMsgErrCode *res_info;
    gpointer res;
    NmpSysMsg *msg;
    guint len;

    G_ASSERT(self != NULL);

    msg = nmp_sysmsg_new_2(msg_id, req, req_size, 0);
    if (G_UNLIKELY(!msg))
    	return NULL;	

    MSG_SET_DSTPOS(msg, dst);
    len = sizeof(NmpMsgErrCode);
	
    err = nmp_app_mod_sync_request((NmpAppMod*)self, &msg); 
    if (G_UNLIKELY(err))	/* send failed */
    {
    	nmp_warning(
    		"<NmpModPolicy> request cmd %d failed!", msg_id
    	);
    
    	nmp_sysmsg_destroy(msg);

    	res_info = nmp_mem_kalloc(len); 
    	if (res_info)
    	{
    		SET_CODE(res_info, err);
    		*res_size = len;
    	}
    
    	return res_info;
    }

    if (G_UNLIKELY(!msg))	/* sent, but no response */ 
    {
        nmp_warning(
    		"<NmpModPolicy> request cmd %d timeout!", msg_id
    	 );
    	 res_info = nmp_mem_kalloc(len);  
    	 err = -E_TIMEOUT;
        if (res_info)
    	{
    		SET_CODE(res_info, err);
    		*res_size = len;
    	}
    	return res_info;
    }

    res = MSG_GET_DATA(msg);
    if (!res)
    {
    	nmp_sysmsg_destroy(msg);
    	return NULL;
    }
    
    res_info = nmp_mem_kalloc(MSG_DATA_SIZE(msg));      
    if (G_UNLIKELY(!res_info))
    {
    	nmp_sysmsg_destroy(msg);
    	return NULL;
    }

    *res_size =  MSG_DATA_SIZE(msg);
    memcpy(res_info, res, *res_size);
    nmp_sysmsg_destroy(msg);

    return res_info;	
}


static NmpSysMsg *
nmp_mod_policy_new_msg(NmpMsgID id, gpointer param, gsize size)
{
	static guint seq = 0;

	return nmp_sysmsg_new_2(id, param, size, ++seq);
}


static void
nmp_mod_policy_add_new_gu(NmpGuid *guid, void *parm)
{
	NmpModPolicy *self = (NmpModPolicy*)parm;
	NmpGuPool *gu_pool = self->gu_pool;

	BUG_ON(!gu_pool);
	nmp_gu_pool_add_gu(gu_pool, guid);

	nmp_print(
		"policy: add new guid:'%s, %s'.", guid->domain_id, guid->guid
	);
}


static void
nmp_mod_policy_del_exist_gu(NmpGuid *guid, void *parm)
{
	NmpModPolicy *self = (NmpModPolicy*)parm;
	NmpGuPool *gu_pool = self->gu_pool;

	BUG_ON(!gu_pool);
	nmp_gu_pool_del_gu(gu_pool, guid);

	nmp_print(
		"policy: del guid:'%s, %s'.", guid->domain_id, guid->guid
	);
}


static NmpMsgFunRet
nmp_mod_policy_gu_diffset_notify_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModPolicy *self = (NmpModPolicy*)app_obj;
	NmpDiffSet *set = MSG_GET_DATA(msg);	

	if (set->add && !nmp_guid_set_empty(set->add))
	{
		nmp_guid_set_foreach(set->add, nmp_mod_policy_add_new_gu, self);
	}

	if (set->del && !nmp_guid_set_empty(set->del))
	{
		nmp_guid_set_foreach(set->del, nmp_mod_policy_del_exist_gu, self);
	}

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static __inline__ void
nmp_mod_policy_adjust_seg(time_t *start, time_t *end)
{
	*end += 1;
}


static __inline__ void
nmp_mod_policy_parse(NmpPolicy *policy, NmpRecordPolicy *p)
{
	gint day, day_upper, seg, seg_upper;
	NmpWeekday *w;
	NmpTimeSeg *s;
	Nmpdate *d;
	time_t start, end;
	gint year, mon, date;

	day_upper = p->weekday_num > WEEKDAYS ? WEEKDAYS
	                                      : p->weekday_num;
	for (day = 0; day < day_upper; ++day)
	{
		w = &p->weekdays[day];
		seg_upper = w->time_seg_num > TIME_SEG_NUM ? TIME_SEG_NUM 
		                                           : w->time_seg_num;
		for (seg = 0; seg < seg_upper; ++seg)
		{
			s = &w->time_segs[seg];
			if (!nmp_get_string_time_range(s->time_seg, &start, &end))
			{
				nmp_mod_policy_adjust_seg(&start, &end);
				nmp_policy_add_time_seg_day(policy, TS_REC_TYPE_AUTO, 
					(w->weekday < 7 ? w->weekday : 0), start, end);
			}
		}
	}

	day_upper = p->day_num > MAX_DAY_NUM ? MAX_DAY_NUM
	                                     : p->day_num;
	for (day = 0; day < day_upper; ++day)
	{
		d = &p->day[day];
		if (!nmp_get_string_date(d->date, &year, &mon, &date))
		{
			seg_upper = d->time_seg_num > TIME_SEG_NUM ? TIME_SEG_NUM 
			                                           : d->time_seg_num;
			for (seg = 0; seg < seg_upper; ++seg)
			{
				s = &d->time_segs[seg];
				if (!nmp_get_string_time_range(s->time_seg, &start, &end))
				{
					nmp_mod_policy_adjust_seg(&start, &end);
					nmp_policy_add_time_seg_date(policy, TS_REC_TYPE_AUTO,
						year, mon, date, start, end);
				}
			}
		}
	}
}


static NmpMsgFunRet
nmp_mod_policy_gupolicy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModPolicy *self = (NmpModPolicy*)app_obj;
	NmpMssGetRecordPolicyRes *res_info = MSG_GET_DATA(msg);
	NmpPolicy *policy;
	gint err;
	NmpGuid guid;

	BUG_ON(!res_info);
	if (RES_CODE(res_info))
	{
		nmp_print(
			"Get GU:'%s,%s' policy failed, err:%d.",
			res_info->domain_id,
			res_info->guid,
			RES_CODE(res_info)
		);

		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}

	policy = nmp_new_policy();
	nmp_mod_policy_parse(policy, &res_info->record_policy);

	nmp_guid_generate(&guid, res_info->domain_id, res_info->guid);
	err = nmp_gu_pool_add_policy(self->gu_pool, &guid, res_info->level, policy);
	if (err)
	{
		nmp_delete_policy(policy);
	}

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static __inline__ void
nmp_mod_policy_on_event_policy(NmpModPolicy *self, gint dst,
	gint message_id, NmpMsg *msg)
{
	if (nmp_mss_mod_deliver_msg((NmpAppMod*)self, dst,
		message_id, msg, 0,
		(void (*)(void*, gsize))FREE_MSG_FUN(msg)))
	{
		FREE_MSG(msg);
	}
}


void
nmp_mod_policy_on_pp_event(gpointer priv, NmpMsg *msg)
{
	NmpModPolicy *self = (NmpModPolicy*)priv;

	switch (msg->id)
	{
	case EVENT_POLICY:
		nmp_mod_policy_on_event_policy(self, BUSSLOT_POS_CMS,
			MESSAGE_QUERY_POLICY, msg);
		break;

	case EVENT_REC:
		nmp_mod_policy_on_event_policy(self, BUSSLOT_POS_STREAM,
		MESSAGE_START_RECORD, msg);
		break;

	case EVENT_STOP_REC:
		nmp_mod_policy_on_event_policy(self, BUSSLOT_POS_STREAM,
		MESSAGE_STOP_RECORD, msg);
		break;

	default:
		FREE_MSG(msg);
		break;
	}
}


static NmpMsgFunRet
nmp_mod_policy_changed_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModPolicy *self = (NmpModPolicy*)app_obj;
	NmpNotifyPolicyChange *req_info = MSG_GET_DATA(msg);
	NmpGuid guid;

	BUG_ON(!req_info);

	if (req_info->all_changed)
	{
		nmp_print("All record policy changed!");
		nmp_gu_pool_flush_all_policy(self->gu_pool);
	}
	else
	{
		nmp_guid_generate(&guid, req_info->domain_id, req_info->guid);
		nmp_gu_pool_flush_policy(self->gu_pool, &guid);
	}

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_policy_query_record_status_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModPolicy *self = (NmpModPolicy*)app_obj;
	NmpQueryRecordStatus *req_info;
	NmpQueryRecordStatusRes res;
	NmpGuid guid;
	gint ret, recording;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	nmp_guid_generate(&guid, req_info->domain_id, req_info->guid);
	ret = nmp_gu_pool_get_record_status(self->gu_pool, &guid, &recording); 
	if (ret || !recording)	/* not found, syncing policy ? waiting ?*/
	{
		memset(&res, 0, sizeof(res));
		SET_CODE(&res, 0);
		strncpy(res.session, req_info->session, USER_NAME_LEN - 1);
		res.status = ret ? ( ret == -EAGAIN ? REC_NODISKS : REC_SYNCING) : REC_WAITING;
		res.status_code = 0;

		nmp_sysmsg_set_private_2(msg, &res, sizeof(res));
		MSG_SET_RESPONSE(msg);
		MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
		return MFR_DELIVER_BACK;
	}
	else
	{
		MSG_SET_DSTPOS(msg, BUSSLOT_POS_STREAM);
		return MFR_DELIVER_BACK;
	}
}


static NmpMsgFunRet
nmp_mod_alarm_link_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModPolicy *self = (NmpModPolicy*)app_obj;
	NmpAlarmLinkRecord *req_info = MSG_GET_DATA(msg);
	NmpGuid guid;

	BUG_ON(!req_info);

	nmp_guid_generate(&guid, req_info->domain_id, req_info->guid);
	nmp_gu_pool_start_alarm(self->gu_pool, &guid, req_info->time_len);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_policy_disk_ready_notify_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModPolicy *self = (NmpModPolicy*)app_obj;

	nmp_gu_pool_set_ready(self->gu_pool);
	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


void
nmp_mod_policy_register_msg_handler(NmpModPolicy *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GUS_DIFFSET_NOTIFY,
        NULL,
        nmp_mod_policy_gu_diffset_notify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_POLICY,
        NULL,
        nmp_mod_policy_gupolicy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GUPOLICY_CHANGED,
        NULL,
        nmp_mod_policy_changed_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_RECORD_STATUS,
        NULL, 
        nmp_mod_policy_query_record_status_b,
        0
    );
	
    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ALARM_LINK_RECORD,
        NULL, 
        nmp_mod_alarm_link_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_REGISTED_NOTIFY,
        NULL, 
        nmp_mod_policy_disk_ready_notify_b,
        0
    );
}


//:~ End
