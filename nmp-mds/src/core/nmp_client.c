#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "nmp_client.h"
#include "nmp_debug.h"
#include "nmp_errno.h"
#include "nmp_version.h"
#include "nmp_utils.h"

#define MAX_MSG_BACKLOG				3000

extern GstRtspWatchFuncs nmp_rtsp_client_watch_funcs;
static gint total_client_objects = 0;
static gint total_session_objects = 0;

static __inline__ void
nmp_rtsp_session_generate_id(JpfRtspSession *s)
{
	static gint static_sid = 0;
	gint sid, random;
	guint reentrant;

	sid = g_atomic_int_exchange_and_add(&static_sid, 1);
	random = rand_r(&reentrant);

	snprintf(
		&s->sid[0],
		CLIENT_SESSION_ID_LEN,
		SESSION_PREFIX"-%04d-%08d",
		((random & 0xffff) ^ (random >> 16)) % 10000,
		sid & 0x7fffffff
	);
	return;
}


JpfRtspSession *
nmp_rtsp_session_new( void )
{
	JpfRtspSession *s;

	s = g_new0(JpfRtspSession, 1);
	s->timeout = DEFAULT_SESSSION_TIMEOUT;
	nmp_rtsp_session_generate_id(s);
	g_atomic_int_add(&total_session_objects, 1);

	nmp_debug(
		"Create new session '%p', total %d.",
		s,
		total_session_objects
	);

	return s;
}


__export gint
nmp_rtsp_session_attach_media(JpfRtspSession *s, 
	JpfRtspMedia *media, JpfSinkerWatch *w)
{
	gint rc;
	g_assert(s && media && w);

	rc = nmp_rtsp_media_add_sinker(media, w);
	if (!rc)
	{
		s->media = media;
		s->sinker = w;
		nmp_rtsp_media_ref(media);
		nmp_media_sinker_ref(w);
	}

	return rc;
}


static __inline__ void
nmp_rtsp_session_detach_media(JpfRtspSession *s)
{
	nmp_debug(
		"Free session '%p', 'media %p', 'sinker %p', %d left.",
		s, 
		s->media, 
		s->sinker, 
		total_session_objects
	);

	if (s->media)
	{
		nmp_rtsp_media_del_sinker(s->media, s->sinker);
		nmp_media_sinker_unref(s->sinker);
		nmp_rtsp_media_unref(s->media);
	}
}


void
nmp_rtsp_session_release(JpfRtspSession *s)
{
	g_assert(s != NULL);

	g_atomic_int_add(&total_session_objects, -1);
	nmp_rtsp_session_detach_media(s);
	g_free(s);
}


JpfRtspClient *
nmp_rtsp_client_new( void )
{
	JpfRtspClient *client;
	
	client = g_new0(JpfRtspClient, 1);
	client->ref_count = 1;
	client->state = JPF_RTSP_STAT_NORMAL;
	client->lock = g_mutex_new();
	client->ttl = client->ttd = DEFAULT_SESSSION_TIMEOUT * 3;
	g_atomic_int_add(&total_client_objects, 1);

	return client;
}


static void
nmp_rtsp_client_session_list_free(gpointer data,
	gpointer user_data)
{
	JpfRtspSession *s;
	g_assert(data != NULL);

	s = (JpfRtspSession*)data;
	nmp_rtsp_session_release(s);
}


static __inline__ void
nmp_rtsp_client_sessions_free(GList *list)
{
	g_list_foreach(
		list, 
		nmp_rtsp_client_session_list_free,
		NULL
	);

	g_list_free(list);
}


static __inline__ void
nmp_rtsp_client_free(JpfRtspClient *client)
{
	g_assert(client != NULL);

	g_atomic_int_add(&total_client_objects, -1);

	nmp_print(
		"Free client '%p' '%s:%d', total %d left.",
		client,
		client->client_ip ?: "--",
		client->port,
		total_client_objects
	);

	g_mutex_free(client->lock);

	if (client->client_ip)
		g_free(client->client_ip);

	BUG_ON(client->sessions);
	BUG_ON(client->watch);

	g_free(client);
}


