#include "nmp_sch.h"
#include "nmp_afx.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_utility.h"


#define REQUEST_INIT_INTERVAL		10
#define REQUEST_URI_INTERVAL		10
#define REQUEST_STM_INTERVAL		15
#define REQUEST_NORMAL_INTERVAL		15

#define STM_TIMEOUT_INTERVAL		10

#define JXJ_MANUFACTORY				"JXJ"
#define JXJ_MANUFACTORY_2			"ENC"
#define MAX_GUID_BITS				24
#define INDEX_OF_LEVEL				20

#define URI_TOKENS		5
#define STM_TOKENS		5

#define STRANGER_PRI_INIT	0

enum
{
	SCH_STAT_INIT,	//@{Added, without facility initializing}
	SCH_STAT_URI,	//@{Requesting URI}
	SCH_STAT_STM,	//${Requesting Stream}
	SCH_STAT_NORMAL,	//@{Recording}
	SCH_STAT_KILL,	//@{To be killed, without facility destructing}
	SCH_STAT_DEAD	//@{To be freed}
};


typedef struct __NmpSchState NmpSchState;
struct __NmpSchState
{
	gint request_interval;	/* tick */
	gint request_left;		/* tick */
};


static guint
nmp_sch_hash_fn(gconstpointer key)
{
	NmpGuid *guid;
	g_assert(key != NULL);

	guid = (NmpGuid*)key;
	return nmp_str_hash_pjw(guid->guid, strlen(guid->guid));
}


static gboolean
nmp_sch_key_equal(gconstpointer a, gconstpointer b)
{
	NmpGuid *guid_1, *guid_2;

	guid_1 = (NmpGuid*)a;
	guid_2 = (NmpGuid*)b;

	return nmp_guid_equal(guid_1, guid_2);
}


static __inline__ void
nmp_sch_guid_to_main_level(NmpGuid *guid)
{
	gint len;

	len = strlen(guid->guid);
	if (len < MAX_GUID_BITS) /* 24BITS */
	{
		nmp_warning(
			"nmp_sch_guid_to_main_level('%s') failed.",
			guid->guid
		);
		return;
	}

	guid->guid[INDEX_OF_LEVEL] = '0';
}


static __inline__ void
nmp_sch_guid_set_level(NmpGuid *guid, gint level)
{
	gint len;

	len = strlen(guid->guid);
	if (len < MAX_GUID_BITS) /* 24BITS */
	{
		nmp_warning(
			"nmp_sch_guid_set_level('%s', %d) failed.",
			guid->guid,
			level
		);
		return;
	}

	guid->guid[INDEX_OF_LEVEL] = '0' + level;
}


static __inline__ NmpSCh *
nmp_sch_new(NmpGuid *guid)
{
	NmpSCh *sch;

	sch = g_new0(NmpSCh, 1);
	nmp_guid_assign(&sch->guid, guid);
	sch->ref_count = 1;
	sch->state = SCH_STAT_INIT;

	if (memcmp(sch->guid.guid, JXJ_MANUFACTORY, 3) &&
		memcmp(sch->guid.guid, JXJ_MANUFACTORY_2, 3))
	{
		sch->stranger = 1;
		sch->stranger_pri = STRANGER_PRI_INIT;
	}

	sch->mutex = g_mutex_new();

	return sch;
}


static __inline__ void
nmp_sch_clear(NmpSCh *sch)
{
	if (sch->initialized)
	{
		if (sch->sch_ops->fin)
		{
			(*sch->sch_ops->fin)(sch);
		}
	}

	g_mutex_free(sch->mutex);

	if (sch->sb)
		nmp_sbuff_kill_unref(sch->sb);

	if (sch->uri)
		g_free(sch->uri);
}


static __inline__ void
nmp_sch_free(NmpSCh *sch)
{
	nmp_print(
		"sch '%s,%s' freed.",
		sch->guid.domain_id,
		sch->guid.guid
	);

	g_free(sch);
}


void
nmp_sch_unref(NmpSCh *sch)
{
	if (g_atomic_int_dec_and_test(&sch->ref_count))
	{
		nmp_sch_clear(sch);
		nmp_sch_free(sch);
	}
}


static __inline__ void
nmp_sch_jump_state(NmpSCh *sch, gint state)
{
	if (state != sch->state)
	{
		sch->state = state;
		g_free(sch->state_data);
		sch->state_data = NULL;
		sch->err_code = 0;
	}
}


