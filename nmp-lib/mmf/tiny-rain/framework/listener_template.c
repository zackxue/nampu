/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#include <arpa/inet.h>
#include "listener_template.h"
#include "server.h"
#include "unix_sock.h"
#include "tr_log.h"


static JBool
listener_tmpl_on_event(JEvent *ev, int revents,
	void *user_data)
{
	listener_tmpl *lt;
	int32_t sock;
	client *cli;
	tr_server *server;

	lt = (listener_tmpl*)j_event_u(ev);

	if (revents & EV_TIMER)
	{
		LOG_I("=== LISTEN WAITING ===");
		return TRUE;
	}

	sock = unix_sock_tcp_accept(lt->sock);
	if (sock < 0 && sock != -EAGAIN)
	{
		LOG_E(
			"listener_tmpl_on_event()->unix_sock_tcp_accept() failed.\n"
		);
		return FALSE;
	}

	cli = listener_generate((listener*)lt, (void*)sock);
	if (cli)
	{
		LOG_I(
			"Generate new client, sock:'%d'.", sock
		);

		server = (tr_server*)listener_get_owner((listener*)lt);
		client_set_add(&server->cs, cli);

		LOG_I(
			"Add new client, sock:'%d'.", sock
		);

		client_attach(cli, server->sched);
		obj_unref(cli);

		LOG_I(
			"Generate attached client, sock:'%d', ref:%d", sock, cli->__super.ref_count
		);
	}
	
	return TRUE;
}


static void
listener_tmpl_on_ev_fin(JEvent *ev)
{
	listener_tmpl *lt;

	lt = (listener_tmpl*)j_event_u(ev);
	obj_unref(lt);
}


static int32_t
listener_tmpl_init(listener *l, void *u)
{
	int32_t err;
	listener_tmpl *lt = (listener_tmpl*)l;
	listener_tmpl_ops *ops = (listener_tmpl_ops*)u;

	lt->port = 0;
	lt->host = 0;
	lt->sock = -1;
	lt->event = NULL;
	lt->ops = NULL;
	lt->user_data = NULL;

	if (ops->init)
	{
		err = (*ops->init)(lt);
		if (err)
		{
			return err;
		}
	}

	lt->ops = ops;
	return 0;
}


static void
listener_tmpl_fin(listener *l)
{
	BUG();
}


static int32_t
listener_tmpl_set_port(listener *l, uint16_t port)
{
	listener_tmpl *lt;
	JEvent *ev;
	int32_t sock;

	sock = unix_sock_bind(L4_TCP, 0, htons(port), FORCE_BIND);
	if (sock < 0)
	{
		return sock;
	}

	unix_sock_tcp_listen(sock);

	ev = j_event_new(sizeof(*ev), sock, EV_READ);
	if (!ev)
	{
		unix_sock_close(sock);
		return -ENOMEM;
	}

	j_event_set_callback(ev, listener_tmpl_on_event,
		l, listener_tmpl_on_ev_fin);
	j_event_set_timeout((JEvent*)ev, 3*1000);
	obj_ref(l);

	BUG_ON(unix_sock_set_flags(sock, O_NONBLOCK));

	lt = (listener_tmpl*)l;
	lt->port = port;
	lt->host = 0;
	lt->sock = sock;
	lt->event = ev;
	lt->user_data = NULL;

	return 0;
}


static int32_t
listener_tmpl_run(listener *l)
{
	listener_tmpl *lt;
	tr_server *server;

	lt = (listener_tmpl*)l;
	if (!lt->event)
	{
		return -EINVAL;
	}

	server = (tr_server*)listener_get_owner((listener*)lt);
	j_sched_add(server->sched, lt->event, 1);

	return 0;
}


static client*
listener_tmpl_generate_client(listener *l, void *parm)
{
	client *c = NULL;
	listener_tmpl *lt = (listener_tmpl*)l;

	if (lt->ops && lt->ops->new_cli)
	{
		c = (*lt->ops->new_cli)(lt, (int32_t)parm);
	}

	return c;
}


static listener_ops l_ops =
{
	.init		= listener_tmpl_init,
	.fin		= listener_tmpl_fin,
	.set_port	= listener_tmpl_set_port,
	.run		= listener_tmpl_run,
	.new_cli	= listener_tmpl_generate_client
};


listener *alloc_listener_tmpl(listener_tmpl_ops *ops)
{
	return listener_alloc(sizeof(listener_tmpl), &l_ops, ops);
}


//:~ End
