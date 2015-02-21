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

#include <math.h>
#include <string.h>
#include "rc_log.h"
#include "rtsp-server.h"
#include "media.h"
#include "rtp.h"
#include "mediautils.h"
#include "rtsp_methods.h"

#define DEFAULT_RTSP_TIMEOUT	30
#define RTSP_BUFFER_SIZE		4096
#define MAX_DATA_PENDING		(4096*32)
#define SETUP_TIMEOUT           8

typedef struct _RTSP_rec RTSP_rec;
struct _RTSP_rec
{
	guint8	*data;
	guint	size;
	guint	id;
};

typedef struct _RTSP_parm RTSP_parm;
struct _RTSP_parm
{
	RTSP_Client *client;
	GstRTSPMessage *message;
};


static gboolean
rtsp_client_rw_cb(GEvent *ev, int revents, void *user_data)
{
	RTSP_Client *client = (RTSP_Client*)ev;

	if (client->rw_func)
	{
		return (*client->rw_func)(ev, revents, user_data);
	}
	return FALSE;
}


static void
rtsp_client_finalize(GEvent *ev)
{
	RTSP_Client *client = (RTSP_Client*)ev;

	if (client->fin_func)
	{
		(*client->fin_func)(ev);
	}
}


RTSP_Client *
rtsp_client_new_base(gsize size, Sock *sock, gint events, gint timeout)
{
	RTSP_Client *client;

	client = (RTSP_Client*)g_event_new(size,
		Sock_fd(sock), events);

	client->sock = sock;
	client->mutex = g_mutex_new();
	client->killed = FALSE;
	client->rtsp_parser = gst_rtsp_parser_new(RTSP_BUFFER_SIZE, NULL);
	client->output = g_queue_new();
	client->write_buffer = NULL;
	client->backlog = 0;
	client->rec_id = 0;
	client->session_list = NULL;

	g_event_set_callback((GEvent*)client, rtsp_client_rw_cb,
		client, rtsp_client_finalize);
	g_event_set_timeout((GEvent*)client, timeout*1000);

	return client;
}


RTSP_Client *
rtsp_client_new(Sock *sock)
{
	RTSP_Client *client;

	client = rtsp_client_new_base(sizeof(RTSP_Client), sock, EV_READ, 0);
	client->rw_func = rtsp_client_default_rw_cb;
	client->fin_func = rtsp_client_default_finalize;

	return client;
}


RTSP_Client *
rtsp_client_ref(RTSP_Client *client)
{
	g_assert(client);
	return (RTSP_Client*)g_event_ref((GEvent*)client);
}


void
rtsp_client_unref(RTSP_Client *client)
{
	g_assert(client);
	g_event_unref((GEvent*)client);
}


static void
rtsp_client_free_rec(gpointer data, gpointer user_data)
{
	RTSP_rec *rec = (RTSP_rec*)data;

	g_free(rec->data);
	g_slice_free(RTSP_rec, rec);
}


static __inline__ void
rtsp_client_destroy_output_queue(GQueue *output)
{
	if (!output)
		return;

	g_queue_foreach(output, rtsp_client_free_rec, NULL);
	g_queue_free(output);
}


void
rtsp_client_default_finalize(GEvent *ev)
{
	RTSP_Client *client = (RTSP_Client*)ev;
	Listen_watch *ower = GET_OWNER(client, Listen_watch);

	g_event_unref((GEvent*)ower);
	g_assert(!client->session_list);
	g_free(client->write_buffer);
	rtsp_client_destroy_output_queue(client->output);
	gst_rtsp_parser_free(client->rtsp_parser);
	g_mutex_free(client->mutex);
	Sock_close(client->sock);
}