static __inline__ void
nmp_sch_set_uri(NmpSCh *sch, gchar *uri)
{
	g_free(sch->uri);
	sch->uri = g_strdup(uri);
}


static void
nmp_sch_free_msg(NmpMsg *msg)
{
	if (!msg)
		return;

	switch (msg->id)
	{
	case EVENT_URI:
		g_free(((NmpUriMsg*)msg)->uri);
		g_free(msg);
		break;

	default:
		g_free(msg);
		break;
	}
}


static NmpMsg *
nmp_sch_gen_event(gpointer pool, gint event, void *v1,
	void *v2, void *v3)
{
	NmpUriMsg *uri_msg;

	switch (event)
	{
	case EVENT_URI:
		uri_msg = g_new0(NmpUriMsg, 1);
		uri_msg->base.id = EVENT_URI;
		uri_msg->base.free = nmp_sch_free_msg;
		nmp_guid_generate(&uri_msg->guid, (gchar*)v1, (gchar*)v2);
		nmp_sch_guid_set_level(&uri_msg->guid, (gint)v3);
		return (NmpMsg*)uri_msg;

	default:
		break;
	}

	return NULL;
}


static __inline__ gint
jfp_sch_try_init(NmpSCh *sch)
{
	gint ret;

	if (sch->initialized)
		return 0;

	if (sch->sch_ops->init)
	{
		ret = (*sch->sch_ops->init)(sch);
		if (ret)
			return 1;
	}

	sch->sb = nmp_sbuff_new(&sch->guid, 0, 1);	/* hd_group 1 */
	if (!sch->sb)
	{
		if (sch->sch_ops->fin)
		{
			(*sch->sch_ops->fin)(sch);
		}

		return 1;
	}

	sch->initialized = 1;
	return 0;
}


static __inline__ void
nmp_sch_init(NmpSCh *sch)
{
	if (!jfp_sch_try_init(sch))
		nmp_sch_jump_state(sch, SCH_STAT_URI);
}


static __inline__ gint
nmp_sch_state_enter(NmpSCh *sch, gint interval_secs)
{
	NmpSchState *sd = (NmpSchState*)sch->state_data;

	if (!sd)
	{
		sd = g_new0(NmpSchState, 1);
		sch->state_data = sd;
		sd->request_interval = interval_secs * TICK_PER_SECOND;
		sd->request_left = 0;
		return 1;	
	}

	return 0;
}


static __inline__ void
nmp_check_sch_state_init(NmpSchPool *sp, NmpSCh *sch)
{
	NmpSchState *sd;

	nmp_sch_state_enter(sch, REQUEST_INIT_INTERVAL);
 	sd = (NmpSchState*)sch->state_data;

	if (--sd->request_left <= 0)
	{
		sd->request_left = sd->request_interval;
		nmp_sch_init(sch);
	}
}


static __inline__ void
nmp_check_sch_state_uri(NmpSchPool *sp, NmpSCh *sch)
{
	NmpSchState *sd;
	gint first_enter;

	first_enter = nmp_sch_state_enter(sch, REQUEST_URI_INTERVAL);
	if (first_enter)
	{
		g_free(sch->uri);
		sch->uri = NULL;
	}

	if (sch->uri)
	{
		nmp_sch_jump_state(sch, SCH_STAT_NORMAL);
		return;
	}

 	sd = (NmpSchState*)sch->state_data;
	if (--sd->request_left <= 0 && !nmp_spool_get_uri_token(sp))
	{
		sd->request_left = sd->request_interval;
		nmp_pool_do_event((NmpPool*)sp, EVENT_URI, sch->guid.domain_id,
			sch->guid.guid, (void*)sch->level);
	}
}


gint
nmp_sch_set_rec_state(NmpSCh *sch)
{
	sch->recording = STM_TIMEOUT_INTERVAL * TICK_PER_SECOND;
	return 0;
}


static __inline__ void
__nmp_sch_normal_request(NmpSchPool *sp, NmpSCh *sch)
{
	NmpSchState *sd = (NmpSchState*)sch->state_data;
 
	if (sch->err_code == EC_STM_NOT_FOUND)
	{
		nmp_sch_jump_state(sch, SCH_STAT_URI);
		return;
	}

	if (!nmp_spool_get_stm_token(sp))
	{
		sd->request_left = sd->request_interval;
		if (sch->sch_ops->open_stream)
		{
			gint ret = (*sch->sch_ops->open_stream)(sch);
			if (ret)
			{
				nmp_print(
					"GU '%s,%s' open stream failed, err:%d.",
					sch->guid.domain_id,
					sch->guid.guid,
					ret
				);
			}
		}
	}
}


