#include "network_sinker.h"
#include "ports.h"
#include "tr_log.h"
#include "tr_factory.h"
#include "unix_sock.h"


static proto_parser*
network_sinker_create_proto_parser(void *u)
{
	network_sinker *sinker = (network_sinker*)u;
	tr_factory *factory;

	factory = get_tr_factory(sinker->factory);
	if (!factory)
	{
		LOG_W(
		"network_sinker_create_proto_parser() failed, "
		"no factory '%d'!",
		sinker->factory);
		return NULL;
	}

	return factory_create_spp(factory);
}


static void
network_sinker_free_proto_parser(proto_parser *parser, void *u)
{
	network_sinker *sinker = (network_sinker*)u;
	tr_factory *factory;

	factory = get_tr_factory(sinker->factory);
	if (!factory)
	{
		LOG_W(
		"network_sinker_free_proto_parser() failed, "
		"no factory '%d'!",
		sinker->factory);
		return;
	}

	factory_destroy_spp(factory, parser);
}


static int32_t
network_sinker_msg_sent(proto_watch *w, uint32_t seq, void *u)
{
//	network_sinker *sinker = (network_sinker*)u;
	return 0;
}


static int32_t
network_sinker_msg_recv(proto_watch *w, msg *m, void *u)
{
//	network_sinker *sinker = (network_sinker*)u;
	return 0;
}


static void
network_sinker_conn_closed(proto_watch *w, void *u)
{
//	network_sinker *sinker = (network_sinker*)u;
}


static void
network_sinker_conn_error(proto_watch *w, int32_t err, void *u)
{
//	network_sinker *sinker = (network_sinker*)u;
}


static proto_watch_ops network_sinker_watch_ops =
{
	.create_proto_parser	= network_sinker_create_proto_parser,
	.release_proto_parser	= network_sinker_free_proto_parser,
	.proto_msg_sent			= network_sinker_msg_sent,
	.proto_msg_recv			= network_sinker_msg_recv,
	.conn_closed			= network_sinker_conn_closed,
	.conn_error				= network_sinker_conn_error
};


static void
network_sinker_on_watch_finalize(void *u)
{
	network_sinker *sinker = (network_sinker*)u;
	media_sinker_unref((media_sinker*)sinker);
}


static __inline__ void *
network_sinker_new_io(int32_t l4_proto, int32_t *port)
{
	int32_t fd, err;
	int32_t port_low, port_hi;

	err = tr_ports_get_pair(&port_low, &port_hi);
	if (err)
	{
		LOG_W(
			"tr_ports_get_pair() failed, system out of ports."
		);
		return NULL;
	}

	BUG_ON(port_low + 1 != port_hi);
	fd = unix_sock_bind(l4_proto, 0, htons(port_low), 0);
	if (fd < 0)
	{
		LOG_W(
			"unix_sock_bind() failed, err:'%d'.", fd
		);
		tr_ports_put_pair(port_low, port_hi);
		return NULL;
	}

	*port = port_low;
	return (void*)fd;
}


static __inline__ void
network_sinker_release_io(void *io)
{
	unix_sock_close((int32_t)io);
}


static __inline__ void
network_sinker_reset(network_sinker *sinker)
{
	int32_t stm_index;

	memset(sinker->stms, 0, sizeof(sinker->stms));
	for (stm_index = 0; stm_index < ST_MAX; ++stm_index)
	{
		sinker->stms[stm_index].stm_type = ST_NONE;
	}	
}


static __inline__ void
network_sinker_clear(network_sinker *sinker)
{
	int32_t stm_index, port;
	proto_watch *watch;

	for (stm_index = ST_MAX - 1; stm_index > ST_NONE; --stm_index)
	{
		if (sinker->stms[stm_index].stm_type != ST_NONE)
		{
			watch = sinker->stms[stm_index].stm_watch;
			port = sinker->stms[stm_index].port;

			if (watch)
			{
				proto_watch_kill_unref(watch);
				tr_ports_put_pair(port, port + 1);
			}

			sinker->stms[stm_index].stm_type = ST_NONE;
		}
	}
}


static __inline__ proto_watch *
network_sinker_create_watch(network_sinker *sinker, void *io)
{
	proto_watch *watch;

	watch = proto_watch_new(io, 0, &network_sinker_watch_ops,
		sinker, network_sinker_on_watch_finalize);
	if (!watch)
	{
		LOG_W("network_sinker()->proto_watch_new() failed.");
		return NULL;
	}

	proto_watch_set_window(watch, DEFAULT_PW_WINSIZE);
	media_sinker_ref((media_sinker*)sinker);
	return watch;
}


