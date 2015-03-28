#include <sys/time.h>
#include "nmp_mod_cms.h"
#include "nmp_mods.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_tq.h"
#include "nmp_guid.h"
#include "nmp_utility.h"
#include "nmp_sysctl.h"
#include "nmp_msg.h"
#include "nmp_cms_msg.h"


#define TIME_DRIFT_DELTA	3
#define TIMER_DRIFT_MAX		3

gpointer
nmp_mod_cms_sync_req(NmpModCms *self, gint dst, NmpMsgID msg_id,			
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
    		"<NmpModCms> request cmd %d failed!", msg_id
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
    		"<NmpModCms> request cmd %d timeout!", msg_id
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


static void
nmp_mod_cms_query_guids_task(void *parm)
{
	NmpModCms *self = (NmpModCms*)parm;
	NmpIDQueryBlock *qb;
	NmpSysMsg *msg;
	gint start_row, num;

	qb = self->guid_qb;
	nmp_id_qb_tick(qb);

	if (!nmp_id_qb_query_check(qb, &start_row, &num))
	{
		nmp_mos_cms_make_query_guids_msg(self, &msg, start_row, num);
		nmp_cms_conn_post_message(self, self->cms_conn, msg);
	}
}


static void
nmp_mod_cms_query_mds_task(void *parm)
{
	NmpModCms *self = (NmpModCms*)parm;
	NmpIDQueryBlock *qb;
	NmpSysMsg *msg;
	gint start_row, num;

	qb = self->mds_qb;
	nmp_id_qb_tick(qb);

	if (!nmp_id_qb_query_check(qb, &start_row, &num))
	{
		nmp_mos_cms_make_query_mds_msg(self, &msg, start_row, num);
		nmp_cms_conn_post_message(self, self->cms_conn, msg);
	}
}


static __inline__ void
nmp_mod_cms_query_ids_n(NmpAppObj *app_obj)
{
	NmpModCms *self = (NmpModCms*)app_obj;

	static NmpTq tq_1, tq_2;
	static gsize init_tqs = FALSE;

	if (g_once_init_enter(&init_tqs))
	{
		TQ_INIT(&tq_1, nmp_mod_cms_query_guids_task, app_obj);
		nmp_add_tq(&tq_1);

		TQ_INIT(&tq_2, nmp_mod_cms_query_mds_task, app_obj);
		nmp_add_tq(&tq_2);

		g_once_init_leave(&init_tqs, TRUE);
	}

	nmp_id_qb_start(self->guid_qb);
	nmp_id_qb_start(self->mds_qb);
}


static __inline__ gint
nmp_mod_cms_check_mode(gint mode)
{
	gint old_mode, new_mode;

	new_mode = !!mode;
	old_mode = nmp_mss_get_running_mode();
	nmp_mss_set_running_mode(new_mode);

	if (old_mode == MSS_RUNNING_MODE_INIT)
		return new_mode == MSS_RUNNING_MODE_REC;
	else
	{
		if (new_mode != old_mode)
		{
			nmp_mss_reboot("Mss Running Mode Change");
			return 0;
		}
		return new_mode == MSS_RUNNING_MODE_REC;
	}
}


void nmp_mod_notify_disk(NmpModCms *self, gint if_query_ids)
{
	NmpSysMsg *msg;
	NmpRegistedNotifyMessage req_info;
	memset(&req_info, 0, sizeof(NmpRegistedNotifyMessage));

	req_info.if_query_ids = if_query_ids;
	msg = nmp_sysmsg_new_2(MESSAGE_REGISTED_NOTIFY, &req_info, 
		sizeof(NmpRegistedNotifyMessage), 0);
	if (G_UNLIKELY(!msg))
	{
		nmp_warning("nmp_sysmsg_new_2 failed!");
		return ;
	}

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_DISK);
	nmp_app_obj_deliver_out((NmpAppObj *)self, msg);
}