JpfRtspClient *
nmp_rtsp_client_ref(JpfRtspClient *client)
{
	g_assert(client != NULL && 
		g_atomic_int_get(&client->ref_count) > 0);

	g_atomic_int_inc(&client->ref_count);
	return client;
}


void
nmp_rtsp_client_unref(JpfRtspClient *client)
{
	g_assert(client != NULL && 
		g_atomic_int_get(&client->ref_count) > 0);

	if (g_atomic_int_dec_and_test(&client->ref_count))
	{
		nmp_rtsp_client_free(client);
	}
}


static void
__nmp_rtsp_client_set_state(JpfRtspClient *client, 
	JpfClientState state)
{
	GList *list;
	GEvent *watch;

	if (client->state == state)
		return;

	if (state == JPF_RTSP_STAT_NORMAL)
		return;

	if (state == JPF_RTSP_STAT_KILLED)
	{
		client->state = JPF_RTSP_STAT_KILLED;

		list = client->sessions;
		client->sessions = NULL;

		watch = (GEvent*)client->watch;
		client->watch = NULL;

		g_mutex_unlock(client->lock);

		if (watch)
		{
			/*
			 * 可能导致finalize()调用，要严防client在finalize()中被unref()销毁
			 */

			g_event_remove(watch);
			g_event_unref(watch);
		}

		if (list)
		{
			nmp_rtsp_client_sessions_free(list);
		}

		g_mutex_lock(client->lock);

		return;
	}

	BUG();
}


static void
nmp_rtsp_client_set_state(JpfRtspClient *client, JpfClientState state)
{
	g_assert(client != NULL);

	g_mutex_lock(client->lock);
	__nmp_rtsp_client_set_state(client, state);
	g_mutex_unlock(client->lock);
}

                                              
static __inline__ gboolean
nmp_rtsp_client_killed(JpfRtspClient *client)
{
	return client->state == JPF_RTSP_STAT_KILLED;
}


void
nmp_rtsp_client_set_illegal(JpfRtspClient *client)
{
	nmp_rtsp_client_set_state(client, JPF_RTSP_STAT_KILLED);
}


static void
nmp_rtsp_client_watch_finalize(JpfRtspClient *client)
{
	g_assert(client != NULL);

	nmp_debug(
		"Client '%p' rtsp connection watch finalized.", client
	);
	nmp_rtsp_client_unref(client);
}


gint
nmp_rtsp_client_timeout(gconstpointer c, gconstpointer  u)
{
	JpfRtspClient *client;

	client = (JpfRtspClient*)c;

	if (--client->ttd < 0)
		return 0;

	return 1;
}


void
nmp_rtsp_client_alive(JpfRtspClient *client)
{
	client->ttd = client->ttl;
}


gboolean
nmp_rtsp_client_accept(JpfRtspClient *client, GEvent *channel,
	gint *errp)
{
  	gint sock, new_sock;
	struct sockaddr_in s;
	socklen_t len;

	sock = g_event_fd(channel);

	bzero(&s, sizeof(s));
	len = sizeof(s);

	new_sock = accept(sock, (struct sockaddr*)&s, &len);
	if (new_sock < 0)
	{
		*errp = -errno;
		return FALSE;
	}

	client->client_ip = g_strdup(inet_ntoa(s.sin_addr));		/* Fix me */
	client->port = ntohs(s.sin_port);

	nmp_print(
		"Accept client '%p' IP '%s:%d'.",
		 client,
		 client->client_ip,
		 client->port
	);

	set_fd_flags(new_sock, O_NONBLOCK);

	client->watch = gst_rtsp_watch_new_2(
		new_sock,
		&nmp_rtsp_client_watch_funcs,
		nmp_rtsp_client_ref(client),
		(GstRtspWatchFin)nmp_rtsp_client_watch_finalize
	);

	return TRUE;
}