RTSP_session *
__rtsp_client_session_new(RTSP_Client *client)
{
	static guint g_sid = 0;

	rtsp_client_remove_dead_session(client);
	g_mutex_lock(client->mutex);
	if (G_UNLIKELY(!client->mutex))
	{
		g_mutex_unlock(client->mutex);
		return NULL;
	}

    RTSP_session *rtsp_s = g_new0(RTSP_session, 1);
    rtsp_s->ref_count = 1;
    rtsp_s->session_id = g_strdup_printf("FOY-%08x-%08x",
                                      ++g_sid,
                                      g_random_int());

	rtsp_s->timeout = DEFAULT_RTSP_TIMEOUT;
	rtsp_s->mutex = g_mutex_new();
	rtsp_s->killed = FALSE;
	rtsp_s->session_timer = time(NULL);

    rtsp_s->play_requests = g_queue_new();

	SET_OWNER(rtsp_s, client);

	client->session_list = g_list_prepend(
		client->session_list, rtsp_s);

	rtsp_session_ref(rtsp_s);
	rtsp_client_ref(client);

	g_mutex_unlock(client->mutex);

	return rtsp_s;
}


static void
rtsp_session_destroy_one_pr(gpointer data, gpointer user_data)
{
	RTSP_Range *range = (RTSP_Range*)data;

	g_free(range);
}


static __inline__ void
rtsp_session_destroy_play_requests(RTSP_session *rtsp_s)
{
	extern void rtsp_session_ranges_free(RTSP_session *session);
	rtsp_session_ranges_free(rtsp_s);
	g_queue_free(rtsp_s->play_requests);
}


static __inline__ void
rtsp_session_finalize(RTSP_session *rtsp_s)
{
	RTSP_Client *client = GET_OWNER(rtsp_s, RTSP_Client);

	rtsp_client_unref(client);
	rtsp_session_destroy_play_requests(rtsp_s);
	g_free(rtsp_s->resource_uri);
	r_close(rtsp_s->resource);
	g_assert(!rtsp_s->rtp_sessions);
	g_mutex_free(rtsp_s->mutex);
	g_free(rtsp_s->session_id);
	g_free(rtsp_s);
}


static __inline__ void
rtsp_session_kill_rtps(RTSP_session *rtsp_s)
{
	GSList *list;
	RTP_session *rtp_s;

	while (rtsp_s->rtp_sessions)
	{
		list = rtsp_s->rtp_sessions;
		rtp_s = list->data;
		rtsp_s->rtp_sessions = g_slist_delete_link(rtsp_s->rtp_sessions, list);
		g_mutex_unlock(rtsp_s->mutex);

		rtp_session_kill(rtp_s);
		rtp_session_unref(rtp_s);

		g_mutex_lock(rtsp_s->mutex);
	}
}


void
rtsp_session_kill(RTSP_session *rtsp_s)
{
	if (rtsp_s->killed)
		return;

	g_mutex_lock(rtsp_s->mutex);
	if (rtsp_s->killed)
	{
		g_mutex_unlock(rtsp_s->mutex);
		return;
	}

	rtsp_s->killed = TRUE;
	rtsp_session_kill_rtps(rtsp_s);

	g_mutex_unlock(rtsp_s->mutex);
}


RTSP_session *
rtsp_session_ref(RTSP_session *rtsp_s)
{
	__OBJECT_REF(rtsp_s);
}


void
rtsp_session_unref(RTSP_session *rtsp_s)
{
	__OBJECT_UNREF(rtsp_s, rtsp_session);
}


static gint
is_session_id(gconstpointer a, gconstpointer b)
{
	RTSP_session *rtsp_s = (RTSP_session*)a;
	gchar *sid = (gchar*)b;

	return strcmp(rtsp_s->session_id, sid);
}


static gint
is_dead_session(gconstpointer a, gconstpointer b)
{
	RTSP_session *rtsp_s = (RTSP_session*)a;

	if (!rtsp_s->session_timer)
		return 1;
	if (rtsp_s->session_timer + SETUP_TIMEOUT <= time(NULL))
		return 0;
	return 1;
}


