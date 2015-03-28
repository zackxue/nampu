#include "nmp_pool.h"
#include "nmp_afx.h"


static __inline__ void
__nmp_on_pool_timer(NmpPool *p)
{
	if (G_UNLIKELY(!p->ops))
		return;

	p->garbage = 0;

	if (p->ops->pre_check)
	{
		(*p->ops->pre_check)(p);
	}

	if (p->ops->check_element)
	{
		g_hash_table_foreach(p->hash_table,
		                     p->ops->check_element,
		                     p);
	}

	if (p->ops->post_check)
	{
		(*p->ops->post_check)(p);
	}

	if (p->ops->garbage_collect && p->garbage > 0)
	{
		g_hash_table_foreach_remove(p->hash_table,
		                            p->ops->garbage_collect,
		                            p);				
	}
}


static gboolean
nmp_on_pool_timer(gpointer user_data)
{
	NmpPool *p = (NmpPool*)user_data;

	g_mutex_lock(p->mutex);
	__nmp_on_pool_timer(p);
	g_mutex_unlock(p->mutex);

	return TRUE;
}


NmpPool *
nmp_pool_new(gsize size, gint freq_msec, NmpPoolOps *ops)
{
	NmpPool *p;

	if (G_UNLIKELY(size < sizeof(NmpPool) || !ops))
		return NULL;

	p = (NmpPool*)g_malloc(size);
	memset(p, 0, size);

	p->hash_table = g_hash_table_new_full(ops->hash_fn,
	                                      ops->key_equal,
	                                      (GDestroyNotify)ops->key_destroy,
	                                      (GDestroyNotify)ops->value_destroy);
	p->mutex = g_mutex_new();
	p->ops = ops;
	p->timer = nmp_set_timer(freq_msec, nmp_on_pool_timer, p);

	return p;
}


gint
nmp_pool_require_lock(NmpPool *p)
{
	G_ASSERT(p);

	g_mutex_lock(p->mutex);
	return 0;
}


gint
nmp_pool_release_lock(NmpPool *p)
{
	G_ASSERT(p);

	g_mutex_unlock(p->mutex);
	return 0;
}


gpointer
nmp_pool_find_nolock(NmpPool *p, gpointer key)
{
	G_ASSERT(p && p->hash_table);

	return  g_hash_table_lookup(p->hash_table, key);
}


void
nmp_pool_add_nolock(NmpPool *p, gpointer key, gpointer value)
{
	G_ASSERT(p && p->hash_table);

	g_hash_table_insert(p->hash_table, key, value);	
}


void
nmp_pool_do_event(NmpPool *p, gint event, void *v1, void *v2, void *v3)
{
	NmpMsg *msg;
	G_ASSERT(p != NULL);

	if (!p->ops || !p->ops->make_msg || !p->ev_handler)
		return;

	msg = (*p->ops->make_msg)(p, event, v1, v2, v3);
	if (msg)
	{
		(*p->ev_handler)(p->ev_private, msg);
	}
}


void
nmp_pool_set_handler(NmpPool *p, NmpEventHandler handler,
	gpointer priv)
{
	G_ASSERT(p != NULL);

	p->ev_handler = handler;
	p->ev_private = priv;
}


//:~ End