void
nmp_rtsp_client_attach(JpfRtspClient *client, void *context)
{
	G_ASSERT(client != NULL);

	gst_rtsp_watch_attach_2(client->watch, context);
}


static GstRTSPResult
nmp_rtsp_client_watch_send_message(JpfRtspClient *client,
	GstRTSPMessage *message)
{
	GstRTSPResult ret = GST_RTSP_ERROR;
	g_assert(client != NULL && message != NULL);

	g_mutex_lock(client->lock);
	if (client->watch)
		ret = gst_rtsp_watch_send_message_2(client->watch, message);
	g_mutex_unlock(client->lock);

	return ret;
}


void
nmp_rtsp_client_send_response(JpfRtspClient *client, JpfRtspSession *session,
	GstRTSPMessage *response)
{
	gchar *str;

	gst_rtsp_message_add_header(response, GST_RTSP_HDR_SERVER, 
		JPF_MEDIA_SERVER_LOGO);
	gst_rtsp_message_remove_header(response, GST_RTSP_HDR_SESSION, -1);

	if (session)
	{
		if (session->timeout != 60)
		{
			str = g_strdup_printf(
				"%s; timeout=%d", 
				session->sid,
				session->timeout
			);
		}
		else
			str = g_strdup(session->sid);

		gst_rtsp_message_take_header(response, GST_RTSP_HDR_SESSION, str);
	}

	nmp_rtsp_client_watch_send_message(client, response);
	gst_rtsp_message_unset(response);
}



static __inline__ void
__nmp_rtsp_client_add_session(JpfRtspClient *client, 
	JpfRtspSession *session)
{
	client->sessions = g_list_prepend(client->sessions, session);
}


static __inline__ gint
nmp_rtsp_client_add_session(JpfRtspClient *client, 
	JpfRtspSession *session)
{
	gint rc = -E_CKILLED;
	g_assert(client != NULL && session != NULL);

	g_mutex_lock(client->lock);
	if (!nmp_rtsp_client_killed(client))
	{
		rc = 0;
		__nmp_rtsp_client_add_session(client, session);
	}
	g_mutex_unlock(client->lock);

	nmp_debug(
		"Add session '%p' to client '%p'%s",
		session,
		client,
		rc ? " failed." : "."
	);

	return rc;
}


static gint
nmp_rtsp_client_session_id_equ(gconstpointer data, 
	gconstpointer user_data)
{
	JpfRtspSession *s;
	g_assert(data != NULL && user_data != NULL);

	s = (JpfRtspSession*)data;
	
	if (!strcmp(s->sid, (gchar*)user_data))
		return 0;

	return 1;
}


JpfRtspSession *
__nmp_rtsp_client_remove_session(JpfRtspClient *client, 
	gchar *session_id)
{
	JpfRtspSession *session;
	GList *list;

	list = g_list_find_custom(
		client->sessions,
		session_id,
		nmp_rtsp_client_session_id_equ
	);

	if (list)
	{
		session = (JpfRtspSession*)list->data;
		client->sessions = g_list_delete_link(
			client->sessions, list);
		return session;
	}

	return NULL;
}



static __inline__ JpfRtspSession *
nmp_rtsp_client_remove_session(JpfRtspClient *client, 
	gchar *session_id)
{
	JpfRtspSession *s = NULL;

	g_mutex_lock(client->lock);
	if (!nmp_rtsp_client_killed(client))
	{
		s = __nmp_rtsp_client_remove_session(
			client, session_id
		);
	}
	g_mutex_unlock(client->lock);

	return s;
}


gint
nmp_rtsp_client_delete_session(JpfRtspClient *client, 
	gchar *session_id)
{
	JpfRtspSession *s;
	g_assert(client != NULL && session_id != NULL);

	s = nmp_rtsp_client_remove_session(client, session_id);
	if (s)
	{
		nmp_rtsp_session_release(s);
		return 0;
	}

	return -1;//-E_NOENT;
}