static NmpMsgFunRet
nmp_mod_cms_register_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpmssRegisterRes *res;
	gint if_query_ids = 0;

	res = (NmpmssRegisterRes*)MSG_GET_DATA(msg);
	BUG_ON(!res);

	if (RES_CODE(res))
	{
		nmp_cms_conn_set_register_failed(self->cms_conn, RES_CODE(res));
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}

	nmp_cms_conn_set_registered(self->cms_conn, res);
	memcpy(g_mss_name, res->mss_name, MSS_NAME_LEN);
	
	if_query_ids = nmp_mod_cms_check_mode(res->mode);
	nmp_mod_notify_disk(self, if_query_ids);
	
//	if (nmp_mod_cms_check_mode(res->mode))
//		nmp_mod_cms_query_ids_n(app_obj);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static __inline__ void
nmp_cms_check_utc_time(gchar *utc_time)
{
	static gint drift_counter = 0;
	gint64 delta, now_utc, server_utc;
	guint year, mon, day, hour, min, sec;

	if (sscanf(utc_time, "%d-%d-%d %d:%d:%d", &year, &mon, &day,
		&hour, &min, &sec) != 6)
	{
		return;
	}

	server_utc = nmp_make_utc_time(year, mon, day, hour, min, sec);
	now_utc = time(NULL);
	delta = now_utc - server_utc;
	if (delta >= -TIME_DRIFT_DELTA && delta <= TIME_DRIFT_DELTA)
	{
		drift_counter = 0;
		return;
	}

	if (++drift_counter < TIMER_DRIFT_MAX)
		return;

	drift_counter = 0;
	nmp_set_system_utc_time(year, mon, day, hour, min, sec);
	nmp_print("Set system utc time: '%s'", utc_time);

	return;
}


static NmpMsgFunRet
nmp_mod_cms_heart_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpMssHeartRes *res;

	res = (NmpMssHeartRes*)MSG_GET_DATA(msg);
	BUG_ON(!self->cms_conn);
	BUG_ON(!res);

	if (!RES_CODE(res))
	{
		nmp_cms_check_utc_time(res->server_time);
		nmp_cms_conn_is_alive(self->cms_conn);
	}

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_cms_change_mode_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	//NmpModCms *self = (NmpModCms*)app_obj;
	NmpMssChange *req;

	req = (NmpMssChange*)MSG_GET_DATA(msg);
	BUG_ON(!req);
	nmp_mod_cms_check_mode(req->mode);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_cms_query_idles_hd_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpQueryAllHd *req_info;
	NmpQueryAllHdRes *res_info;
	gint ret, size = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, sizeof(*req_info), &size);
    if (res_info)
    {
        ret = RES_CODE(res_info);    
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);	
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    	return MFR_DELIVER_BACK;
    }
    else
    {
		nmp_warning("<NmpModCms> query idles disks failed, oom!");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
    }
}


static NmpMsgFunRet
nmp_mod_cms_query_record_status_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpQueryRecordStatus *req_info;
	NmpQueryRecordStatusRes res;
	gint sys_mode;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	sys_mode = nmp_mss_get_running_mode();
	if (sys_mode == MSS_RUNNING_MODE_CONF)
	{
		memset(&res, 0, sizeof(res));
		SET_CODE(&res, 0);
		strncpy(res.session, req_info->session, USER_NAME_LEN - 1);
		res.status = REC_INITSTAT;
		res.status_code = 0;

		nmp_sysmsg_set_private_2(msg, &res, sizeof(res));
		MSG_SET_RESPONSE(msg);
		return MFR_DELIVER_BACK;		
	}

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_POLICY);
	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_cms_query_record_status_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpNetIO *io;

	io = nmp_cms_conn_get_io(self->cms_conn);
	if (!io)
	{
		nmp_print(
			"Response 'query record status' failed, connection disconnected."
		);
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;	
	}

	nmp_sysmsg_attach_io(msg, io);
	nmp_net_unref_io(io);

	return MFR_DELIVER_AHEAD;	
}


