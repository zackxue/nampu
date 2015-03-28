#include "nmp_sbuff.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_mod_buffering.h"


NmpSBuff *
nmp_sbuff_new(NmpGuid *guid, gint flags, gint local_tags)
{
	NmpModBuffering *mb = nmp_mod_buffering_get();

	BUG_ON(!mb);
	return nmp_mod_buffering_new_sbuff(mb, guid, flags, local_tags);
}


static __inline__ void
nmp_sbuff_kill(NmpSBuff *sb)
{
	NmpModBuffering *mb = nmp_mod_buffering_get();

	BUG_ON(!mb);
	if (sb->ops && sb->ops->sb_kill)
	{
		(*sb->ops->sb_kill)(sb);
	}
	nmp_mod_buffering_del_sbuff(mb, sb);
}


gint
nmp_sbuff_flush_pending(NmpSBuff *sb)
{
	NmpModBuffering *mb;
	gint ret = -EAGAIN;

	g_atomic_int_set(&sb->action, 1);
	if (g_atomic_int_compare_and_exchange(&sb->pending, 0, 1))
	{
		mb = nmp_mod_buffering_get();
		BUG_ON(!mb);
		nmp_mod_buffering_flush_pending(mb, sb);
		ret = 0;
	}
	g_atomic_int_set(&sb->action, 0);

	return ret;
}


NmpSBuff *
nmp_sbuff_ref(NmpSBuff *sb)
{
	G_ASSERT(sb && g_atomic_int_get(&sb->ref_count) > 0);

	g_atomic_int_add(&sb->ref_count, 1);
	return sb;
}


void
nmp_sbuff_unref(NmpSBuff *sb)
{
	G_ASSERT(sb && g_atomic_int_get(&sb->ref_count) > 0);

	if (g_atomic_int_dec_and_test(&sb->ref_count))
	{
		if (sb->ops && sb->ops->sb_fin)
		{
			(*sb->ops->sb_fin)(sb);
		}
		g_free(sb);
	}
}


void
nmp_sbuff_kill_unref(NmpSBuff *sb)
{
	G_ASSERT(sb);

	nmp_sbuff_kill(sb);
	nmp_sbuff_unref(sb);
}


gint
nmp_sbuff_write(NmpSBuff *sb, gchar *data, gsize size, gint flags)
{
	G_ASSERT(sb && data);

	if (sb->ops && sb->ops->sb_write)
	{
		return (*sb->ops->sb_write)(sb, data, size, flags);
	}

	return -EINVAL;
}


gint
nmp_sbuff_flush(NmpSBuff *sb)
{
	G_ASSERT(sb);

	if (sb->ops && sb->ops->sb_flush)
	{
		return (*sb->ops->sb_flush)(sb);
	}

	return -EINVAL;	
}


gint
nmp_sbuff_pause(NmpSBuff *sb)
{
	G_ASSERT(sb);

	if (sb->ops && sb->ops->sb_pause)
	{
		return (*sb->ops->sb_pause)(sb);
	}

	return -EINVAL;
}


//:~ End
