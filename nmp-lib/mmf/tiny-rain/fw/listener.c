#include "listener.h"
#include "unix_sock.h"


static __inline__ void
listener_finalize(listener *l)
{
}


static __inline__ void
on_obj_fin(obj *p)
{
	listener *l = (listener*)p;

	if (l->ops && l->ops->fin)
	{
		(*l->ops->fin)(l);
	}

	listener_finalize(l);
}


listener *listener_alloc(uint32_t size, listener_ops *ops,
	void *u)
{
	listener *l;
	int32_t err;

	if (size < sizeof(*l))
		return NULL;

	l = (listener*)obj_new(size, on_obj_fin);
	l->server = NULL;

	if (ops && ops->init)
	{
		err = (*ops->init)(l, u);
		if (err)
		{
			listener_finalize(l);
			obj_unref(l);
			return NULL;
		}
	}

	l->ops = ops;
	return l;
}


int32_t listener_bind(listener *l, uint16_t port)
{
	int32_t err = -EINVAL;

	if (l->ops && l->ops->set_port)
	{
		err = (*l->ops->set_port)(l, port);
	}

	return err;
}


int32_t listener_start(listener *l)
{
	int32_t err = -EINVAL;

	if (l->ops && l->ops->run)
	{
		err = (*l->ops->run)(l);
	}

	return err;	
}


client *listener_generate(listener *l, void *parm)
{
	client *c = NULL;

	if (l->ops && l->ops->new_cli)
	{
		c = (*l->ops->new_cli)(l, parm);
	}
	else
	{
		unix_sock_close((int32_t)parm);
	}

	return c;
}


void listener_set_owner(listener *l, void *p)
{
	l->server = p;
}


void *listener_get_owner(listener *l)
{
	return l->server;
}


//:~ End