static NmpMsgFunRet
nmp_mod_cms_add_hdgroup_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpAddHdGroup *req_info;
	NmpAddHdGroupRes *res_info;
	gint ret, size = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, sizeof(*req_info), &size);
    if (res_info)
    {
        ret = RES_CODE(res_info);    
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    	return MFR_DELIVER_BACK;
    }
    else
    {
		nmp_warning("<NmpModCms> add hd group failed, oom!");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
    }
}


static NmpMsgFunRet
nmp_mod_cms_query_hdgroups_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpQueryAllHdGroup *req_info;
	NmpQueryAllHdGroupRes *res_info;

	gint ret, size = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, sizeof(*req_info), &size);
    if (res_info)
    {
        ret = RES_CODE(res_info);    
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    	return MFR_DELIVER_BACK;
    }
    else
    {
		nmp_warning("<NmpModCms> query hd group failed, oom!");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
    }
}


static NmpMsgFunRet
nmp_mod_cms_query_hdgroup_info_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpQueryHdGroupInfo *req_info;
	NmpQueryHdGroupInfoRes *res_info;

	gint ret, size = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, sizeof(*req_info), &size);
    if (res_info)
    {
        ret = RES_CODE(res_info);    
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    	return MFR_DELIVER_BACK;
    }
    else
    {
		nmp_warning("<NmpModCms> query group info failed, oom!");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
    }
}


static NmpMsgFunRet
nmp_mod_cms_add_group_disk_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpAddHdToGroup *req_info;
	NmpAddHdToGroupRes *res_info;

	gint ret, size = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, sizeof(*req_info), &size);
    if (res_info)
    {
        ret = RES_CODE(res_info);
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    		return MFR_DELIVER_BACK;
    }
    else
    {
		nmp_warning("<NmpModCms> add disk to group failed, oom!");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
    }
}


static NmpMsgFunRet
nmp_mod_cms_del_group_disk_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpDelHdFromGroup *req_info;
	NmpDelHdFromGroupRes *res_info;

	gint ret, size = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, sizeof(*req_info), &size);
    if (res_info)
    {
        ret = RES_CODE(res_info);
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    		return MFR_DELIVER_BACK;
    }
    else
    {
		nmp_warning("<NmpModCms> del disk from group failed, oom!");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
    }
}


static NmpMsgFunRet
nmp_mod_cms_del_hdgroup_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpDelHdGroup *req_info;
	NmpDelHdGroupRes *res_info;

	gint ret, size = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, sizeof(*req_info), &size);
    if (res_info)
    {
        ret = RES_CODE(res_info);    
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    		return MFR_DELIVER_BACK;
    }
    else
    {
		nmp_warning("<NmpModCms> del disk group failed, oom!");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
    }
}


static NmpMsgFunRet
nmp_mod_cms_get_format_progress_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpGetHdFormatProgress *req_info;
	NmpGetHdFormatProgressRes *res_info;

	gint ret, size = 0;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, sizeof(*req_info), &size);
    if (res_info)
    {
        ret = RES_CODE(res_info);    
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
    		return MFR_DELIVER_BACK;
    }
    else
    {
		nmp_warning("<NmpModCms> get hd progress failed, oom!");
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
    }
}


static __inline__ void
nmp_mod_cms_deliver_msg(NmpModCms *self, gint dst, gint msg_id, void *parm,
	gint size, void (*destroy)(void *, gsize size))
{
	if (nmp_mss_mod_deliver_msg((NmpAppMod*)self, dst, msg_id, parm,
		size, destroy))
	{
		destroy(parm, size);
	}
}


static __inline__ void
nmp_mod_cms_deliver_msg_2(NmpModCms *self, gint dst, gint msg_id, void *parm,
	gint size)
{
	nmp_mss_mod_deliver_msg_2((NmpAppMod*)self, dst, msg_id, parm, size);
}