RTSP_session *
rtsp_client_session_find(RTSP_Client *client, gchar *sid)
{
	RTSP_session *rtsp_s = NULL;
	GList *list;
	g_assert(client != NULL);

	if (G_UNLIKELY(!sid))
		return NULL;

	g_mutex_lock(client->mutex);
	if (G_UNLIKELY(client->killed))
	{
		g_mutex_unlock(client->mutex);
		return NULL;
	}

	list = g_list_find_custom(client->session_list,
		sid, is_session_id);
	if (list)
	{
		rtsp_s = list->data;
		rtsp_session_ref(rtsp_s);
	}

	g_mutex_unlock(client->mutex);

	return rtsp_s;
}


RTSP_session *
rtsp_client_find_dead_session(RTSP_Client *client)
{
	RTSP_session *rtsp_s = NULL;
	GList *list;

	g_mutex_lock(client->mutex);
	if (G_UNLIKELY(client->killed))
	{
		g_mutex_unlock(client->mutex);
		return NULL;
	}

	list = g_list_find_custom(client->session_list,
		NULL, is_dead_session);
	if (list)
	{
		rtsp_s = list->data;
		rtsp_session_ref(rtsp_s);
	}
	g_mutex_unlock(client->mutex);

	return rtsp_s;
}


void
rtsp_client_session_remove(RTSP_Client *client, RTSP_session *rtsp_s)
{
	GList *list;
	g_assert(client != NULL && rtsp_s != NULL);

	g_mutex_lock(client->mutex);
	if (client->killed)
	{
		g_mutex_unlock(client->mutex);
		return;
	}

	list = g_list_find(client->session_list, rtsp_s);
	if (list)
	{
		client->session_list = g_list_delete_link(
			client->session_list, list);
	}

	g_mutex_unlock(client->mutex);

	if (list)
	{
		rtsp_session_unref(rtsp_s);
		//@{unref client in rtsp_session_finalize()}
	}
}


void
rtsp_client_remove_dead_session(RTSP_Client *client)
{
	RTSP_session *rtsp_s;

	for (;;)
	{
		rtsp_s = rtsp_client_find_dead_session(client);
		if (!rtsp_s)
			break;
		rtsp_client_session_remove(client, rtsp_s);
		rtsp_session_kill(rtsp_s);
		rtsp_session_unref(rtsp_s);
	}
}


gint
rtsp_client_would_block(RTSP_Client *client, gsize size)
{
	gint block = 0;

	g_mutex_lock(client->mutex);
	if (client->backlog + size > MAX_DATA_PENDING)
	{
		block = 1;
	}
	g_mutex_unlock(client->mutex);

	return block;
}


static __inline__ GstRTSPResult
rtsp_client_write_bytes(gint fd, const guint8 *buffer, guint *idx,
	guint size, gint *err)
{
	guint left;

	if (G_UNLIKELY (*idx > size))
	{
		*err = EINVAL;
		return GST_RTSP_ERROR;
	}

	left = size - *idx;

	while (left)
	{
		gint r;

		r = send(fd, &buffer[*idx], left, MSG_DONTWAIT);
		if (G_UNLIKELY(r == 0))
			return GST_RTSP_EINTR;
		else if (G_UNLIKELY(r < 0))
		{
			*err = errno;
			if (*err == EAGAIN)
				return GST_RTSP_EINTR;

			if (*err != EINTR)
				return GST_RTSP_ESYS;

			*err = 0;
		}
		else
		{
			left -= r;
			 *idx += r;
		}
  	}

	return GST_RTSP_OK;
}


