#include "tr_log.h"
#include "network_client.h"
#include "tr_factory.h"
#include "unix_sock.h"

#define DEFAULT_TIMEOUT_SEC		15
#define DEFAULT_CHECK_TIMES		3

enum
{
	STATE_INIT,
	STATE_KILLED
};


typedef struct __init_parm init_parm;
struct __init_parm
{
	void *opts;
	void *io;
	uint32_t factory;
};


static __inline__ int32_t
__network_client_killed(network_client *nc)
{
	return nc->state == STATE_KILLED;
}


static proto_parser *
network_client_create_parser(void *u)
{
	network_client *nc = (network_client*)u;
	tr_factory *factory;

	factory = get_tr_factory(nc->factory);
	if (!factory)
	{
		LOG_W(
		"network_client_create_parser() failed, "
		"no factory '%d'!",
		nc->factory);
		return NULL;
	}

	return factory_create_scpp(factory);
}


static void
network_client_release_parser(proto_parser *parser, void *u)
{
	network_client *nc = (network_client*)u;
	tr_factory *factory;

	factory = get_tr_factory(nc->factory);
	if (!factory)
	{
		LOG_W(
		"network_client_release_parser() failed, "
		"no factory '%d'!",
		nc->factory);
		BUG();
		return;
	}

	factory_destroy_scpp(factory, parser);
}


static int32_t
network_client_msg_sent(proto_watch *w, uint32_t seq, void *u)
{
	int32_t err = -EINVAL;
	network_client *nc = (network_client*)u;

	if (nc->ops && nc->ops->msg_sent)
	{
		err = (*nc->ops->msg_sent)(nc, seq);
	}

	return err;
}


static __inline__ int32_t
__network_client_msg_recv(network_client *nc, msg *m)
{
	int32_t err = -EINVAL;

	if (nc->ops && nc->ops->msg_recv)
	{
		err = (*nc->ops->msg_recv)(nc, m);
	}

	return err;
}


static int32_t
network_client_msg_recv(proto_watch *w, msg *m, void *u)
{
	int32_t err = 0;
	network_client *nc = (network_client*)u;

	if (!__network_client_killed(nc))
	{
		err = __network_client_msg_recv(nc, m);
	}
	return err;
}


static __inline__ void
__network_client_conn_closed(network_client *nc)
{
	if (nc->ops && nc->ops->closed)
	{
		(*nc->ops->closed)(nc);		
	}
}


static void
network_client_conn_closed(proto_watch *w, void *u)
{
	network_client *nc = (network_client*)u;

	network_client_ref(nc);
	__network_client_conn_closed(nc);
	network_client_kill_unref(nc);
}


static __inline__ void
__network_client_conn_error(network_client *nc, int32_t err)
{
	if (nc->ops && nc->ops->error)
	{
		(*nc->ops->error)(nc, err);
	}
}


static void
network_client_conn_error(proto_watch *w, int32_t err, void *u)
{
	network_client *nc = (network_client*)u;

	network_client_ref(nc);
	__network_client_conn_error(nc, err);
	network_client_kill_unref(nc);
}


static proto_watch_ops network_client_watch_impl =
{
	.create_proto_parser	= network_client_create_parser,
	.release_proto_parser	= network_client_release_parser,
	.proto_msg_sent			= network_client_msg_sent,
	.proto_msg_recv			= network_client_msg_recv,
	.conn_closed			= network_client_conn_closed,
	.conn_error				= network_client_conn_error
};


static void
proto_watch_on_finalize(void *u)
{
	network_client *nc = (network_client*)u;
	obj_unref(nc);
}


static __inline__ int32_t
init_this(network_client *nc, uint32_t factory, void *io)
{
	int32_t fd = (int32_t)io;

	nc->ip[0] = '\0';
	unix_sock_get_peer(fd, nc->ip, MAX_IP4_LEN);

	nc->factory = factory;
	nc->proto_watch = proto_watch_new(io, DEFAULT_TIMEOUT_SEC * DEFAULT_CHECK_TIMES * 1000, 
		&network_client_watch_impl, nc, proto_watch_on_finalize);
	if (!nc->proto_watch)
	{
		LOG_W("network_client()->proto_watch_new() failed.");
		unix_sock_close((int32_t)io);
		return -EINVAL;
	}

	proto_watch_set_window(nc->proto_watch, DEFAULT_PW_WINSIZE);
	obj_ref(nc);
	nc->state = 0;
	nc->lock = LOCK_NEW();

	return 0;
}


static __inline__ void
release_proto_watch(network_client *nc)
{
	if (nc->proto_watch)
	{
		proto_watch_kill_unref(nc->proto_watch);
		nc->proto_watch = NULL;
	}
}


static __inline__ void
finalize_this(network_client *nc)
{
	LOCK_DEL(nc->lock);
	release_proto_watch(nc);
}