static NmpMsgFunRet
nmp_mod_cms_query_guids_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpMssGetGuidRes *res_info;
	gint err, got = 0, terminate = 0;

	res_info = MSG_GET_DATA(msg);

	if (!nmp_id_qb_set_total(self->guid_qb, res_info->total_count) &&
		res_info->back_count <= res_info->total_count)
	{
		while (got < res_info->back_count)
		{
			err = nmp_id_qb_add_entry(self->guid_qb,
				res_info->guid_info[got].domain_id,
				res_info->guid_info[got].guid);
			if (err)
			{
				terminate = 1;
				break;
			}

			++got;
		}
	}

	if (!terminate)
	{
		NmpDiffSet *set = nmp_id_qb_get_diffset(self->guid_qb);
		if (set)
		{
			if (!nmp_diff_set_empty(set))
			{
				nmp_mod_cms_deliver_msg(self, BUSSLOT_POS_POLICY, 
					MESSAGE_GUS_DIFFSET_NOTIFY, set, sizeof(*set),
					(void (*)(void *, gsize))nmp_diff_set_delete);
			}
			else
			{
				nmp_diff_set_delete(set);
			}
		}
	}

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_cms_query_mds_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpMssGetMdsRes *res_info;
	gint err, got = 0, terminate = 0;

	res_info = MSG_GET_DATA(msg);

	if (!nmp_id_qb_set_total(self->mds_qb, res_info->total_count) &&
		res_info->back_count <= res_info->total_count)
	{
		while (got < res_info->back_count)
		{
			err = nmp_id_qb_add_entry(self->mds_qb,
				"",
				res_info->mds_info[got].mds_id);
			if (err)
			{
				terminate = 1;
				break;
			}

			++got;
		}
	}

	if (!terminate)
	{
		NmpDiffSet *set = nmp_id_qb_get_diffset(self->mds_qb);
		if (set)
		{
			if (!nmp_diff_set_empty(set))
			{
				nmp_mod_cms_deliver_msg(self, BUSSLOT_POS_VOD, 
					MESSAGE_MDS_DIFFSET_NOTIFY, set, sizeof(*set),
					(void (*)(void *, gsize))nmp_diff_set_delete);
			}
			else
			{
				nmp_diff_set_delete(set);
			}
		}
	}

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;	
}


static NmpMsgFunRet
nmp_mod_cms_query_policy_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpMssGetRecordPolicy *req;
	NmpPolicyMsg *pm;
	NmpNetIO *io;

	pm = MSG_GET_DATA(msg);

	io = nmp_cms_conn_get_io(self->cms_conn);
	if (!io)
	{
		nmp_print(
			"Request gu:[%s,%s] rec-policy failed, connection disconnected.", 
			pm->guid.domain_id,
			pm->guid.guid
		);
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;	
	}

	req = nmp_mem_kalloc(sizeof(*req));
	if (G_UNLIKELY(!req))
	{
		nmp_warning(
			"Request gu:[%s,%s] rec-policy failed, oom!", 
			pm->guid.domain_id,
			pm->guid.guid
		);
		nmp_sysmsg_destroy(msg);
		nmp_net_unref_io(io);
		return MFR_ACCEPTED;		
	}

	memset(req, 0, sizeof(*req));
	strncpy(req->domain_id, pm->guid.domain_id, DOMAIN_ID_LEN - 1);
	strncpy(req->guid, pm->guid.guid, MAX_ID_LEN - 1);
	strncpy(req->mss_id, (gchar*)nmp_get_sysctl_value(SC_MSS_ID), MSS_ID_LEN -1);

	nmp_sysmsg_attach_io(msg, io);
	nmp_sysmsg_set_private(msg, req, sizeof(*req), nmp_mem_kfree);

	nmp_net_unref_io(io);
	return MFR_DELIVER_AHEAD;
}