static __inline__ GstRTSPResult
rtsp_client_write_data(RTSP_Client *client, const guint8 *data,
    guint size, guint  *id)
{
	GstRTSPResult res;
	RTSP_rec *rec;
	guint off = 0;
	gboolean pending = FALSE;

	g_mutex_lock(client->mutex);

  	if (!client->write_buffer && !client->output->length)
	{
		res = rtsp_client_write_bytes(Sock_fd(client->sock), data, &off, size,
			&client->err_code);
		if (res != GST_RTSP_EINTR)
		{
			if (id != NULL)
				*id = 0;
			g_free((gpointer)data);
			goto done;
    	}
  	}

	if (client->backlog >=  MAX_DATA_PENDING)
	{
		res = GST_RTSP_EINTR;
		goto done;
	}

	rec = g_slice_new(RTSP_rec);
	if (off == 0)
	{
		rec->data = (guint8*)data;
		rec->size = size;
	}
	else
	{
		rec->data = g_memdup(data + off, size - off);
		rec->size = size - off;
		g_free((gpointer)data);
	}

	do {
		/* make sure rec->id is never 0 */
		rec->id = ++client->rec_id;
	} while (G_UNLIKELY (rec->id == 0));

	g_queue_push_head(client->output, rec);
	client->backlog += rec->size;

	pending = TRUE;

	if (id != NULL)
		*id = rec->id;

	res = GST_RTSP_OK;

done:
	g_mutex_unlock(client->mutex);

	if (pending)
		g_event_add_events((GEvent*)client, EV_WRITE);

	return res;
}


static __inline__ GstRTSPResult
__rtsp_client_send_message(RTSP_Client *client, GstRTSPMessage *message, guint *id)
{
	GString *str;
	guint size;

	g_return_val_if_fail (client != NULL, GST_RTSP_EINVAL);
	g_return_val_if_fail (message != NULL, GST_RTSP_EINVAL);

	str = gst_rtsp_message_to_string(message);
	size = str->len;

	return rtsp_client_write_data(client, (guint8 *)g_string_free(str, FALSE), size, id);
}


void
rtsp_client_send_request(RTSP_Client *client, GstRTSPMessage *request)
{
	g_assert(client != NULL && request != NULL);
	__rtsp_client_send_message(client, request, NULL);
}


void
rtsp_client_send_response(RTSP_Client *client, RTSP_session *session,
	GstRTSPMessage *response)
{
	gchar *str;

	gst_rtsp_message_add_header(response, GST_RTSP_HDR_SERVER, "LibFoy");
	gst_rtsp_message_remove_header(response, GST_RTSP_HDR_SESSION, -1);

	if (session)
	{
		if (session->timeout != 60)
		{
			str = g_strdup_printf(
				"%s; timeout=%d",
				session->session_id,
				session->timeout
			);
		}
		else
		{
			str = g_strdup(session->session_id);
		}

		if (str)
		{
			gst_rtsp_message_take_header(response, GST_RTSP_HDR_SESSION, str);
		}
	}

	__rtsp_client_send_message(client, response, NULL);
	gst_rtsp_message_unset(response);
}


void
rtsp_client_send_generic_response(RTSP_Client *client, GstRTSPStatusCode code,
	RTSP_Ps *state)
{
	gst_rtsp_message_init_response(state->response, code,
		gst_rtsp_status_as_text(code), state->request);

	rtsp_client_send_response(client, NULL, state->response);
}


gint
rtsp_client_send_message(RTSP_Client *client, GstRTSPMessage *msg)
{
	__rtsp_client_send_message(client, msg, NULL);
	return 0;
}


