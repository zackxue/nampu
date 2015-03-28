#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "nmp_client_mng.h"
#include "nmp_debug.h"
#include "nmp_timer.h"
#include "nmpevsched.h"
#include "nmp_utils.h"


#define DEFAULT_LISTEN_ADDRESS			"0.0.0.0"
#define MAX_LISTEN_BACKLOG				5

static __inline__ void
nmp_rtsp_client_mng_remove_death(JpfClientMng *client_mng);


static __inline__ void
nmp_rtsp_client_mng_on_timer(JpfClientMng *client_mng)
{
	nmp_rtsp_client_mng_remove_death(client_mng);
}


static gboolean
nmp_rtsp_client_mng_timer(gpointer user_data)
{
	JpfClientMng *client_mng;
	G_ASSERT(user_data != NULL);

	client_mng = (JpfClientMng*)user_data;
	nmp_rtsp_client_mng_on_timer(client_mng);

	return TRUE;
}


JpfClientMng *
nmp_rtsp_client_mng_new(gint service)
{
	JpfClientMng *client_mng;

	client_mng = g_new0(JpfClientMng, 1);
	client_mng->address = g_strdup(DEFAULT_LISTEN_ADDRESS);
	client_mng->service = g_strdup_printf("%d", service);

	client_mng->timer = nmp_set_timer(
		1000,
		nmp_rtsp_client_mng_timer,
		client_mng
	);

	client_mng->lock = g_mutex_new();
	return client_mng;	
}


JpfClientMng *
nmp_rtsp_client_mng_ref(JpfClientMng *client_mng)
{
	return client_mng;
}


void
nmp_rtsp_client_mng_unref(JpfClientMng *client_mng)
{
}


static __inline__ void
__nmp_rtsp_client_mng_add_client(JpfClientMng *client_mng, 
	JpfRtspClient *client)
{
	client_mng->clients = g_list_prepend(
		client_mng->clients, client);
	nmp_rtsp_client_ref(client);
	client->client_mng = client_mng;
}


static __inline__ void
nmp_rtsp_client_mng_add_client(JpfClientMng *client_mng, 
	JpfRtspClient *client)
{
	g_assert(client_mng != NULL && client != NULL);
	
	g_mutex_lock(client_mng->lock);
	__nmp_rtsp_client_mng_add_client(client_mng, client);
	g_mutex_unlock(client_mng->lock);
}


static __inline__ void
__nmp_rtsp_client_mng_remove_client(JpfClientMng *client_mng, 
	JpfRtspClient *client)
{
	GList *list;

	list = g_list_find(client_mng->clients, client);
	if (list)
	{
		client_mng->clients = g_list_delete_link(
			client_mng->clients, list);
		nmp_rtsp_client_unref(client);
	}
}


void
nmp_rtsp_client_mng_remove_client(JpfClientMng *client_mng, 
	JpfRtspClient *client)
{
	g_assert(client_mng != NULL && client != NULL);
	
	g_mutex_lock(client_mng->lock);
	__nmp_rtsp_client_mng_remove_client(client_mng, client);
	g_mutex_unlock(client_mng->lock);	
}



static __inline__ void
nmp_rtsp_client_to_kill(JpfRtspClient *client)	/* timeout kill */
{
	nmp_print(
		"Cli-Timer remove client '%p' '%s:%d'.",
		client,
		client->client_ip ?: "--",
		client->port
	);

	nmp_rtsp_client_set_illegal(client);
}


static __inline__ void
nmp_rtsp_client_mng_remove_batch(GList *list)
{
	JpfRtspClient *cli;

	while (list)
	{
		cli = list->data;
		list = g_list_delete_link(list, list);
		nmp_rtsp_client_to_kill(cli);
		nmp_rtsp_client_unref(cli);
	}
}


static __inline__ void
__nmp_rtsp_client_mng_remove_death(JpfClientMng *client_mng)
{
	GList *temp = NULL;
	GList *list;

	while (client_mng->clients)
	{
		list = g_list_find_custom(client_mng->clients, NULL, nmp_rtsp_client_timeout);
		if (!list)
			break;

		client_mng->clients = g_list_remove_link(client_mng->clients, list);
		temp =  g_list_concat(temp, list);	/* isolate to temp list */
	}

	if (!temp)
		return;

	g_mutex_unlock(client_mng->lock);	/* reduce locks dependence */
	nmp_rtsp_client_mng_remove_batch(temp);	
	g_mutex_lock(client_mng->lock);
}


