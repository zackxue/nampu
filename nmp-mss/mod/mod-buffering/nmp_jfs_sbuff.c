#include <fcntl.h>
#include "nmp_rec.h"
#include "nmp_mod_buffering.h"
#include "nmp_sdl.h"
#include "stor_api.h"
#include "nmp_clock.h"
#include "nmp_errno.h"

#define MAX_PENDING_SIZE	(2 << 20)

typedef struct __NmpJfsSBuff NmpJfsSBuff;
struct __NmpJfsSBuff
{
	NmpSBuff	__super;
	gpointer    h_file;
	GMutex		*lock;
	GList		*data;
	gsize		data_size;
	gint		close_request;
	gint        direct_writing;
};


typedef struct __NmpBlock NmpBlock;
struct __NmpBlock
{
	gint		flags;
	gsize		size;
	gchar		data[0];
};


static __inline__ NmpBlock *
nmp_jfs_jfssbuff_new_blk(gchar *data, gsize size,
	gint flags)
{
	NmpBlock *blk;
	gsize real_size = sizeof(NmpBlock) + size;

	blk = (NmpBlock*)g_malloc(real_size);
	blk->flags = flags;
	blk->size = size;
	memcpy(blk->data, data, size);
	return blk;
}


static __inline__ gpointer
nmp_jfs_jfssbuff_set_hfile(NmpJfsSBuff *sb, gpointer h)
{
	gpointer old;

	g_mutex_lock(sb->lock);
	old = sb->h_file;
	sb->h_file = h;
	g_mutex_unlock(sb->lock);

	return old;
}


static __inline__ gint
nmp_jfs_jfssbuff_prepare(NmpSBuff *sb)
{
	NmpJfsSBuff *_sb = (NmpJfsSBuff*)sb;
	stor_fwrite_handle_t *h;
	gint err;
	gchar *ch;

	ch = g_strdup_printf("%s@%s", sb->guid.domain_id, sb->guid.guid);
	h = stor_file_openW(sb->local_tags, ch, (int)(-1), &err);
	if (!h)
	{
		nmp_print(
			"jfs-sb: stor_file_openW() gu '%s' failed, err:'%d'",
			ch,
			err
		);
		g_free(ch);
		if (!err)
		{
			err = -ENOMEM;
		}
		return err;
	}

	nmp_print(
		"jfs-sb: stor_file_openW() gu '%s' ok",
		ch
	);

	g_free(ch);
	nmp_jfs_jfssbuff_set_hfile(_sb, h);

	return 0;
}


static gint
nmp_jfs_sbuff_init(NmpSBuff *sb)
{
	NmpJfsSBuff *_sb = (NmpJfsSBuff*)sb;

	_sb->h_file = NULL;
	_sb->lock = g_mutex_new();
	_sb->data = NULL;
	_sb->data_size = 0;
	_sb->close_request = 0;
	_sb->direct_writing = 0;

	return 0;
}


static __inline__ gint
nmp_jfs_jfssbuff_write_pending(NmpJfsSBuff *sb, gchar *data, gsize size,
	gint flags)
{
	NmpBlock *blk;

	if (sb->data_size + size <= MAX_PENDING_SIZE)
	{
		blk = nmp_jfs_jfssbuff_new_blk(data, size, flags);
		sb->data = g_list_append(sb->data, blk);
		sb->data_size += size;
	}
	else
	{//@{drop}
	}

	nmp_sbuff_flush_pending((NmpSBuff*)sb);
	return 0;
}


static __inline__ gint
nmp_jfs_jfssbuff_write(NmpJfsSBuff *sb, gchar *data, gsize size,
	gint flags)
{
	stor_fwrite_handle_t *h;
	stor_frame_info_t i;

	h = (stor_fwrite_handle_t*)sb->h_file;
	if (h)
	{
		memset(&i, 0, sizeof(i));
		i.type = (flags&REC_IFRAME_TAG) ? STOR_FRAME_VIDEO_I : 0;
		i.is_windex = flags&REC_IFRAME_TAG;
		i.time = SDL_V1_GET_VALUE(data, ts_1);
		i.tags = (flags&(~REC_IFRAME_TAG));		/* User tags, RC_TYPE_AUTO | RC_TYPE_ALARM etc. */
		i.size = (int)size;
		return stor_file_write(h, data, &i);
	}
	return 0;
}


static __inline__ void
nmp_jfs_sbuff_enter_direct_write(NmpJfsSBuff *sb)
{
	g_atomic_int_set(&sb->direct_writing, 1);
}


static __inline__ void
nmp_jfs_sbuff_out_direct_write(NmpJfsSBuff *sb)
{
	g_atomic_int_set(&sb->direct_writing, 0);
}


static __inline__ void
nmp_jfs_sbuff_wait_direct_write(NmpJfsSBuff *sb)
{
	while (g_atomic_int_get(&sb->direct_writing))
	{
		usleep(10*1000);
	}
}


