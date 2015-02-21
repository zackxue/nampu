/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <strings.h>
#include <signal.h>
#include "rtsp-server.h"
#include "rc_log.h"
#include "rtsp.h"
#include "ports.h"
#include "bufferqueue.h"

#define RTSP_PORT			"6553"
#define RTSP_HOST			"0.0.0.0"
#define DEFAULT_LOOPS		1
#define DEFAULT_TIMEOUT		3

RTSP_server *
rtsp_server_new( void )
{
	RTSP_server *server;

	server = g_new0(RTSP_server, 1);
	server->ref_count = 1;
	server->host = g_strdup(RTSP_HOST);
	server->port = g_strdup(RTSP_PORT);

	return server;
}


static __inline__ void
rtsp_listener_add_conn(Listen_watch *listener, RTSP_Client *client)
{
	/**
	 * Notes:
	 * conn list owns reference 1 of client object.
	*/
	g_mutex_lock(listener->lock);
	listener->conns = g_list_prepend(listener->conns, client);
	rtsp_client_ref(client);
	g_mutex_unlock(listener->lock);
}


void
rtsp_listener_del_conn(Listen_watch *listener, RTSP_Client *client)
{
	GList *list;

	g_mutex_lock(listener->lock);
	list = g_list_find(listener->conns, client);
	if (list)
	{
		listener->conns = g_list_delete_link(listener->conns, list);
	}
	g_mutex_unlock(listener->lock);

	if (list)
	{
		rtsp_client_unref(client);
	}
}


gint
__set_fd_flags(gint fd, gint flgs)
{
    gint old_flgs;

    old_flgs = fcntl(fd, F_GETFL, 0);
    if (G_UNLIKELY(old_flgs < 0))
        return -errno;

    old_flgs |= flgs;

    if (fcntl(fd, F_SETFL, old_flgs) < 0)
        return -errno;

    return 0;
}


