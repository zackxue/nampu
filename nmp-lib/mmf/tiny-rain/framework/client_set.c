/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#include "client_set.h"


void client_set_init(client_set *cs)
{
	cs->lock = LOCK_NEW();
	INIT_LIST_HEAD(&cs->c_list);
}


static __inline__ int32_t
__client_set_add(client_set *cs, client *c)
{
	int32_t err = -EINVAL;

	if (list_empty(&c->list_node))
	{
		err = 0;
		c->cs = cs;
		list_add(&c->list_node, &cs->c_list);
	}

	return err;
}


int32_t client_set_add(client_set *cs, client *c)
{
	int32_t err;

	AQUIRE_LOCK(cs->lock);
	err = __client_set_add(cs, c);
	if (!err)
	{
		client_ref(c);
	}
	RELEASE_LOCK(cs->lock);

	return err;
}


static __inline__ int32_t
__client_set_del(client_set *cs, client *c)
{
	int32_t err = -EINVAL;

	if (!list_empty(&c->list_node))
	{
		err = 0;
		c->cs = NULL;
		list_del_init(&c->list_node);
	}

	return err;
}


void client_set_del(client_set *cs, client *c)
{
	int32_t err;

	AQUIRE_LOCK(cs->lock);
	err = __client_set_del(cs, c);
	RELEASE_LOCK(cs->lock);

	if (!err)
	{
		client_unref(c);
	}
}


//:~ End
