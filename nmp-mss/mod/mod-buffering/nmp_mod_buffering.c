#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_mods.h"
#include "nmp_mod_buffering.h"
#include "nmp_sbuffs.h"

#define MAX_FLUSH_THREADS		4
G_DEFINE_TYPE(NmpModBuffering, nmp_mod_buffering, NMP_TYPE_APPMOD);
static NmpModBuffering *pthis = NULL;


static void
nmp_mod_buffering_flush_task(gpointer data, gpointer user_data)
{
	NmpSBuff *sb = (NmpSBuff*)data;
	BUG_ON(!sb);

	do {
		nmp_sbuff_flush(sb);
		g_atomic_int_set(&sb->pending, 0);
	}while (g_atomic_int_get(&sb->action));

	nmp_sbuff_unref(sb);
}


static void 
nmp_mod_buffering_init(NmpModBuffering *self)
{
	self->ops = NULL;
	self->sb_list = NULL;
	self->mutex = g_mutex_new();
	self->tp_flush = g_thread_pool_new(nmp_mod_buffering_flush_task,
	                                   self,
	                                   MAX_FLUSH_THREADS,
	                                   FALSE,
	                                   NULL);

	pthis = self;
}


gint
nmp_mod_buffering_setup(NmpAppMod *am_self)
{
	NmpModBuffering *self = (NmpModBuffering*)am_self;
	G_ASSERT(self != NULL);

	nmp_app_mod_set_name(am_self, "MOD-BUFFERING");
	nmp_mod_buffering_register_sb_ops(self, &nmp_jfs_sbuff_ops);

	return 0;
}


static void
nmp_mod_buffering_class_init(NmpModBufferingClass *k_class)
{
	NmpAppModClass *am_class = (NmpAppModClass*)k_class;

	am_class->setup_mod	= nmp_mod_buffering_setup;
}


void
nmp_mod_buffering_insert( void )
{
	NmpAppMod *mod;

	mod = g_object_new(NMP_TYPE_MODBUFFERING, NULL);
	if (G_UNLIKELY(!mod))
	{
		nmp_error("<NmpModBuffering> alloc mod buffering failed!");
		FATAL_ERROR_EXIT;
	}

	nmp_app_mod_setup((NmpAppMod*)mod);
	nmp_afx_mod_insert(BUSSLOT_POS_BUFFERING, mod);
}


NmpModBuffering *
nmp_mod_buffering_get( void )
{
	return pthis;
}


void
nmp_mod_buffering_register_sb_ops(NmpModBuffering *mb, NmpSBuffOps *ops)
{
	G_ASSERT(mb && ops);
	mb->ops = ops;
}


NmpSBuffOps *
nmp_mod_buffering_get_sb_ops(NmpModBuffering *mb)
{
	BUG_ON(!mb->ops);
	return mb->ops;
}


static __inline__ void
nmp_mod_buffering_add_sb(NmpModBuffering *mb, NmpSBuff *sb)
{
	g_mutex_lock(mb->mutex);
	mb->sb_list = g_list_prepend(mb->sb_list, sb);
	g_mutex_unlock(mb->mutex);
}


static __inline__ void
nmp_mod_buffering_remove_sb(NmpModBuffering *mb, NmpSBuff *sb)
{
	g_mutex_lock(mb->mutex);
	mb->sb_list = g_list_remove(mb->sb_list, sb);
	g_mutex_unlock(mb->mutex);
}


NmpSBuff *
nmp_mod_buffering_new_sbuff(NmpModBuffering *mb, NmpGuid *guid,
	guint flags, guint local_tags)
{
	NmpSBuff *sb;
	NmpSBuffOps *ops;

	ops = nmp_mod_buffering_get_sb_ops(mb);
	if (!ops)
		return NULL;

	sb = g_malloc0(ops->sb_size);
	nmp_guid_assign(&sb->guid, guid);
	sb->ops = ops;
	sb->ref_count =  1;
	sb->pending = 0;
	sb->action = 0;
	sb->flags = flags;
	sb->local_tags = local_tags;

	if (ops->sb_init)
	{
		if ((*ops->sb_init)(sb))
		{
			g_free(sb);
			return NULL;
		}
	}

	nmp_mod_buffering_add_sb(mb, sb);
	return sb;
}


void
nmp_mod_buffering_del_sbuff(NmpModBuffering *mb, NmpSBuff *sb)
{
	G_ASSERT(mb && sb);

	nmp_mod_buffering_remove_sb(mb, sb);
}


void
nmp_mod_buffering_flush_pending(NmpModBuffering *mb, NmpSBuff *sb)
{
	G_ASSERT(mb && sb);

	nmp_sbuff_ref(sb);
	g_thread_pool_push(mb->tp_flush, sb, NULL);
}

//:~ End
