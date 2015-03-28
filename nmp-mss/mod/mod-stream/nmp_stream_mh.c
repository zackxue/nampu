#include "nmp_mod_stream.h"
#include "nmp_mods.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_memory.h"
#include "nmp_tq.h"
#include "nmp_message.h"


void
nmp_mod_stream_on_pp_event(gpointer priv, NmpMsg *msg)
{
	NmpModStream *self = (NmpModStream*)priv;

	switch (msg->id)
	{
	case EVENT_URI:
		if (nmp_mss_mod_deliver_msg((NmpAppMod*)self, BUSSLOT_POS_CMS,
			MESSAGE_QUERY_URI, msg, 0,
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


static NmpMsgFunRet
nmp_mod_stream_uri_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModStream *self = (NmpModStream*)app_obj;
	NmpMssGetRouteRes *res_info = MSG_GET_DATA(msg);
	NmpGuid guid;	

	BUG_ON(!res_info);
	if (RES_CODE(res_info))
	{
		nmp_print(
			"Get GU:'%s,%s' uri failed, err:%d.",
			res_info->domain_id,
			res_info->guid,
			RES_CODE(res_info)
		);

		nmp_sysmsg_destroy(msg);
		return MFR_ACCEPTED;
	}

	nmp_guid_generate(&guid, res_info->domain_id, res_info->guid);
	nmp_spool_add_sch_uri(self->spool, &guid, res_info->url);

	nmp_sysmsg_destroy(msg);
	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_stream_start_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModStream *self = (NmpModStream*)app_obj;
	NmpRecMsg *pm = MSG_GET_DATA(msg);
	NmpSchPool	*spool = self->spool;

	nmp_spool_add_sch(spool, &pm->guid, pm->rec_type, pm->hd_grp, pm->level);
	nmp_sysmsg_destroy(msg);

	return MFR_ACCEPTED;
}


static NmpMsgFunRet
nmp_mod_stream_stop_record_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModStream *self = (NmpModStream*)app_obj;
	NmpRecMsg *pm = MSG_GET_DATA(msg);
	NmpSchPool	*spool = self->spool;

	nmp_spool_del_sch(spool, &pm->guid);
	nmp_sysmsg_destroy(msg);

	return MFR_ACCEPTED;
}


static __inline__ gint
nmp_mod_stream_map_status(gint status)
{
	gint s;

	switch (status)
	{
	case ST_PREPARE:
		s = REC_PREPARE;
		break;

	case ST_URL:
		s = REC_URL;
		break;

	case ST_REQUEST:
		s = REC_REQUETING;
		break;

	case ST_RECORD:
		s = REC_RECORDING;
		break;

	case ST_DYING:
		s = REC_DYING;
		break;
		
	default:
		BUG();
	}

	return s;
}


static NmpMsgFunRet
nmp_mod_stream_query_record_status_b(NmpAppObj *app_obj, NmpSysMsg *msg)
{
	NmpModStream *self = (NmpModStream*)app_obj;
	NmpSchPool	*spool = self->spool;
	NmpQueryRecordStatus *req_info;
	NmpQueryRecordStatusRes res;
	NmpGuid guid;
	gint status;

	req_info = MSG_GET_DATA(msg);
	BUG_ON(!req_info);

	nmp_guid_generate(&guid, req_info->domain_id, req_info->guid);
	memset(&res, 0, sizeof(res));
	SET_CODE(&res, 0);
	strncpy(res.session, req_info->session, USER_NAME_LEN - 1);
	nmp_spool_get_record_status(spool, &guid, &status, &res.status_code);
	res.status = nmp_mod_stream_map_status(status);

	nmp_sysmsg_set_private_2(msg, &res, sizeof(res));
	MSG_SET_RESPONSE(msg);
	MSG_SET_DSTPOS(msg, BUSSLOT_POS_CMS);
	return MFR_DELIVER_BACK;
}


void
nmp_mod_stream_register_msg_handler(NmpModStream *self)
{
	NmpAppMod *super_self = (NmpAppMod*)self;

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_URI,
        NULL,
        nmp_mod_stream_uri_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_START_RECORD,
        NULL,
        nmp_mod_stream_start_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_STOP_RECORD,
        NULL,
        nmp_mod_stream_stop_record_b,
        0
    );

    nmp_app_mod_register_msg(
        super_self,
        MESSAGE_QUERY_RECORD_STATUS,
        NULL,
        nmp_mod_stream_query_record_status_b,
        0
    );
}


//:~ End