static int32_t
network_client_init(client *c, void *data)
{
	network_client *nc = (network_client*)c;
	init_parm *parm = (init_parm*)data;
	int32_t err;

	err = init_this(nc, parm->factory, parm->io);
	if (err)
	{
		LOG_W("network_client_init()->init_this() failed.");
		return err;
	}

	nc->ops = parm->opts;
	if (nc->ops && nc->ops->init)
	{
		err = (*nc->ops->init)(nc);
		if (err)
		{
			LOG_W("network_client_init()->(*nc->ops->init)() failed.");
			nc->ops = NULL;
			finalize_this(nc); //@{obj_unref() can't reach this layer}
			return err;
		}
	}

	return 0;
}


static void
network_client_finalize(client *c)
{
	network_client *nc = (network_client*)c;

	if (nc->ops && nc->ops->fin)
	{
		(*nc->ops->fin)(nc);
	}

	finalize_this(nc);
}


static __inline__ void
network_client_self_kill(network_client *nc)
{
	proto_watch *pw = NULL;

	AQUIRE_LOCK(nc->lock);
	if (!__network_client_killed(nc))
	{
		nc->state = STATE_KILLED;
		pw = nc->proto_watch;
		nc->proto_watch = NULL;
	}
	RELEASE_LOCK(nc->lock);

	if (pw)
	{
		proto_watch_kill_unref(pw);
	}
}


static void
network_client_kill(client *c)
{
	network_client *nc = (network_client*)c;

	if (nc->ops && nc->ops->kill)
	{
		(*nc->ops->kill)(nc);
	}
	network_client_self_kill(nc);
}


static __inline__ session *
__network_client_create_session(network_client *nc, void *p)
{
	session *s = NULL;

	if (nc->ops && nc->ops->create_session)
	{
		s = (*nc->ops->create_session)(nc, p);
	}

	return s;
}


static session *
network_client_create_session(client *c, void *p)
{
	session *s;
	network_client *nc = (network_client*)c;

	s = __network_client_create_session(nc, p);
	return s;
}


static __inline__ int32_t
__network_client_attach(network_client *nc, void *loop)
{
	int32_t err = -EINVAL;

	if (nc->proto_watch)
	{
		err = proto_watch_attach(nc->proto_watch, loop);
	}

	return err;
}


static int32_t
network_client_attach(client *c, void *loop)
{
	int32_t err;
	network_client *nc = (network_client*)c;

	err = __network_client_attach(nc, loop);
	return err;
}


static client_ops client_ops_impl =
{
	.init				= network_client_init,
	.fin				= network_client_finalize,
	.kill				= network_client_kill,
	.create_s			= network_client_create_session,
	.attach				= network_client_attach,
};


network_client *
network_client_new(uint32_t size, network_client_ops *ops,
	uint32_t factory, void *io)
{
	init_parm parm;

	if (size < sizeof(network_client))
	{
		unix_sock_close((int32_t)io);
		return NULL;
	}

	parm.opts = ops;
	parm.io = io;
	parm.factory = factory;

	return (network_client*)client_alloc(
		size, &client_ops_impl,&parm);
}


network_client *
network_client_ref(network_client *nc)
{
	return (network_client*)client_ref((client*)nc);
}


void
network_client_kill_unref(network_client *nc)
{
	client_kill_unref((client*)nc);
}


int32_t
network_client_consumable(network_client *nc, uint32_t size)
{
	int32_t err = -EKILLED;
	proto_watch *pw = NULL;

	AQUIRE_LOCK(nc->lock);
	if (!__network_client_killed(nc))
	{
		pw = proto_watch_ref(nc->proto_watch);
	}
	RELEASE_LOCK(nc->lock);

	if (pw)
	{
		err = proto_watch_writeable(pw, size);
		proto_watch_unref(pw);
	}

	return err;
}


int32_t
network_client_send_msg(network_client *nc, msg *m, uint32_t seq,
	uint32_t flags)
{
	int32_t err = -EKILLED;
	proto_watch *pw = NULL;

	AQUIRE_LOCK(nc->lock);
	if (!__network_client_killed(nc))
	{
		pw = proto_watch_ref(nc->proto_watch);
	}
	RELEASE_LOCK(nc->lock);

	if (pw)
	{
		err = proto_watch_write(pw, m, seq, flags);
		proto_watch_unref(pw);
	}
	else
	{
		msg_unref(m);
	}

	return err;
}


int32_t
network_client_send_mb(network_client *nc, mem_block *mb,
	uint32_t flags)
{
	int32_t err = -EKILLED;
	proto_watch *pw = NULL;

	AQUIRE_LOCK(nc->lock);
	if (!__network_client_killed(nc))
	{
		pw = proto_watch_ref(nc->proto_watch);
	}
	RELEASE_LOCK(nc->lock);

	if (pw)
	{
		err = proto_watch_write_mb(pw, mb, flags);
		proto_watch_unref(pw);
	}
	else
	{
		free_memblock(mb);
	}

	return err;
}


//:~ End
