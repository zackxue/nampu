/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#include <stdlib.h>
#include "client.h"
#include "session.h"
#include "tr_log.h"
#include "media_sinker.h"

enum
{
	STAT_NORMAL,
	STAT_KILLED
};

static atomic_t session_objs = ATOMIC_INIT;

static __inline__ void
generate_session_id(uint8_t *id, uint32_t size)
{
	static int32_t sid = 0;

	snprintf((char*)id,
		size,
		/* "Foy-%04d-%04d", */
		"%04d%04d",
		atomic_inc(&sid) % 10000,
		rand() % 10000);
}


static __inline__ void
init_self(session *s)
{
	s->state = STAT_NORMAL;
	INIT_LIST_HEAD(&s->list);
	generate_session_id(s->id, SESS_MAX_LEN);
	s->client = NULL;
	s->media_sinker = NULL;
	s->lock = LOCK_NEW();
	s->ops = NULL;
	atomic_inc(&session_objs);
}


static __inline__ void
fin_self(session *s)
{
	LOCK_DEL(s->lock);
	BUG_ON(s->media_sinker);
	if (s->client)
	{
		client_unref(s->client);
	}
	BUG_ON(!list_empty(&s->list));
	atomic_dec(&session_objs);

	LOG_V(
		"Session objs count: %d.",
		atomic_get(&session_objs)
	);
}


static __inline__ void
on_obj_fin(obj *p)
{
	session *s = (session*)p;

	if (s->ops && s->ops->fin)
	{
		(*s->ops->fin)(s);
	}

	fin_self(s);
}


session *
session_alloc(uint32_t size, session_ops *ops, void *u)
{
	session *s;
	int32_t err;

	if (size < sizeof(*s))
		return NULL;

	s = (session*)obj_new(size, on_obj_fin);
	if (s)
	{
		init_self(s);
		s->ops = ops;

		if (ops && ops->init)
		{
			err = (*ops->init)(s, u);
			if (err)
			{
				s->ops = NULL;
				obj_unref(s);	//@{fin this}
				return NULL;
			}
		}
	}

	return s;
}


session *
session_ref(session *s)
{
	if (s)
	{
		obj_ref(s);
	}

	return s;
}


void
session_unref(session *s)
{
	if (s)
	{
		obj_unref(s);
	}
}


int32_t
session_add(session *s, void *_client)
{
	client *c = (client*)_client;

	if (!list_empty(&s->list) || s->client)
		return -EEXIST;

	list_add(&s->list, &c->s_list);
	s->client = client_ref(c);	//@{unref on fin}
	return 0;
}


int32_t
session_del(session *s, void *_client)
{
	client *c = (client*)_client;

	BUG_ON(c != s->client);
	if (list_empty(&s->list))
		return -EINVAL;

	list_del_init(&s->list);
	return 0;
}


static __inline__ int32_t
__session_killed(session *s)
{
	return s->state == STAT_KILLED;
}


static __inline__ void
session_kill(session *s)
{
	media_sinker *ms = NULL;

	AQUIRE_LOCK(s->lock);
	if (!__session_killed(s))
	{
		s->state = STAT_KILLED;
		if (s->ops && s->ops->kill)
		{
			(*s->ops->kill)(s);
		}
		ms = (media_sinker*)s->media_sinker;
		s->media_sinker = NULL;
	}
	RELEASE_LOCK(s->lock);

	if (ms)
	{
		media_sinker_kill_unref(ms);
	}
}


void
session_kill_unref(session *s)
{
	session_kill(s);
	session_unref(s);
}


int32_t
session_add_sinker(session *s, void *sinker)
{
	media_sinker *ms = (media_sinker*)sinker;
	int32_t err = -EKILLED;

	AQUIRE_LOCK(s->lock);
	if (!__session_killed(s))
	{
		err = -EEXIST;
		if(!s->media_sinker)
		{
			s->media_sinker = media_sinker_ref(ms);
			err = media_sinker_set_owner(ms, s);
		}
	}
	RELEASE_LOCK(s->lock);

	return err;
}


int32_t
session_ready(session *s)
{
	int32_t err = -EKILLED;
	media_sinker *ms = NULL;

	AQUIRE_LOCK(s->lock);
	if (!__session_killed(s))
	{
		err = 0;
		if (s->ops && s->ops->ready)
		{
			err = (*s->ops->ready)(s);
		}

		if (!err)
		{
			ms = media_sinker_ref(s->media_sinker);
        }
	}
	RELEASE_LOCK(s->lock);

	if (ms)
	{
		err = media_sinker_ready(ms);
		media_sinker_unref(ms);
	}

	return err;
}

int32_t
session_ctl(session *s, int32_t cmd, void *data)
{
    int32_t err = -EKILLED;
	media_sinker *ms = NULL;

	AQUIRE_LOCK(s->lock);
	if (!__session_killed(s))
	{
		err = 0;
		if (!err)
		{
			ms = media_sinker_ref(s->media_sinker);
        }
	}
	RELEASE_LOCK(s->lock);

	if (ms)
	{
		err = media_sinker_ctl(ms, cmd, data);
		media_sinker_unref(ms);
	}

	return err;
}

int32_t
session_set_sinker_config(session *s, void *in, void *data)
{
	int32_t err = -EKILLED;
	media_sinker *ms = NULL;

	AQUIRE_LOCK(s->lock);
	if (!__session_killed(s))
	{
		err = -EINVAL;
		if (s->media_sinker)
		{
			ms = media_sinker_ref(s->media_sinker);
		}
	}
	RELEASE_LOCK(s->lock);

	if (ms)
	{
		err = media_sinker_set_config(ms, in, data);
		media_sinker_unref(ms);
	}

	return err;
}


int32_t
session_get_sinker_config(session *s, void *in, void *data)
{
	int32_t err = -EKILLED;
	media_sinker *ms = NULL;

	AQUIRE_LOCK(s->lock);
	if (!__session_killed(s))
	{
		err = -EINVAL;
		if (s->media_sinker)
		{
			ms = media_sinker_ref(s->media_sinker);
		}
	}
	RELEASE_LOCK(s->lock);

	if (ms)
	{
		err = media_sinker_get_config(ms, in, data);
		media_sinker_unref(ms);
	}

	return err;
}


uint8_t *
session_id(session *s)
{
	return s->id;
}


int32_t
session_equal(session *s, uint8_t *id)
{
	return !strcmp(__str(s->id), __str(id));
}


//:~ End