static NmpMsgFunRet
nmp_mod_cms_query_policy_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
//	NmpModCms *self = (NmpModCms*)app_obj;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_POLICY);
	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_cms_query_uri_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpMssGetRoute *req;
	NmpUriMsg *pm;
	NmpNetIO *io;

	pm = MSG_GET_DATA(msg);

	io = nmp_cms_conn_get_io(self->cms_conn);
	if (!io)
	{
		nmp_print(
			"Request gu:[%s,%s] uri failed, connection disconnected.", 
			pm->guid.domain_id,
			pm->guid.guid
		);
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;	
	}

	req = nmp_mem_kalloc(sizeof(*req));
	if (G_UNLIKELY(!req))
	{
		nmp_warning(
			"Request gu:[%s,%s] uri failed, oom!", 
			pm->guid.domain_id,
			pm->guid.guid
		);
		nmp_sysmsg_destroy(msg);
		nmp_net_unref_io(io);
		return MFR_ACCEPTED;
	}

	memset(req, 0, sizeof(*req));
	strncpy(req->domain_id, pm->guid.domain_id, DOMAIN_ID_LEN - 1);
	strncpy(req->guid, pm->guid.guid, MAX_ID_LEN - 1);
	strncpy(req->mss_id, (gchar*)nmp_get_sysctl_value(SC_MSS_ID), MSS_ID_LEN -1);
	strncpy(req->cms_ip, (gchar*)nmp_get_sysctl_value(SC_CMS_HOST), MAX_IP_LEN -1);

	nmp_sysmsg_attach_io(msg, io);
	nmp_sysmsg_set_private(msg, req, sizeof(*req), nmp_mem_kfree);

	nmp_net_unref_io(io);
	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_cms_query_uri_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
//	NmpModCms *self = (NmpModCms*)app_obj;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_STREAM);
	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_cms_gulist_changed_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;

	nmp_id_qb_delay(self->guid_qb);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_cms_gupolicy_changed_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
//	NmpModCms *self = (NmpModCms*)app_obj;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_POLICY);
	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_cms_query_mdsip_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
//	NmpModCms *self = (NmpModCms*)app_obj;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_VOD);
	return MFR_DELIVER_AHEAD;
}



static NmpMsgFunRet
nmp_mod_cms_query_mdsip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpMssGetMdsIp *req;
	NmpMdsIpMsg *pm;
	NmpNetIO *io;

	pm = MSG_GET_DATA(msg);

	io = nmp_cms_conn_get_io(self->cms_conn);
	if (!io)
	{
		nmp_print(
			"Request mds '%s' ip failed, connection disconnected.", 
			pm->mds_id
		);
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;	
	}

	req = nmp_mem_kalloc(sizeof(*req));
	if (G_UNLIKELY(!req))
	{
		nmp_warning(
			"Request mds '%s' ip failed, oom.", 
			pm->mds_id
		);
		nmp_sysmsg_destroy(msg);
		nmp_net_unref_io(io);
		return MFR_ACCEPTED;
	}

	memset(req, 0, sizeof(*req));
	strncpy(req->mss_id, (gchar*)nmp_get_sysctl_value(SC_MSS_ID), MSS_ID_LEN - 1);
	strncpy(req->mds_id, pm->mds_id, MDS_ID_LEN - 1);
	strncpy(req->cms_ip, (gchar*)nmp_get_sysctl_value(SC_CMS_HOST), MAX_IP_LEN -1);

	nmp_sysmsg_attach_io(msg, io);
	nmp_sysmsg_set_private(msg, req, sizeof(*req), nmp_mem_kfree);

	nmp_net_unref_io(io);
	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_cms_query_record_list_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
