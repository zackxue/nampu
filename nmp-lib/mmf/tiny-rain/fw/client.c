/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#include "client.h"
#include "client_set.h"
#include "tr_log.h"

static atomic_t client_objs = ATOMIC_INIT;

static __inline__ void
init_this(client *c)
{
	INIT_LIST_HEAD(&c->list_node);
	INIT_LIST_HEAD(&c->s_list);
	c->cs = NULL;
	c->sched = NULL;
	c->lock = LOCK_NEW();
	c->ops = NULL;
	atomic_inc(&client_objs);
}


static __inline__ void
fin_this(client *c)
{
	LOCK_DEL(c->lock);
	BUG_ON(!list_empty(&c->list_node));
	BUG_ON(!list_empty(&c->s_list));
	atomic_dec(&client_objs);

	LOG_V(
		"Client objs count:%d.",
		atomic_get(&client_objs)
	);
}


static __inline__ void
on_obj_fin(obj *p)
{
	client *c = (client*)p;

	if (c->ops && c->ops->fin)
	{
		(*c->ops->fin)(c);
	}

	fin_this(c);
}


client *client_alloc(uint32_t size, client_ops *ops, void *u)
{
	int32_t err;
	client *c;

	if (size < sizeof(*c))
		return NULL;

	c = (client*)obj_new(size, on_obj_fin);
	if (c)
	{
		init_this(c);
		c->ops = ops;

		if (ops && ops->init)
		{
			err = (*ops->init)(c, u);
			if (err)
			{
				c->ops = NULL;
				obj_unref(c);	//@{fin_this()}
				return NULL;
			}
		}
	}

	return c;
}


client *client_ref(client *c)
{
	if (c)
	{
		obj_ref(c);
	}

	return c;
}


void client_unref(client *c)
{
	if (c)
	{
		obj_unref(c);
	}
}


static __inline__ void
client_kill_all_s(client *c)
{
	session *s;
	struct list_head tmp, *l;

	AQUIRE_LOCK(c->lock);
	list_add(&tmp, &c->s_list);
	list_del_init(&c->s_list);
	RELEASE_LOCK(c->lock);

	while (!list_empty(&tmp))
	{
		l = tmp.next;
		list_del_init(l);

		s = list_entry(l, session, list);
		session_kill_unref(s);
	}
}


static __inline__ void
client_self_kill(client *c)
{
	client_kill_all_s(c);

	if (c->cs)
	{
		client_set_del((client_set*)c->cs, c);
	}
}


static __inline__ void
client_kill(client *c)
{
	if (c->ops && c->ops->kill)
	{
		(*c->ops->kill)(c);
	}
	client_self_kill(c);
}


void client_kill_unref(client *c)
{
	client_kill(c);
	client_unref(c);
}


static __inline__ int32_t
__client_add_s(client *c, session *s)
{
	int32_t err;

	if (!(err = session_add(s, c)))
	{
		session_ref(s);
		return 0;
	}
	return err;
}


static __inline__ int32_t
client_add_s(client *c, session *s)
{
	int32_t err;

	AQUIRE_LOCK(c->lock);
	err = __client_add_s(c, s);
	RELEASE_LOCK(c->lock);

	return err;
}


static __inline__ session *
__client_find_s(client *c, uint8_t *id)
{
	session *s;
	struct list_head *l;

	list_for_each(l, &c->s_list)
	{
		s = list_entry(l, session, list);
		if (session_equal(s, id))
		{
			return s;
		}
	}

	return NULL;
}


static __inline__ int32_t
__client_del_s(client *c, session *s)
{
	if (!__client_find_s(c, session_id(s)))
		return -ENOENT;

	return session_del(s, c);
}


void client_del_s(client *c, session *s)
{
	int32_t err;

	AQUIRE_LOCK(c->lock);
	err = __client_del_s(c, s);
	RELEASE_LOCK(c->lock);

	if (!err)
	{
		session_unref(s);
	}
}


session *client_create_s(client *c, void *p)
{
	session *s = NULL;

	if (c->ops && c->ops->create_s)
	{
		s = (*c->ops->create_s)(c, p);
		if (s)
		{
			client_add_s(c, s);
		}
	}
	return s;
}


session *client_find_and_get_s(client *c, uint8_t *id)
{
	session *s;

	AQUIRE_LOCK(c->lock);
	s = __client_find_s(c, id);
	if (s)
	{
		session_ref(s);
	}
	RELEASE_LOCK(c->lock);

	return s;
}


void client_kill_unref_s(client *c, session *s)
{
	client_del_s(c, s);
	session_kill_unref(s);
}


int32_t client_attach(client *c, void *sched)
{
	int32_t err = -EINVAL;

	c->sched = sched;
	if (c->ops && c->ops->attach)
	{
		err = (*c->ops->attach)(c, sched);
	}

	return err;
}


//:~ End