static __inline__ void
__nmp_sch_normal_chk_timeout(NmpSchPool *sp, NmpSCh *sch)
{
	NmpSchState *sd = (NmpSchState*)sch->state_data;

	sd->request_left = 0;
	if (sch->recording > 0)
		--sch->recording;
	if (sch->recording <= 0)
	{
		sch->err_code = EC_STM_NOT_FOUND;
		if (sch->sb)
		{
			nmp_sbuff_pause(sch->sb);
		}
	}
}


static __inline__ void
__nmp_sch_state_normal(NmpSchPool *sp, NmpSCh *sch)
{
	if (!sch->recording)
	{
		__nmp_sch_normal_request(sp, sch);
	}
	else
	{
		__nmp_sch_normal_chk_timeout(sp, sch);
	}
}


static __inline__ void
nmp_check_sch_state_normal(NmpSchPool *sp, NmpSCh *sch)
{
	NmpSchState *sd;

	nmp_sch_state_enter(sch, REQUEST_NORMAL_INTERVAL);
 	sd = (NmpSchState*)sch->state_data;

	if (--sd->request_left <= 0)
	{
		__nmp_sch_state_normal(sp, sch);
	}
}


static __inline__ void
nmp_check_sch_state_kill(NmpSchPool *sp, NmpSCh *sch)
{
	nmp_print(
		"GU '%s, %s' stream killed.",
		sch->guid.domain_id,
		sch->guid.guid
	);

	if (sch->sch_ops->close_stream)
	{
		(*sch->sch_ops->close_stream)(sch);
	}

	if (sch->sb)
	{
		nmp_sbuff_kill_unref(sch->sb);
		sch->sb = NULL;
	}

	nmp_sch_jump_state(sch, SCH_STAT_DEAD);
	++((NmpPool*)sp)->garbage;
}


static __inline__ void
nmp_check_sch_locked(NmpSchPool *sp, NmpSCh *sch)
{
	switch (sch->state)
	{
	case SCH_STAT_INIT:
		nmp_check_sch_state_init(sp, sch);
		break;

	case SCH_STAT_URI:
		nmp_check_sch_state_uri(sp, sch);
		break;

	case SCH_STAT_NORMAL:
		nmp_check_sch_state_normal(sp, sch);
		break;

	case SCH_STAT_KILL:
		nmp_check_sch_state_kill(sp, sch);
		break;

	case SCH_STAT_DEAD:
		break;

	default:
		break;
	}
}


static void
nmp_sch_check_element(gpointer key, gpointer value,
	gpointer user_data)
{
	NmpSchPool *sp;
	NmpSCh *sch;
	g_assert(key == value);

	sp = (NmpSchPool*)user_data;
	sch = (NmpSCh*)value;

	g_mutex_lock(sch->mutex);
	nmp_check_sch_locked(sp, sch);
	g_mutex_unlock(sch->mutex);	
}


static gboolean
nmp_sch_is_garbage(gpointer key, gpointer value, gpointer pool)
{
	NmpSCh *sch = (NmpSCh*)value;
//	NmpSchPool *sp = (NmpSchPool*)pool;

	if (sch->state == SCH_STAT_DEAD)
		return TRUE;

	return FALSE;
}


static void
nmp_sch_pre_check(gpointer pool)
{
	NmpSchPool *sp = (NmpSchPool*)pool;

	sp->uri_tokens = URI_TOKENS;
	sp->stm_tokens = STM_TOKENS;
}


static NmpPoolOps g_sch_pool_ops =
{
	.hash_fn = nmp_sch_hash_fn,
	.key_equal = nmp_sch_key_equal,
	.value_destroy = (void (*)(gpointer))nmp_sch_unref,

	.check_element = nmp_sch_check_element,
	.garbage_collect = nmp_sch_is_garbage,

	.make_msg = nmp_sch_gen_event,
	.pre_check = nmp_sch_pre_check	
};


NmpSchPool *
nmp_spool_new(gint freq_msec, NmpSchOps *ops)
{
	NmpSchPool *sp;

	if (!ops)
		return NULL;

	sp = (NmpSchPool*)nmp_pool_new(sizeof(NmpSchPool),
		freq_msec, &g_sch_pool_ops);
	if (sp)
	{
		sp->sch_ops = ops;
	}

	return sp;
}