static __inline__ JpfRtspSession *
__nmp_rtsp_client_find_session(JpfRtspClient *client, 
	gchar *session_id)
{
	JpfRtspSession *session;
	GList *list;

	list = g_list_find_custom(
		client->sessions,
		session_id,
		nmp_rtsp_client_session_id_equ
	);

	if (list)
	{
		session = (JpfRtspSession*)list->data;
		return session;
	}

	return NULL;	
}



JpfRtspSession *
nmp_rtsp_client_find_session(JpfRtspClient *client, 
	gchar *session_id)
{
	JpfRtspSession *s = NULL;
	g_assert(client != NULL);

	if (!session_id )
		return NULL;

	g_mutex_lock(client->lock);
	if (!nmp_rtsp_client_killed(client))
	{
		s = __nmp_rtsp_client_find_session(
			client, session_id
		);
	}
	g_mutex_unlock(client->lock);

	return s;
}


gint
nmp_rtsp_client_play_failed(JpfRtspClient *client, 
	gchar *session_id)
{
	JpfRtspSession *s;
	gint ret = -E_CKILLED;
	g_assert(client != NULL && session_id != NULL);

	g_mutex_lock(client->lock);
	if (!nmp_rtsp_client_killed(client))
	{
		ret = -E_NOSESSION;
		s = __nmp_rtsp_client_find_session(client, session_id);
		if (s)
		{
			ret = 0;
		}
	}
	g_mutex_unlock(client->lock);

	return ret;
}


gint
nmp_rtsp_client_play_session(JpfRtspClient *client, 
	gchar *session_id)
{
	JpfRtspSession *s;
	gint ret = -E_CKILLED;
	g_assert(client != NULL && session_id != NULL);

	g_mutex_lock(client->lock);
	if (!nmp_rtsp_client_killed(client))
	{
		ret = -E_NOSESSION;
		s = __nmp_rtsp_client_find_session(client, session_id);
		if (s)
		{
			ret = 0;
			nmp_rtsp_media_sinker_play(s->media, s->sinker);
		}
	}
	g_mutex_unlock(client->lock);

	return ret;
}


__export JpfRtspSession *
nmp_rtsp_create_session(JpfRtspClient *client, JpfRtspMedia *media, 
	GstRTSPTransport *ct)
{
	JpfRtspSession *s;
	JpfSinkerWatch *w;
	gint rc;

	w = nmp_media_create_sinker(media, ct->lower_transport, NULL);
	if (G_UNLIKELY(!w))
	{
		nmp_warning(
			"Create sinker failed, client '%p' '%s' for media '%p'.", 
			client, client->client_ip, media
		);
		return NULL;
	}

	s = nmp_rtsp_session_new();
	rc = nmp_rtsp_session_attach_media(s, media, w);
	if (rc)
	{
		nmp_print(
			"Attach session '%p' to media '%p' failed.",
			s, media
		);

		nmp_rtsp_session_release(s);
		s = NULL;
	}
	else
	{
		nmp_print(
			"Attach session '%p' to media '%p'.",
			s, media
		);

		if (nmp_rtsp_client_add_session(client, s))
		{
			nmp_rtsp_session_release(s);
			s = NULL;
		}
	}

	nmp_media_sinker_unref(w);
	return s;
}


gint
nmp_rtsp_client_send_message(void *cli, GstRTSPMessage *msg)
{
	JpfRtspClient *client = (JpfRtspClient*)cli;
	GstRTSPResult res;

	if (client->watch)
	{
		res = gst_rtsp_watch_send_message_2(client->watch, msg);
		if (res == GST_RTSP_OK)
			return 0;
	}

	return -1;
}

gboolean
nmp_rtsp_client_would_block(void *cli)
{
	gboolean full = TRUE;	
	JpfRtspClient *client = (JpfRtspClient*)cli;

	if (client->watch)
	{
		full = gst_rtsp_watch_is_full(client->watch);
	}

	return full;
}

//:~ End
