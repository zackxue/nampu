#ifndef __NMP_POOL_H__
#define __NMP_POOL_H__

#include <glib.h>
#include "nmp_msg.h"

G_BEGIN_DECLS

typedef void (*NmpEventHandler)(gpointer priv, NmpMsg *msg);

typedef struct __NmpPoolOps NmpPoolOps;
struct __NmpPoolOps				/* determine how to create a pool */
{
	guint (*hash_fn)(gconstpointer key);
	gboolean (*key_equal)(gconstpointer key1, gconstpointer key2);

	void (*key_destroy)(gpointer key);
	void (*value_destroy)(gpointer value);

	void (*check_element)(gpointer key, gpointer value, gpointer pool);
	gboolean (*garbage_collect)(gpointer key, gpointer value, gpointer pool);

	NmpMsg *(*make_msg)(gpointer pool, gint event, void *v1, void *v2, void *v3);

	void (*pre_check)(gpointer pool);
	void (*post_check)(gpointer pool);
};


typedef struct __NmpPool NmpPool;
struct __NmpPool
{
	GHashTable	*hash_table;
	GMutex	*mutex;

	NmpPoolOps	*ops;

	/* state check timer */
	guint	timer;

	/* counters */
	gint	total;
	gint	garbage;

	/* event hander */
	NmpEventHandler ev_handler;
	gpointer ev_private;
};


NmpPool *nmp_pool_new(gsize size, gint freq_msec, NmpPoolOps *ops);

gint nmp_pool_require_lock(NmpPool *p);
gint nmp_pool_release_lock(NmpPool *p);

gpointer nmp_pool_find_nolock(NmpPool *p, gpointer key);
void nmp_pool_add_nolock(NmpPool *p, gpointer key, gpointer value);

void nmp_pool_do_event(NmpPool *p, gint event, void *v1, void *v2, void *v3);

void nmp_pool_set_handler(NmpPool *p, NmpEventHandler handler,
	gpointer priv);

G_END_DECLS

#endif	/* __NMP_POOL_H__ */
