#include "nmp_mod_vod.h"
#include "nmp_mods.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_tq.h"
#include "nmp_message.h"
#include "nmp_guid.h"
#include "nmp_msg.h"
#include "nmp_utility.h"

#define FILE_PREALLOC_SIZE			(256 * 1024 * 1024)		/* now file prealloc-size: 256MB */
#define FILE_SIZE_DELTA             (6 * 1024 * 1024)

void
nmp_mod_vod_on_pp_event(gpointer priv, NmpMsg *msg)
{
	NmpModVod *self = (NmpModVod*)priv;

	switch (msg->id)
	{
	case EVENT_MDSIP:
		if (nmp_mss_mod_deliver_msg((NmpAppMod*)self, BUSSLOT_POS_CMS,
			MESSAGE_GET_MDS_IP, msg, 0,
			(void (*)(void*, gsize))FREE_MSG_FUN(msg)))
		{
			FREE_MSG(msg);
		}	
		break;		

	default:
		FREE_MSG(msg);
		break;
	}
}


static void
nmp_mod_vod_add_new_mds(NmpGuid *guid, void *parm)
{
	NmpModVod *self = (NmpModVod*)parm;
	NmpMdsPool *mds_pool = self->mds_pool;

	BUG_ON(!mds_pool);
	nmp_spool_add_mds(mds_pool, guid->guid);
}


static void
nmp_mod_vod_del_exist_mds(NmpGuid *guid, void *parm)
{
	NmpModVod *self = (NmpModVod*)parm;
	NmpMdsPool *mds_pool;

	mds_pool = (NmpMdsPool*)self->mds_pool;

	BUG_ON(!mds_pool);
	nmp_spool_del_mds(mds_pool, guid->guid);
}


static NmpMsgFunRet
nmp_mod_vod_mds_diffset_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModVod *self = (NmpModVod*)app_obj;
	NmpDiffSet *set = MSG_GET_DATA(msg);	

	if (set->add && !nmp_guid_set_empty(set->add))
	{
		nmp_guid_set_foreach(set->add, nmp_mod_vod_add_new_mds, self);
	}

	if (set->del && !nmp_guid_set_empty(set->del))
	{
		nmp_guid_set_foreach(set->del, nmp_mod_vod_del_exist_mds, self);
	}


	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}



static NmpMsgFunRet
nmp_mod_vod_mds_ip_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModVod *self = (NmpModVod*)app_obj;
	NmpMssGetMdsIpRes *res_info = MSG_GET_DATA(msg);

	BUG_ON(!res_info);
	if (RES_CODE(res_info))
	{
		nmp_print(
			"Get mds '%s' ip failed, err:%d.",
			res_info->mds_id,
			RES_CODE(res_info)
		);

		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}

	nmp_spool_add_mds_ip(self->mds_pool, res_info->mds_id, res_info->mds_ip, res_info->port);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static __inline__ guint
nmp_mod_vod_get_record_size(guint size)
{
	gint64 _size = FILE_PREALLOC_SIZE;

	_size -= size;
	if (_size >= -FILE_SIZE_DELTA && _size <= FILE_SIZE_DELTA)
		return FILE_PREALLOC_SIZE;

	return size;
}


static NmpMsgFunRet
nmp_mod_vod_records_list_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModVod *self = (NmpModVod*)app_obj;
	NmpRecords *records;
	NmpRecordList *l;
	gint ret, index = 0, count;
	NmpGetMssStoreLog *req_info;
	NmpGetStoreLogRes *res_info;
	gchar *ch;
	gsize res_size = sizeof(*res_info);
	GList *list;
	NmpRecord *r;

	records = self->records;
	req_info = (NmpGetMssStoreLog*)MSG_GET_DATA(msg);
	BUG_ON(!records);
	BUG_ON(!req_info);

	l = nmp_record_list_new();
	ch = g_strdup_printf("%s@%s", req_info->domain_id, req_info->guid);

	ret = nmp_records_get_list_2(records, l, ch,
		nmp_make_time_t(req_info->begin_time),
		nmp_make_time_t(req_info->end_time),
		req_info->begin_node,
		req_info->end_node - req_info->begin_node + 1,
		req_info->record_type);

	if (ret >= 0)
	{
		count = nmp_record_list_length(l);
		res_size += count * sizeof(NmpStoreLog);
		res_info = (NmpGetStoreLogRes*)nmp_mem_kalloc(res_size);

		list = l->list;
		for (; index < count; ++index)
		{
			if (!list)
				break;

			r = (NmpRecord*)list->data;

			res_info->store_list[index].record_type = r->flags;

			nmp_make_time_str(res_info->store_list[index].begin_time,
				TIME_LEN, r->start);

			nmp_make_time_str(res_info->store_list[index].end_time,
				TIME_LEN, r->end);

			strncpy(res_info->store_list[index].property, r->property,
				FILE_PROPERTY_LEN - 1);
			res_info->store_list[index].property[FILE_PROPERTY_LEN - 1] = 0;

			res_info->store_list[index].file_size = nmp_mod_vod_get_record_size(r->size);
			list = g_list_next(list);
		}
	}
	else
	{
		res_info = (NmpGetStoreLogRes*)nmp_mem_kalloc(res_size);
	}

	memset(res_info, 0, sizeof(*res_info));
	SET_CODE(res_info, (ret >= 0 ? 0 : ret));
	strncpy(res_info->session, req_info->session, SESSION_ID_LEN - 1);
	strncpy(res_info->domain_id, req_info->domain_id, DOMAIN_ID_LEN - 1);
	strncpy(res_info->guid, req_info->guid, MAX_ID_LEN - 1);
	res_info->total_num = ret >= 0 ? ret : 0;
	res_info->req_num = index;

	nmp_record_list_free(l);

	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	nmp_sysmsg_set_private(msg, res_info, res_size, nmp_mem_kfree);

	return MFR_DELIVER_BACK;
}


void
nmp_mod_vod_register_msg_handler(NmpModVod *self)
{
	NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_MDS_DIFFSET_NOTIFY,
        NULL,
        nmp_mod_vod_mds_diffset_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_MDS_IP,
        NULL,
        nmp_mod_vod_mds_ip_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_GET_RECORDS_LIST,
        NULL,
        nmp_mod_vod_records_list_b,
        0
    );
}


//:~ End
