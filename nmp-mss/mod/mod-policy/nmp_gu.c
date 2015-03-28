#include <string.h>
#include "nmp_gu.h"
#include "nmp_timer.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_guid.h"
#include "nmp_clock.h"

#define TICK_PER_SEC							2
#define POLICY_TOKENS							16
#define TRIG_DRIFT								5

enum
{
	GU_STAT_INIT,
	GU_STAT_FLUSH,
	GU_STAT_NOMAL,
	GU_STAT_KILL,
	GU_SATE_DEAD			/* waiting for memory free */
};

enum
{
	TRIG_TYPE_ALARM,
	TRIG_TYPE_MANUAL
};

#define STAT_FLUSH_INTERVAL						10
#define STAT_NORMAL_INTERVAL					(5*60)


typedef struct __NmpGuState NmpGuState;
struct __NmpGuState
{
	gint request_interval;	/* tick */
	gint request_left;
};

static gint dump_policy = 0;
void nmp_policy_pool_dump(NmpGuPool *p);


static guint
nmp_str_hash(const gchar *start, gsize len)
{
    guint g, h = 0;
    const gchar *end;
 
    end = start + len;
    while (start < end)
    {
        h = (h << 4) + *start;
        ++start;
 
        if ((g = (h & 0xF0000000)))
        {
            h ^= (g >> 24);
            h ^= g;
        }
    }
 
    return h;
}


static __inline__ void
nmp_trig_rec_free(NmpTrigRecCtl *rc)
{
	g_free(rc);
}


static __inline__ NmpTrigRecCtl *
nmp_trig_rec_new( void )
{
	return g_new0(NmpTrigRecCtl, 1);
}


static __inline__ NmpTrigRecCtl *
nmp_trig_rec_start(gint seconds)
{
	NmpTrigRecCtl *rc;

	rc = nmp_trig_rec_new();
	rc->start = nmp_clock_get_time();
	rc->lasts = seconds + TRIG_DRIFT;

	return rc;
}


static __inline__ void
nmp_trig_rec_merge(NmpTrigRecCtl *rc, gint seconds)
{
	time_t now;

	if (seconds <= 0)
		return;

	now = nmp_clock_get_time();
	if (rc->start + rc->lasts < now + seconds + TRIG_DRIFT)
	{
		rc->start = now;
		rc->lasts = seconds + TRIG_DRIFT;
	}
}


static __inline__ gint
nmp_trig_rec_check(NmpTrigRecCtl *rc)
{
	time_t now = nmp_clock_get_time();
	if (now >= rc->start && now < rc->start + rc->lasts)
		return 0;

	return -1;
}


static __inline__ NmpGu *
nmp_gu_new(NmpGuid *guid)
{
	NmpGu *gu;

	gu = g_new0(NmpGu, 1);
	nmp_guid_assign(&gu->guid, guid);
	gu->state = GU_STAT_INIT;

	return gu;
}


static void
nmp_gu_free(NmpGu *gu)
{
	g_assert(gu != NULL);

	if (gu->manual)
		nmp_trig_rec_free(gu->manual);

	if (gu->alarm)
		nmp_trig_rec_free(gu->alarm);

	if (gu->time_policy)
		nmp_delete_policy(gu->time_policy);

	g_free(gu->state_data);
	g_free(gu);
}


static guint
nmp_gu_hash_fn(gconstpointer key)
{
	NmpGuid *_key;
	g_assert(key != NULL);

	_key = (NmpGuid*)key;
	
	return nmp_str_hash(_key->guid, strlen(_key->guid));
}


static gboolean
nmp_gu_key_equal(gconstpointer a, gconstpointer b)
{
	NmpGuid *gu_a, *gu_b;

	gu_a = (NmpGuid*)a;
	gu_b = (NmpGuid*)b;

	return nmp_guid_equal(gu_a, gu_b);
}


static void
nmp_gu_pool_free_msg(NmpMsg *msg)
{
	if (!msg)
		return;

	switch (msg->id)
	{
	case EVENT_REC:
		g_free(msg);
		break;

	case EVENT_POLICY:
	case EVENT_STOP_REC:
		g_free(msg);
		break;

	default:
		g_free(msg);
		break;
	}		
}