static __inline__ void
rtsp_request_handler(RTSP_Client *client, GstRTSPMessage *request)
{
	GstRTSPMethod method;
	GstRTSPVersion version;
	const gchar *url_str;
	GstRTSPUrl *url;
	RTSP_Ps state = { NULL };
	GstRTSPMessage response = { 0 };

	state.request = request;
	state.response = &response;

	gst_rtsp_message_parse_request(request, &method, &url_str, &version);
	if (version != GST_RTSP_VERSION_1_0)
	{
		rtsp_client_send_generic_response(
			client, 
			GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED,
			&state
		);
	    return;
	}

	state.method = method;

	if (gst_rtsp_url_parse(url_str, &url) != GST_RTSP_OK)
	{
    	rtsp_client_send_generic_response(
    		client,
    		GST_RTSP_STS_BAD_REQUEST,
    		&state
    	);
		return;
	}

	state.url = url;

	switch (method)
	{
	case GST_RTSP_OPTIONS:
		rtsp_handle_options_request(client, &state);
		break;

	case GST_RTSP_DESCRIBE:
		rtsp_handle_describe_request(client, &state);
		break;

	case GST_RTSP_SETUP:
		rtsp_handle_setup_request(client, &state);
		break;

	case GST_RTSP_PLAY:
		rtsp_handle_play_request(client, &state);
		break;

	case GST_RTSP_SET_PARAMETER:
		rtsp_handle_setp_request(client, &state);
		break;

	case GST_RTSP_GET_PARAMETER:
		rtsp_handle_getp_request(client, &state);
		break;

	case GST_RTSP_PAUSE:
		rtsp_handle_pause_request(client, &state);
		break;

	case GST_RTSP_TEARDOWN:
		rtsp_handle_teardown_request(client, &state);
		break;

	default:
		break;
	}

	gst_rtsp_url_free(url);
}


static void
rtsp_client_message_handle_task(gpointer data, gpointer user_data)
{
	RTSP_parm *parm = (RTSP_parm*)data;
	RTSP_Client *client = parm->client;
	GstRTSPMessage *message = parm->message;

	switch (gst_rtsp_message_get_type(message))
	{
	case GST_RTSP_MESSAGE_REQUEST:
		rtsp_request_handler(client, message);
		break;

	case GST_RTSP_MESSAGE_RESPONSE:
//		gst_rtsp_message_dump(message);
		break;

	default:
		break;
	}

	rtsp_client_unref(client);
	gst_rtsp_message_free(message);
	g_free(parm);
}


static __inline__ void
rtsp_client_handler_message(RTSP_Client *client, GstRTSPMessage *message)
{
	GstRTSPMessage *request;
	RTSP_parm *parm;
	static gsize init_handler_threads = FALSE;
	static GThreadPool *handler_threads_pool;

	if (g_once_init_enter(&init_handler_threads))
	{
		handler_threads_pool = g_thread_pool_new(rtsp_client_message_handle_task,
			NULL, -1, FALSE, NULL);
		g_once_init_leave(&init_handler_threads, TRUE);
	}

	if (gst_rtsp_message_get_type(message) != GST_RTSP_MESSAGE_REQUEST)
		return;

	request = gst_rtsp_message_dup_request(message);
	if (!request)
	{
		rc_log(RC_LOG_ERR, "[RC] rtsp-server dup client message failed.");
		return;
	}

	parm = g_new0(RTSP_parm, 1);
	parm->client = rtsp_client_ref(client);
	parm->message = request;

	g_thread_pool_push(handler_threads_pool, parm, NULL);
}


static __inline__ void
rtsp_client_kill_sessions(RTSP_Client *client)
{
	GList *list;
	RTSP_session *rtsp_s;

	while (client->session_list)
	{
		list = client->session_list;
		rtsp_s = list->data;
		client->session_list = g_list_delete_link(client->session_list, list);
		g_mutex_unlock(client->mutex);

		rtsp_session_kill(rtsp_s);
		rtsp_session_unref(rtsp_s);

		g_mutex_lock(client->mutex);
	}
}


void
rtsp_client_kill(RTSP_Client *client)
{
	g_assert(client != NULL);

	if (client->killed)	/* previous check, almost hit if so */
		return;

	g_mutex_lock(client->mutex);

	if (G_UNLIKELY(client->killed))
	{
		g_mutex_unlock(client->mutex);
		return;
	}

	client->killed = TRUE;	
	rtsp_client_kill_sessions(client);

	g_mutex_unlock(client->mutex);
}


