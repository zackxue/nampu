#include "nmp_mds.h"
#include "nmp_afx.h"
#include "nmp_errno.h"
#include "nmp_debug.h"
#include "nmp_utility.h"

#define INIT_INTERVAL			10
#define REQUEST_INTERVAL		10
#define NORMAL_INTERVAL			10
#define CONN_TOKENS		5

enum
{
	MDS_STAT_INIT,
	MDS_STAT_IP,
	MDS_STAT_NORMAL,			/* ok */
	MDS_STAT_KILL,
	MDS_STAT_DEAD
};


typedef struct __NmpMdsState NmpMdsState;
struct __NmpMdsState
{
	gint request_interval;	/* tick */
	gint request_left;
};


static guint
nmp_mds_hash_fn(gconstpointer id)
{
	gchar *str = (gchar*)id;
	return nmp_str_hash_pjw(str, strlen(str));
}


static gboolean
nmp_mds_key_equal(gconstpointer a, gconstpointer b)
{
	gchar *skey1, *skey2;

	skey1 = (gchar*)a;
	skey2 = (gchar*)b;

	return !strcmp(skey1, skey2);
}


static __inline__ NmpMds *
nmp_mds_new(gchar *id)
{
	NmpMds *mds;

	mds = g_new0(NmpMds, 1);
	mds->id = g_strdup(id);
	mds->ref_count = 1;
	mds->state = MDS_STAT_INIT;
	mds->mutex = g_mutex_new();

	return mds;
}


static __inline__ void
nmp_mds_clear(NmpMds *mds)
{
	if (mds->initialized)
	{
		if (mds->mds_ops->fin)
		{
			(*mds->mds_ops->fin)(mds);
		}
	}

	g_mutex_free(mds->mutex);

	g_free(mds->ip);
	g_free(mds->id);
}


static __inline__ void
nmp_mds_free(NmpMds *mds)
{
	g_free(mds);
}


void
nmp_mds_ref(NmpMds *mds)
{
	G_ASSERT(g_atomic_int_get(&mds->ref_count) > 0);

	g_atomic_int_add(&mds->ref_count, 1);	
}


void
nmp_mds_unref(NmpMds *mds)
{
	G_ASSERT(g_atomic_int_get(&mds->ref_count) > 0);

	if (g_atomic_int_dec_and_test(&mds->ref_count))
	{
		nmp_mds_clear(mds);
		nmp_mds_free(mds);
	}
}


static __inline__ void
nmp_mds_jump_state(NmpMds *mds, gint state)
{
	if (state != mds->state)
	{
		mds->state = state;
		g_free(mds->state_data);
		mds->state_data = NULL;
	}
}


static __inline__ void
nmp_mds_set_ip(NmpMds *mds, gchar *ip, gint port)
{
	g_free(mds->ip);
	mds->ip = g_strdup(ip);
	mds->port = port;
}


static void
nmp_mds_free_msg(NmpMsg *msg)
{
	if (!msg)
		return;

	switch (msg->id)
	{
	case EVENT_MDSIP:
		g_free(((NmpMdsIpMsg*)msg)->mds_id);
		g_free(msg);
		break;

	default:
		g_free(msg);
		break;
	}
}


static NmpMsg *
nmp_mds_gen_event(gpointer pool, gint event, void *v1,
	void *v2, void *v3)
{
	NmpMdsIpMsg *mdsip_msg;

	switch (event)
	{
	case EVENT_MDSIP:
		mdsip_msg = g_new0(NmpMdsIpMsg, 1);
		mdsip_msg->base.id = EVENT_MDSIP;
		mdsip_msg->base.free = nmp_mds_free_msg;
		mdsip_msg->mds_id = g_strdup(v1);
		return (NmpMsg*)mdsip_msg;

	default:
		break;
	}

	return NULL;
}


static __inline__ gint
nmp_mds_state_enter(NmpMds *mds, gint interval_secs)
{
	NmpMdsState *sd = (NmpMdsState*)mds->state_data;

	if (!sd)
	{
		sd = g_new0(NmpMdsState, 1);
		mds->state_data = sd;
		sd->request_interval = interval_secs * TICK_PER_SECOND;
		sd->request_left = 0;
		return 1;
	}

	return 0;
}