//	NmpModCms *self = (NmpModCms*)app_obj;

	MSG_SET_DSTPOS(msg, BUSSLOT_POS_VOD);
	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_cms_query_record_list_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpNetIO *io;

	io = nmp_cms_conn_get_io(self->cms_conn);
	if (!io)
	{
		nmp_print(
			"Request record list failed, connection disconnected."
		);
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}

	nmp_sysmsg_attach_io(msg, io);
	nmp_net_unref_io(io);

	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_cms_sync_req_to_disk(NmpAppObj *app_obj, NmpSysMsg *msg, 
	void *req_info, void *res_info, gint req_size, char *operate)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	gint ret, size = 0;
	
	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	
	res_info = nmp_mod_cms_sync_req(self, BUSSLOT_POS_DISK, MSG_GETID(msg),
		req_info, req_size, &size);
	if (res_info)
	{
		ret = RES_CODE(res_info);
		MSG_SET_RESPONSE(msg);
		SET_CODE(res_info, -ret);
		nmp_sysmsg_set_private(msg, res_info, size, nmp_mem_kfree);
		return MFR_DELIVER_BACK;
	}
	else
	{
		nmp_warning("<NmpModCms> operate:%s failed!", operate);
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}
}


static NmpMsgFunRet
nmp_mod_add_one_ipsan_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssAddOneIpsan *req_info = NULL;
	NmpMssAddOneIpsanRes *res_info = NULL;

	return nmp_mod_cms_sync_req_to_disk(app_obj, msg, req_info, res_info, 
		sizeof(*req_info), "add one ipsan");
}


static NmpMsgFunRet
nmp_mod_get_ipsans_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssGetIpsans *req_info = NULL;
	NmpMssGetIpsansRes *res_info = NULL;

	return nmp_mod_cms_sync_req_to_disk(app_obj, msg, req_info, res_info, 
		sizeof(*req_info), "get ipsans");
}


static NmpMsgFunRet
nmp_mod_set_ipsans_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssSetIpsans *req_info = NULL;
	NmpMssSetIpsansRes *res_info = NULL;

	return nmp_mod_cms_sync_req_to_disk(app_obj, msg, req_info, res_info, 
		sizeof(*req_info), "set ipsans");
}


static NmpMsgFunRet
nmp_mod_get_initiator_name_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssGetInitiatorName *req_info = NULL;
	NmpMssGetInitiatorNameRes *res_info = NULL;

	return nmp_mod_cms_sync_req_to_disk(app_obj, msg, req_info, res_info, 
		sizeof(*req_info), "get initiator name");
}


static NmpMsgFunRet
nmp_mod_set_initiator_name_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssSetInitiatorName *req_info = NULL;
	NmpMssSetInitiatorNameRes *res_info = NULL;

	return nmp_mod_cms_sync_req_to_disk(app_obj, msg, req_info, res_info, 
		sizeof(*req_info), "set initiator name");
}


static NmpMsgFunRet
nmp_mod_get_one_ipsan_detail_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssGetOneIpsanDetail *req_info = NULL;
	NmpMssGetOneIpsanDetailRes *res_info = NULL;

	return nmp_mod_cms_sync_req_to_disk(app_obj, msg, req_info, res_info, 
		sizeof(*req_info), "get one ipsan detail");
}


static NmpMsgFunRet
nmp_mod_del_one_ipsan_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpMssDelOneIpsan *req_info = NULL;
	NmpMssDelOneIpsanRes *res_info = NULL;

	return nmp_mod_cms_sync_req_to_disk(app_obj, msg, req_info, res_info, 
		sizeof(*req_info), "del one ipsan");
}


static NmpMsgFunRet
nmp_mod_cms_registed_notify_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpRegistedNotifyMessageRes *res;

	res = (NmpRegistedNotifyMessageRes*)MSG_GET_DATA(msg);
	BUG_ON(!res);

	if (res->if_query_ids)
		nmp_mod_cms_query_ids_n(app_obj);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_notify_message_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModCms *self = (NmpModCms*)app_obj;
	NmpNetIO *io;

	io = nmp_cms_conn_get_io(self->cms_conn);
	if (!io)
	{
		nmp_print(
			"Request record list failed, connection disconnected."
		);
		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}

	nmp_sysmsg_attach_io(msg, io);
	nmp_net_unref_io(io);

	return MFR_DELIVER_AHEAD;
}