NmpMsg *
nmp_gu_pool_make_msg(gint event, void *v1, void *v2,  void *v3)
{
	NmpRecMsg *rec_msg;
	NmpPolicyMsg *pol_msg;

	switch (event)
	{
	case EVENT_REC:
		rec_msg = g_new0(NmpRecMsg, 1);
		rec_msg->base.id = EVENT_REC;
		rec_msg->base.free = nmp_gu_pool_free_msg;
		nmp_guid_assign(&rec_msg->guid, (NmpGuid*)v1);
		rec_msg->rec_type = (gint)v2;
		rec_msg->level = (gint)v3;
		return (NmpMsg*)rec_msg;

	case EVENT_STOP_REC:
		rec_msg = g_new0(NmpRecMsg, 1);
		rec_msg->base.id = EVENT_STOP_REC;
		rec_msg->base.free = nmp_gu_pool_free_msg;
		nmp_guid_assign(&rec_msg->guid, (NmpGuid*)v1);
		return (NmpMsg*)rec_msg;

	case EVENT_POLICY:
		pol_msg = g_new0(NmpPolicyMsg, 1);
		pol_msg->base.id = EVENT_POLICY;
		pol_msg->base.free = nmp_gu_pool_free_msg;
		nmp_guid_assign(&pol_msg->guid, (NmpGuid*)v1);
		return (NmpMsg*)pol_msg;

	default:
		break;
	}

	return NULL;
}


static __inline__ void
nmp_gu_pool_gen_event(NmpGuPool *p, gint event, void *v1,
	void *v2, void *v3)
{
	NmpMsg *msg = nmp_gu_pool_make_msg(event, v1, v2, v3);
	if (msg)
	{
		if (p->ev_handler)
			(*p->ev_handler)(p->ev_private, msg);
		else
			FREE_MSG(msg);
	}
}


static __inline__ gint
nmp_gu_state_enter(NmpGu *gu, gint interval_secs)
{
	NmpGuState *sd = (NmpGuState*)gu->state_data;

	if (!sd)
	{
		sd = g_new0(NmpGuState, 1);
		gu->state_data = sd;
		sd->request_interval = interval_secs * TICK_PER_SEC;
		sd->request_left = 0;
		return 1;	
	}

	return 0;
}


static __inline__ void
nmp_jump_gu_state(NmpGu *gu, gint state)
{
	if (gu->state != state)
	{
		g_free(gu->state_data);
		gu->state_data = NULL;
		gu->state = state;
	}
}


static __inline__ gint
nmp_get_request_token(NmpGuPool *p)
{
	if (--p->policy_tokens >= 0)
		return 0;
	return -EAGAIN;
}


static __inline__ void
nmp_check_gu_state_init(NmpGuPool *p, NmpGu *gu)
{
	nmp_jump_gu_state(gu, GU_STAT_FLUSH);
}


static __inline__ void
nmp_check_gu_state_flush(NmpGuPool *p, NmpGu *gu)
{
	NmpGuState *sd;

	nmp_gu_state_enter(gu, STAT_FLUSH_INTERVAL);
	sd = (NmpGuState*)gu->state_data;

	if (gu->time_policy && !gu->policy_stale)
	{
		nmp_jump_gu_state(gu, GU_STAT_NOMAL);
		return;
	}

	if (--sd->request_left <= 0 && !nmp_get_request_token(p))
	{
		sd->request_left = sd->request_interval;
		nmp_gu_pool_gen_event(p, EVENT_POLICY, &gu->guid, NULL, NULL);
	}
}


static __inline__ void
__nmp_check_gu_state_normal(NmpGuPool *p, NmpGu *gu)
{
	NmpGuState *sd;

	nmp_gu_state_enter(gu, STAT_NORMAL_INTERVAL);
	sd = (NmpGuState*)gu->state_data;

	if (gu->policy_stale && !nmp_get_request_token(p))
	{
		//nmp_jump_gu_state(gu, GU_STAT_FLUSH);
		sd->request_left = sd->request_interval;
		nmp_gu_pool_gen_event(p, EVENT_POLICY, &gu->guid, NULL, NULL);
		return;
	}

	if (--sd->request_left <= 0 && !nmp_get_request_token(p))
	{
		sd->request_left = sd->request_interval;
		nmp_gu_pool_gen_event(p, EVENT_POLICY, &gu->guid, NULL, NULL);
	}
}