static __inline__ void
nmp_mds_init(NmpMds *mds)
{
	if (mds->initialized)
		return;

	if (mds->mds_ops->init_inst)
	{
		(*mds->mds_ops->init_inst)(mds);
	}

	mds->initialized = 1;
	nmp_mds_jump_state(mds, MDS_STAT_IP);
	return;
}


static __inline__ void
nmp_check_mds_state_init(NmpMdsPool *sp, NmpMds *mds)
{
	NmpMdsState *sd;

	nmp_mds_state_enter(mds, INIT_INTERVAL);
	sd = (NmpMdsState*)mds->state_data;

	if (--sd->request_left <= 0)
	{
		sd->request_left = sd->request_interval;
		nmp_mds_init(mds);
	}
}


static __inline__ void
nmp_check_mds_state_ip(NmpMdsPool *sp, NmpMds *mds)
{
	NmpMdsState *sd;
	gint first;

	first = nmp_mds_state_enter(mds, REQUEST_INTERVAL);
	if (first)
	{
		g_free(mds->ip);
		mds->ip = NULL;
	}

	if (mds->ip)
	{
		nmp_mds_jump_state(mds, MDS_STAT_NORMAL);
		return;		
	}

	sd = (NmpMdsState*)mds->state_data;

	if (--sd->request_left <= 0 && !nmp_spool_get_conn_token(sp))
	{
		sd->request_left = sd->request_interval;
		nmp_pool_do_event((NmpPool*)sp, EVENT_MDSIP, mds->id, NULL, NULL);
	}
}


static __inline__ void
nmp_mds_normal_connected(NmpMdsPool *sp, NmpMds *mds)
{
}


static __inline__ void
nmp_mds_normal_unconnected(NmpMdsPool *sp, NmpMds *mds)
{
	gint ret = -EINVAL;

	g_atomic_int_set(&mds->connected, 1);
	if (mds->mds_ops->connect)
	{
		ret = (*mds->mds_ops->connect)(mds);
	}

	if (ret)
	{
		g_atomic_int_set(&mds->connected, 0);
		nmp_print(
			"Connect MDS '%s' failed, err:%d.", mds->id, ret
		);
	}

	/* update it */
	nmp_pool_do_event((NmpPool*)sp, EVENT_MDSIP, mds->id, NULL, NULL);
}


static __inline__ void
nmp_check_mds_state_normal(NmpMdsPool *sp, NmpMds *mds)
{
	NmpMdsState *sd;

	nmp_mds_state_enter(mds, NORMAL_INTERVAL);
	sd = (NmpMdsState*)mds->state_data;

	if (--sd->request_left > 0)
		return;

	if (g_atomic_int_get(&mds->connected))
	{
		sd->request_left = sd->request_interval;
		nmp_mds_normal_connected(sp, mds);
	}
	else
	{
		sd->request_left = sd->request_interval;
		nmp_mds_normal_unconnected(sp, mds);
	}
}


static __inline__ void
nmp_check_mds_state_kill(NmpMdsPool *sp, NmpMds *mds)
{
	nmp_print(
		"MDS '%s' connection killed.", mds->id
	);

	if (sp->mds_ops->disconnect)
	{
		(*sp->mds_ops->disconnect)(mds);
	}

	nmp_mds_jump_state(mds, MDS_STAT_DEAD);
	++((NmpPool*)sp)->garbage;
}


static __inline__ void
nmp_check_mds_locked(NmpMdsPool *sp, NmpMds *mds)
{
	switch (mds->state)
	{
	case MDS_STAT_INIT:
		nmp_check_mds_state_init(sp, mds);
		break;

	case MDS_STAT_IP:
		nmp_check_mds_state_ip(sp, mds);
		break;

	case MDS_STAT_NORMAL:
		nmp_check_mds_state_normal(sp, mds);
		break;

	case MDS_STAT_KILL:
		nmp_check_mds_state_kill(sp, mds);
		break;

	case MDS_STAT_DEAD:
		break;

	default:
		break;
	}
}


