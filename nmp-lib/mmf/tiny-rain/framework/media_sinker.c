/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#include "media_sinker.h"
#include "media.h"
#include "tr_log.h"

enum
{
	STAT_INIT,
	STAT_READY,
	STAT_KILLED
};

static atomic_t media_sinker_objs = ATOMIC_INIT;

static __inline__ void
init_self(media_sinker *sinker)
{
	sinker->state = STAT_INIT;
	INIT_LIST_HEAD(&sinker->list);
	sinker->medium = NULL;
	sinker->host = NULL;
	sinker->lock = LOCK_NEW();
	sinker->ops = NULL;
	atomic_inc(&media_sinker_objs);
}


static __inline__ void
fin_self(media_sinker *sinker)
{
	LOCK_DEL(sinker->lock);

	if (sinker->host)
	{
		session_unref(sinker->host);
	}

	if(sinker->medium)
	{
		media_unref(sinker->medium);
	}

	atomic_dec(&media_sinker_objs);
	LOG_V(
		"Media sinker objs count: %d.",
		atomic_get(&media_sinker_objs)
	);
}


static __inline__ void
on_obj_fin(obj *p)
{
	media_sinker *sinker = (media_sinker*)p;

	if (sinker->ops && sinker->ops->fin)
	{
		(*sinker->ops->fin)(sinker);
	}

	fin_self(sinker);
}


media_sinker *
media_sinker_alloc(uint32_t size, media_sinker_ops *ops, void *u)
{
	int32_t err;
	media_sinker *sinker;

	if (size < sizeof(*sinker))
		return NULL;

	sinker = (media_sinker*)obj_new(size, on_obj_fin);
	if (sinker)
	{
		init_self(sinker);
		sinker->ops = ops;

		if (ops && ops->init)
		{
			err = (*ops->init)(sinker, u);
			if (err)
			{
				sinker->ops = NULL;
				obj_unref(sinker);
				return NULL;
			}
		}
	}

	return sinker;
}


media_sinker *
media_sinker_ref(media_sinker *sinker)
{
	if (sinker)
	{
		obj_ref(sinker);
	}

	return sinker;
}


void
media_sinker_unref(media_sinker *sinker)
{
	if (sinker)
	{
		obj_unref(sinker);
	}
}


static __inline__ int32_t
__media_sinker_killed(media_sinker *sinker)
{
	return sinker->state == STAT_KILLED;
}


static __inline__ void
media_sinker_kill(media_sinker *sinker)
{
	media *medium = NULL;

	AQUIRE_LOCK(sinker->lock);
	if (!__media_sinker_killed(sinker))
	{
		sinker->state = STAT_KILLED;
		if (sinker->ops && sinker->ops->kill)
		{
			(*sinker->ops->kill)(sinker);
		}
		medium = sinker->medium;
	}
	RELEASE_LOCK(sinker->lock);

	if (medium)
	{
		media_remove_sinker(medium, &sinker->list);
	}
}


void
media_sinker_kill_unref(media_sinker *sinker)
{
	media_sinker_kill(sinker);
	media_sinker_unref(sinker);
}


int32_t
media_sinker_ready(media_sinker *sinker)
{
	int32_t err = -EKILLED;
	media *medium = NULL;

	AQUIRE_LOCK(sinker->lock);
	if (!__media_sinker_killed(sinker))
	{
		err = 0;
		if (sinker->state != STAT_READY)
		{
			medium = sinker->medium;
			sinker->state = STAT_READY;
		}
	}
	RELEASE_LOCK(sinker->lock);

	if (medium)
	{
		err = media_ready_to_start(medium);
	}

	return err;
}

int32_t
media_sinker_ctl(media_sinker *sinker, int32_t cmd, void *data)
{
	int32_t err = -EKILLED;
	media *medium = NULL;

	AQUIRE_LOCK(sinker->lock);
	if (!__media_sinker_killed(sinker))
	{
		err = 0;
		if (sinker->state == STAT_READY)
		{
			medium = sinker->medium;
		}
	}
	RELEASE_LOCK(sinker->lock);

	if (medium)
	{
		err = media_play_ctl(medium, cmd, data);
	}

	return err;
}

int32_t
media_sinker_consumable(media_sinker *sinker, int32_t stm_i,
	uint32_t size)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(sinker->lock);
	if (!__media_sinker_killed(sinker))
	{
		err = 0;
		if (sinker->ops && sinker->ops->consumable)
		{
			err = (*sinker->ops->consumable)(sinker, stm_i, size);
		}
	}
	RELEASE_LOCK(sinker->lock);

	return err;
}


int32_t
media_sinker_set_config(media_sinker *sinker, void *in,
	void *data)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(sinker->lock);
	if (!__media_sinker_killed(sinker))
	{
		err = -EPERM;
		if (sinker->ops && sinker->ops->set_config)
		{
			err = (*sinker->ops->set_config)(sinker, in, data);
		}
	}
	RELEASE_LOCK(sinker->lock);

	return err;
}


int32_t
media_sinker_get_config(media_sinker *sinker, void *in,
	void *data)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(sinker->lock);
	if (!__media_sinker_killed(sinker))
	{
		err = -EPERM;
		if (sinker->ops && sinker->ops->get_config)
		{
			err = (*sinker->ops->get_config)(sinker, in, data);
		}
	}
	RELEASE_LOCK(sinker->lock);

	return err;
}


int32_t
media_sinker_set_owner(media_sinker *sinker, session *s)
{
	if (sinker->state == STAT_KILLED || sinker->host)
		return -EINVAL;

	sinker->host = session_ref(s);	//@{unref on fin}
	return 0;
}


int32_t
media_sinker_fill(media_sinker *sinker, int32_t stm_i, msg *m, uint32_t seq)
{
	int32_t err = -EKILLED;

	AQUIRE_LOCK(sinker->lock);
	if (!__media_sinker_killed(sinker))
	{
		err = 0; /* if failed, drop the msg silently */
		if (sinker->state == STAT_READY)
		{ 
			if (sinker->ops && sinker->ops->consume)
			{
				err = (*sinker->ops->consume)(sinker, stm_i, m, seq);
			}
		}
	}
	RELEASE_LOCK(sinker->lock);

	return err;	
}


//:~ End