static __inline__ void
nmp_check_gu_state_normal(NmpGuPool *p, NmpGu *gu)
{
	gint rec_type = 0;
	gint ts_rec;

	BUG_ON(gu->time_policy == (NmpPolicy*)-1);	/* fixme: remove */
	ts_rec = nmp_policy_check(gu->time_policy);
	if (ts_rec&TS_REC_TYPE_AUTO)
		rec_type |= REC_TYPE_AUTO;

	if (gu->alarm)
	{
		if (!nmp_trig_rec_check(gu->alarm))
			rec_type |= REC_TYPE_ALRM;
		else
		{
			nmp_trig_rec_free(gu->alarm);
			gu->alarm = NULL;
		}
	}

	if (gu->manual)
	{
		if (!nmp_trig_rec_check(gu->manual))
			rec_type |= REC_TYPE_MANU;
		else
		{
			nmp_trig_rec_free(gu->manual);
			gu->alarm = NULL;
		}	
	}

	if (gu->rec_state)
	{
		if (rec_type)
		{
			if (gu->last_level != gu->level)
			{
				nmp_gu_pool_gen_event(p, EVENT_STOP_REC, &gu->guid,
					NULL, NULL);
				gu->rec_state = 0;
				gu->last_level = gu->level;
			}
			else
			{
				if (gu->rec_state != rec_type)
				{
					nmp_gu_pool_gen_event(p, EVENT_REC, &gu->guid,
						(void*)rec_type, (void*)gu->last_level);
					gu->rec_state = rec_type;			
				}
			}
		}
		else
		{
			nmp_gu_pool_gen_event(p, EVENT_STOP_REC, &gu->guid,
				NULL, NULL);
			gu->rec_state = 0;
			gu->last_level = gu->level;
		}
	}
	else
	{
		if (rec_type)
		{
			gu->last_level = gu->level;
			nmp_gu_pool_gen_event(p, EVENT_REC, &gu->guid,
				(void*)rec_type, (void*)gu->last_level);
			gu->rec_state = rec_type;
		}
	}

	__nmp_check_gu_state_normal(p, gu);
}


static __inline__ void
nmp_check_gu_state_kill(NmpGuPool *p, NmpGu *gu)
{
	if (gu->rec_state)
	{
		nmp_gu_pool_gen_event(p, EVENT_STOP_REC, &gu->guid,
			NULL, NULL);
	}

	nmp_jump_gu_state(gu, GU_SATE_DEAD);
	++p->dead_gus;
}


static __inline__ void
nmp_check_gu(NmpGuPool *p, NmpGu *gu)
{
	switch (gu->state)
	{
	case GU_STAT_INIT:
		nmp_check_gu_state_init(p, gu);
		break;

	case GU_STAT_FLUSH:
		nmp_check_gu_state_flush(p, gu);
		break;

	case GU_STAT_NOMAL:
		nmp_check_gu_state_normal(p, gu);
		break;

	case GU_STAT_KILL:
		nmp_check_gu_state_kill(p, gu);
		break;

	case GU_SATE_DEAD:
		break;

	default:
		BUG();
		break;
	}
}


static void
nmp_gu_timer_check(gpointer key, gpointer value,
	gpointer user_data)
{
	NmpGuPool *p;
	NmpGu *gu;
	g_assert(key == value);

	p = (NmpGuPool*)user_data;
	gu = (NmpGu*)value;

	if (p->ready)
	{
		nmp_check_gu(p, gu);
	}
}


static gboolean
nmp_gu_is_state_dead(gpointer key, gpointer value, gpointer user_data)
{
	NmpGuPool *p = (NmpGuPool*)user_data;
	NmpGu *gu = (NmpGu*)key;

	if (gu->state == GU_SATE_DEAD)
	{
		--p->dead_gus;
		return TRUE;
	}
	return FALSE;
}


