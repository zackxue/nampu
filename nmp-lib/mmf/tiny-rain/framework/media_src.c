#include "tr_log.h"
#include "media_src.h"
#include "media.h"

enum
{
	STAT_NORMAL,
	STAT_KILLED
};

static atomic_t media_src_objs = ATOMIC_INIT;

static __inline__ void
init_self(media_src *src, void *u)
{
	src->state = STAT_NORMAL;
	src->medium = NULL;
	src->user = u;
	src->lock = LOCK_NEW();
	src->ops = NULL;
	atomic_inc(&media_src_objs);
}


static __inline__ void
fin_self(media_src *src)
{
	LOCK_DEL(src->lock);
	if (src->medium)
	{
		media_unref(src->medium);
	}

	atomic_dec(&media_src_objs);
	LOG_V("Media src objs count:%d.",
		atomic_get(&media_src_objs)
	);
}


static void
on_obj_fin(obj *p)
{
	media_src *src = (media_src*)p;

	if (src->ops && src->ops->fin)
	{
		(*src->ops->fin)(src);
	}

	fin_self(src);
}


media_src *
media_src_alloc(uint32_t size, media_src_ops *ops, void *u)
{
	int32_t err;
	media_src *src;

	if (size < sizeof(*src))
		return NULL;

	src = (media_src*)obj_new(size, on_obj_fin);
	if (src)
	{
		init_self(src, u);
		src->ops = ops;

		if (ops && ops->init)
		{
			err = (*ops->init)(src);
			if (err)
			{
				LOG_W(
				"media_src_alloc()->(*ops->init)() failed.");
				src->ops = NULL;
				obj_unref(src);
				return NULL;
			}
		}
	}

	return src;
}


static __inline__ int32_t
__media_src_killed(media_src *src)
{
	return src->state == STAT_KILLED;
}


static __inline__ void
__media_src_kill(media_src *src)
{
	if (__media_src_killed(src))
		return;

	src->state = STAT_KILLED;
	if (src->ops && src->ops->kill)
	{
		(*src->ops->kill)(src);
	}
}


static __inline__ void
media_src_kill(media_src *src)
{
	AQUIRE_LOCK(src->lock);
	__media_src_kill(src);
	RELEASE_LOCK(src->lock);
}


media_src *
media_src_ref(media_src *src)
{
	if (src)
	{
		obj_ref(src);
	}

	return src;
}


void
media_src_unref(media_src *src)
{
	if (src)
	{
		obj_unref(src);
	}
}


void
media_src_kill_unref(media_src *src)
{
	media_src_kill(src);
	media_src_unref(src);
}


static __inline__ int32_t
__media_src_ctl(media_src *src, int32_t cmd, void *data)
{
	int32_t err = -EPERM;

	if (src->ops && src->ops->ctl)
	{
		err = (*src->ops->ctl)(src, cmd, data);
	}

	return err;
}


int32_t
media_src_ctl(media_src *src, int32_t cmd, void *data)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(src->lock);
	if (!__media_src_killed(src))
	{
		err = __media_src_ctl(src, cmd, data);
	}
	RELEASE_LOCK(src->lock);

	return err;
}


static __inline__ int32_t
__media_src_probe(media_src *src, media_uri *mrl,
	media_info *msi)
{
	int32_t err = -EPERM;

	if (src->ops && src->ops->probe)
	{
		err = (*src->ops->probe)(src, mrl, msi);
	}

	return err;
}


int32_t
media_src_probe(media_src *src, media_uri *mrl,
	media_info *msi)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(src->lock);
	if (!__media_src_killed(src))
	{
		err = __media_src_probe(src, mrl, msi);
	}
	RELEASE_LOCK(src->lock);

	return err;
}


static __inline__ int32_t
__media_src_open(media_src *src, media_uri *mrl)
{
	int32_t err = -EPERM;

	if (src->ops && src->ops->open)
	{
		err = (*src->ops->open)(src, mrl);
	}

	return err;	
}


int32_t
media_src_open(media_src *src, media_uri *mrl)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(src->lock);
	if (!__media_src_killed(src))
	{
		err = __media_src_open(src, mrl);
	}
	RELEASE_LOCK(src->lock);

	return err;	
}


static __inline__ int32_t
__media_src_play(media_src *src)
{
	int32_t err = -EPERM;

	if (src->ops && src->ops->play)
	{
		err = (*src->ops->play)(src);
	}

	return err;	
}


int32_t
media_src_play(media_src *src)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(src->lock);
	if (!__media_src_killed(src))
	{
		err = __media_src_play(src);
	}
	RELEASE_LOCK(src->lock);

	return err;	
}


static __inline__ int32_t
__media_src_pause(media_src *src)
{
	int32_t err = -EPERM;

	if (src->ops && src->ops->pause)
	{
		err = (*src->ops->pause)(src);
	}

	return err;	
}


int32_t
media_src_pause(media_src *src)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(src->lock);
	if (!__media_src_killed(src))
	{
		err = __media_src_pause(src);
	}
	RELEASE_LOCK(src->lock);

	return err;
}


static __inline__ int32_t
__media_src_lseek(media_src *src, uint32_t ts)
{
	int32_t err = -EPERM;

	if (src->ops && src->ops->lseek)
	{
		err = (*src->ops->lseek)(src, ts);
	}

	return err;	
}



int32_t
media_src_lseek(media_src *src, uint32_t ts)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(src->lock);
	if (!__media_src_killed(src))
	{
		err = __media_src_lseek(src, ts);
	}
	RELEASE_LOCK(src->lock);

	return err;
}


int32_t
media_src_would_block(media_src *src, int32_t stm_idx, void *data,
	uint32_t size)
{
	if (src->medium)
	{
		return media_would_block(src->medium, stm_idx, data, size);
	}
	return -EAGAIN;
}


int32_t
media_src_fill_info(media_src *src, media_info *msi, int32_t err)
{
	if (src->medium)
	{
		return media_info_fill((media*)src->medium, msi, err);
	}
	return -EINVAL;
}


int32_t
media_src_open_end(media_src *src, int32_t err)
{
	if (src->medium)
	{
		return media_open_end((media*)src->medium, err);
	}
	return -EINVAL;	
}


int32_t
media_src_produce(media_src *src, int32_t stm_index,
	void *data, uint32_t size)
{
	int32_t err = -EKILLED;

	if (!__media_src_killed(src)) //@{without lock}
	{
		err = 0;
		if (src->medium)
		{
			err = media_fill_stream_from_source(
				src->medium, stm_index, data, size
			);
		}
	}

	return err;
}

//:~ End