static int32_t
network_sinker_init(media_sinker *_sinker, void *u)
{
	int32_t stm_index, port, interleaved = 0;
	void *io;
	proto_watch *watch = NULL;
	sinker_param *sp = (sinker_param*)u;
	network_sinker *sinker = (network_sinker*)_sinker;

	network_sinker_reset(sinker);
	sinker->factory = sp->factory;
	sinker->l4_proto = sp->l4_proto;

	for (stm_index = 0; stm_index < ST_MAX; ++stm_index)
	{
		if (sp->stms_type[stm_index] != ST_NONE)
		{
			if (sp->l4_proto != L4_NONE)
			{
				io = network_sinker_new_io(sp->l4_proto, &port);
				if (!io)
				{
					goto init_failed;
				}
	
				watch = network_sinker_create_watch(sinker, io);
				if (!watch)
				{
					network_sinker_release_io(io);
					tr_ports_put_pair(port, port + 1);
					goto init_failed;
				}

				sinker->stms[stm_index].stm_type = sp->stms_type[stm_index];
				sinker->stms[stm_index].port = port;
				sinker->stms[stm_index].interleaved = interleaved;
				sinker->stms[stm_index].stm_watch = watch;
			}
			else
			{
				sinker->stms[stm_index].stm_type = sp->stms_type[stm_index];
				sinker->stms[stm_index].port = interleaved;
				sinker->stms[stm_index].interleaved = interleaved;
				sinker->stms[stm_index].stm_watch = NULL;
			}

			interleaved += 2;
		}
		else
		{
			sinker->stms[stm_index].stm_type = ST_NONE;
		}
	}

	return 0;

init_failed:
	network_sinker_clear(sinker);
	return -EINVAL;
}


static void
network_sinker_finalize(media_sinker *sinker)
{
	network_sinker *ns = (network_sinker*)sinker;

	if (ns->ops && ns->ops->fin)
	{
		(*ns->ops->fin)(ns);
	}
	network_sinker_clear((network_sinker*)sinker);
}


static void
network_sinker_kill(media_sinker *sinker)
{
	network_sinker *ns = (network_sinker*)sinker;

	if (ns->ops && ns->ops->kill)
	{
		(*ns->ops->kill)(ns);
	}
	network_sinker_clear((network_sinker*)sinker);
}


static int32_t
network_sinker_consumable(media_sinker *sinker, int32_t stm_i, uint32_t size)
{
	network_sinker *ns = (network_sinker*)sinker;
	int32_t err;
	proto_watch *pw;

	if (!VALID_STM_TYPE(stm_i))
		return -EINVAL;

	if (ns->stms[stm_i].stm_type == ST_NONE)
		return -EINVAL;

	pw = ns->stms[stm_i].stm_watch;
	err = 0;

	if (ns->ops && ns->ops->consumable)
		err = (*ns->ops->consumable)(ns, pw, stm_i, size);
	return err;
}


static int32_t
network_sinker_consume(media_sinker *sinker, int32_t stm_i, msg *m,
	uint32_t seq)
{
	network_sinker *ns = (network_sinker*)sinker;
	int32_t ret;
	proto_watch *pw;

	if (!VALID_STM_TYPE(stm_i))
		return -EINVAL;

	if (ns->stms[stm_i].stm_type == ST_NONE)
		return -EINVAL;

	pw = ns->stms[stm_i].stm_watch;
	ret = -EPERM;

	if (ns->ops && ns->ops->consume)
		ret = (*ns->ops->consume)(ns, pw, stm_i, m, seq);
	return ret;
}


static int32_t
network_sinker_get_config(media_sinker *sinker, void *in,
	void *data)
{
	int32_t err = -EINVAL;
	network_sinker *ns = (network_sinker*)sinker;
	
	if (ns->ops && ns->ops->get_config)
	{
		err = (*ns->ops->get_config)(ns, in, data);
	}
	return err;
}


static int32_t
network_sinker_set_config(media_sinker *sinker, void *in,
	void *data)
{
	int32_t err = -EINVAL;
	network_sinker *ns = (network_sinker*)sinker;

	if (ns->ops && ns->ops->set_config)
	{
		err = (*ns->ops->set_config)(ns, in, data);
	}
	return err;
}


static media_sinker_ops mso_network_sinker =
{
	.init		 = network_sinker_init,
	.fin		 = network_sinker_finalize,
	.kill		 = network_sinker_kill,
	.consumable  = network_sinker_consumable,
	.consume	 = network_sinker_consume,
	.get_config  = network_sinker_get_config,
	.set_config  = network_sinker_set_config
};


network_sinker *
network_sinker_alloc(uint32_t size, network_sinker_ops *ops, sinker_param *sp)
{
	media_sinker *ms;
	network_sinker *ns;
	int32_t err;

	if (size < sizeof(network_sinker))
		return NULL;

	ms = media_sinker_alloc(size, &mso_network_sinker, sp);
	if (ms)
	{
		ns = (network_sinker*)ms;
		ns->ops = ops;

		if (ops && ops->init)
		{
			err = (*ops->init)(ns, sp);
			if (err)
			{
				ns->ops = NULL;
				media_sinker_kill_unref(ms);
				return NULL;
			}
		}
	}

	return (network_sinker*)ms;
}


//:~ End
