#include "server.h"
#include "alloc.h"
#include "tr_factory.h"


static __inline__ tr_server *
alloc_server( void )
{
	tr_server *s;

	s = tr_alloc0(sizeof(*s));
	return s;
}


tr_server *tr_create_server(uint32_t factory)
{
	tr_server *s;
	tr_factory *f;

	f = get_tr_factory(factory);
	if (!f)
	{
		return NULL;
	}

	if (init_factory(f))
	{
		return NULL;
	}

	s = alloc_server();
	if (s)
	{
		client_set_init(&s->cs);
		s->l = factroy_create_sclr(f);
		s->sched = factory_create_scheduler(f);

		listener_set_owner(s->l, s);
	}

	return s;
}


int32_t server_bind_port(tr_server *svr, uint16_t port)
{
	if (!svr || !svr->l)
	{
		return -EINVAL;
	}

	return listener_bind(svr->l, port);
}


int32_t start_server(tr_server *svr)
{
	if (!svr || !svr->l)
	{
		return -EINVAL;
	}

	return listener_start(svr->l);
}


//:~ End