static __inline__ void
__nmp_policy_pool_check(NmpGuPool *p)
{
	p->policy_tokens = POLICY_TOKENS;

	g_hash_table_foreach(p->hash_table,
					    nmp_gu_timer_check,
					    p);

	if (p->dead_gus > 0)
	{
		g_hash_table_foreach_remove(p->hash_table,
					    			nmp_gu_is_state_dead,
					    			p);	
	}
}


static gboolean
nmp_on_pool_timer(gpointer user_data)
{
	NmpGuPool *p = (NmpGuPool*)user_data;

	g_mutex_lock(p->mutex);

	if (G_LIKELY(!dump_policy))
		__nmp_policy_pool_check(p);
	else
	{
		dump_policy = 0;
		nmp_policy_pool_dump(p);
	}

	g_mutex_unlock(p->mutex);

	return TRUE;
}


NmpGuPool *
nmp_new_gu_pool( void )
{
	NmpGuPool *p;

	p = g_new0(NmpGuPool, 1);
	p->hash_table = g_hash_table_new_full(nmp_gu_hash_fn,
										  nmp_gu_key_equal,
										  NULL,
										  (GDestroyNotify)nmp_gu_free);

	p->mutex = g_mutex_new();
	p->timer = nmp_set_timer(1000/TICK_PER_SEC, nmp_on_pool_timer, p);

	return p;
}


gint
nmp_gu_pool_set_ready(NmpGuPool *p)
{
	G_ASSERT(p != NULL);

	p->ready = 1;
}


static __inline__ NmpGu *
__nmp_gu_pool_find_gu(NmpGuPool *p, NmpGuid *guid)
{
	return g_hash_table_lookup(p->hash_table, guid);
}


static __inline__ gint
__nmp_gu_pool_add_gu(NmpGuPool *p, NmpGuid *guid)
{
	NmpGu *g;

	g = __nmp_gu_pool_find_gu(p, guid);
	if (g)
	{
		nmp_jump_gu_state(g, GU_STAT_FLUSH);
		return 0;
	}

	g = nmp_gu_new(guid);

	g_hash_table_insert(p->hash_table,
					    &g->guid,
					    g);
	return 0;
}


gint
nmp_gu_pool_add_gu(NmpGuPool *p, NmpGuid *guid)
{
	gint ret;
	g_assert(p != NULL && guid != NULL);

	g_mutex_lock(p->mutex);
	ret = __nmp_gu_pool_add_gu(p, guid);
	g_mutex_unlock(p->mutex);

	return ret;
}


static __inline__ gint
__nmp_gu_pool_del_gu(NmpGuPool *p, NmpGuid *guid)
{
	NmpGu *g;

	g = __nmp_gu_pool_find_gu(p, guid);
	if (g)
	{
		nmp_jump_gu_state(g, GU_STAT_KILL);
		return 0;
	}

	return -ENOENT;
}


gint
nmp_gu_pool_del_gu(NmpGuPool *p, NmpGuid *guid)
{
	gint ret;
	g_assert(p != NULL && guid != NULL);

	g_mutex_lock(p->mutex);
	ret = __nmp_gu_pool_del_gu(p, guid);
	g_mutex_unlock(p->mutex);

	return ret;
}


static __inline__ gint
__nmp_gu_pool_add_policy(NmpGuPool *p, NmpGuid *guid, gint level,
	NmpPolicy *policy)
{
	NmpGu *g;

	g = __nmp_gu_pool_find_gu(p, guid);
	if (g)
	{
		if (g->state == GU_STAT_FLUSH || g->state == GU_STAT_NOMAL)
		{
			if (g->time_policy)
				nmp_delete_policy(g->time_policy);
			BUG_ON(policy == (NmpPolicy*)-1);	/* fixme: remove */
			g->level = level;
			g->time_policy = policy;
			g->policy_stale = 0;
			return 0;
		}

		return -EINVAL;
	}

	return -ENOENT;	
}