static NmpMsgFunRet
nmp_mod_alarm_link_record_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_POLICY);
	return MFR_DELIVER_AHEAD;
}


static void nmp_will_reboot()
{
	system("sleep 1 && reboot &");
};

static NmpMsgFunRet
nmp_mod_system_reboot_f(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpSystemReboot *req_info = NULL;
	NmpSystemRebootRes res_info;
	memset(&res_info, 0, sizeof(res_info));

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);
	
	SET_CODE(&res_info, 0);
	strncpy(res_info.session, req_info->session, USER_NAME_LEN - 1);
	
	nmp_sysmsg_set_private_2(msg, &res_info, sizeof(res_info));
	MSG_SET_RESPONSE(msg);
	
	nmp_will_reboot();
	
	return MFR_DELIVER_BACK;
}


void
nmp_mod_cms_register_msg_handler(NmpModCms *self)
{
    NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_REGISTER_CMS,
        nmp_mod_cms_register_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_KEEPALIVE_CMS,
        nmp_mod_cms_heart_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_NOTIFY_MSS_MODE,
        nmp_mod_cms_change_mode_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_IDLES_HD,
        nmp_mod_cms_query_idles_hd_f, 
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_RECORD_STATUS,
        nmp_mod_cms_query_record_status_f, 
        nmp_mod_cms_query_record_status_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_HD_GROUP,
        nmp_mod_cms_add_hdgroup_f, 
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_HD_GROUPS,
        nmp_mod_cms_query_hdgroups_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_HD_GROUP_INFO,
        nmp_mod_cms_query_hdgroup_info_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_GROUP_DISK,
        nmp_mod_cms_add_group_disk_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_GROUP_DISK,
        nmp_mod_cms_del_group_disk_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_HD_GROUP,
        nmp_mod_cms_del_hdgroup_f,
        NULL,
        0
    );


    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_HDFMT_PROGRESS,
        nmp_mod_cms_get_format_progress_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_GUIDS,
        nmp_mod_cms_query_guids_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ALL_MDS,
        nmp_mod_cms_query_mds_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_POLICY,
        nmp_mod_cms_query_policy_f,
        nmp_mod_cms_query_policy_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_URI,
        nmp_mod_cms_query_uri_f,
        nmp_mod_cms_query_uri_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GULIST_CHANGED,
        nmp_mod_cms_gulist_changed_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GUPOLICY_CHANGED,
        nmp_mod_cms_gupolicy_changed_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MDS_IP,
        nmp_mod_cms_query_mdsip_f,
        nmp_mod_cms_query_mdsip_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_RECORDS_LIST,
        nmp_mod_cms_query_record_list_f,
        nmp_mod_cms_query_record_list_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ADD_ONE_IPSAN,
        nmp_mod_add_one_ipsan_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_IPSANS,
        nmp_mod_get_ipsans_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_IPSANS,
        nmp_mod_set_ipsans_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_INITIATOR_NAME,
        nmp_mod_get_initiator_name_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SET_INITIATOR_NAME,
        nmp_mod_set_initiator_name_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_ONE_IPSAN_DETAIL,
        nmp_mod_get_one_ipsan_detail_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_DEL_ONE_IPSAN,
        nmp_mod_del_one_ipsan_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_REGISTED_NOTIFY,
        NULL, 
        nmp_mod_cms_registed_notify_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_NOTIFY_MESSAGE,
        NULL,
        nmp_mod_notify_message_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_ALARM_LINK_RECORD,
        nmp_mod_alarm_link_record_f,
        NULL,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_SYSTEM_REBOOT,
        nmp_mod_system_reboot_f,
        NULL,
        0
    );

}


//:~End