void
rtsp_client_orphan(RTSP_Client *client)
{
	extern void rtsp_listener_del_conn(Listen_watch *listener,
		RTSP_Client *client);

	g_assert(client != NULL);
	Listen_watch *listener = GET_OWNER(client, Listen_watch);

	rtsp_listener_del_conn(listener, client);
}


static __inline__ gboolean
rtsp_client_read(GEvent *ev, int revents, void *user_data)
{
	RTSP_Client *client = (RTSP_Client*)user_data;
	GstRTSPResult res;
	GstRTSPMessage *message = NULL;
	
	while (TRUE)
	{
		res = gst_rtsp_parser_recv(client->rtsp_parser, g_event_fd(ev),
			&message, &client->err_code);
		if (res == GST_RTSP_OK)
		{
//			gst_rtsp_message_dump(message);
			rtsp_client_handler_message(client, message);
			continue;
		}

		else if (res == GST_RTSP_EINTR)
		{
			return TRUE;
		}

		else if (res == GST_RTSP_EEOF)
		{
			rc_log(
				RC_LOG_INFO,
				"[CLIENT] Client '%s:%d' reset connection.",
				get_remote_host(client->sock),
				get_remote_port(client->sock)
			);
			break;
		}

		else
		{
			rc_log(
				RC_LOG_INFO,
				"[CLIENT] Recv error, client '%s:%d', reset.",
				get_remote_host(client->sock),
				get_remote_port(client->sock)
			);
			break;
		}
	}

	rtsp_client_kill(client);
	rtsp_client_orphan(client);

    return FALSE;
}


static __inline__ gboolean
rtsp_client_write(GEvent *ev, void *user_data)
{
	GstRTSPResult res;
	RTSP_rec *rec;
	RTSP_Client *client = (RTSP_Client*)user_data;

	g_mutex_lock(client->mutex);

	do {
		if (!client->write_buffer)
		{
			rec = g_queue_pop_tail(client->output);
			if (rec == NULL)
			{
				g_mutex_unlock(client->mutex);
				g_event_remove_events_sync((GEvent*)client, EV_WRITE);
				return TRUE;
			}

			client->write_buffer = rec->data;
			client->write_off = 0;
			client->write_size = rec->size;

			if (client->backlog < rec->size)
				client->backlog = rec->size;

			client->backlog -= rec->size;
			g_slice_free (RTSP_rec, rec);
		}

		res = rtsp_client_write_bytes(g_event_fd(ev), client->write_buffer,
			&client->write_off, client->write_size, &client->err_code);

		g_mutex_unlock(client->mutex);

		if (res != GST_RTSP_OK)
		{
			if (res == GST_RTSP_EINTR)
			{
				return TRUE;
			}
			else
			{
				rtsp_client_kill(client);
				rtsp_client_orphan(client);
				return FALSE;
			}
		}

		g_mutex_lock(client->mutex);

		g_free(client->write_buffer);
		client->write_buffer = NULL;

	} while (TRUE);

	g_mutex_unlock(client->mutex);

	return TRUE;
}


static __inline__ gboolean
rtsp_client_timeout(GEvent *ev, int revents, void *user_data)
{//@{TODO}
	RTSP_Client *client = (RTSP_Client*)ev;

	client->err_code = ETIME;
	rtsp_client_kill(client);
	rtsp_client_orphan(client);
	return FALSE;
}


gboolean
rtsp_client_default_rw_cb(GEvent *ev, int revents, void *user_data)
{
	if (revents & EV_READ)
	{
		if (!rtsp_client_read(ev, revents, user_data))
			return FALSE;
	}

	if (revents & EV_WRITE)
	{
		if (!rtsp_client_write(ev, user_data))
			return FALSE;
	}

	if (revents & EV_TIMER)
	{
		if (!rtsp_client_timeout(ev, revents, user_data))
			return FALSE;
	}

	return TRUE;
}

//:~ End