static __inline__ void
nmp_rtsp_client_mng_remove_death(JpfClientMng *client_mng)
{
	g_mutex_lock(client_mng->lock);
	__nmp_rtsp_client_mng_remove_death(client_mng);
	g_mutex_unlock(client_mng->lock);	
}


static GEvent *
nmp_rtsp_client_mng_create_listen_ev(JpfClientMng *client_mng)
{
	GEvent *e_listen;
	gint ret, tag_onoff, sockfd = -1;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

#ifdef USE_SOLINGER
  	struct linger linger;
#endif

	g_return_val_if_fail(client_mng != NULL, NULL);

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	ret = getaddrinfo(client_mng->address, client_mng->service,
		&hints, &result);
	if (ret != 0)
    	goto close_error;

	for (rp = result; rp; rp = rp->ai_next)
	{
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd < 0)
			continue;

		tag_onoff = 1;
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
			(void *)&tag_onoff, sizeof(tag_onoff));

		if (!bind(sockfd, rp->ai_addr, rp->ai_addrlen))
			break;

		close(sockfd);
		sockfd = -1;
	}

	freeaddrinfo(result);

	if (sockfd == -1)
		goto close_error;

#ifdef USE_SOLINGER
	linger.l_onoff = 1;
	linger.l_linger = 5;
	if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (void *)&linger, 
		sizeof (linger)) < 0)
    goto close_error;
#endif

	set_fd_flags(sockfd, O_NONBLOCK);

	if (listen(sockfd, MAX_LISTEN_BACKLOG) < 0)
		goto close_error;

	e_listen = g_event_new(sizeof(GEvent), sockfd, EV_READ);

	return e_listen;

close_error:
	if (sockfd >= 0)
		close (sockfd);
    return NULL;
}


static gboolean
nmp_rtsp_client_mng_io_func(GEvent *channel, gint revents,
	void *user_data)
{
	JpfClientMng *client_mng = (JpfClientMng*)user_data;
	JpfRtspClient *client;
	gint err;

	if (revents & EV_READ)
	{
		for (;;)
		{
			client = nmp_rtsp_client_new();
			if (G_UNLIKELY(!client))
			{
				nmp_error(
					"Create client object failed, OOM?"
				);
				FATAL_ERROR_EXIT;
	
				return FALSE;
			}
	
			client->client_mng = client_mng;
			if (G_UNLIKELY(!nmp_rtsp_client_accept(client, channel, &err)))
			{
				if (err == -EAGAIN)
				{
					nmp_rtsp_client_unref(client);
					break;
				}

				nmp_error(
					"Listen fd accept failed, err:'%d'.", err
				);
				FATAL_ERROR_EXIT;

				return FALSE;
			}
	
			nmp_rtsp_client_mng_add_client(client_mng, client);
			nmp_rtsp_client_attach(client, NULL);
			nmp_rtsp_client_unref(client);
		}

		return TRUE;
	}

	return TRUE;
}


static void
nmp_rtsp_client_listen_watch_fin(GEvent *ev)
{
	JpfClientMng *cli_mng;

	cli_mng = (JpfClientMng*)g_event_u(ev);
	nmp_rtsp_client_mng_unref(cli_mng);
}


static GEvent *
nmp_rtsp_client_mng_create_watch(JpfClientMng *client_mng)
{
	GEvent *e_listen;
	g_assert(client_mng != NULL);

	e_listen = nmp_rtsp_client_mng_create_listen_ev(client_mng);
	if (e_listen == NULL)
		goto no_channel;

	nmp_rtsp_client_mng_ref(client_mng);

	g_event_set_callback(e_listen, nmp_rtsp_client_mng_io_func,
		client_mng, nmp_rtsp_client_listen_watch_fin);

	return e_listen;

no_channel:
	nmp_warning("Failed to create client listen IO channel!");
	return NULL;
}


guint
nmp_rtsp_client_mng_attach(JpfClientMng *client_mng)
{
	GEvent *e_listen;
	g_assert(client_mng != NULL);

	e_listen = nmp_rtsp_client_mng_create_watch(client_mng);
	if (G_UNLIKELY(!e_listen))
		return 0;

	g_scheduler_add(e_listen, 1);
	g_event_unref(e_listen);

	return 1;
}

//:~ End
