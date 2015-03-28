#include "nmp_errno.h"
#include "nmp_id_query.h"

#define STATE_INPROGRESS_TTL		15
#define STATE_DELAY_TTL				10
#define STATE_IDLE_TTL				(60*10)
#define ENTRY_PER_QUERY				200


NmpIDQueryBlock *
nmp_id_qb_new( void )
{
	NmpIDQueryBlock *qb;

	qb = g_new0(NmpIDQueryBlock, 1);
	qb->old_set = nmp_guid_set_new();
	qb->mutex = g_mutex_new();

	return qb;
}


static __inline__ void
nmp_id_qb_reset_result(NmpIDQueryBlock *qb)
{
	qb->total_guids = 0;
	qb->got_guids = 0;
	qb->next_row = 0;

	if (qb->new_set)
	{
		nmp_guid_set_delete(qb->new_set);
		qb->new_set = NULL;
	}
}


static __inline__ void
__nmp_id_qb_set_state(NmpIDQueryBlock *qb, gint state)
{
	qb->state = state;

	switch (qb->state)
	{
	case STATE_INPROGRESS:
		nmp_id_qb_reset_result(qb);
		qb->new_set = nmp_guid_set_new();
		qb->state_ttl = STATE_INPROGRESS_TTL;
		break;

	case STATE_RETRY_DELAY:
		nmp_id_qb_reset_result(qb);
		qb->state_ttl = STATE_DELAY_TTL;
		break;

	case STATE_IDLE:
		nmp_id_qb_reset_result(qb);
		qb->state_ttl = STATE_IDLE_TTL;
		break;
	}
}


void
nmp_id_qb_delay(NmpIDQueryBlock *qb)
{
	G_ASSERT(qb != NULL);

	g_mutex_lock(qb->mutex);
	__nmp_id_qb_set_state(qb, STATE_RETRY_DELAY);
	g_mutex_unlock(qb->mutex);
}


static __inline__ gint
__nmp_id_qb_add_entry(NmpIDQueryBlock *qb, gchar *domain, gchar *guid)
{
	if (qb->state != STATE_INPROGRESS)
		return -E_INVAL;

	if (nmp_guid_set_add(qb->new_set, domain, guid))	/* CMS changed guid list */
	{
		__nmp_id_qb_set_state(qb, STATE_RETRY_DELAY);
		return -E_AGAIN;
	}

	++qb->got_guids;
	++qb->next_row;

	return 0;
}


gint
nmp_id_qb_add_entry(NmpIDQueryBlock *qb, gchar *domain, gchar *guid)
{
	gint ret;
	G_ASSERT(qb && domain && guid);

	g_mutex_lock(qb->mutex);
	ret = __nmp_id_qb_add_entry(qb, domain, guid);
	g_mutex_unlock(qb->mutex);

	return ret;
}


static __inline__ void
nmp_id_qb_leave_state(NmpIDQueryBlock *qb)
{///{@state machine automatically jump}
	switch (qb->state)
	{
	case STATE_IDLE:
		__nmp_id_qb_set_state(qb, STATE_INPROGRESS);
		break;

	case STATE_INPROGRESS:
		__nmp_id_qb_set_state(qb, STATE_RETRY_DELAY);
		break;

	case STATE_RETRY_DELAY:
		__nmp_id_qb_set_state(qb, STATE_INPROGRESS);
		break;
	}
}


void 
nmp_id_qb_start(NmpIDQueryBlock *qb)
{
	G_ASSERT(qb != NULL);

	g_mutex_lock(qb->mutex);
	if (!qb->running)
	{
		qb->running = 1;
		__nmp_id_qb_set_state(qb, STATE_INPROGRESS);
	}
	g_mutex_unlock(qb->mutex);
}


void 
nmp_id_qb_stop(NmpIDQueryBlock *qb)
{
	G_ASSERT(qb != NULL);

	g_mutex_lock(qb->mutex);

	if (qb->running)
	{
		qb->running = 0;
		__nmp_id_qb_set_state(qb, STATE_IDLE);
	}

	g_mutex_unlock(qb->mutex);
}


void
nmp_id_qb_tick(NmpIDQueryBlock *qb)
{
	G_ASSERT(qb != NULL);

	g_mutex_lock(qb->mutex);

	if (qb->running)
	{
		++qb->tick_counter;
		if (--qb->state_ttl <= 0)
			nmp_id_qb_leave_state(qb);
	}

	g_mutex_unlock(qb->mutex);
}


gint
nmp_id_qb_set_total(NmpIDQueryBlock *qb, gint total)
{
	gint err = 0;
	G_ASSERT(qb != NULL);

	g_mutex_lock(qb->mutex);

	if (qb->state != STATE_INPROGRESS)
		err = -E_INVAL;
	else
	{
		if (qb->total_guids > 0 && qb->total_guids != total)
		{//list changed.
			__nmp_id_qb_set_state(qb, STATE_RETRY_DELAY);
			err = -E_INVAL;
		}
		else
		{
			qb->total_guids = total;
		}
	}

	g_mutex_unlock(qb->mutex);
	return err;
}


gint
nmp_id_qb_query_check(NmpIDQueryBlock *qb, gint *start_row, gint *num)
{
	gint err = 0;
	gint left;
	G_ASSERT(qb != NULL && start_row != NULL && num != NULL);

	g_mutex_lock(qb->mutex);

	if (qb->state != STATE_INPROGRESS)
		err = -E_INVAL;
	else
	{
		if (qb->total_guids > 0 && qb->got_guids >= qb->total_guids)
			err = -EEXIST;
		else
		{
			left = qb->total_guids - qb->got_guids;

			*start_row = qb->next_row;
			*num =  left > ENTRY_PER_QUERY ? ENTRY_PER_QUERY : left;

			if (!*num)
				*num = ENTRY_PER_QUERY;
		}
	}

	g_mutex_unlock(qb->mutex);

	return err;
}


NmpDiffSet *
nmp_id_qb_get_diffset(NmpIDQueryBlock *qb)
{
	NmpDiffSet *set = NULL;
	G_ASSERT(qb != NULL);

	g_mutex_lock(qb->mutex);

	if (qb->state == STATE_INPROGRESS &&
		qb->total_guids >= 0 &&
		qb->got_guids >= qb->total_guids)
	{
		set = nmp_diff_set_new();
		set->add = nmp_guid_set_diff(qb->new_set, qb->old_set);
		set->del = nmp_guid_set_diff(qb->old_set, qb->new_set);

		nmp_guid_set_delete(qb->old_set);
		qb->old_set = qb->new_set;
		qb->new_set = NULL;
		__nmp_id_qb_set_state(qb, STATE_IDLE);
	}

	g_mutex_unlock(qb->mutex);

	return set;
}


//:~ End