static void
nmp_mds_check_element(gpointer key, gpointer value,
	gpointer user_data)
{
	NmpMdsPool *sp;
	NmpMds *mds;

	sp = (NmpMdsPool*)user_data;
	mds = (NmpMds*)value;

	g_mutex_lock(mds->mutex);
	nmp_check_mds_locked(sp, mds);
	g_mutex_unlock(mds->mutex);	
}


static gboolean
nmp_mds_is_garbage(gpointer key, gpointer value, gpointer pool)
{
	NmpMds *mds = (NmpMds*)value;

	if (mds->state == MDS_STAT_DEAD)
		return TRUE;

	return FALSE;
}


static void
nmp_mds_pre_check(gpointer pool)
{
	NmpMdsPool *sp = (NmpMdsPool*)pool;

	sp->con_tokens = CONN_TOKENS;
}


static NmpPoolOps g_mds_pool_ops =
{
	.hash_fn = nmp_mds_hash_fn,
	.key_equal = nmp_mds_key_equal,
	.value_destroy = (void (*)(gpointer))nmp_mds_unref,

	.check_element = nmp_mds_check_element,
	.garbage_collect = nmp_mds_is_garbage,

	.make_msg = nmp_mds_gen_event,
	.pre_check = nmp_mds_pre_check	
};


NmpMdsPool *
nmp_spool_mds_new(gint freq_msec, NmpMdsOps *ops)
{
	NmpMdsPool *sp;

	if (!ops)
		return NULL;

	sp = (NmpMdsPool*)nmp_pool_new(sizeof(NmpMdsPool),
		freq_msec, &g_mds_pool_ops);
	if (sp)
	{
		sp->mds_ops = ops;
		if (ops->init_pool)
		{
			if ((*ops->init_pool)(sp))
				FATAL_ERROR_EXIT;
		}
	}

	return sp;
}


static __inline__ NmpMds *
nmp_spool_find(NmpMdsPool *sp, gchar *id)
{
	return (NmpMds*)nmp_pool_find_nolock((NmpPool*)sp, id);
}


gint
nmp_spool_add_mds(NmpMdsPool *sp, gchar *id)
{
	NmpMds *mds;
	gint ret = -EEXIST;
	G_ASSERT(sp && id);

	nmp_pool_require_lock((NmpPool*)sp);

	mds = nmp_spool_find(sp, id);
	if (!mds)
	{
		mds = nmp_mds_new(id);
		mds->mds_ops = sp->mds_ops;
		mds->pool = sp;
		nmp_pool_add_nolock((NmpPool*)sp, mds->id, mds);
	}
	else
	{
		if (mds->state == MDS_STAT_KILL)
			nmp_mds_jump_state(mds, MDS_STAT_INIT);
	}

	nmp_pool_release_lock((NmpPool*)sp);

	return ret;
}


gint
nmp_spool_del_mds(NmpMdsPool *sp, gchar *id)
{
	NmpMds *mds;
	gint ret = -ENOENT;
	G_ASSERT(sp && id);

	nmp_pool_require_lock((NmpPool*)sp);

	mds = nmp_spool_find(sp, id);
	if (mds)
	{
		nmp_mds_jump_state(mds, MDS_STAT_KILL);
		ret = 0;
	}

	nmp_pool_release_lock((NmpPool*)sp);

	return ret;
}


gint
nmp_spool_add_mds_ip(NmpMdsPool *sp, gchar *id, gchar *ip, gint port)
{
	NmpMds *mds;
	gint ret = -ENOENT;
	G_ASSERT(sp && id && ip);

	nmp_pool_require_lock((NmpPool*)sp);

	mds = nmp_spool_find(sp, id);
	if (mds)
	{
		if (mds->state == MDS_STAT_IP || mds->state == MDS_STAT_NORMAL)
			nmp_mds_set_ip(mds, ip, port);
	}

	nmp_pool_release_lock((NmpPool*)sp);

	return ret;	
}


gint
nmp_spool_get_conn_token(NmpMdsPool *sp)
{
	G_ASSERT(sp);

	if (sp->con_tokens <= 0)
		return -EAGAIN;

	--sp->con_tokens;
	return 0;
}


//:~ End