static gint
nmp_jfs_sbuff_write(NmpSBuff *sb, gchar *data, gsize size,
	gint flags)
{
	NmpJfsSBuff *_sb = (NmpJfsSBuff*)sb;
	gint ret = 0, done = 0;

	nmp_jfs_sbuff_enter_direct_write(_sb);

	if (!g_atomic_int_get(&_sb->close_request))
	{
		g_mutex_lock(_sb->lock);
		if (!_sb->h_file || _sb->data)
		{
			nmp_jfs_jfssbuff_write_pending(_sb, data, size, flags);
			done = 1;
		}
		g_mutex_unlock(_sb->lock);

		if (!done)
		{
			ret = nmp_jfs_jfssbuff_write(_sb, data, size, flags);
		}
	}

	nmp_jfs_sbuff_out_direct_write(_sb);
	return ret;
}


static void
nmp_jfs_jfssbuff_flush_one(gpointer data, gpointer user_data)
{
	NmpJfsSBuff *sb = (NmpJfsSBuff*)user_data;
	NmpBlock *blk = (NmpBlock*)data;

	nmp_jfs_jfssbuff_write(sb, blk->data, blk->size, blk->flags);
	g_free(blk);
}


static __inline__ void
nmp_jfs_jfssbuff_flush(NmpJfsSBuff *sb)
{
	g_mutex_lock(sb->lock);
	g_list_foreach(sb->data, nmp_jfs_jfssbuff_flush_one, sb);
	g_list_free(sb->data);
	sb->data = NULL;
	sb->data_size = 0;
	g_mutex_unlock(sb->lock);
}


static __inline__ void
nmp_jfs_jfssbuff_open(NmpJfsSBuff *sb)
{
	gint ret;

	ret = nmp_jfs_jfssbuff_prepare((NmpSBuff*)sb);
	if (!ret)
	{
		nmp_jfs_jfssbuff_flush(sb);
	}
}


static void
nmp_jfs_jfssbuff_free_blk(gpointer data, gpointer user_data)
{
	NmpBlock *blk = (NmpBlock*)data;
	g_free(blk);
}


static __inline__ void
nmp_jfs_jfssbuff_close(NmpJfsSBuff *sb)
{
	NmpSBuff *_sb = (NmpSBuff*)sb;
	stor_fwrite_handle_t *h;
	gchar *ch;

	h = (stor_fwrite_handle_t*)nmp_jfs_jfssbuff_set_hfile(sb, NULL);
	if (h)
	{
		nmp_jfs_sbuff_wait_direct_write(sb);
		ch = g_strdup_printf("%s@%s", _sb->guid.domain_id, _sb->guid.guid);
		nmp_print(
			"jfs-sb: fin, stor_file_closeW() gu '%s",
			ch
		);

		g_free(ch);
		stor_file_closeW(h, nmp_clock_get_time());
	}
}


static gint
nmp_jfs_sbuff_flush(NmpSBuff *sb)
{
	NmpJfsSBuff *_sb = (NmpJfsSBuff*)sb;

	if (_sb->h_file)
	{
		nmp_jfs_jfssbuff_flush(_sb);
	}
	else
	{
		nmp_jfs_jfssbuff_open(_sb);
	}

	if (g_atomic_int_get(&_sb->close_request))
	{
		nmp_jfs_jfssbuff_close(_sb);
		g_atomic_int_set(&_sb->close_request, 0);
	}

	return 0;
}


static void
nmp_jfs_sbuff_kill(NmpSBuff *sb)
{
	NmpJfsSBuff *_sb = (NmpJfsSBuff*)sb;

	g_atomic_int_set(&_sb->close_request, 1);
	nmp_sbuff_flush_pending(sb);
}


static gint
nmp_jfs_sbuff_pause(NmpSBuff *sb)
{
	nmp_jfs_sbuff_kill(sb);
	return 0;
}


static void
nmp_jfs_sbuff_fin(NmpSBuff *sb)
{
	NmpJfsSBuff *_sb = (NmpJfsSBuff*)sb;
	_sb = (NmpJfsSBuff*)sb;
	BUG_ON(_sb->h_file);

	g_list_foreach(_sb->data, nmp_jfs_jfssbuff_free_blk, NULL);
	g_list_free(_sb->data);
	g_mutex_free(_sb->lock);
}


NmpSBuffOps nmp_jfs_sbuff_ops =
{
	.sb_size	= sizeof(NmpJfsSBuff),
	.sb_init	= nmp_jfs_sbuff_init,
	.sb_pause	= nmp_jfs_sbuff_pause,
	.sb_write	= nmp_jfs_sbuff_write,
	.sb_flush	= nmp_jfs_sbuff_flush,
	.sb_kill	= nmp_jfs_sbuff_kill,
	.sb_fin		= nmp_jfs_sbuff_fin	
};


//:~ End