static __inline__ NmpSCh *
nmp_spool_find(NmpSchPool *sp, NmpGuid *guid)
{
	G_ASSERT(sp && guid);

	return (NmpSCh*)nmp_pool_find_nolock((NmpPool*)sp, guid);
}


gint
nmp_spool_add_sch(NmpSchPool *sp, NmpGuid *guid,
	gint rec, gint hd_grp, gint level)
{
	NmpSCh *sch;
	gint ret = 0;
	G_ASSERT(sp && guid);

	nmp_pool_require_lock((NmpPool*)sp);

	sch = nmp_spool_find(sp, guid);
	if (sch)
	{
		sch->level = level;
		sch->rec_type = rec;
		sch->local_flags = hd_grp;

		if (sch->state == SCH_STAT_KILL)
			nmp_sch_jump_state(sch, SCH_STAT_INIT);
	}
	else
	{
		sch = nmp_sch_new(guid);
		sch->sch_ops = sp->sch_ops;
		sch->level = level;
		sch->rec_type = rec;
		sch->local_flags = hd_grp;
		sch->pool = sp;
		nmp_pool_add_nolock((NmpPool*)sp, &sch->guid, sch);
	}

	nmp_pool_release_lock((NmpPool*)sp);

	return ret;
}


gint
nmp_spool_del_sch(NmpSchPool *sp, NmpGuid *guid)
{
	NmpSCh *sch;
	gint ret = -ENOENT;
	G_ASSERT(sp && guid);

	nmp_pool_require_lock((NmpPool*)sp);

	sch = nmp_spool_find(sp, guid);
	if (sch)
	{
		nmp_sch_jump_state(sch, SCH_STAT_KILL);
		ret = 0;
	}

	nmp_pool_release_lock((NmpPool*)sp);

	return ret;
}


gint
nmp_spool_add_sch_uri(NmpSchPool *sp, NmpGuid *guid,
	gchar *uri)
{
	NmpSCh *sch;
	gint ret = -ENOENT;
	G_ASSERT(sp && guid);

	nmp_sch_guid_to_main_level(guid);

	nmp_pool_require_lock((NmpPool*)sp);

	sch = nmp_spool_find(sp, guid);
	if (sch)
	{
		if (sch->state == SCH_STAT_URI || sch->state == SCH_STAT_NORMAL)
			nmp_sch_set_uri(sch, uri);
	}

	nmp_pool_release_lock((NmpPool*)sp);

	return ret;	
}


gint
nmp_spool_get_uri_token(NmpSchPool *sp)
{
	G_ASSERT(sp);

	if (sp->uri_tokens <= 0)
		return -EAGAIN;

	--sp->uri_tokens;
	return 0;
}


gint
nmp_spool_get_stm_token(NmpSchPool *sp)
{
	G_ASSERT(sp);

	if (sp->stm_tokens <= 0)
		return -EAGAIN;

	--sp->stm_tokens;
	return 0;
}


static __inline__ void
__nmp_spool_get_record_status(NmpSchPool *sp, NmpGuid *guid,
	gint *status, gint *status_code)
{
	NmpSCh *sch;

	sch = nmp_spool_find(sp, guid);
	if (sch)
	{
		switch (sch->state)
		{
		case SCH_STAT_INIT:
			*status = ST_PREPARE;
			*status_code = 0;
			break;

		case SCH_STAT_URI:
			*status = ST_URL;
			*status_code = 0;
			break;

		case SCH_STAT_STM:
			*status = ST_REQUEST;
			*status_code = 0;
			break;

		case SCH_STAT_NORMAL:
			if (sch->recording)
			{
				*status = ST_RECORD;
				*status_code = 0;
			}
			else
			{
				*status = ST_REQUEST;
				*status_code = sch->err_code;
			}
			break;

		case SCH_STAT_KILL:
		case SCH_STAT_DEAD:
			*status = ST_DYING;
			*status_code = sch->err_code;
			break;

		default:
			*status = ST_PREPARE;
			*status_code = 0;			
			break;
		}
	}
	else
	{
		*status = ST_PREPARE;
		*status_code = 0;
	}
}


void
nmp_spool_get_record_status(NmpSchPool *sp, NmpGuid *guid,
	gint *status, gint *status_code)
{
	G_ASSERT(sp && guid && status && status_code);

	nmp_pool_require_lock((NmpPool*)sp);
	__nmp_spool_get_record_status(sp, guid, status, status_code);
	nmp_pool_release_lock((NmpPool*)sp);
}


//:~ End