gint
nmp_gu_pool_add_policy(NmpGuPool *p, NmpGuid *guid, gint level,
	NmpPolicy *policy)
{
	gint ret;
	g_assert(p && guid && policy);

	g_mutex_lock(p->mutex);
	ret = __nmp_gu_pool_add_policy(p, guid, level, policy);
	g_mutex_unlock(p->mutex);

	return ret;
}


static __inline__ gint
__nmp_gu_pool_flush_policy(NmpGuPool *p, NmpGuid *guid)
{
	NmpGu *g;

	g = __nmp_gu_pool_find_gu(p, guid);
	if (g)
	{
		g->policy_stale = 1;
		return 0;
	}

	return -ENOENT;		
}


gint
nmp_gu_pool_flush_policy(NmpGuPool *p, NmpGuid *guid)
{
	gint ret;
	g_assert(p && guid);

	g_mutex_lock(p->mutex);
	ret = __nmp_gu_pool_flush_policy(p, guid);
	g_mutex_unlock(p->mutex);

	return ret;
}


static void
nmp_gu_pool_flush_one(gpointer key, gpointer value, gpointer user_data)
{
	NmpGu *gu;
	g_assert(key == value);

	gu = (NmpGu*)value;
	gu->policy_stale = 1;
}


static __inline__ gint
__nmp_gu_pool_flush_all_policy(NmpGuPool *p)
{
	g_hash_table_foreach(p->hash_table, nmp_gu_pool_flush_one, p);
	return 0;
}


gint
nmp_gu_pool_flush_all_policy(NmpGuPool *p)
{
	gint ret;

	g_mutex_lock(p->mutex);
	ret = __nmp_gu_pool_flush_all_policy(p);
	g_mutex_unlock(p->mutex);

	return ret;
}


static __inline__ gint
__nmp_gu_pool_get_record_status(NmpGuPool *p, NmpGuid *guid,
	gint *recording)
{
	NmpGu *g;

	if (!p->ready)
	{
		return -EAGAIN;
	}

	g = __nmp_gu_pool_find_gu(p, guid);
	if (g)
	{
		*recording = g->rec_state;
		return 0;
	}

	return -ENOENT;
}


gint
nmp_gu_pool_get_record_status(NmpGuPool *p, NmpGuid *guid,
	gint *recording)
{
	gint ret;
	g_assert(p && guid && recording);

	g_mutex_lock(p->mutex);
	ret = __nmp_gu_pool_get_record_status(p, guid, recording);
	g_mutex_unlock(p->mutex);

	return ret;	
}


void
nmp_gu_pool_set_handler(NmpGuPool *p, NmpEventHandler handler,
	gpointer priv)
{
	g_assert(p != NULL);

	p->ev_handler = handler;
	p->ev_private = priv;
}


static void
nmp_on_sigusr_1(int sig)
{
	dump_policy = 1;
}


void
nmp_sig_setup_sigusr1( void )
{
	signal(SIGUSR1, nmp_on_sigusr_1);
}


static __inline__ gint
nmp_pu_pool_start_trig(NmpGuPool *p, NmpGuid *guid, gint seconds,
	gint type)
{
	gint ret;
	NmpGu *g;
	NmpTrigRecCtl **prc;

	ret = -ENOENT;
	g_mutex_lock(p->mutex);
	g = __nmp_gu_pool_find_gu(p, guid);
	if (g)
	{
		prc = (type == TRIG_TYPE_ALARM) ? &g->alarm : &g->manual;
		if (*prc)
		{
			nmp_trig_rec_merge(*prc, seconds);
		}
		else
		{
			*prc = nmp_trig_rec_start(seconds);
		}

		ret = 0;
	}
	g_mutex_unlock(p->mutex);

	return ret;
}


gint
nmp_gu_pool_start_alarm(NmpGuPool *p, NmpGuid *guid, gint seconds)
{
	g_assert(p && guid);

	return nmp_pu_pool_start_trig(p, guid, seconds, TRIG_TYPE_ALARM);
}


gint
nmp_gu_pool_start_manual(NmpGuPool *p, NmpGuid *guid, gint seconds)
{
	g_assert(p && guid);

	return nmp_pu_pool_start_trig(p, guid, seconds, TRIG_TYPE_MANUAL);
}


//:~ End