Sock *
Sock_connect_nowait(char const *host, gint port, Sock *binded,
	sock_type socktype, void *ctx)
{
	Sock *sock;
	gint fd;
	struct sockaddr_in sin;

	sock = Sock_bind(NULL, "0", binded, socktype, ctx);
	if (!sock)
		return sock;

	fd = Sock_fd(sock);
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(host);

	__set_fd_flags(fd, O_NONBLOCK);
	if (connect(fd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		if (errno == EINPROGRESS)
			return sock;

		Sock_close(sock);
		return NULL;
	}

	return sock;
}


static gboolean
rtsp_client_incoming_cb(GEvent *ev, gint revents, void *user_data)
{
	Listen_watch *listener = (Listen_watch*)ev;
	Sock *client_sock, *sock = listener->sock;
	RTSP_Client *client;

    if ((client_sock = Sock_accept(sock, NULL)) == NULL)
    	return TRUE;

	__set_fd_flags(Sock_fd(client_sock), O_NONBLOCK);
	client = rtsp_client_new(client_sock);
	rtsp_listener_add_conn(listener, client);
	SET_OWNER(client, listener);
	g_event_ref((GEvent*)listener);
	g_scheduler_add((GEvent*)client, LOOP_WEIGHT_RTSP);

	rtsp_client_unref(client);	/* Orignal one */

	printf("New client %p, socket:%d!!\n", client, Sock_fd(sock));
	return TRUE;
}


static void
on_listener_finalize(GEvent *ev)
{
	Listen_watch *listener = (Listen_watch*)ev;
	RTSP_server *server = GET_OWNER(listener, RTSP_server);

	rtsp_server_unref(server);	
}


void
rtsp_server_set_port(RTSP_server *server, gint port)
{
	if (!server)
		return;

	g_free(server->port);
	server->port = g_strdup_printf("%d", port);
}


static __inline__ gint
__rtsp_server_bind_port(RTSP_server *server)
{
	Listen_watch *listener;
	Sock *sock;
	g_assert(server);

	sock = Sock_bind(server->host, server->port, NULL, TCP, NULL);
	if (!sock) {
		rc_log(RC_LOG_ERR, "[RC] rtsp-server Sock_bind() on %s:%s failed.",
			server->host, server->port);
		return EC_BIND;
	}

    if(Sock_listen(sock, SOMAXCONN)) {
        rc_log(RC_LOG_ERR, "[RC] rtsp-server Sock_listen() failed.");
        Sock_close(sock);
        return EC_LISTEN;
    }

	listener = (Listen_watch*)g_event_new(sizeof(Listen_watch),
		Sock_fd(sock), EV_READ);
	listener->sock = sock;
	listener->lock = g_mutex_new();
	listener->conns = NULL;

	__set_fd_flags(Sock_fd(sock), O_NONBLOCK);
	g_event_set_callback((GEvent*)listener, rtsp_client_incoming_cb,
		server, on_listener_finalize);

	server->listener = listener;
	SET_OWNER(listener, server);
	rtsp_server_ref(server);

	g_scheduler_add((GEvent*)listener, LOOP_WEIGHT_RTSP);

	return 0;
}


gint
rtsp_server_bind_port(RTSP_server *server)
{
	if (!server)
		return EC_ARGS;

	static gsize init_environment = FALSE;
	if (g_once_init_enter(&init_environment))
	{
		g_scheduler_init(DEFAULT_LOOPS);
		jpf_media_ports_set_range(20000, 25000);
		bq_init();
		signal(SIGPIPE, SIG_IGN);
		g_once_init_leave(&init_environment, TRUE);
	}

	return __rtsp_server_bind_port(server);
}


static __inline__ void
rtsp_server_kill_listener(Listen_watch *listener)
{//@{TODO}
}


void
rtsp_server_finalize(RTSP_server *server)
{
	g_assert(server);
	g_free(server->host);
	g_free(server->port);
	g_free(server);
}


RTSP_server *
rtsp_server_ref(RTSP_server *server)
{
	__OBJECT_REF(server);
}


void
rtsp_server_unref(RTSP_server *server)
{
	__OBJECT_UNREF(server, rtsp_server);
}


static __inline__ void
rtsp_server_kill(RTSP_server *server)
{
	rtsp_server_kill_listener(server->listener);
	g_event_unref((GEvent*)server->listener);
}


void rtsp_server_free(RTSP_server *server)
{
	g_assert(server);

	rtsp_server_kill(server);
	rtsp_server_unref(server);	
}


Reverse_connector *
rtsp_server_reverse_connect(RTSP_server *server, gchar *dest_ip,
	gint dest_port, gchar *puid, gint ka_freq, gint l4proto,
	Connect_exp cb, gpointer u)
{
	Reverse_connector *ctr;
	Sock *sock;
	gchar port[8];

	if (!server || !server->listener)
		return NULL;

	sock = Sock_connect_nowait(dest_ip, dest_port, NULL, TCP, NULL);
	if (!sock)
		return NULL;

	ctr = (Reverse_connector*)rtsp_client_new_base(
		sizeof(Reverse_connector),
		sock,
		EV_WRITE,
		ka_freq <= 0 ? DEFAULT_TIMEOUT : ka_freq
	);

	snprintf(port, 8, "%d", dest_port);
	rtsp_reverse_cntr_init(ctr, dest_ip, port, puid, ka_freq, l4proto, cb, u);

	rtsp_listener_add_conn(server->listener, (RTSP_Client*)ctr);
	SET_OWNER((RTSP_Client*)ctr, server->listener);
	g_event_ref((GEvent*)server->listener);
	g_scheduler_add((GEvent*)ctr, LOOP_WEIGHT_RTSP);

	return ctr;
}


void
rtsp_server_release_reverse_connector(RTSP_server *server,
	Reverse_connector *r_cntr)
{
	if (!server || !r_cntr)
		return;

	rtsp_client_kill((RTSP_Client*)r_cntr);
	rtsp_client_orphan((RTSP_Client*)r_cntr);
	g_event_remove((GEvent*)r_cntr);
	g_event_unref((GEvent*)r_cntr);

	printf("kill connector:%p!!\n", r_cntr);
}


//:~ End
